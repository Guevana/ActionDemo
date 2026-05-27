#include "AI/ADEnemyAIController.h"

#include "Character/Base/ADCharacterBase.h"
#include "Character/Enemy/ADEnemyCharacter.h"
#include "Character/Enemy/ADEnemyConfigData.h"
#include "Components/StateTreeComponent.h"
#include "Components/Target/ADTargetingComponent.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"

AADEnemyAIController::AADEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	CombatStateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("CombatStateTreeComponent"));

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 1800.0f;
	SightConfig->LoseSightRadius = 2200.0f;
	SightConfig->PeripheralVisionAngleDegrees = 75.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*AIPerceptionComponent);
}

void AADEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsControlledEnemyDead())
	{
		StopCombatLogicForDeath();
		return;
	}

	ValidateCombatTarget();
}

void AADEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	bCombatLogicStoppedForDeath = false;
	ConfigureFromEnemyConfig();

	if (AADEnemyCharacter* EnemyCharacter = GetControlledEnemy())
	{
		EnemyCharacter->OnCharacterDeath.AddUniqueDynamic(this, &AADEnemyAIController::HandleControlledEnemyDeath);
	}

	if (AIPerceptionComponent != nullptr)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AADEnemyAIController::HandleTargetPerceptionUpdated);
		AIPerceptionComponent->RequestStimuliListenerUpdate();
	}

	if (CombatStateTreeComponent != nullptr)
	{
		CombatStateTreeComponent->StartLogic();
	}
}

void AADEnemyAIController::OnUnPossess()
{
	ClearCombatTarget();

	if (CombatStateTreeComponent != nullptr)
	{
		CombatStateTreeComponent->StopLogic(TEXT("Enemy unpossessed"));
	}

	if (AIPerceptionComponent != nullptr)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(this, &AADEnemyAIController::HandleTargetPerceptionUpdated);
	}

	if (AADEnemyCharacter* EnemyCharacter = GetControlledEnemy())
	{
		EnemyCharacter->OnCharacterDeath.RemoveDynamic(this, &AADEnemyAIController::HandleControlledEnemyDeath);
	}

	Super::OnUnPossess();
}

UStateTreeComponent* AADEnemyAIController::GetCombatStateTreeComponent() const
{
	return CombatStateTreeComponent;
}

AActor* AADEnemyAIController::GetCombatTarget() const
{
	return CombatTarget;
}

bool AADEnemyAIController::HasCombatTarget() const
{
	return IsValid(CombatTarget);
}

float AADEnemyAIController::GetTargetDistance() const
{
	const APawn* ControlledPawn = GetPawn();
	if (ControlledPawn == nullptr || !IsValid(CombatTarget))
	{
		return TNumericLimits<float>::Max();
	}

	return FVector::Distance(ControlledPawn->GetActorLocation(), CombatTarget->GetActorLocation());
}

bool AADEnemyAIController::IsTargetInAttackRange() const
{
	const APawn* ControlledPawn = GetPawn();
	if (ControlledPawn == nullptr || !IsValid(CombatTarget))
	{
		return false;
	}

	return FVector::DistSquared(ControlledPawn->GetActorLocation(), CombatTarget->GetActorLocation()) <= FMath::Square(AttackRange);
}

bool AADEnemyAIController::IsControlledEnemyHitReacting() const
{
	const AADEnemyCharacter* EnemyCharacter = GetControlledEnemy();
	return EnemyCharacter != nullptr && EnemyCharacter->IsHitReacting();
}

bool AADEnemyAIController::IsControlledEnemyDead() const
{
	const AADEnemyCharacter* EnemyCharacter = GetControlledEnemy();
	return EnemyCharacter != nullptr && EnemyCharacter->IsDead();
}

float AADEnemyAIController::GetAttackRange() const
{
	return AttackRange;
}

float AADEnemyAIController::GetLoseTargetDistance() const
{
	return LoseTargetDistance;
}

void AADEnemyAIController::SetCombatTarget(AActor* NewTarget)
{
	if (NewTarget != nullptr && !IsValidCombatTarget(NewTarget))
	{
		NewTarget = nullptr;
	}

	if (CombatTarget == NewTarget)
	{
		return;
	}

	if (IsValid(CombatTarget))
	{
		CombatTarget->OnDestroyed.RemoveDynamic(this, &AADEnemyAIController::HandleCombatTargetDestroyed);
		if (AADCharacterBase* TargetCharacter = Cast<AADCharacterBase>(CombatTarget))
		{
			TargetCharacter->OnCharacterDeath.RemoveDynamic(this, &AADEnemyAIController::HandleCombatTargetDeath);
		}
	}

	CombatTarget = NewTarget;

	if (IsValid(CombatTarget))
	{
		CombatTarget->OnDestroyed.AddUniqueDynamic(this, &AADEnemyAIController::HandleCombatTargetDestroyed);
		if (AADCharacterBase* TargetCharacter = Cast<AADCharacterBase>(CombatTarget))
		{
			TargetCharacter->OnCharacterDeath.AddUniqueDynamic(this, &AADEnemyAIController::HandleCombatTargetDeath);
		}
		UE_LOG(LogTemp, Log, TEXT("[ActionDemo] Enemy AI target acquired: %s"), *CombatTarget->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[ActionDemo] Enemy AI target cleared."));
	}

	SyncCombatTargetToPawn();
}

void AADEnemyAIController::ClearCombatTarget()
{
	SetCombatTarget(nullptr);
}

void AADEnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed() && IsValidCombatTarget(Actor))
	{
		SetCombatTarget(Actor);
		return;
	}

	if (Actor == CombatTarget)
	{
		ClearCombatTarget();
	}
}

void AADEnemyAIController::HandleCombatTargetDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == CombatTarget)
	{
		ClearCombatTarget();
	}
}

void AADEnemyAIController::HandleControlledEnemyDeath(AADCharacterBase* DeadCharacter)
{
	if (DeadCharacter == GetPawn())
	{
		StopCombatLogicForDeath();
	}
}

void AADEnemyAIController::HandleCombatTargetDeath(AADCharacterBase* DeadCharacter)
{
	if (DeadCharacter == CombatTarget)
	{
		ClearCombatTarget();
	}
}

bool AADEnemyAIController::IsValidCombatTarget(const AActor* Actor) const
{
	const AADCharacterBase* TargetCharacter = Cast<AADCharacterBase>(Actor);
	if (!IsValid(Actor) || Actor == GetPawn() || TargetCharacter == nullptr || TargetCharacter->IsDead())
	{
		return false;
	}

	const APawn* TargetPawn = Cast<APawn>(Actor);
	return TargetPawn != nullptr && TargetPawn->IsPlayerControlled();
}

AADEnemyCharacter* AADEnemyAIController::GetControlledEnemy() const
{
	return Cast<AADEnemyCharacter>(GetPawn());
}

void AADEnemyAIController::ConfigureFromEnemyConfig()
{
	const AADEnemyCharacter* EnemyCharacter = GetControlledEnemy();
	const UADEnemyConfigData* EnemyConfig = EnemyCharacter != nullptr ? EnemyCharacter->GetEnemyConfig() : nullptr;
	if (EnemyConfig != nullptr)
	{
		AttackRange = EnemyConfig->AttackRange;
		LoseTargetDistance = EnemyConfig->LoseTargetDistance;

		if (SightConfig != nullptr)
		{
			SightConfig->SightRadius = EnemyConfig->SightRadius;
			SightConfig->LoseSightRadius = EnemyConfig->LoseSightRadius;
			SightConfig->PeripheralVisionAngleDegrees = EnemyConfig->PeripheralVisionAngleDegrees;
		}
	}

	if (AIPerceptionComponent != nullptr && SightConfig != nullptr)
	{
		AIPerceptionComponent->ConfigureSense(*SightConfig);
		AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}

	if (AADEnemyCharacter* MutableEnemy = GetControlledEnemy())
	{
		if (UADTargetingComponent* TargetingComponent = MutableEnemy->GetTargetingComponent())
		{
			const float TargetingRadius = FMath::Max(LoseTargetDistance, SightConfig != nullptr ? SightConfig->LoseSightRadius : LoseTargetDistance);
			TargetingComponent->SearchRadius = TargetingRadius;
			TargetingComponent->MaxViewAngleDegrees = 180.0f;
			TargetingComponent->bRequireLineOfSight = false;
		}
	}
}

void AADEnemyAIController::SyncCombatTargetToPawn() const
{
	const AADEnemyCharacter* EnemyCharacter = GetControlledEnemy();
	if (EnemyCharacter == nullptr || EnemyCharacter->GetTargetingComponent() == nullptr)
	{
		return;
	}

	EnemyCharacter->GetTargetingComponent()->SetCurrentTarget(CombatTarget);
}

void AADEnemyAIController::ValidateCombatTarget()
{
	if (!IsValid(CombatTarget))
	{
		if (CombatTarget != nullptr)
		{
			ClearCombatTarget();
		}
		return;
	}

	if (!IsValidCombatTarget(CombatTarget) || GetTargetDistance() > LoseTargetDistance)
	{
		ClearCombatTarget();
	}
}

void AADEnemyAIController::StopCombatLogicForDeath()
{
	if (bCombatLogicStoppedForDeath)
	{
		return;
	}

	bCombatLogicStoppedForDeath = true;
	StopMovement();
	ClearCombatTarget();

	if (CombatStateTreeComponent != nullptr)
	{
		CombatStateTreeComponent->StopLogic(TEXT("Controlled enemy died"));
	}

	if (AIPerceptionComponent != nullptr)
	{
		AIPerceptionComponent->ForgetAll();
	}
}

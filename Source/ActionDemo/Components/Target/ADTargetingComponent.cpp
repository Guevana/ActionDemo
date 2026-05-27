#include "Components/Target/ADTargetingComponent.h"

#include "Character/Base/ADCharacterBase.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

UADTargetingComponent::UADTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UADTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValidTargetCandidate(CurrentTarget))
	{
		CurrentTarget = nullptr;
	}
}

AActor* UADTargetingComponent::AcquireBestTarget()
{
	TArray<AActor*> Candidates;
	GatherTargetCandidates(Candidates);

	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Lowest();

	for (AActor* Candidate : Candidates)
	{
		const float CandidateScore = ScoreTargetCandidate(Candidate);
		if (CandidateScore > BestScore)
		{
			BestScore = CandidateScore;
			BestTarget = Candidate;
		}
	}

	SetCurrentTarget(BestTarget);
	return CurrentTarget;
}

AActor* UADTargetingComponent::ToggleLockTarget()
{
	if (IsValid(CurrentTarget))
	{
		ClearCurrentTarget();
		return nullptr;
	}

	return AcquireBestTarget();
}

AActor* UADTargetingComponent::SwitchTarget(int32 Direction)
{
	if (!IsValid(CurrentTarget))
	{
		return AcquireBestTarget();
	}

	FVector ViewOrigin;
	FVector ViewForward;
	FVector ViewRight;
	if (!GetTargetViewData(ViewOrigin, ViewForward, ViewRight))
	{
		return CurrentTarget;
	}

	TArray<AActor*> Candidates;
	GatherTargetCandidates(Candidates);

	AActor* BestTarget = nullptr;
	float BestSideScore = TNumericLimits<float>::Lowest();
	const float DesiredSign = Direction >= 0 ? 1.0f : -1.0f;

	for (AActor* Candidate : Candidates)
	{
		if (Candidate == CurrentTarget)
		{
			continue;
		}

		const FVector ToCandidate = (Candidate->GetActorLocation() - ViewOrigin).GetSafeNormal();
		const float Side = FVector::DotProduct(ToCandidate, ViewRight);
		if (FMath::Sign(Side) != FMath::Sign(DesiredSign))
		{
			continue;
		}

		const float ForwardScore = FMath::Max(0.0f, FVector::DotProduct(ToCandidate, ViewForward));
		const float SideScore = FMath::Abs(Side) + ForwardScore;
		if (SideScore > BestSideScore)
		{
			BestSideScore = SideScore;
			BestTarget = Candidate;
		}
	}

	if (BestTarget == nullptr)
	{
		BestTarget = AcquireBestTarget();
	}
	else
	{
		SetCurrentTarget(BestTarget);
	}

	return CurrentTarget;
}

void UADTargetingComponent::SetCurrentTarget(AActor* NewTarget)
{
	if (NewTarget != nullptr && !IsValidTargetCandidate(NewTarget))
	{
		NewTarget = nullptr;
	}

	if (CurrentTarget == NewTarget)
	{
		return;
	}

	AActor* OldTarget = CurrentTarget;
	if (IsValid(CurrentTarget))
	{
		CurrentTarget->OnDestroyed.RemoveDynamic(this, &UADTargetingComponent::HandleCurrentTargetDestroyed);
		if (AADCharacterBase* CurrentTargetCharacter = Cast<AADCharacterBase>(CurrentTarget))
		{
			CurrentTargetCharacter->OnCharacterDeath.RemoveDynamic(this, &UADTargetingComponent::HandleCurrentTargetDeath);
		}
	}

	CurrentTarget = NewTarget;

	if (IsValid(CurrentTarget))
	{
		CurrentTarget->OnDestroyed.AddUniqueDynamic(this, &UADTargetingComponent::HandleCurrentTargetDestroyed);
		if (AADCharacterBase* CurrentTargetCharacter = Cast<AADCharacterBase>(CurrentTarget))
		{
			CurrentTargetCharacter->OnCharacterDeath.AddUniqueDynamic(this, &UADTargetingComponent::HandleCurrentTargetDeath);
		}
		UE_LOG(LogTemp, Log, TEXT("[ActionDemo] Target locked: %s"), *CurrentTarget->GetName());
	}
	else if (OldTarget != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("[ActionDemo] Target unlocked."));
	}

	OnTargetChanged.Broadcast(OldTarget, CurrentTarget);
}

void UADTargetingComponent::ClearCurrentTarget()
{
	SetCurrentTarget(nullptr);
}

bool UADTargetingComponent::HasCurrentTarget() const
{
	return IsValid(CurrentTarget) && IsAliveTarget(CurrentTarget);
}

void UADTargetingComponent::HandleCurrentTargetDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == CurrentTarget)
	{
		SetCurrentTarget(nullptr);
	}
}

void UADTargetingComponent::HandleCurrentTargetDeath(AADCharacterBase* DeadCharacter)
{
	if (DeadCharacter == CurrentTarget)
	{
		SetCurrentTarget(nullptr);
	}
}

bool UADTargetingComponent::IsAliveTarget(const AActor* Candidate) const
{
	const AADCharacterBase* CandidateCharacter = Cast<AADCharacterBase>(Candidate);
	return CandidateCharacter == nullptr || !CandidateCharacter->IsDead();
}

void UADTargetingComponent::GatherTargetCandidates(TArray<AActor*>& OutCandidates) const
{
	OutCandidates.Reset();

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (World == nullptr || OwnerActor == nullptr)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ADTargetingOverlap), false, OwnerActor);
	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		OwnerActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(SearchRadius),
		QueryParams);

	if (!bHasOverlap)
	{
		return;
	}

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* Candidate = Result.GetActor();
		if (IsValidTargetCandidate(Candidate))
		{
			OutCandidates.AddUnique(Candidate);
		}
	}
}

bool UADTargetingComponent::IsValidTargetCandidate(const AActor* Candidate) const
{
	const AActor* OwnerActor = GetOwner();
	if (!IsValid(Candidate) || Candidate == OwnerActor)
	{
		return false;
	}

	if (!Candidate->IsA<AADCharacterBase>())
	{
		return false;
	}

	if (!IsAliveTarget(Candidate))
	{
		return false;
	}

	FVector ViewOrigin;
	FVector ViewForward;
	FVector ViewRight;
	if (!GetTargetViewData(ViewOrigin, ViewForward, ViewRight))
	{
		return false;
	}

	const FVector ToCandidate = Candidate->GetActorLocation() - ViewOrigin;
	if (ToCandidate.SizeSquared() > FMath::Square(SearchRadius))
	{
		return false;
	}

	const float DotToTarget = FVector::DotProduct(ToCandidate.GetSafeNormal(), ViewForward);
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(MaxViewAngleDegrees));
	if (DotToTarget < MinDot)
	{
		return false;
	}

	return !bRequireLineOfSight || HasLineOfSightToTarget(Candidate);
}

bool UADTargetingComponent::HasLineOfSightToTarget(const AActor* Candidate) const
{
	UWorld* World = GetWorld();
	const AActor* OwnerActor = GetOwner();
	if (World == nullptr || OwnerActor == nullptr || Candidate == nullptr)
	{
		return false;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ADTargetingLineOfSight), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	const FVector Start = OwnerActor->GetActorLocation();
	const FVector End = Candidate->GetActorLocation();
	const bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, LineOfSightTraceChannel.GetValue(), QueryParams);
	return !bHit || HitResult.GetActor() == Candidate;
}

float UADTargetingComponent::ScoreTargetCandidate(const AActor* Candidate) const
{
	FVector ViewOrigin;
	FVector ViewForward;
	FVector ViewRight;
	if (Candidate == nullptr || !GetTargetViewData(ViewOrigin, ViewForward, ViewRight))
	{
		return TNumericLimits<float>::Lowest();
	}

	const FVector ToCandidate = Candidate->GetActorLocation() - ViewOrigin;
	const float Distance = ToCandidate.Size();
	const float DirectionScore = FMath::Max(0.0f, FVector::DotProduct(ToCandidate.GetSafeNormal(), ViewForward));
	const float DistanceScore = 1.0f - FMath::Clamp(Distance / FMath::Max(SearchRadius, 1.0f), 0.0f, 1.0f);

	return DirectionScore * DirectionScoreWeight + DistanceScore * DistanceScoreWeight;
}

bool UADTargetingComponent::GetTargetViewData(FVector& OutOrigin, FVector& OutForward, FVector& OutRight) const
{
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return false;
	}

	if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		if (const AController* Controller = OwnerPawn->GetController())
		{
			FRotator ViewRotation;
			Controller->GetPlayerViewPoint(OutOrigin, ViewRotation);
			OutForward = ViewRotation.Vector().GetSafeNormal();
			OutRight = FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::Y);
			return true;
		}
	}

	OutOrigin = OwnerActor->GetActorLocation();
	OutForward = OwnerActor->GetActorForwardVector().GetSafeNormal();
	OutRight = OwnerActor->GetActorRightVector().GetSafeNormal();
	return true;
}

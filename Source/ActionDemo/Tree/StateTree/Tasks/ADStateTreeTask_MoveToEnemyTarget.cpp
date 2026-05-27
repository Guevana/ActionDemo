#include "Tree/StateTree/Tasks/ADStateTreeTask_MoveToEnemyTarget.h"

#include "AI/ADEnemyAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "Character/Base/ADCharacterBase.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"

UADStateTreeTask_MoveToEnemyTarget::UADStateTreeTask_MoveToEnemyTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCallTick = true;
}

AADEnemyAIController* UADStateTreeTask_MoveToEnemyTarget::ResolveEnemyController(FStateTreeExecutionContext& Context) const
{
	AActor* OwnerActor = GetOwnerActor(Context);
	if (AADEnemyAIController* EnemyController = Cast<AADEnemyAIController>(OwnerActor))
	{
		return EnemyController;
	}

	const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(OwnerActor);
	return Character != nullptr ? Cast<AADEnemyAIController>(Character->GetController()) : nullptr;
}

EStateTreeRunStatus UADStateTreeTask_MoveToEnemyTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	AADEnemyAIController* EnemyController = ResolveEnemyController(Context);
	if (EnemyController == nullptr || !EnemyController->HasCombatTarget())
	{
		return EStateTreeRunStatus::Failed;
	}

	if (EnemyController->IsControlledEnemyDead())
	{
		EnemyController->StopMovement();
		return EStateTreeRunStatus::Failed;
	}

	if (EnemyController->IsControlledEnemyHitReacting())
	{
		EnemyController->StopMovement();
		return EStateTreeRunStatus::Running;
	}

	if (EnemyController->IsTargetInAttackRange())
	{
		EnemyController->StopMovement();
		return EStateTreeRunStatus::Succeeded;
	}

	const float MoveAcceptanceRadius = bUseAttackRangeAsAcceptanceRadius ? EnemyController->GetAttackRange() : AcceptanceRadius;
	const EPathFollowingRequestResult::Type MoveResult = EnemyController->MoveToActor(
		EnemyController->GetCombatTarget(),
		MoveAcceptanceRadius,
		true,
		true,
		true,
		nullptr,
		true);

	return MoveResult == EPathFollowingRequestResult::Failed ? EStateTreeRunStatus::Failed : EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UADStateTreeTask_MoveToEnemyTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	AADEnemyAIController* EnemyController = ResolveEnemyController(Context);
	if (EnemyController == nullptr || !EnemyController->HasCombatTarget())
	{
		return EStateTreeRunStatus::Failed;
	}

	if (EnemyController->IsControlledEnemyDead())
	{
		EnemyController->StopMovement();
		return EStateTreeRunStatus::Failed;
	}

	if (EnemyController->IsControlledEnemyHitReacting())
	{
		EnemyController->StopMovement();
		return EStateTreeRunStatus::Running;
	}

	if (EnemyController->IsTargetInAttackRange())
	{
		EnemyController->StopMovement();
		return EStateTreeRunStatus::Succeeded;
	}

	const float MoveAcceptanceRadius = bUseAttackRangeAsAcceptanceRadius ? EnemyController->GetAttackRange() : AcceptanceRadius;
	EnemyController->MoveToActor(
		EnemyController->GetCombatTarget(),
		MoveAcceptanceRadius,
		true,
		true,
		true,
		nullptr,
		true);

	return EStateTreeRunStatus::Running;
}

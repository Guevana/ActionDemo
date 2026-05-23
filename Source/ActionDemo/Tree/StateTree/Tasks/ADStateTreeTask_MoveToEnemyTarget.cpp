#include "Tree/StateTree/Tasks/ADStateTreeTask_MoveToEnemyTarget.h"

#include "AI/ADEnemyAIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

UADStateTreeTask_MoveToEnemyTarget::UADStateTreeTask_MoveToEnemyTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCallTick = true;
}

EStateTreeRunStatus UADStateTreeTask_MoveToEnemyTarget::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	AADEnemyAIController* EnemyController = Cast<AADEnemyAIController>(GetOwnerActor(Context));
	if (EnemyController == nullptr || !EnemyController->HasCombatTarget())
	{
		return EStateTreeRunStatus::Failed;
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

	AADEnemyAIController* EnemyController = Cast<AADEnemyAIController>(GetOwnerActor(Context));
	if (EnemyController == nullptr || !EnemyController->HasCombatTarget())
	{
		return EStateTreeRunStatus::Failed;
	}

	if (EnemyController->IsTargetInAttackRange())
	{
		EnemyController->StopMovement();
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

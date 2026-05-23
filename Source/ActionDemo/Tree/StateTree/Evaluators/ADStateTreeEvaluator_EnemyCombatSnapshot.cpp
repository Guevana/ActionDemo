#include "Tree/StateTree/Evaluators/ADStateTreeEvaluator_EnemyCombatSnapshot.h"

#include "AI/ADEnemyAIController.h"
#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "StateTreeExecutionContext.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"

void UADStateTreeEvaluator_EnemyCombatSnapshot::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);
	RefreshSnapshot(Context);
}

void UADStateTreeEvaluator_EnemyCombatSnapshot::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);
	RefreshSnapshot(Context);
}

void UADStateTreeEvaluator_EnemyCombatSnapshot::RefreshSnapshot(FStateTreeExecutionContext& Context)
{
	const AADEnemyAIController* EnemyController = Cast<AADEnemyAIController>(GetOwnerActor(Context));
	if (EnemyController != nullptr)
	{
		CombatTarget = EnemyController->GetCombatTarget();
		TargetDistance = EnemyController->GetTargetDistance();
		bHasCombatTarget = EnemyController->HasCombatTarget();
		bIsTargetInAttackRange = EnemyController->IsTargetInAttackRange();
	}
	else
	{
		CombatTarget = nullptr;
		TargetDistance = TNumericLimits<float>::Max();
		bHasCombatTarget = false;
		bIsTargetInAttackRange = false;
	}

	const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(GetOwnerActor(Context));
	bHasActiveAction = Character != nullptr && Character->GetCombatComponent() != nullptr && Character->GetCombatComponent()->HasActiveAction();
}

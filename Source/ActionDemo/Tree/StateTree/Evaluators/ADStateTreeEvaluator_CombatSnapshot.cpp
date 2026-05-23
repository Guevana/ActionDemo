#include "Tree/StateTree/Evaluators/ADStateTreeEvaluator_CombatSnapshot.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Input/ADAbilityQueueComponent.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"
#include "StateTreeExecutionContext.h"

void UADStateTreeEvaluator_CombatSnapshot::TreeStart(FStateTreeExecutionContext& Context)
{
	Super::TreeStart(Context);
	RefreshSnapshot(Context);
}

void UADStateTreeEvaluator_CombatSnapshot::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);
	RefreshSnapshot(Context);
}

void UADStateTreeEvaluator_CombatSnapshot::RefreshSnapshot(FStateTreeExecutionContext& Context)
{
	const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(GetOwnerActor(Context));
	if (Character == nullptr)
	{
		BufferedInputTag = FGameplayTag();
		CurrentActionTag = FGameplayTag();
		LastCompletedActionTag = FGameplayTag();
		bHasActiveAction = false;
		return;
	}

	if (Character->GetAbilityQueueComponent() != nullptr)
	{
		BufferedInputTag = Character->GetAbilityQueueComponent()->GetBufferedInputTag();
	}
	else
	{
		BufferedInputTag = FGameplayTag();
	}

	if (Character->GetCombatComponent() != nullptr)
	{
		CurrentActionTag = Character->GetCombatComponent()->CurrentActionTag;
		LastCompletedActionTag = Character->GetCombatComponent()->LastCompletedActionTag;
		bHasActiveAction = Character->GetCombatComponent()->HasActiveAction();
	}
	else
	{
		CurrentActionTag = FGameplayTag();
		LastCompletedActionTag = FGameplayTag();
		bHasActiveAction = false;
	}
}

#include "Tree/StateTree/Conditions/ADStateTreeCondition_LastCompletedActionMatches.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"
#include "StateTreeExecutionContext.h"

bool UADStateTreeCondition_LastCompletedActionMatches::TestCondition(FStateTreeExecutionContext& Context) const
{
	const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(GetOwnerActor(Context));
	if (Character == nullptr || Character->GetCombatComponent() == nullptr || !ExpectedActionTag.IsValid())
	{
		return false;
	}

	return Character->GetCombatComponent()->LastCompletedActionTag.MatchesTagExact(ExpectedActionTag);
}

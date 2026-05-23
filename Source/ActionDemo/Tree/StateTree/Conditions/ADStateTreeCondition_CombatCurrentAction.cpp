#include "Tree/StateTree/Conditions/ADStateTreeCondition_CombatCurrentAction.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"
#include "StateTreeExecutionContext.h"

bool UADStateTreeCondition_CombatCurrentAction::TestCondition(FStateTreeExecutionContext& Context) const
{
	const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(GetOwnerActor(Context));
	if (Character == nullptr || Character->GetCombatComponent() == nullptr)
	{
		return false;
	}

	const UADCombatComponent* CombatComponent = Character->GetCombatComponent();
	switch (Mode)
	{
	case EADCombatCurrentActionConditionMode::HasAnyAction:
		return CombatComponent->HasActiveAction();
	case EADCombatCurrentActionConditionMode::HasNoAction:
		return !CombatComponent->HasActiveAction();
	case EADCombatCurrentActionConditionMode::MatchesExpectedTag:
		return CombatComponent->CurrentActionTag.MatchesTagExact(ExpectedActionTag);
	case EADCombatCurrentActionConditionMode::InCancelWindow:
		return CombatComponent->bInCancelWindow;
	case EADCombatCurrentActionConditionMode::NotInCancelWindow:
		return !CombatComponent->bInCancelWindow;
	default:
		return false;
	}
}

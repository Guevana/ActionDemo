#include "Tree/StateTree/Tasks/ADStateTreeTask_PlayCombatAbility.h"

#include "AbilitySystem/ADAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Input/ADAbilityQueueComponent.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"
#include "StateTreeExecutionContext.h"

UADStateTreeTask_PlayCombatAbility::UADStateTreeTask_PlayCombatAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCallTick = true;
}

EStateTreeRunStatus UADStateTreeTask_PlayCombatAbility::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	TrackedActionSerial = INDEX_NONE;
	TrackedActionTag = FGameplayTag();

	AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(GetOwnerActor(Context));
	if (Character == nullptr || Character->GetADAbilitySystemComponent() == nullptr || Character->GetAbilityQueueComponent() == nullptr || Character->GetCombatComponent() == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (ConsumedInputTag.IsValid())
	{
		FGameplayTag BufferedInputTag;
		if (!Character->GetAbilityQueueComponent()->ConsumeBufferedInput(BufferedInputTag) || !BufferedInputTag.MatchesTagExact(ConsumedInputTag))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	if (!AbilityClass || !Character->GetADAbilitySystemComponent()->TryActivateGrantedAbilityByClass(AbilityClass))
	{
		return EStateTreeRunStatus::Failed;
	}

	const UADCombatComponent* CombatComponent = Character->GetCombatComponent();
	if (!ExpectedActionTag.IsValid() || !CombatComponent->CurrentActionTag.MatchesTagExact(ExpectedActionTag))
	{
		return EStateTreeRunStatus::Failed;
	}

	TrackedActionTag = CombatComponent->CurrentActionTag;
	TrackedActionSerial = CombatComponent->CurrentActionSerial;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus UADStateTreeTask_PlayCombatAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(GetOwnerActor(Context));
	if (Character == nullptr || Character->GetCombatComponent() == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}

	const UADCombatComponent* CombatComponent = Character->GetCombatComponent();
	if (!TrackedActionTag.IsValid() || TrackedActionSerial == INDEX_NONE)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (CombatComponent->IsCurrentActionContext(TrackedActionTag, TrackedActionSerial))
	{
		return EStateTreeRunStatus::Running;
	}

	if (CombatComponent->LastCompletedActionTag.MatchesTagExact(TrackedActionTag) && CombatComponent->LastCompletedActionSerial == TrackedActionSerial)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (CombatComponent->HasActiveAction())
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Failed;
}

void UADStateTreeTask_PlayCombatAbility::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::ExitState(Context, Transition);
	
	if (RunStatus == EStateTreeRunStatus::Failed)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ActionDemo] PlayCombatAbility Task failed. Owner=%s AbilityClass=%s ExpectedActionTag=%s"),
			*GetOwnerActor(Context)->GetName(),
			*AbilityClass.GetDefaultObject()->GetName(),
			*ExpectedActionTag.ToString());
	}
	
	TrackedActionSerial = INDEX_NONE;
	TrackedActionTag = FGameplayTag();
}

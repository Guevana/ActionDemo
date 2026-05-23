#include "Components/Combat/ADCombatComponent.h"

#include "AbilitySystem/ADAbilitySystemComponent.h"
#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatHitReceiverInterface.h"
#include "Core/Tags/ADGameplayTags.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemBlueprintLibrary.h"

UADCombatComponent::UADCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UADCombatComponent::NotifyActionStartedWithContext(const FGameplayTag& ActionTag, TSubclassOf<UGameplayAbility> AbilityClass)
{
	CurrentActionTag = ActionTag;
	CurrentActionSerial = ++NextActionSerial;
	CurrentActionAbilityClass = AbilityClass;
	CurrentActionStartTime = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : -1.0f;
	LastCompletedActionTag = FGameplayTag();
	LastCompletedActionSerial = INDEX_NONE;
	ResetCancelWindowState();

	return CurrentActionSerial;
}

void UADCombatComponent::NotifyActionEndedByContext(const FGameplayTag& ActionTag, int32 ActionSerial)
{
	if (!ActionTag.IsValid())
	{
		return;
	}

	if (!IsCurrentActionContext(ActionTag, ActionSerial))
	{
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("[ActionDemo] Ignore stale action end. EndedAction=%s EndedSerial=%d CurrentAction=%s CurrentSerial=%d"),
			*ActionTag.ToString(),
			ActionSerial,
			*CurrentActionTag.ToString(),
			CurrentActionSerial);
		return;
	}

	LastCompletedActionTag = CurrentActionTag;
	LastCompletedActionSerial = CurrentActionSerial;
	CurrentActionTag = FGameplayTag();
	CurrentActionSerial = INDEX_NONE;
	CurrentActionAbilityClass = nullptr;
	CurrentActionStartTime = -1.0f;
	ResetCancelWindowState();
	UE_LOG(
			LogTemp,
			Verbose,
			TEXT("[ActionDemo] LastCompletedActionTag=%s"),*LastCompletedActionTag.ToString());
}

bool UADCombatComponent::IsCurrentActionContext(const FGameplayTag& ActionTag, int32 ActionSerial) const
{
	if (!CurrentActionTag.MatchesTagExact(ActionTag))
	{
		return false;
	}

	return ActionSerial == INDEX_NONE || CurrentActionSerial == ActionSerial;
}

void UADCombatComponent::SetCancelWindowEnabled(bool bEnabled)
{
	const bool bWasInCancelWindow = bInCancelWindow;
	bInCancelWindow = bEnabled;
	if (!bEnabled)
	{
		CancelWindowNotifyCount = 0;
	}

	SyncCancelWindowTag();
	BroadcastCancelWindowOpenedIfNeeded(bWasInCancelWindow);
}

void UADCombatComponent::BeginCancelWindowFromNotify()
{
	const bool bWasInCancelWindow = bInCancelWindow;
	++CancelWindowNotifyCount;
	bInCancelWindow = true;
	SyncCancelWindowTag();
	BroadcastCancelWindowOpenedIfNeeded(bWasInCancelWindow);
}

void UADCombatComponent::EndCancelWindowFromNotify()
{
	CancelWindowNotifyCount = FMath::Max(0, CancelWindowNotifyCount - 1);
	bInCancelWindow = CancelWindowNotifyCount > 0;
	SyncCancelWindowTag();
}

void UADCombatComponent::ResetCancelWindowState()
{
	CancelWindowNotifyCount = 0;
	bInCancelWindow = false;
	SyncCancelWindowTag();
}

bool UADCombatComponent::HasActiveAction() const
{
	return CurrentActionTag.IsValid();
}

void UADCombatComponent::HandleHitConfirmed(const FADCombatHitEventData& HitData)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ActionDemo] Combat hit confirmed. Source=%s Target=%s Action=%s Event=%s LockedTarget=%d"),
		HitData.InstigatorCharacter != nullptr ? *HitData.InstigatorCharacter->GetName() : TEXT("None"),
		HitData.TargetCharacter != nullptr ? *HitData.TargetCharacter->GetName() : TEXT("None"),
		*HitData.SourceActionTag.ToString(),
		*HitData.HitEventTag.ToString(),
		HitData.bHitLockedTarget);

	OnCombatHitConfirmed.Broadcast(HitData);
	SendHitGameplayEvent(HitData);
}

void UADCombatComponent::SyncCancelWindowTag()
{
	const AActor* OwnerActor = GetOwner();
	const AADCharacterBase* Character = Cast<AADCharacterBase>(OwnerActor);
	if (Character == nullptr || Character->GetADAbilitySystemComponent() == nullptr)
	{
		return;
	}

	if (bInCancelWindow)
	{
		Character->GetADAbilitySystemComponent()->AddLooseGameplayTag(ADGameplayTags::Ability_Cancel_Active);
	}
	else
	{
		Character->GetADAbilitySystemComponent()->RemoveLooseGameplayTag(ADGameplayTags::Ability_Cancel_Active);
	}
}

void UADCombatComponent::BroadcastCancelWindowOpenedIfNeeded(bool bWasInCancelWindow)
{
	if (!bWasInCancelWindow && bInCancelWindow)
	{
		OnCancelWindowOpened.Broadcast();
	}
}

void UADCombatComponent::SendHitGameplayEvent(const FADCombatHitEventData& HitData) const
{
	if (HitData.TargetCharacter == nullptr || !HitData.HitEventTag.IsValid())
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = HitData.HitEventTag;
	EventData.Instigator = HitData.InstigatorCharacter;
	EventData.Target = HitData.TargetCharacter;
	EventData.ContextHandle.AddHitResult(HitData.HitResult, true);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitData.TargetCharacter, HitData.HitEventTag, EventData);

	if (HitData.TargetCharacter->GetClass()->ImplementsInterface(UADCombatHitReceiverInterface::StaticClass()))
	{
		IADCombatHitReceiverInterface::Execute_ReceiveCombatHit(HitData.TargetCharacter, HitData);
	}
}

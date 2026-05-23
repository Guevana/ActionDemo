#include "AbilitySystem/Abilities/ADGA_Target_Lock.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Target/ADTargetingComponent.h"
#include "Core/Tags/ADGameplayTags.h"

UADGA_Target_Lock::UADGA_Target_Lock()
{
	InputTag = ADGameplayTags::Input_Target_Lock;
	AbilityTag = ADGameplayTags::Ability_Target_Lock;
	bCancelAbilityOnRepeatedInput = true;
}

void UADGA_Target_Lock::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UADTargetingComponent* TargetingComponent = ResolveTargetingComponent();
	if (TargetingComponent == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveTargetingComponent = TargetingComponent;
	TargetingComponent->OnTargetChanged.AddUniqueDynamic(this, &UADGA_Target_Lock::HandleTargetChanged);

	AActor* LockedTarget = TargetingComponent->ToggleLockTarget();
	if (!IsValid(LockedTarget))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	K2_OnAbilityActivated();
}

void UADGA_Target_Lock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UADTargetingComponent* TargetingComponent = ActiveTargetingComponent)
	{
		TargetingComponent->OnTargetChanged.RemoveDynamic(this, &UADGA_Target_Lock::HandleTargetChanged);
		if (TargetingComponent->HasCurrentTarget())
		{
			TargetingComponent->ClearCurrentTarget();
		}
	}

	ActiveTargetingComponent = nullptr;
	K2_OnAbilityEnded(bWasCancelled);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UADGA_Target_Lock::HandleTargetChanged(AActor* OldTarget, AActor* NewTarget)
{
	if (!bIsAbilityEnding && !IsValid(NewTarget))
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	}
}

UADTargetingComponent* UADGA_Target_Lock::ResolveTargetingComponent() const
{
	const AADCharacterBase* Character = Cast<AADCharacterBase>(GetAvatarActorFromActorInfo());
	return Character != nullptr ? Character->GetTargetingComponent() : nullptr;
}

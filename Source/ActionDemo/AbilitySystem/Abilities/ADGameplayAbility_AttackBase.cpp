#include "AbilitySystem/Abilities/ADGameplayAbility_AttackBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Animation/AnimInstance.h"
#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Core/Tags/ADGameplayTags.h"
#include "GameFramework/Character.h"

UADGameplayAbility_AttackBase::UADGameplayAbility_AttackBase()
{
	InputTag = ADGameplayTags::Input_Attack_Light;
}

void UADGameplayAbility_AttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bAttackFinishTaskStarted = false;
	AttackMontageTask = nullptr;

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	NotifyAttackStarted();

	const bool bMontageTaskStarted = bAutoPlayMontage && PlayAttackMontage();

	if (bAutoEndAbility && !bMontageTaskStarted)
	{
		StartAttackTimelineTasks(DefaultAttackDuration);
	}

	K2_OnAbilityActivated();
	K2_OnAttackStarted();
}

void UADGameplayAbility_AttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bWasCancelled)
	{
		StopAttackMontage();
	}

	NotifyAttackEnded();
	K2_OnAbilityEnded(bWasCancelled);
	bAttackFinishTaskStarted = false;
	AttackMontageTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UADGameplayAbility_AttackBase::NotifyAttackStarted()
{
	AADCharacterBase* Character = Cast<AADCharacterBase>(GetAvatarActorFromActorInfo());
	if (Character == nullptr || Character->GetCombatComponent() == nullptr)
	{
		return;
	}

	RegisteredActionSerial = Character->GetCombatComponent()->NotifyActionStartedWithContext(AbilityTag, GetClass());
	bCombatStateRegistered = true;
}

void UADGameplayAbility_AttackBase::NotifyAttackEnded()
{
	if (!bCombatStateRegistered)
	{
		
		UE_LOG(
			LogTemp,
			Verbose,
			TEXT("[ActionDemo] bCombatStateRegistered=false"));
		return;
	}

	if (AADCharacterBase* Character = Cast<AADCharacterBase>(GetAvatarActorFromActorInfo()))
	{
		if (Character->GetCombatComponent() != nullptr)
		{
			Character->GetCombatComponent()->NotifyActionEndedByContext(AbilityTag, RegisteredActionSerial);
		}
	}

	bCombatStateRegistered = false;
	RegisteredActionSerial = INDEX_NONE;
}

bool UADGameplayAbility_AttackBase::PlayAttackMontage()
{
	if (AttackMontage == nullptr)
	{
		return false;
	}

	AttackMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("AttackMontage"),
		AttackMontage,
		1.0f,
		NAME_None,
		true);

	if (AttackMontageTask == nullptr)
	{
		return false;
	}

	AttackMontageTask->OnCompleted.AddDynamic(this, &UADGameplayAbility_AttackBase::HandleAttackMontageCompleted);
	AttackMontageTask->OnInterrupted.AddDynamic(this, &UADGameplayAbility_AttackBase::HandleAttackMontageInterrupted);
	AttackMontageTask->OnCancelled.AddDynamic(this, &UADGameplayAbility_AttackBase::HandleAttackMontageCancelled);
	AttackMontageTask->ReadyForActivation();
	return true;
}

void UADGameplayAbility_AttackBase::StopAttackMontage() const
{
	const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr || AttackMontage == nullptr || Character->GetMesh() == nullptr)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(CancelMontageBlendOutTime, AttackMontage);
	}
}

void UADGameplayAbility_AttackBase::StartAttackTimelineTasks(float AttackDuration)
{
	if (!bIsAbilityEnding && !bAttackFinishTaskStarted && AttackDuration > 0.0f)
	{
		bAttackFinishTaskStarted = true;
		UAbilityTask_WaitDelay* FinishTask = UAbilityTask_WaitDelay::WaitDelay(this, AttackDuration);
		FinishTask->OnFinish.AddDynamic(this, &UADGameplayAbility_AttackBase::HandleAttackFinished);
		FinishTask->ReadyForActivation();
	}
}

void UADGameplayAbility_AttackBase::HandleAttackFinished()
{
	K2_OnAttackFinished();
	EndAbility(this->CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UADGameplayAbility_AttackBase::HandleAttackMontageCompleted()
{
	if (bIsAbilityEnding)
	{
		return;
	}

	K2_OnAttackFinished();
	EndAbility(this->CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UADGameplayAbility_AttackBase::HandleAttackMontageInterrupted()
{
	if (bIsAbilityEnding)
	{
		return;
	}

	EndAbility(this->CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

void UADGameplayAbility_AttackBase::HandleAttackMontageCancelled()
{
	if (bIsAbilityEnding)
	{
		return;
	}

	EndAbility(this->CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

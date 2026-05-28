#include "AbilitySystem/Abilities/ADGameplayAbility_AttackBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystem/Effects/ADGameplayEffect_Damage.h"
#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Target/ADTargetingComponent.h"
#include "Core/Tags/ADGameplayTags.h"
#include "GameFramework/Character.h"
#include "MotionWarpingComponent.h"

UADGameplayAbility_AttackBase::UADGameplayAbility_AttackBase()
{
	InputTag = ADGameplayTags::Input_Attack_Light;
	DamageEffectClass = UADGameplayEffect_Damage::StaticClass();
	ActivationBlockedTags.AddTag(ADGameplayTags::State_Dead);
	ActivationBlockedTags.AddTag(ADGameplayTags::State_Hit_React);
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

	TryUpdateLockOnWarpTarget();
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

bool UADGameplayAbility_AttackBase::TryUpdateLockOnWarpTarget()
{
	if (!bUpdateLockOnWarpTargetOnCommit || LockOnWarpTargetName.IsNone())
	{
		return false;
	}

	AADCharacterBase* Character = Cast<AADCharacterBase>(GetAvatarActorFromActorInfo());
	if (Character == nullptr || Character->IsDead() || Character->GetMotionWarpingComponent() == nullptr)
	{
		return false;
	}

	if (!bUseLockedTargetForWarping || Character->GetTargetingComponent() == nullptr)
	{
		ClearLockOnWarpTarget();
		return false;
	}

	AActor* CurrentTarget = Character->GetTargetingComponent()->CurrentTarget;
	if (!IsValidLockOnWarpTarget(CurrentTarget))
	{
		ClearLockOnWarpTarget();
		return false;
	}

	FVector ToTarget = CurrentTarget->GetActorLocation() - Character->GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.IsNearlyZero())
	{
		ClearLockOnWarpTarget();
		return false;
	}

	const FVector ForwardToTarget = ToTarget.GetSafeNormal();
	const FVector RightToTarget = FVector::CrossProduct(FVector::UpVector, ForwardToTarget).GetSafeNormal();
	const FVector TargetLocation = CurrentTarget->GetActorLocation()
		+ ForwardToTarget * LockOnWarpTargetForwardOffset
		+ RightToTarget * LockOnWarpTargetRightOffset
		+ FVector::UpVector * LockOnWarpTargetUpOffset;
	const FRotator TargetRotation(0.0f, ForwardToTarget.Rotation().Yaw, 0.0f);

	Character->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromTransform(
		LockOnWarpTargetName,
		FTransform(TargetRotation, TargetLocation));
	return true;
}

void UADGameplayAbility_AttackBase::ClearLockOnWarpTarget() const
{
	const AADCharacterBase* Character = Cast<AADCharacterBase>(GetAvatarActorFromActorInfo());
	if (Character != nullptr && Character->GetMotionWarpingComponent() != nullptr && !LockOnWarpTargetName.IsNone())
	{
		Character->GetMotionWarpingComponent()->RemoveWarpTarget(LockOnWarpTargetName);
	}
}

bool UADGameplayAbility_AttackBase::IsValidLockOnWarpTarget(const AActor* TargetActor) const
{
	const AADCharacterBase* Character = Cast<AADCharacterBase>(GetAvatarActorFromActorInfo());
	const AADCharacterBase* TargetCharacter = Cast<AADCharacterBase>(TargetActor);
	return Character != nullptr &&
		IsValid(TargetActor) &&
		TargetActor != Character &&
		TargetCharacter != nullptr &&
		!TargetCharacter->IsDead();
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

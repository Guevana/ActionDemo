#include "AbilitySystem/Abilities/ADGameplayAbility_ReceiveHit.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/ADAbilitySystemComponent.h"
#include "AbilitySystem/Effects/ADGameplayEffect_Damage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/Base/ADCharacterBase.h"
#include "Core/Tags/ADGameplayTags.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"

UADGameplayAbility_ReceiveHit::UADGameplayAbility_ReceiveHit()
{
	AbilityTag = ADGameplayTags::Ability_Hit_React;
	bRetriggerInstancedAbility = true;
	DefaultDamageEffectClass = UADGameplayEffect_Damage::StaticClass();
	ActivationBlockedTags.AddTag(ADGameplayTags::State_Dead);

	FAbilityTriggerData HitTrigger;
	HitTrigger.TriggerTag = ADGameplayTags::Event_Hit_Confirm;
	HitTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(HitTrigger);
}

void UADGameplayAbility_ReceiveHit::ActivateAbility(
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

	const bool bAppliedDamage = ApplyDamageEffect(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData != nullptr)
	{
		K2_OnHitReceived(*TriggerEventData, 0.0f);
	}

	if (!bAppliedDamage || !ShouldEnterHitReact(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	BeginHitReact(ActorInfo, TriggerEventData, 0.0f);
}

void UADGameplayAbility_ReceiveHit::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bHitReactActive)
	{
		if (ActorInfo != nullptr && ActorInfo->AbilitySystemComponent != nullptr)
		{
			ActorInfo->AbilitySystemComponent->RemoveLooseGameplayTag(ADGameplayTags::State_Hit_React);
		}

		K2_OnHitReactEnded(ActiveHitReactDamage, bWasCancelled);
	}

	bHitReactActive = false;
	ActiveHitReactDamage = 0.0f;
	HitReactDelayTask = nullptr;
	HitReactMontageTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UADGameplayAbility_ReceiveHit::ApplyDamageEffect(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData) const
{
	if (ActorInfo == nullptr || ActorInfo->AbilitySystemComponent == nullptr)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = ActorInfo->AbilitySystemComponent.Get();
	if (TargetASC->HasMatchingGameplayTag(ADGameplayTags::Status_Invincible) ||
		TargetASC->HasMatchingGameplayTag(ADGameplayTags::State_Dead))
	{
		return false;
	}

	const TSubclassOf<UGameplayEffect> DamageEffectClass = ResolveDamageEffectClass(TriggerEventData);
	if (DamageEffectClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] ReceiveHit has no damage effect class."));
		return false;
	}

	UAbilitySystemComponent* SourceASC = nullptr;
	if (TriggerEventData != nullptr)
	{
		if (const AADCharacterBase* InstigatorCharacter = Cast<AADCharacterBase>(TriggerEventData->Instigator.Get()))
		{
			SourceASC = InstigatorCharacter->GetADAbilitySystemComponent();
		}
	}

	UAbilitySystemComponent* SpecASC = SourceASC != nullptr ? SourceASC : TargetASC;
	FGameplayEffectContextHandle EffectContext = TriggerEventData != nullptr && TriggerEventData->ContextHandle.IsValid()
		? TriggerEventData->ContextHandle
		: SpecASC->MakeEffectContext();
	if (TriggerEventData != nullptr && TriggerEventData->Instigator != nullptr)
	{
		EffectContext.AddInstigator(const_cast<AActor*>(TriggerEventData->Instigator.Get()), const_cast<AActor*>(TriggerEventData->Instigator.Get()));
	}

	FGameplayEffectSpecHandle SpecHandle = SpecASC->MakeOutgoingSpec(
		DamageEffectClass,
		GetAbilityLevel(Handle, ActorInfo),
		EffectContext);
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return false;
	}

	const float DamageScale = TriggerEventData != nullptr
		? FMath::Max(0.0f, TriggerEventData->EventMagnitude)
		: 1.0f;
	SpecHandle.Data->SetSetByCallerMagnitude(ADGameplayTags::Data_DamageScale, DamageScale);

	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ActionDemo] ReceiveHit applied damage effect. Target=%s Effect=%s"),
		ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("None"),
		*GetNameSafe(DamageEffectClass.Get()));

	return true;
}

bool UADGameplayAbility_ReceiveHit::ShouldEnterHitReact(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (ActorInfo == nullptr || ActorInfo->AbilitySystemComponent == nullptr || HitReactDuration <= 0.0f)
	{
		return false;
	}

	if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(ADGameplayTags::State_Dead))
	{
		return false;
	}

	const AADCharacterBase* Character = Cast<AADCharacterBase>(ActorInfo->AvatarActor.Get());
	return Character == nullptr || !Character->IsDead();
}

void UADGameplayAbility_ReceiveHit::BeginHitReact(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData, float DamageAmount)
{
	if (ActorInfo == nullptr || ActorInfo->AbilitySystemComponent == nullptr)
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
		return;
	}

	if (bCancelActiveAttackOnHit)
	{
		if (UADAbilitySystemComponent* ADASC = Cast<UADAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
		{
			ADASC->CancelActiveAttackAbilities();
		}
	}

	if (bStopMovementOnHit)
	{
		StopActiveMovement(ActorInfo);
	}

	ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(ADGameplayTags::State_Hit_React);
	bHitReactActive = true;
	ActiveHitReactDamage = DamageAmount;

	if (TriggerEventData != nullptr)
	{
		K2_OnHitReactStarted(*TriggerEventData, DamageAmount);
	}

	PlayHitReactMontage();

	HitReactDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, HitReactDuration);
	if (HitReactDelayTask == nullptr)
	{
		EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
		return;
	}

	HitReactDelayTask->OnFinish.AddDynamic(this, &UADGameplayAbility_ReceiveHit::HandleHitReactFinished);
	HitReactDelayTask->ReadyForActivation();
}

void UADGameplayAbility_ReceiveHit::StopActiveMovement(const FGameplayAbilityActorInfo* ActorInfo) const
{
	AADCharacterBase* Character = ActorInfo != nullptr ? Cast<AADCharacterBase>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (Character == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (AController* Controller = Character->GetController())
	{
		Controller->StopMovement();
	}
}

void UADGameplayAbility_ReceiveHit::PlayHitReactMontage()
{
	if (HitReactMontage == nullptr)
	{
		return;
	}

	HitReactMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		TEXT("HitReactMontage"),
		HitReactMontage,
		HitReactMontageRate,
		NAME_None,
		true);

	if (HitReactMontageTask == nullptr)
	{
		return;
	}

	HitReactMontageTask->OnInterrupted.AddDynamic(this, &UADGameplayAbility_ReceiveHit::HandleHitReactMontageInterrupted);
	HitReactMontageTask->OnCancelled.AddDynamic(this, &UADGameplayAbility_ReceiveHit::HandleHitReactMontageCancelled);
	HitReactMontageTask->ReadyForActivation();
}

void UADGameplayAbility_ReceiveHit::HandleHitReactFinished()
{
	if (bIsAbilityEnding)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UADGameplayAbility_ReceiveHit::HandleHitReactMontageInterrupted()
{
	if (bIsAbilityEnding)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

void UADGameplayAbility_ReceiveHit::HandleHitReactMontageCancelled()
{
	if (bIsAbilityEnding)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

TSubclassOf<UGameplayEffect> UADGameplayAbility_ReceiveHit::ResolveDamageEffectClass(const FGameplayEventData* TriggerEventData) const
{
	if (TriggerEventData != nullptr && TriggerEventData->OptionalObject != nullptr)
	{
		const UClass* OptionalEffectClass = Cast<UClass>(TriggerEventData->OptionalObject.Get());
		if (OptionalEffectClass != nullptr && OptionalEffectClass->IsChildOf(UGameplayEffect::StaticClass()))
		{
			return const_cast<UClass*>(OptionalEffectClass);
		}
	}

	return DefaultDamageEffectClass;
}

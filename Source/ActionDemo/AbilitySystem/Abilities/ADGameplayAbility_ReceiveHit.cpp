#include "AbilitySystem/Abilities/ADGameplayAbility_ReceiveHit.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Effects/ADGameplayEffect_Damage.h"
#include "Core/Tags/ADGameplayTags.h"
#include "GameplayEffect.h"

UADGameplayAbility_ReceiveHit::UADGameplayAbility_ReceiveHit()
{
	AbilityTag = ADGameplayTags::Ability_Hit_React;
	DefaultDamageEffectClass = UADGameplayEffect_Damage::StaticClass();

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

	const float DamageAmount = ResolveDamageAmount(TriggerEventData);
	const bool bAppliedDamage = ApplyDamageEffect(Handle, ActorInfo, ActivationInfo, TriggerEventData, DamageAmount);

	if (TriggerEventData != nullptr)
	{
		K2_OnHitReceived(*TriggerEventData, bAppliedDamage ? DamageAmount : 0.0f);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UADGameplayAbility_ReceiveHit::ApplyDamageEffect(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData,
	float DamageAmount) const
{
	if (ActorInfo == nullptr || ActorInfo->AbilitySystemComponent == nullptr || DamageAmount <= 0.0f)
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

	FGameplayEffectContextHandle EffectContext = TriggerEventData != nullptr && TriggerEventData->ContextHandle.IsValid()
		? TriggerEventData->ContextHandle
		: TargetASC->MakeEffectContext();
	if (TriggerEventData != nullptr && TriggerEventData->Instigator != nullptr)
	{
		EffectContext.AddInstigator(const_cast<AActor*>(TriggerEventData->Instigator.Get()), const_cast<AActor*>(TriggerEventData->Instigator.Get()));
	}

	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
		DamageEffectClass,
		GetAbilityLevel(Handle, ActorInfo),
		EffectContext);
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return false;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(ADGameplayTags::Data_Damage, -FMath::Abs(DamageAmount));
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ActionDemo] ReceiveHit applied damage. Target=%s Damage=%.2f Effect=%s"),
		ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("None"),
		DamageAmount,
		*GetNameSafe(DamageEffectClass.Get()));

	return true;
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

float UADGameplayAbility_ReceiveHit::ResolveDamageAmount(const FGameplayEventData* TriggerEventData) const
{
	if (TriggerEventData != nullptr)
	{
		return FMath::Max(0.0f, TriggerEventData->EventMagnitude);
	}

	return DefaultDamageAmount;
}

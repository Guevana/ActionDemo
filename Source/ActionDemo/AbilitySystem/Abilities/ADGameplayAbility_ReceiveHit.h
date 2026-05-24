#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "ADGameplayAbility_ReceiveHit.generated.h"

class UGameplayEffect;

/**
 * 标准受击 Ability。
 * 由 Event.Hit.Confirm 触发，目标侧消费命中事件并应用伤害 GE。
 */
UCLASS(Blueprintable)
class ACTIONDEMO_API UADGameplayAbility_ReceiveHit : public UADGameplayAbility
{
	GENERATED_BODY()

public:
	UADGameplayAbility_ReceiveHit();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Hit", meta = (ClampMin = "0.0"))
	float DefaultDamageAmount = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Hit")
	TSubclassOf<UGameplayEffect> DefaultDamageEffectClass;

	bool ApplyDamageEffect(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData,
		float DamageAmount) const;

	TSubclassOf<UGameplayEffect> ResolveDamageEffectClass(const FGameplayEventData* TriggerEventData) const;

	float ResolveDamageAmount(const FGameplayEventData* TriggerEventData) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|Hit", meta = (DisplayName = "On Hit Received"))
	void K2_OnHitReceived(const FGameplayEventData& TriggerEventData, float DamageAmount);
};

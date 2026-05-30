#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "ADGameplayAbility_ReceiveHit.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;
class UAnimMontage;
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

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Hit")
	TSubclassOf<UGameplayEffect> DefaultDamageEffectClass;

	/** 受击硬直持续时间。为 0 时只扣血和触发蓝图受击事件。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|HitReact", meta = (ClampMin = "0.0"))
	float HitReactDuration = 0.35f;

	/** 可选受击表现 Montage。硬直生命周期仍由 HitReactDuration 驱动。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|HitReact", meta = (ClampMin = "0.0"))
	float HitReactMontageRate = 1.0f;

	/** 受击后是否打断当前攻击 Ability。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|HitReact")
	bool bCancelActiveAttackOnHit = true;

	/** 受击硬直开始时是否停止当前移动/AI MoveTo。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|HitReact")
	bool bStopMovementOnHit = true;

	bool ApplyDamageEffect(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) const;

	bool ShouldEnterHitReact(const FGameplayAbilityActorInfo* ActorInfo) const;

	void BeginHitReact(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData, float DamageAmount);

	void StopActiveMovement(const FGameplayAbilityActorInfo* ActorInfo) const;

	void PlayHitReactMontage();

	UFUNCTION()
	void HandleHitReactFinished();

	UFUNCTION()
	void HandleHitReactMontageInterrupted();

	UFUNCTION()
	void HandleHitReactMontageCancelled();

	TSubclassOf<UGameplayEffect> ResolveDamageEffectClass(const FGameplayEventData* TriggerEventData) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|Hit", meta = (DisplayName = "On Hit Received"))
	void K2_OnHitReceived(const FGameplayEventData& TriggerEventData, float DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|HitReact", meta = (DisplayName = "On Hit React Started"))
	void K2_OnHitReactStarted(const FGameplayEventData& TriggerEventData, float DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|HitReact", meta = (DisplayName = "On Hit React Ended"))
	void K2_OnHitReactEnded(float DamageAmount, bool bWasCancelled);

	bool bHitReactActive = false;

	float ActiveHitReactDamage = 0.0f;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> HitReactDelayTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> HitReactMontageTask;
};

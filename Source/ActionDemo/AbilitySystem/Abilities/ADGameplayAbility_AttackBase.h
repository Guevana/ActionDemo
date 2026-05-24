#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "ADGameplayAbility_AttackBase.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitDelay;
class UAnimMontage;
class UADCombatActionData;
class UGameplayEffect;

/**
 * 攻击类 Ability 通用基类。
 * C++ 负责攻击 Ability 的公共生命周期，取消窗口时机由动画通知状态驱动。
 */
UCLASS(Blueprintable, Abstract)
class ACTIONDEMO_API UADGameplayAbility_AttackBase : public UADGameplayAbility
{
	GENERATED_BODY()

public:
	UADGameplayAbility_AttackBase();

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

	/** 攻击动作对应的 Montage。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 攻击总时长，默认用于自动结束 Ability(一般需长于montage长度)。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack", meta = (ClampMin = "0.0"))
	float DefaultAttackDuration = 0.35f;
	
	/** 无 Montage 或 Montage 未启动时，是否使用 DefaultAttackDuration 兜底结束 Ability。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack")
	bool bAutoEndAbility = true;
	
	/** 是否在激活后自动播放 Montage。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack")
	bool bAutoPlayMontage = true;

	/** 连段取消旧 Ability 时 Montage 的淡出时间。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack", meta = (ClampMin = "0.0"))
	float CancelMontageBlendOutTime = 0.08f;
	
	/** 攻击 Ability 可驱动的数据配置。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack")
	TObjectPtr<UADCombatActionData> ActionData;

	/** 命中后传给目标受击 Ability 的基础伤害值。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack|Damage", meta = (ClampMin = "0.0"))
	float DamageAmount = 10.0f;

	/** 命中后由目标侧受击 Ability 应用的伤害 GameplayEffect。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attack|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

protected:
	/** 统一登记攻击开始时的战斗状态。 */
	void NotifyAttackStarted();

	/** 统一登记攻击结束时的战斗状态。 */
	void NotifyAttackEnded();

	/** 若配置了 Montage，则通过 AbilityTask 播放并等待生命周期事件。 */
	bool PlayAttackMontage();

	/** 取消 Ability 时停止当前攻击 Montage，避免状态已切换但旧动画仍播放。 */
	void StopAttackMontage() const;

	/** 启动自动结束任务。 */
	void StartAttackTimelineTasks(float AttackDuration);

	/** 自动结束 Ability 的回调。 */
	UFUNCTION()
	void HandleAttackFinished();

	/** Montage 正常播放完成时结束 Ability。 */
	UFUNCTION()
	void HandleAttackMontageCompleted();

	/** Montage 被其他动画覆盖或中断时取消 Ability。 */
	UFUNCTION()
	void HandleAttackMontageInterrupted();

	/** Montage Task 被 Ability 取消时的回调。 */
	UFUNCTION()
	void HandleAttackMontageCancelled();

	/** 蓝图层攻击开始回调。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|Attack", meta = (DisplayName = "On Attack Started"))
	void K2_OnAttackStarted();

	/** 蓝图层攻击结束回调。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|Attack", meta = (DisplayName = "On Attack Finished"))
	void K2_OnAttackFinished();

	/** 当前 Ability 生命周期内是否已登记战斗状态。 */
	bool bCombatStateRegistered = false;

	/** 当前 Ability 登记到 CombatComponent 后获得的动作实例编号。 */
	int32 RegisteredActionSerial = INDEX_NONE;

	/** 防止 Montage 时长和默认时长同时启动两个自动结束任务。 */
	bool bAttackFinishTaskStarted = false;

	/** 当前攻击 Montage 播放任务。 */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> AttackMontageTask;
};

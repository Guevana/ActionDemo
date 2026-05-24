#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Combat/ADCombatHitTypes.h"
#include "GameplayTagContainer.h"
#include "ADCombatComponent.generated.h"

class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FADCombatHitConfirmedSignature, const FADCombatHitEventData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FADCancelWindowOpenedSignature);

/**
 * 战斗主控组件。
 * 维护动作上下文、取消窗口和命中事件入口，是 Ability 与表现层之间的协调点。
 */
UCLASS(ClassGroup=(ActionDemo), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class ACTIONDEMO_API UADCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UADCombatComponent();

	/** 当前正在执行的动作标签。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Combat")
	FGameplayTag CurrentActionTag;

	/** 当前动作实例编号，用于识别旧 Ability / 旧 Notify 的过期回调。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Combat")
	int32 CurrentActionSerial = INDEX_NONE;

	/** 当前动作对应的 Ability 类，供调试和后续 StateTree 精确跟踪使用。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Combat")
	TSubclassOf<UGameplayAbility> CurrentActionAbilityClass;

	/** 当前动作开始时的世界时间。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Combat")
	float CurrentActionStartTime = -1.0f;

	/** 最近一次完成的动作标签，供 StateTree 判定连段跳转。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Combat")
	FGameplayTag LastCompletedActionTag;

	/** 最近一次完成的动作实例编号。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Combat")
	int32 LastCompletedActionSerial = INDEX_NONE;

	/** 当前是否处于可取消窗口。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Combat")
	bool bInCancelWindow = false;

	/** 标准命中确认事件，供表现层和蓝图监听。 */
	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Combat")
	FADCombatHitConfirmedSignature OnCombatHitConfirmed;

	/** 取消窗口从关闭切换到开启时广播，用于驱动预输入消费。 */
	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Combat")
	FADCancelWindowOpenedSignature OnCancelWindowOpened;

	/** 标记一个动作开始执行，并返回本次动作实例编号。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Combat")
	int32 NotifyActionStartedWithContext(const FGameplayTag& ActionTag, TSubclassOf<UGameplayAbility> AbilityClass);

	/** 标记指定动作实例结束；若实例编号已过期，则忽略旧动作的结束通知。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Combat")
	void NotifyActionEndedByContext(const FGameplayTag& ActionTag, int32 ActionSerial);

	/** 当前动作实例是否仍然匹配。 */
	UFUNCTION(BlueprintPure, Category = "ActionDemo|Combat")
	bool IsCurrentActionContext(const FGameplayTag& ActionTag, int32 ActionSerial) const;

	/** 强制设置当前取消窗口状态。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Combat")
	void SetCancelWindowEnabled(bool bEnabled);

	/** 由动画通知状态驱动取消窗口开启。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Combat")
	void BeginCancelWindowFromNotify();

	/** 由动画通知状态驱动取消窗口关闭。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Combat")
	void EndCancelWindowFromNotify();

	/** 清空取消窗口状态。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Combat")
	void ResetCancelWindowState();

	/** 当前是否存在正在执行的动作。 */
	UFUNCTION(BlueprintPure, Category = "ActionDemo|Combat")
	bool HasActiveAction() const;

	/** 命中检测统一入口。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Combat")
	void HandleHitConfirmed(const FADCombatHitEventData& HitData);

	/** 根据当前动作 Ability 配置补全命中伤害数据。 */
	void FillHitDataFromCurrentAction(FADCombatHitEventData& HitData) const;

protected:
	/** 同步 Combat 状态到 ASC 标签。 */
	void SyncCancelWindowTag();

	/** 当取消窗口刚开启时广播一次。 */
	void BroadcastCancelWindowOpenedIfNeeded(bool bWasInCancelWindow);

	void SendHitGameplayEvent(const FADCombatHitEventData& HitData) const;

	/** 当前通知状态层级计数，防止多个通知重叠时误关窗口。 */
	int32 CancelWindowNotifyCount = 0;

	/** 动作实例编号生成器，只在动作开始时递增。 */
	int32 NextActionSerial = 0;
};

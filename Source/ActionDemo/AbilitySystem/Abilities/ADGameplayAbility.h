#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ADGameplayAbility.generated.h"

/**
 * 项目通用 Ability 基类。
 * C++ 负责统一的 Ability 生命周期骨架，蓝图子类负责参数配置与细节表现扩展。
 */
UCLASS(Blueprintable, Abstract)
class ACTIONDEMO_API UADGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UADGameplayAbility();

	/** Ability 对应的输入标签，用于从输入缓冲或 StateTree 请求激活。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Ability")
	FGameplayTag InputTag;

	/** Ability 的核心分类标签。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Ability")
	FGameplayTag AbilityTag;

	/** 已激活时再次收到同一输入标签，是否取消当前 Ability。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Ability")
	bool bCancelAbilityOnRepeatedInput = false;

protected:
	/** Ability 提交成功后的蓝图扩展点。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|Ability", meta = (DisplayName = "On Ability Activated"))
	void K2_OnAbilityActivated();

	/** Ability 结束前的蓝图扩展点。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ActionDemo|Ability", meta = (DisplayName = "On Ability Ended"))
	void K2_OnAbilityEnded(bool bWasCancelled);
};

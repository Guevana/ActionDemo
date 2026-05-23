#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ADAbilitySystemComponent.generated.h"

/**
 * 项目 AbilitySystemComponent 基类。
 * 用于承接通用的 Ability 激活、输入和标签工具逻辑。
 */
UCLASS()
class ACTIONDEMO_API UADAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** 尝试激活带有指定输入标签的 Ability。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Ability")
	bool TryActivateAbilityByInputTag(const FGameplayTag& InputTag);

	/** 尝试激活指定 Ability 类对应的已授予实例。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Ability")
	bool TryActivateGrantedAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);

	/** 取消当前仍在执行的攻击 Ability，用于连段派生前回收上一段动作所有权。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Ability")
	bool CancelActiveAttackAbilities();
};

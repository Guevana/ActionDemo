#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ADCombatActionData.generated.h"

class UADGameplayAbility;

USTRUCT(BlueprintType)
struct FADCombatActionTransition
{
	GENERATED_BODY()

	/** 触发该派生所需的输入标签。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat")
	FGameplayTag InputTag;

	/** 满足条件时将跳转到的下一段 Ability。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat")
	TSubclassOf<UADGameplayAbility> NextAbilityClass;
};

/**
 * 动作配置数据资产。
 * 后续用于描述连段、派生、取消窗口等规则。
 */
UCLASS(BlueprintType)
class ACTIONDEMO_API UADCombatActionData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 当前动作支持的后续派生列表。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat")
	TArray<FADCombatActionTransition> Transitions;

	/** 当前动作是否允许在空中执行。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat")
	bool bAllowInAir = false;
};

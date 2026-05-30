#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputTriggers.h"
#include "ADInputConfigData.generated.h"

class UADGameplayAbility;
class UGameplayEffect;
class UInputAction;
class UInputMappingContext;
class AADPlayerController;

USTRUCT(BlueprintType)
struct FADInputMappingContextEntry
{
	GENERATED_BODY()

	/** 需要添加到本地玩家子系统的 MappingContext。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input")
	TObjectPtr<UInputMappingContext> MappingContext = nullptr;

	/** MappingContext 的优先级。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input")
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FADAbilityInputBinding
{
	GENERATED_BODY()

	/** 对应的输入动作资源。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input")
	TObjectPtr<UInputAction> InputAction = nullptr;

	/** 输入动作映射到的 GameplayTag。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input")
	FGameplayTag InputTag;
	
	/** 触发时调用的函数名（在ADPlayerController中）。 */
	UPROPERTY(EditAnywhere, Category = "ActionDemo|Input", meta = (GetOptions = "ActionDemo.ADPlayerController.GetRemoteFunctionNames"))
	FName FunctionNameToCall;
	
	/** 触发该输入逻辑的 Enhanced Input 事件。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input")
	ETriggerEvent TriggerEvent = ETriggerEvent::Triggered;
};

/**
 * 玩家输入与启动 Ability 配置资产。
 * C++ 负责读取并执行逻辑，具体资源绑定由蓝图或数据资产配置。
 */
UCLASS(BlueprintType)
class ACTIONDEMO_API UADInputConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	/** 启动时需要添加的输入上下文。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input")
	TArray<FADInputMappingContextEntry> MappingContexts;

	/** 需要绑定的输入动作。所有战斗操作都应通过 InputTag 触发 Ability 或 StateTree 决策。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input")
	TArray<FADAbilityInputBinding> AbilityInputBindings;

	/** 角色启动时授予的 Ability 列表。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Ability")
	TArray<TSubclassOf<UADGameplayAbility>> StartupAbilities;

	/** 角色启动时应用到自身的属性 GameplayEffect。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Attributes")
	TArray<TSubclassOf<UGameplayEffect>> StartupAttributeEffects;
	
};

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "ADStateTreeTask_PlayCombatAbility.generated.h"

class UADGameplayAbility;

/**
 * StateTree 攻击执行任务。
 * 在进入状态时激活指定 Ability，并在动作完成后向 StateTree 返回成功。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeTask_PlayCombatAbility : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UADStateTreeTask_PlayCombatAbility(const FObjectInitializer& ObjectInitializer);

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	/** 当前状态需要激活的 Ability。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	TSubclassOf<UADGameplayAbility> AbilityClass;

	/** 状态预期驱动的动作标签。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag ExpectedActionTag;

	/** 进入状态时需要消费的输入标签。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag ConsumedInputTag;

	/** 本 StateTree 任务正在跟踪的动作实例编号。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	int32 TrackedActionSerial = INDEX_NONE;

	/** 本 StateTree 任务正在跟踪的动作标签。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag TrackedActionTag;
};

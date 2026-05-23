#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "ADStateTreeEvaluator_CombatSnapshot.generated.h"

/**
 * 将当前战斗相关快照同步到 StateTree。
 * 当前主要用于调试与资产绑定，可供后续条件/任务扩展复用。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeEvaluator_CombatSnapshot : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	void RefreshSnapshot(FStateTreeExecutionContext& Context);

public:
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag BufferedInputTag;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag CurrentActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag LastCompletedActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bHasActiveAction = false;
};

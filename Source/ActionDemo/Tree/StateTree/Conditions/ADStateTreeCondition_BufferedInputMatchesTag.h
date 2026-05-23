#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeConditionBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "ADStateTreeCondition_BufferedInputMatchesTag.generated.h"

/**
 * 判断当前输入缓冲是否匹配指定标签。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeCondition_BufferedInputMatchesTag : public UStateTreeConditionBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag ExpectedInputTag;
};

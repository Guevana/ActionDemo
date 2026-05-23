#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeConditionBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "ADStateTreeCondition_LastCompletedActionMatches.generated.h"

/**
 * 判断最近一次完成的动作是否匹配指定标签。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeCondition_LastCompletedActionMatches : public UStateTreeConditionBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FGameplayTag ExpectedActionTag;
};

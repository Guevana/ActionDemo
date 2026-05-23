#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeConditionBlueprintBase.h"
#include "GameplayTagContainer.h"
#include "ADStateTreeCondition_CombatCurrentAction.generated.h"

UENUM(BlueprintType)
enum class EADCombatCurrentActionConditionMode : uint8
{
	HasAnyAction,
	HasNoAction,
	MatchesExpectedTag,
	InCancelWindow,
	NotInCancelWindow
};

/**
 * 判断当前战斗动作状态。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeCondition_CombatCurrentAction : public UStateTreeConditionBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	EADCombatCurrentActionConditionMode Mode = EADCombatCurrentActionConditionMode::HasAnyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree", meta = (EditCondition = "Mode == EADCombatCurrentActionConditionMode::MatchesExpectedTag"))
	FGameplayTag ExpectedActionTag;
};

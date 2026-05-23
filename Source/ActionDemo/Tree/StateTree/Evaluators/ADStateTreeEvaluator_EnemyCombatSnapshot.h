#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "ADStateTreeEvaluator_EnemyCombatSnapshot.generated.h"

/**
 * 将 Enemy AIController 上的目标与距离信息同步到 StateTree。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeEvaluator_EnemyCombatSnapshot : public UStateTreeEvaluatorBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual void TreeStart(FStateTreeExecutionContext& Context) override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	void RefreshSnapshot(FStateTreeExecutionContext& Context);

public:
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	TObjectPtr<AActor> CombatTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	float TargetDistance = TNumericLimits<float>::Max();

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bHasCombatTarget = false;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bIsTargetInAttackRange = false;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bHasActiveAction = false;
};

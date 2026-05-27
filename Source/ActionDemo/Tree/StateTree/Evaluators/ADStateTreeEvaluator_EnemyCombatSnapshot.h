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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	TObjectPtr<AActor> CombatTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	float TargetDistance = TNumericLimits<float>::Max();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bHasCombatTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bIsTargetInAttackRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bHasActiveAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bIsHitReacting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bIsDead = false;
};

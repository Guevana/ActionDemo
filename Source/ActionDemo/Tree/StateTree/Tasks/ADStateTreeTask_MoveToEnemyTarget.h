#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "ADStateTreeTask_MoveToEnemyTarget.generated.h"

/**
 * Enemy StateTree 追击任务。
 * 以 AIController 当前 CombatTarget 为目标移动，进入攻击范围后返回成功。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeTask_MoveToEnemyTarget : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UADStateTreeTask_MoveToEnemyTarget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bUseAttackRangeAsAcceptanceRadius = true;
};

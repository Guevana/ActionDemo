#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "ADStateTreeTask_PlayMontage.generated.h"

class UAnimInstance;
class UAnimMontage;

/**
 * StateTree Montage 播放任务。
 * 在进入状态时播放指定 Montage，可等待 Montage 结束后再返回状态结果。
 */
UCLASS(Blueprintable, EditInlineNew)
class ACTIONDEMO_API UADStateTreeTask_PlayMontage : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UADStateTreeTask_PlayMontage(const FObjectInitializer& ObjectInitializer);

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	/** 需要播放的 Montage。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	TObjectPtr<UAnimMontage> Montage;

	/** Montage 播放速率。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree", meta = (ClampMin = "0.01"))
	float PlayRate = 1.0f;

	/** Montage 起始播放时间。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree", meta = (ClampMin = "0.0"))
	float StartPosition = 0.0f;

	/** 可选起始 Section。留空时从 StartPosition 播放。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	FName StartSectionName = NAME_None;

	/** 播放前是否停止当前 AnimInstance 上的其他 Montage。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bStopAllMontages = true;

	/** 是否等待 Montage 结束；关闭时进入状态成功播放后立即返回成功。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bWaitForMontageEnd = true;

	/** 等待结束时，Montage 被打断是否返回失败。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bTreatInterruptedAsFailure = true;

	/** StateTree 离开状态时是否停止仍在播放的 Montage。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	bool bStopOnExit = true;

	/** 离开状态停止 Montage 时使用的 BlendOut 时间。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree", meta = (ClampMin = "0.0"))
	float ExitBlendOutTime = 0.1f;

	UAnimInstance* ResolveAnimInstance(FStateTreeExecutionContext& Context) const;
	void HandleMontageEnded(UAnimMontage* EndedMontage, bool bInterrupted);
	void ResetRuntimeState();

	TWeakObjectPtr<UAnimInstance> ActiveAnimInstance;
	TWeakObjectPtr<UAnimMontage> ActiveMontage;
	bool bMontageStarted = false;
	bool bMontageEnded = false;
	bool bMontageInterrupted = false;
};

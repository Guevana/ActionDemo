#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ADTargetingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FADTargetChangedSignature, AActor*, OldTarget, AActor*, NewTarget);

/**
 * 锁定与目标管理组件。
 * C++ 负责候选目标筛选、评分和目标引用维护，蓝图负责镜头、UI 和表现微调。
 */
UCLASS(ClassGroup=(ActionDemo), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class ACTIONDEMO_API UADTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UADTargetingComponent();

	/** 当前锁定目标，是锁定系统的唯一事实来源。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Target")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	/** 目标变更时广播，供 UI、镜头和蓝图表现层监听。 */
	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Target")
	FADTargetChangedSignature OnTargetChanged;

	/** 最大锁定搜索半径。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Target", meta = (ClampMin = "0.0"))
	float SearchRadius = 1800.0f;

	/** 只锁定视野夹角内的目标。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Target", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxViewAngleDegrees = 75.0f;

	/** 朝向评分权重，越高越偏向视野中心。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Target", meta = (ClampMin = "0.0"))
	float DirectionScoreWeight = 0.65f;

	/** 距离评分权重，越高越偏向近目标。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Target", meta = (ClampMin = "0.0"))
	float DistanceScoreWeight = 0.35f;

	/** 是否要求锁定目标与角色之间无遮挡。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Target")
	bool bRequireLineOfSight = true;

	/** 可视性检测使用的通道。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Target")
	TEnumAsByte<ECollisionChannel> LineOfSightTraceChannel = ECC_Visibility;

	/** 搜索并锁定当前评分最高的目标。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Target")
	AActor* AcquireBestTarget();

	/** 若已有目标则解锁，否则锁定最佳目标。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Target")
	AActor* ToggleLockTarget();

	/** 在当前候选目标中按左右方向切换，Direction >= 0 表示向右，否则向左。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Target")
	AActor* SwitchTarget(int32 Direction = 1);

	/** 设置当前锁定目标。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Target")
	void SetCurrentTarget(AActor* NewTarget);

	/** 清空当前目标。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Target")
	void ClearCurrentTarget();

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Target")
	bool HasCurrentTarget() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleCurrentTargetDestroyed(AActor* DestroyedActor);

	void GatherTargetCandidates(TArray<AActor*>& OutCandidates) const;
	bool IsValidTargetCandidate(const AActor* Candidate) const;
	bool HasLineOfSightToTarget(const AActor* Candidate) const;
	float ScoreTargetCandidate(const AActor* Candidate) const;
	bool GetTargetViewData(FVector& OutOrigin, FVector& OutForward, FVector& OutRight) const;
};

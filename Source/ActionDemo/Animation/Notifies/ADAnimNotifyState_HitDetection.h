#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/Combat/ADHitDetectionComponent.h"
#include "ADAnimNotifyState_HitDetection.generated.h"

/**
 * Montage 命中检测通知状态。
 * 只负责将动画时机与可调盒体参数传给 HitDetectionComponent。
 */
UCLASS(meta = (DisplayName = "AD Hit Detection"))
class ACTIONDEMO_API UADAnimNotifyState_HitDetection : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UADAnimNotifyState_HitDetection();

	virtual FString GetNotifyName_Implementation() const override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FADHitDetectionWindowConfig HitConfig;

protected:
	UADHitDetectionComponent* FindHitDetectionComponent(USkeletalMeshComponent* MeshComp) const;
};

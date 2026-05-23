#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ADAnimNotifyState_CancelWindow.generated.h"

/**
 * 取消窗口动画通知状态。
 * 在蒙太奇进入和离开该通知状态时，同步更新 CombatComponent 与 ASC 的取消窗口状态。
 */
UCLASS(meta = (DisplayName = "AD Cancel Window"))
class ACTIONDEMO_API UADAnimNotifyState_CancelWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

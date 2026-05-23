#include "Animation/Notifies/ADAnimNotifyState_CancelWindow.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

FString UADAnimNotifyState_CancelWindow::GetNotifyName_Implementation() const
{
	return TEXT("AD Cancel Window");
}

void UADAnimNotifyState_CancelWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	AADCharacterBase* Character = Cast<AADCharacterBase>(MeshComp->GetOwner());
	if (Character != nullptr && Character->GetCombatComponent() != nullptr)
	{
		Character->GetCombatComponent()->BeginCancelWindowFromNotify();
	}
}

void UADAnimNotifyState_CancelWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	AADCharacterBase* Character = Cast<AADCharacterBase>(MeshComp->GetOwner());
	if (Character != nullptr && Character->GetCombatComponent() != nullptr)
	{
		Character->GetCombatComponent()->EndCancelWindowFromNotify();
	}
}

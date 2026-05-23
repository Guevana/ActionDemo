#include "Animation/Notifies/ADAnimNotifyState_HitDetection.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/Tags/ADGameplayTags.h"

UADAnimNotifyState_HitDetection::UADAnimNotifyState_HitDetection()
{
	HitConfig.HitEventTag = ADGameplayTags::Event_Hit_Confirm;
}

FString UADAnimNotifyState_HitDetection::GetNotifyName_Implementation() const
{
	return TEXT("AD Hit Detection");
}

void UADAnimNotifyState_HitDetection::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UADHitDetectionComponent* HitDetectionComponent = FindHitDetectionComponent(MeshComp))
	{
		HitDetectionComponent->BeginHitWindow(HitConfig);
	}
}

void UADAnimNotifyState_HitDetection::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (UADHitDetectionComponent* HitDetectionComponent = FindHitDetectionComponent(MeshComp))
	{
		HitDetectionComponent->TickHitWindow(FrameDeltaTime);
	}
}

void UADAnimNotifyState_HitDetection::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UADHitDetectionComponent* HitDetectionComponent = FindHitDetectionComponent(MeshComp))
	{
		HitDetectionComponent->EndHitWindow();
	}
}

UADHitDetectionComponent* UADAnimNotifyState_HitDetection::FindHitDetectionComponent(USkeletalMeshComponent* MeshComp) const
{
	if (MeshComp == nullptr)
	{
		return nullptr;
	}

	AADCharacterBase* Character = Cast<AADCharacterBase>(MeshComp->GetOwner());
	if (Character == nullptr)
	{
		return nullptr;
	}

	return Character->GetHitDetectionComponent();
}

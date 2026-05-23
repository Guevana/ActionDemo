#include "Components/Combat/ADHitDetectionComponent.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Target/ADTargetingComponent.h"
#include "Core/Tags/ADGameplayTags.h"
#include "DrawDebugHelpers.h"

UADHitDetectionComponent::UADHitDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ActiveConfig.HitEventTag = ADGameplayTags::Event_Hit_Confirm;
}

void UADHitDetectionComponent::BeginHitWindow(const FADHitDetectionWindowConfig& InConfig)
{
	ActiveConfig = InConfig;
	if (!ActiveConfig.HitEventTag.IsValid())
	{
		ActiveConfig.HitEventTag = ADGameplayTags::Event_Hit_Confirm;
	}

	HitActorsThisWindow.Reset();
	bHitWindowActive = true;
	bHasPreviousTraceTransform = BuildTraceTransform(PreviousTraceCenter, PreviousTraceRotation);

	UE_LOG(LogTemp, Log, TEXT("[ActionDemo] Hit window begin. EventTag=%s"), *ActiveConfig.HitEventTag.ToString());
}

void UADHitDetectionComponent::TickHitWindow(float DeltaTime)
{
	if (!bHitWindowActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (World == nullptr || OwnerActor == nullptr)
	{
		return;
	}

	FVector CurrentCenter;
	FQuat CurrentRotation;
	if (!BuildTraceTransform(CurrentCenter, CurrentRotation))
	{
		return;
	}

	const FVector Start = bHasPreviousTraceTransform ? PreviousTraceCenter : CurrentCenter;
	const FVector End = CurrentCenter;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ADHitDetectionSweep), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);

	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		CurrentRotation,
		ActiveConfig.TraceChannel.GetValue(),
		FCollisionShape::MakeBox(ActiveConfig.BoxHalfExtent),
		QueryParams);

	for (const FHitResult& HitResult : HitResults)
	{
		ProcessHitResult(HitResult);
	}

	if (ActiveConfig.bDrawDebug)
	{
		DrawDebugBox(World, CurrentCenter, ActiveConfig.BoxHalfExtent, CurrentRotation, FColor::Red, false, 0.08f);
		if (!Start.Equals(End))
		{
			DrawDebugLine(World, Start, End, FColor::Red, false, 0.08f, 0, 1.0f);
		}
	}

	PreviousTraceCenter = CurrentCenter;
	PreviousTraceRotation = CurrentRotation;
	bHasPreviousTraceTransform = true;
}

void UADHitDetectionComponent::EndHitWindow()
{
	if (!bHitWindowActive)
	{
		return;
	}

	bHitWindowActive = false;
	bHasPreviousTraceTransform = false;
	HitActorsThisWindow.Reset();

	UE_LOG(LogTemp, Log, TEXT("[ActionDemo] Hit window end."));
}

bool UADHitDetectionComponent::IsHitWindowActive() const
{
	return bHitWindowActive;
}

bool UADHitDetectionComponent::BuildTraceTransform(FVector& OutCenter, FQuat& OutRotation) const
{
	const AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return false;
	}

	const FTransform OwnerTransform = OwnerActor->GetActorTransform();
	OutCenter = OwnerTransform.TransformPosition(ActiveConfig.LocalOffset);
	OutRotation = OwnerTransform.GetRotation() * ActiveConfig.LocalRotation.Quaternion();
	return true;
}

void UADHitDetectionComponent::ProcessHitResult(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	AADCharacterBase* OwnerCharacter = Cast<AADCharacterBase>(GetOwner());
	if (OwnerCharacter == nullptr || !IsValid(HitActor) || HitActor == OwnerCharacter)
	{
		return;
	}

	AADCharacterBase* TargetCharacter = Cast<AADCharacterBase>(HitActor);
	if (TargetCharacter == nullptr)
	{
		return;
	}

	const TObjectKey<AActor> HitActorKey(HitActor);
	if (HitActorsThisWindow.Contains(HitActorKey))
	{
		return;
	}

	HitActorsThisWindow.Add(HitActorKey);

	FADCombatHitEventData HitData;
	HitData.InstigatorCharacter = OwnerCharacter;
	HitData.TargetCharacter = TargetCharacter;
	HitData.HitEventTag = ActiveConfig.HitEventTag;
	HitData.HitResult = HitResult;
	HitData.HitLocation = HitResult.ImpactPoint;
	HitData.HitNormal = HitResult.ImpactNormal;
	HitData.HitDirection = (TargetCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();

	if (const UADCombatComponent* CombatComponent = OwnerCharacter->GetCombatComponent())
	{
		HitData.SourceActionTag = CombatComponent->CurrentActionTag;
	}

	if (const UADTargetingComponent* TargetingComponent = OwnerCharacter->GetTargetingComponent())
	{
		HitData.bHitLockedTarget = TargetingComponent->CurrentTarget == TargetCharacter;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ActionDemo] Hit confirmed. Source=%s Target=%s Action=%s Event=%s"),
		*OwnerCharacter->GetName(),
		*HitActor->GetName(),
		*HitData.SourceActionTag.ToString(),
		*HitData.HitEventTag.ToString());

	OnHitConfirmed.Broadcast(HitData);

	if (UADCombatComponent* CombatComponent = OwnerCharacter->GetCombatComponent())
	{
		CombatComponent->HandleHitConfirmed(HitData);
	}
}

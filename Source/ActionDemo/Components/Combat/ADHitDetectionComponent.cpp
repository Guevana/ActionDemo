#include "Components/Combat/ADHitDetectionComponent.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Equipment/ADWeaponManagerComponent.h"
#include "Components/Target/ADTargetingComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/Tags/ADGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Equipment/ADWeaponBase.h"

namespace
{
	constexpr float MinSocketAxisLength = 1.0f;
}

UADHitDetectionComponent::UADHitDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ActiveConfig.HitEventTag = ADGameplayTags::Event_Hit_Confirm;
}

void UADHitDetectionComponent::BeginHitWindow(const FADHitDetectionWindowConfig& InConfig)
{
	ActiveConfig = InConfig;
	UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Hit window skipped: socket-driven hit detection requires BeginHitWindowFromMesh."));
	ResetHitWindowState();
}

void UADHitDetectionComponent::BeginHitWindowFromMesh(const FADHitDetectionWindowConfig& InConfig, USkeletalMeshComponent* SourceMeshComp)
{
	ResetHitWindowState();

	ActiveConfig = InConfig;
	if (!ActiveConfig.HitEventTag.IsValid())
	{
		ActiveConfig.HitEventTag = ADGameplayTags::Event_Hit_Confirm;
	}

	ActiveSourceMeshComponent = ResolveSocketSourceMesh(SourceMeshComp);
	if (ActiveSourceMeshComponent == nullptr)
	{
		return;
	}

	FVector InitialCenter;
	FQuat InitialRotation;
	FVector InitialHalfExtent;
	FVector InitialStartSocketLocation;
	FVector InitialEndSocketLocation;
	if (!BuildSocketTraceShape(InitialCenter, InitialRotation, InitialHalfExtent, InitialStartSocketLocation, InitialEndSocketLocation))
	{
		ResetHitWindowState();
		return;
	}

	PreviousTraceCenter = InitialCenter;
	bHitWindowActive = true;
	bHasPreviousTraceTransform = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ActionDemo] Hit window begin. EventTag=%s Source=%s Mesh=%s StartSocket=%s EndSocket=%s"),
		*ActiveConfig.HitEventTag.ToString(),
		ActiveConfig.SocketSource == EADHitDetectionSocketSource::EquippedWeapon ? TEXT("EquippedWeapon") : TEXT("CharacterMesh"),
		*GetNameSafe(ActiveSourceMeshComponent),
		*ActiveConfig.StartSocketName.ToString(),
		*ActiveConfig.EndSocketName.ToString());
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
	FVector CurrentHalfExtent;
	FVector CurrentStartSocketLocation;
	FVector CurrentEndSocketLocation;
	if (!BuildSocketTraceShape(CurrentCenter, CurrentRotation, CurrentHalfExtent, CurrentStartSocketLocation, CurrentEndSocketLocation))
	{
		return;
	}

	const FVector Start = bHasPreviousTraceTransform ? PreviousTraceCenter : CurrentCenter;
	const FVector End = CurrentCenter;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ADHitDetectionSweep), false, OwnerActor);
	QueryParams.AddIgnoredActor(OwnerActor);
	if (ActiveSourceMeshComponent != nullptr)
	{
		if (AActor* SourceMeshOwner = ActiveSourceMeshComponent->GetOwner())
		{
			if (SourceMeshOwner != OwnerActor)
			{
				QueryParams.AddIgnoredActor(SourceMeshOwner);
			}
		}
	}

	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		CurrentRotation,
		ActiveConfig.TraceChannel.GetValue(),
		FCollisionShape::MakeBox(CurrentHalfExtent),
		QueryParams);

	for (const FHitResult& HitResult : HitResults)
	{
		ProcessHitResult(HitResult);
	}

	if (ActiveConfig.bDrawDebug)
	{
		DrawDebugBox(World, CurrentCenter, CurrentHalfExtent, CurrentRotation, FColor::Red, false, 0.08f);
		DrawDebugLine(World, CurrentStartSocketLocation, CurrentEndSocketLocation, FColor::Yellow, false, 0.08f, 0, 1.5f);
		if (!Start.Equals(End))
		{
			DrawDebugLine(World, Start, End, FColor::Red, false, 0.08f, 0, 1.0f);
		}
	}

	PreviousTraceCenter = CurrentCenter;
	bHasPreviousTraceTransform = true;
}

void UADHitDetectionComponent::EndHitWindow()
{
	if (!bHitWindowActive)
	{
		ResetHitWindowState();
		return;
	}

	ResetHitWindowState();

	UE_LOG(LogTemp, Log, TEXT("[ActionDemo] Hit window end."));
}

bool UADHitDetectionComponent::IsHitWindowActive() const
{
	return bHitWindowActive;
}

USkeletalMeshComponent* UADHitDetectionComponent::ResolveSocketSourceMesh(USkeletalMeshComponent* NotifySourceMeshComp) const
{
	switch (ActiveConfig.SocketSource)
	{
	case EADHitDetectionSocketSource::CharacterMesh:
		if (NotifySourceMeshComp == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Hit window skipped: character mesh source is invalid."));
		}
		return NotifySourceMeshComp;

	case EADHitDetectionSocketSource::EquippedWeapon:
		{
			const AADCharacterBase* OwnerCharacter = Cast<AADCharacterBase>(GetOwner());
			if (OwnerCharacter == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Hit window skipped: weapon socket source requires AADCharacterBase owner."));
				return nullptr;
			}

			const UADWeaponManagerComponent* WeaponManager = OwnerCharacter->GetWeaponManagerComponent();
			if (WeaponManager == nullptr || !WeaponManager->HasEquippedWeapon())
			{
				UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Hit window skipped: SocketSource is EquippedWeapon but no equipped weapon is set."));
				return nullptr;
			}

			AADWeaponBase* EquippedWeapon = WeaponManager->GetEquippedWeapon();
			USkeletalMeshComponent* WeaponMeshComponent = WeaponManager->GetEquippedWeaponMeshComponent();
			if (WeaponMeshComponent == nullptr)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[ActionDemo] Hit window skipped: equipped weapon has no weapon mesh. Weapon=%s"),
					*GetNameSafe(EquippedWeapon));
			}
			return WeaponMeshComponent;
		}

	default:
		UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Hit window skipped: unknown socket source."));
		return nullptr;
	}
}

bool UADHitDetectionComponent::BuildSocketTraceShape(
	FVector& OutCenter,
	FQuat& OutRotation,
	FVector& OutHalfExtent,
	FVector& OutStartSocketLocation,
	FVector& OutEndSocketLocation) const
{
	const AActor* OwnerActor = GetOwner();
	const USkeletalMeshComponent* SourceMeshComp = ActiveSourceMeshComponent;
	if (OwnerActor == nullptr || SourceMeshComp == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Hit window skipped: source mesh is invalid."));
		return false;
	}

	if (ActiveConfig.StartSocketName.IsNone() || ActiveConfig.EndSocketName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Hit window skipped: start or end socket name is None."));
		return false;
	}

	if (!SourceMeshComp->DoesSocketExist(ActiveConfig.StartSocketName) || !SourceMeshComp->DoesSocketExist(ActiveConfig.EndSocketName))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ActionDemo] Hit window skipped: missing socket. Mesh=%s StartSocket=%s Exists=%d EndSocket=%s Exists=%d"),
			*GetNameSafe(SourceMeshComp),
			*ActiveConfig.StartSocketName.ToString(),
			SourceMeshComp->DoesSocketExist(ActiveConfig.StartSocketName),
			*ActiveConfig.EndSocketName.ToString(),
			SourceMeshComp->DoesSocketExist(ActiveConfig.EndSocketName));
		return false;
	}

	const FTransform StartSocketTransform = SourceMeshComp->GetSocketTransform(ActiveConfig.StartSocketName, RTS_World);
	const FTransform EndSocketTransform = SourceMeshComp->GetSocketTransform(ActiveConfig.EndSocketName, RTS_World);

	OutStartSocketLocation = StartSocketTransform.GetLocation();
	OutEndSocketLocation = EndSocketTransform.GetLocation();

	const FVector AxisVector = OutEndSocketLocation - OutStartSocketLocation;
	const float AxisLength = AxisVector.Size();
	if (AxisLength < MinSocketAxisLength)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ActionDemo] Hit window skipped: socket axis is too short. StartSocket=%s EndSocket=%s Length=%.2f"),
			*ActiveConfig.StartSocketName.ToString(),
			*ActiveConfig.EndSocketName.ToString(),
			AxisLength);
		return false;
	}

	const FVector AxisX = AxisVector / AxisLength;
	FVector AxisZ = StartSocketTransform.GetUnitAxis(EAxis::Z);
	AxisZ = (AxisZ - AxisX * FVector::DotProduct(AxisZ, AxisX)).GetSafeNormal();

	if (AxisZ.IsNearlyZero())
	{
		AxisZ = OwnerActor->GetActorUpVector();
		AxisZ = (AxisZ - AxisX * FVector::DotProduct(AxisZ, AxisX)).GetSafeNormal();
	}

	if (AxisZ.IsNearlyZero())
	{
		AxisZ = FVector::CrossProduct(AxisX, FVector::RightVector).GetSafeNormal();
	}

	if (AxisZ.IsNearlyZero())
	{
		AxisZ = FVector::UpVector;
	}

	OutCenter = (OutStartSocketLocation + OutEndSocketLocation) * 0.5f;
	OutRotation = FRotationMatrix::MakeFromXZ(AxisX, AxisZ).ToQuat();
	OutHalfExtent = FVector(
		AxisLength * 0.5f + FMath::Max(0.0f, ActiveConfig.SocketAxisPadding),
		FMath::Max(0.0f, ActiveConfig.CrossSectionHalfExtent.X),
		FMath::Max(0.0f, ActiveConfig.CrossSectionHalfExtent.Y));

	return true;
}

void UADHitDetectionComponent::ResetHitWindowState()
{
	bHitWindowActive = false;
	bHasPreviousTraceTransform = false;
	ActiveSourceMeshComponent = nullptr;
	HitActorsThisWindow.Reset();
	PreviousTraceCenter = FVector::ZeroVector;
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
		CombatComponent->FillHitDataFromCurrentAction(HitData);
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

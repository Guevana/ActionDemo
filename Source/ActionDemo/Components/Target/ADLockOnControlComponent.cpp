#include "Components/Target/ADLockOnControlComponent.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Target/ADTargetingComponent.h"
#include "Game/ADPlayerController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

UADLockOnControlComponent::UADLockOnControlComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UADLockOnControlComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AADCharacterBase>(GetOwner());
	TargetingComponent = OwnerCharacter != nullptr ? OwnerCharacter->GetTargetingComponent() : nullptr;

	if (TargetingComponent != nullptr)
	{
		TargetingComponent->OnTargetChanged.AddUniqueDynamic(this, &UADLockOnControlComponent::HandleTargetChanged);
		if (IsValidLockTarget(TargetingComponent->CurrentTarget))
		{
			StartLockControl();
		}
	}
}

void UADLockOnControlComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TargetingComponent != nullptr)
	{
		TargetingComponent->OnTargetChanged.RemoveDynamic(this, &UADLockOnControlComponent::HandleTargetChanged);
	}

	StopLockControl(true);

	Super::EndPlay(EndPlayReason);
}

void UADLockOnControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter == nullptr || OwnerCharacter->IsDead() || TargetingComponent == nullptr)
	{
		StopLockControl(true);
		return;
	}

	AActor* CurrentTarget = TargetingComponent->CurrentTarget;
	if (!IsValidLockTarget(CurrentTarget))
	{
		StopLockControl(true);
		return;
	}

	if (bFaceTargetWhileLocked)
	{
		UpdateFacingRotation(DeltaTime, CurrentTarget);
	}

	if (bControlCameraWhileLocked)
	{
		UpdateCameraRotation(DeltaTime, CurrentTarget);
	}
}

void UADLockOnControlComponent::HandleTargetChanged(AActor* OldTarget, AActor* NewTarget)
{
	if (IsValidLockTarget(NewTarget))
	{
		StartLockControl();
	}
	else
	{
		StopLockControl(true);
	}
}

void UADLockOnControlComponent::StartLockControl()
{
	if ((!bFaceTargetWhileLocked && !bControlCameraWhileLocked && !bUpdateControllerMovementModeWhileLocked) || OwnerCharacter == nullptr)
	{
		return;
	}

	if (bFaceTargetWhileLocked)
	{
		CacheMovementRotationSettings();
		ApplyLockMovementRotationSettings();
	}

	ApplyControllerMovementMode(TargetingComponent != nullptr ? TargetingComponent->CurrentTarget : nullptr);

	bLockControlActive = true;
	SetComponentTickEnabled(bFaceTargetWhileLocked || bControlCameraWhileLocked);
}

void UADLockOnControlComponent::StopLockControl(bool bRestoreMovementSettings)
{
	if (!bLockControlActive && !bHasCachedMovementRotationSettings)
	{
		ClearControllerMovementMode();
		SetComponentTickEnabled(false);
		return;
	}

	bLockControlActive = false;
	SetComponentTickEnabled(false);

	if (bRestoreMovementSettings)
	{
		RestoreMovementRotationSettings();
	}

	ClearControllerMovementMode();
}

void UADLockOnControlComponent::CacheMovementRotationSettings()
{
	if (bHasCachedMovementRotationSettings || !bFaceTargetWhileLocked || !bOverrideMovementRotation || OwnerCharacter == nullptr)
	{
		return;
	}

	if (const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		bCachedOrientRotationToMovement = MovementComponent->bOrientRotationToMovement;
		bCachedUseControllerDesiredRotation = MovementComponent->bUseControllerDesiredRotation;
		bHasCachedMovementRotationSettings = true;
	}
}

void UADLockOnControlComponent::ApplyLockMovementRotationSettings() const
{
	if (!bFaceTargetWhileLocked || !bOverrideMovementRotation || OwnerCharacter == nullptr)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = false;
		MovementComponent->bUseControllerDesiredRotation = false;
	}
}

void UADLockOnControlComponent::RestoreMovementRotationSettings()
{
	if (!bHasCachedMovementRotationSettings || !bOverrideMovementRotation || OwnerCharacter == nullptr)
	{
		bHasCachedMovementRotationSettings = false;
		return;
	}

	if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = bCachedOrientRotationToMovement;
		MovementComponent->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
	}

	bHasCachedMovementRotationSettings = false;
}

void UADLockOnControlComponent::UpdateFacingRotation(float DeltaTime, const AActor* CurrentTarget) const
{
	if (OwnerCharacter == nullptr || CurrentTarget == nullptr)
	{
		return;
	}

	FVector ToTarget = CurrentTarget->GetActorLocation() - OwnerCharacter->GetActorLocation();
	ToTarget.Z = 0.0f;

	if (ToTarget.SizeSquared() <= FMath::Square(MinRotationDistance))
	{
		return;
	}

	const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
	const FRotator DesiredYawRotation(0.0f, ToTarget.Rotation().Yaw, 0.0f);
	const FRotator CurrentYawRotation(0.0f, CurrentRotation.Yaw, 0.0f);
	const FRotator InterpolatedYawRotation = FMath::RInterpTo(CurrentYawRotation, DesiredYawRotation, DeltaTime, RotationInterpSpeed);

	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw = InterpolatedYawRotation.Yaw;
	OwnerCharacter->SetActorRotation(NewRotation);
}

void UADLockOnControlComponent::UpdateCameraRotation(float DeltaTime, const AActor* CurrentTarget) const
{
	if (OwnerCharacter == nullptr || CurrentTarget == nullptr)
	{
		return;
	}

	AController* Controller = OwnerCharacter->GetController();
	if (Controller == nullptr)
	{
		return;
	}

	FVector ViewOrigin;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewOrigin, ViewRotation);

	const FVector FocusLocation = CurrentTarget->GetActorLocation() + FVector(0.0f, 0.0f, TargetFocusHeight);
	const FVector ToFocus = FocusLocation - ViewOrigin;
	if (ToFocus.SizeSquared() <= FMath::Square(MinCameraFocusDistance))
	{
		return;
	}

	FRotator DesiredControlRotation = ToFocus.Rotation();
	DesiredControlRotation.Pitch = FMath::Clamp(
		FRotator::NormalizeAxis(DesiredControlRotation.Pitch),
		FMath::Min(MinCameraPitchAngle, MaxCameraPitchAngle),
		FMath::Max(MinCameraPitchAngle, MaxCameraPitchAngle));
	DesiredControlRotation.Roll = 0.0f;

	const FRotator CurrentControlRotation = Controller->GetControlRotation();
	const FRotator InterpolatedYawRotation = FMath::RInterpTo(
		FRotator(0.0f, CurrentControlRotation.Yaw, 0.0f),
		FRotator(0.0f, DesiredControlRotation.Yaw, 0.0f),
		DeltaTime,
		CameraYawInterpSpeed);

	FRotator NewControlRotation = CurrentControlRotation;
	NewControlRotation.Yaw = InterpolatedYawRotation.Yaw;
	NewControlRotation.Pitch = FMath::FInterpTo(
		FRotator::NormalizeAxis(CurrentControlRotation.Pitch),
		DesiredControlRotation.Pitch,
		DeltaTime,
		CameraPitchInterpSpeed);
	NewControlRotation.Roll = 0.0f;

	Controller->SetControlRotation(NewControlRotation);
}

void UADLockOnControlComponent::ApplyControllerMovementMode(AActor* CurrentTarget) const
{
	if (!bUpdateControllerMovementModeWhileLocked || !IsValidLockTarget(CurrentTarget) || OwnerCharacter == nullptr)
	{
		return;
	}

	AADPlayerController* PlayerController = Cast<AADPlayerController>(OwnerCharacter->GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	PlayerController->SetLockOnMovementMode(
		CurrentTarget,
		LockOnMoveMinTargetDistance,
		bAllowLockOnForwardMoveAtCloseRange);
}

void UADLockOnControlComponent::ClearControllerMovementMode() const
{
	if (!bUpdateControllerMovementModeWhileLocked || OwnerCharacter == nullptr)
	{
		return;
	}

	AADPlayerController* PlayerController = Cast<AADPlayerController>(OwnerCharacter->GetController());
	if (PlayerController != nullptr)
	{
		PlayerController->SetFreeMovementMode();
	}
}

bool UADLockOnControlComponent::IsValidLockTarget(const AActor* Target) const
{
	if (!IsValid(Target) || Target == OwnerCharacter)
	{
		return false;
	}

	const AADCharacterBase* TargetCharacter = Cast<AADCharacterBase>(Target);
	return TargetCharacter != nullptr && !TargetCharacter->IsDead();
}

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ADLockOnControlComponent.generated.h"

class AADCharacterBase;
class UADTargetingComponent;

/**
 * Consumes TargetingComponent target facts and drives lock-on character control.
 * This component owns presentation/control behavior only; target selection stays in TargetingComponent.
 */
UCLASS(ClassGroup=(ActionDemo), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class ACTIONDEMO_API UADLockOnControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UADLockOnControlComponent();

	/** Whether locked characters should rotate toward CurrentTarget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn")
	bool bFaceTargetWhileLocked = true;

	/** Whether lock-on temporarily overrides CharacterMovement rotation settings. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn")
	bool bOverrideMovementRotation = true;

	/** Actor yaw interpolation speed while facing the locked target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn", meta = (ClampMin = "0.0"))
	float RotationInterpSpeed = 12.0f;

	/** Skip facing updates when owner and target are too close on the horizontal plane. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn", meta = (ClampMin = "0.0"))
	float MinRotationDistance = 50.0f;

	/** Whether lock-on should smoothly steer the camera toward CurrentTarget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Camera")
	bool bControlCameraWhileLocked = true;

	/** ControlRotation yaw interpolation speed while locked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Camera", meta = (ClampMin = "0.0"))
	float CameraYawInterpSpeed = 8.0f;

	/** ControlRotation pitch interpolation speed while locked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Camera", meta = (ClampMin = "0.0"))
	float CameraPitchInterpSpeed = 6.0f;

	/** Vertical offset above target actor location used as the camera focus point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Camera")
	float TargetFocusHeight = 80.0f;

	/** Lowest camera pitch allowed while lock-on camera control is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Camera", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MinCameraPitchAngle = -45.0f;

	/** Highest camera pitch allowed while lock-on camera control is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Camera", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MaxCameraPitchAngle = 35.0f;

	/** Skip camera focus updates when the camera viewpoint is too close to the focus point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Camera", meta = (ClampMin = "0.0"))
	float MinCameraFocusDistance = 100.0f;

	/** Whether lock-on should switch the owning player controller into lock-on movement mode. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Movement")
	bool bUpdateControllerMovementModeWhileLocked = true;

	/** Distance used by the controller to suppress forward/back lock-on movement at close range. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Movement", meta = (ClampMin = "0.0"))
	float LockOnMoveMinTargetDistance = 80.0f;

	/** Whether the controller should still allow forward/back lock-on movement inside the minimum distance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|LockOn|Movement")
	bool bAllowLockOnForwardMoveAtCloseRange = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void HandleTargetChanged(AActor* OldTarget, AActor* NewTarget);

	void StartLockControl();
	void StopLockControl(bool bRestoreMovementSettings = true);
	void CacheMovementRotationSettings();
	void ApplyLockMovementRotationSettings() const;
	void RestoreMovementRotationSettings();
	void UpdateFacingRotation(float DeltaTime, const AActor* CurrentTarget) const;
	void UpdateCameraRotation(float DeltaTime, const AActor* CurrentTarget) const;
	void ApplyControllerMovementMode(AActor* CurrentTarget) const;
	void ClearControllerMovementMode() const;
	bool IsValidLockTarget(const AActor* Target) const;

	UPROPERTY(Transient)
	TObjectPtr<AADCharacterBase> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UADTargetingComponent> TargetingComponent;

	bool bLockControlActive = false;
	bool bHasCachedMovementRotationSettings = false;
	bool bCachedOrientRotationToMovement = false;
	bool bCachedUseControllerDesiredRotation = false;
};

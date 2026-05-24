#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Combat/ADCombatHitTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/ObjectKey.h"
#include "ADHitDetectionComponent.generated.h"

class AADCharacterBase;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EADHitDetectionSocketSource : uint8
{
	CharacterMesh UMETA(DisplayName = "Character Mesh"),
	EquippedWeapon UMETA(DisplayName = "Equipped Weapon")
};

USTRUCT(BlueprintType)
struct FADHitDetectionWindowConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	EADHitDetectionSocketSource SocketSource = EADHitDetectionSocketSource::CharacterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FName StartSocketName = TEXT("Hit_Start");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FName EndSocketName = TEXT("Hit_End");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit", meta = (ClampMin = "0.0"))
	float SocketAxisPadding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FVector2D CrossSectionHalfExtent = FVector2D(40.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FGameplayTag HitEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	bool bDrawDebug = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FADHitConfirmedSignature, const FADCombatHitEventData&, HitData);

/**
 * 近战命中检测组件。
 * 动画通知状态只提供时机和盒体参数，真实 Sweep、去重和事件分发集中在这里。
 */
UCLASS(ClassGroup=(ActionDemo), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class ACTIONDEMO_API UADHitDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UADHitDetectionComponent();

	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Hit")
	FADHitConfirmedSignature OnHitConfirmed;

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Hit")
	void BeginHitWindow(const FADHitDetectionWindowConfig& InConfig);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Hit")
	void BeginHitWindowFromMesh(const FADHitDetectionWindowConfig& InConfig, USkeletalMeshComponent* SourceMeshComp);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Hit")
	void TickHitWindow(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Hit")
	void EndHitWindow();

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Hit")
	bool IsHitWindowActive() const;

protected:
	USkeletalMeshComponent* ResolveSocketSourceMesh(USkeletalMeshComponent* NotifySourceMeshComp) const;
	bool BuildSocketTraceShape(FVector& OutCenter, FQuat& OutRotation, FVector& OutHalfExtent, FVector& OutStartSocketLocation, FVector& OutEndSocketLocation) const;
	void ProcessHitResult(const FHitResult& HitResult);
	void ResetHitWindowState();

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	bool bHitWindowActive = false;

	FADHitDetectionWindowConfig ActiveConfig;
	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> ActiveSourceMeshComponent = nullptr;

	TSet<TObjectKey<AActor>> HitActorsThisWindow;
	FVector PreviousTraceCenter = FVector::ZeroVector;
	bool bHasPreviousTraceTransform = false;
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Combat/ADCombatHitTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/ObjectKey.h"
#include "ADHitDetectionComponent.generated.h"

class AADCharacterBase;

USTRUCT(BlueprintType)
struct FADHitDetectionWindowConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FVector BoxHalfExtent = FVector(60.0f, 40.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FVector LocalOffset = FVector(120.0f, 0.0f, 40.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionDemo|Hit")
	FRotator LocalRotation = FRotator::ZeroRotator;

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
	void TickHitWindow(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Hit")
	void EndHitWindow();

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Hit")
	bool IsHitWindowActive() const;

protected:
	bool BuildTraceTransform(FVector& OutCenter, FQuat& OutRotation) const;
	void ProcessHitResult(const FHitResult& HitResult);

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	bool bHitWindowActive = false;

	FADHitDetectionWindowConfig ActiveConfig;
	TSet<TObjectKey<AActor>> HitActorsThisWindow;
	FVector PreviousTraceCenter = FVector::ZeroVector;
	FQuat PreviousTraceRotation = FQuat::Identity;
	bool bHasPreviousTraceTransform = false;
};

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ADCombatHitTypes.generated.h"

class AADCharacterBase;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FADCombatHitEventData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	TObjectPtr<AADCharacterBase> InstigatorCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	TObjectPtr<AADCharacterBase> TargetCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	FGameplayTag SourceActionTag;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	FGameplayTag HitEventTag;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	FHitResult HitResult;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	FVector HitDirection = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	bool bHitLockedTarget = false;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Hit")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};

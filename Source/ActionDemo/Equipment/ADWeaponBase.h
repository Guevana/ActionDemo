#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADWeaponBase.generated.h"

class AADCharacterBase;
class USkeletalMeshComponent;

/**
 * 武器基类。
 * 第一版只提供武器 Mesh 和持有者引用，供命中检测从武器插槽读取扫掠范围。
 */
UCLASS(BlueprintType, Blueprintable)
class ACTIONDEMO_API AADWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AADWeaponBase();

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	USkeletalMeshComponent* GetWeaponMeshComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	AADCharacterBase* GetOwningCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	void SetOwningCharacter(AADCharacterBase* InOwningCharacter);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Weapon")
	TObjectPtr<AADCharacterBase> OwningCharacter = nullptr;
};

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ADWeaponManagerComponent.generated.h"

class AADCharacterBase;
class AADWeaponBase;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FADEquippedWeaponChangedSignature, AADWeaponBase*, OldWeapon, AADWeaponBase*, NewWeapon);

/**
 * 武器管理组件。
 * 维护角色当前装备武器，并提供统一的武器查询与切换入口。
 */
UCLASS(ClassGroup=(ActionDemo), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class ACTIONDEMO_API UADWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UADWeaponManagerComponent();

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	AADCharacterBase* GetOwningCharacter() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	AADWeaponBase* GetEquippedWeapon() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	USkeletalMeshComponent* GetEquippedWeaponMeshComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	bool HasEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	void SetEquippedWeapon(AADWeaponBase* InWeapon);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	void ClearEquippedWeapon();

	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Weapon")
	FADEquippedWeaponChangedSignature OnEquippedWeaponChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 当前装备武器；武器插槽命中检测会从该 Actor 的 Mesh 读取插槽。 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	TObjectPtr<AADWeaponBase> EquippedWeapon = nullptr;

	void BindWeaponOwner(AADWeaponBase* Weapon) const;
	void ClearWeaponOwner(AADWeaponBase* Weapon) const;
};

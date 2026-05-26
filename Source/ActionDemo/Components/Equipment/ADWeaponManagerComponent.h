#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ADWeaponManagerComponent.generated.h"

class AADCharacterBase;
class AADWeaponBase;
class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct FADWeaponSpawnConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	TSubclassOf<AADWeaponBase> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	FName AttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	FTransform AttachRelativeTransform = FTransform::Identity;
};

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

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	int32 GetEquippedWeaponIndex() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	int32 GetWeaponInstanceCount() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Weapon")
	AADWeaponBase* GetWeaponInstanceAtIndex(int32 WeaponIndex) const;

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	bool SpawnConfiguredWeapons();

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	AADWeaponBase* SpawnWeaponFromConfig(const FADWeaponSpawnConfig& WeaponConfig);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	void SetEquippedWeapon(AADWeaponBase* InWeapon);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	void ClearEquippedWeapon();

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	bool EquipWeaponByIndex(int32 WeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	bool EquipNextWeapon();

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Weapon")
	bool EquipPreviousWeapon();

	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Weapon")
	FADEquippedWeaponChangedSignature OnEquippedWeaponChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	TArray<FADWeaponSpawnConfig> WeaponSpawnConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	bool bSpawnWeaponsOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon", meta = (ClampMin = "0"))
	int32 InitialWeaponIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	FName DefaultAttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	FTransform DefaultAttachRelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	bool bHideUnequippedWeapons = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	bool bEnableEquippedWeaponCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	bool bDestroySpawnedWeaponsOnEndPlay = true;

	/** 当前装备武器；武器插槽命中检测会从该 Actor 的 Mesh 读取插槽。 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	TObjectPtr<AADWeaponBase> EquippedWeapon = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "ActionDemo|Weapon")
	TArray<TObjectPtr<AADWeaponBase>> WeaponInstances;

	UPROPERTY(Transient)
	TArray<FADWeaponSpawnConfig> WeaponInstanceConfigs;

	UPROPERTY(Transient)
	TArray<bool> bGeneratedWeaponInstances;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Weapon")
	int32 EquippedWeaponIndex = INDEX_NONE;

	UPROPERTY(Transient)
	bool bConfiguredWeaponsSpawned = false;

	void BindWeaponOwner(AADWeaponBase* Weapon) const;
	void ClearWeaponOwner(AADWeaponBase* Weapon) const;
	void RegisterWeaponInstance(AADWeaponBase* Weapon, const FADWeaponSpawnConfig& WeaponConfig, bool bGeneratedByManager);
	bool IsManagedWeapon(const AADWeaponBase* Weapon) const;
	int32 FindWeaponIndex(const AADWeaponBase* Weapon) const;
	bool EquipWeaponByDirection(int32 Direction);
	void AttachWeaponToCharacter(AADWeaponBase* Weapon, const FADWeaponSpawnConfig& WeaponConfig) const;
	void RefreshWeaponActivationStates() const;
	void SetWeaponActive(AADWeaponBase* Weapon, bool bActive) const;
	FName ResolveAttachSocketName(const FADWeaponSpawnConfig& WeaponConfig) const;
	FTransform ResolveAttachRelativeTransform(const FADWeaponSpawnConfig& WeaponConfig) const;
};

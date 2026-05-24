#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ADCharacterBase.generated.h"

class UADAbilityQueueComponent;
class UADAbilitySystemComponent;
class UADAttributeSet;
class UADCombatComponent;
class UADHitDetectionComponent;
class UADTargetingComponent;
class UADWeaponManagerComponent;
class AADWeaponBase;
class AADCharacterBase;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FADCharacterDeathSignature, AADCharacterBase*, DeadCharacter);

/**
 * 所有战斗实体的统一角色基类。
 * 该类只负责组装战斗相关组件，并暴露统一访问接口。
 */
UCLASS()
class ACTIONDEMO_API AADCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AADCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	UADAbilitySystemComponent* GetADAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	UADCombatComponent* GetCombatComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	UADAbilityQueueComponent* GetAbilityQueueComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	UADTargetingComponent* GetTargetingComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	UADHitDetectionComponent* GetHitDetectionComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	UADWeaponManagerComponent* GetWeaponManagerComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	UADAttributeSet* GetADAttributeSet() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Equipment")
	AADWeaponBase* GetEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Equipment")
	void SetEquippedWeapon(AADWeaponBase* InWeapon);

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Character")
	bool IsDead() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ActionDemo|Character")
	void HandleDeath();
	virtual void HandleDeath_Implementation();

	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Character")
	FADCharacterDeathSignature OnCharacterDeath;

protected:
	virtual void BeginPlay() override;

	/** 角色的 GAS 组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Components")
	TObjectPtr<UADAbilitySystemComponent> AbilitySystemComponent;

	/** 角色基础属性集。 */
	UPROPERTY()
	TObjectPtr<UADAttributeSet> AttributeSet;

	/** 战斗主控组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Components")
	TObjectPtr<UADCombatComponent> CombatComponent;

	/** 输入缓冲组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Components")
	TObjectPtr<UADAbilityQueueComponent> AbilityQueueComponent;

	/** 锁敌组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Components")
	TObjectPtr<UADTargetingComponent> TargetingComponent;

	/** 命中检测组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Components")
	TObjectPtr<UADHitDetectionComponent> HitDetectionComponent;

	/** 武器管理组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Components")
	TObjectPtr<UADWeaponManagerComponent> WeaponManagerComponent;

	/** 初始化 GAS ActorInfo。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Character")
	virtual void InitializeAbilityActorInfo();

	void BindAttributeDelegates();

	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Character")
	bool bIsDead = false;

	bool bAttributeDelegatesBound = false;
};

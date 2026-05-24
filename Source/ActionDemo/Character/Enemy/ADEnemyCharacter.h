#pragma once

#include "CoreMinimal.h"
#include "Character/Base/ADCharacterBase.h"
#include "Components/Combat/ADCombatHitReceiverInterface.h"
#include "ADEnemyCharacter.generated.h"

class UADEnemyConfigData;
class UADGameplayAbility;
class AADEnemyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FADEnemyHitReceivedSignature, const FADCombatHitEventData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FADEnemyDeathSignature, AADEnemyCharacter*, DeadEnemy);

/**
 * 敌人角色基类。
 * 保持轻量：负责敌人配置、启动 Ability 授予和受击事件入口。
 */
UCLASS()
class ACTIONDEMO_API AADEnemyCharacter : public AADCharacterBase, public IADCombatHitReceiverInterface
{
	GENERATED_BODY()

public:
	AADEnemyCharacter();

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Enemy")
	UADEnemyConfigData* GetEnemyConfig() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Enemy")
	AADCharacterBase* GetLastHitInstigator() const;

	virtual void ReceiveCombatHit_Implementation(const FADCombatHitEventData& HitData) override;
	virtual void HandleDeath_Implementation() override;

	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Enemy")
	FADEnemyHitReceivedSignature OnEnemyHitReceived;

	UPROPERTY(BlueprintAssignable, Category = "ActionDemo|Enemy")
	FADEnemyDeathSignature OnEnemyDeath;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	void GrantStartupAbilitiesFromConfig();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Config")
	TObjectPtr<UADEnemyConfigData> EnemyConfig;

	/** 敌人默认受击 Ability；确保未配置资产时也能消费命中伤害事件。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Ability")
	TSubclassOf<UADGameplayAbility> DefaultHitReactionAbilityClass;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Enemy")
	TObjectPtr<AADCharacterBase> LastHitInstigator = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Enemy")
	FADCombatHitEventData LastReceivedHit;

	bool bStartupAbilitiesGranted = false;
};

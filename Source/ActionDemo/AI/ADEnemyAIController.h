#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ADEnemyAIController.generated.h"

class AADEnemyCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UStateTreeComponent;
struct FAIStimulus;

/**
 * 近战敌人的首版 AI 控制器。
 * Controller 持有感知、目标事实和宏观 StateTree，EnemyCharacter 仅负责实体装配。
 */
UCLASS()
class ACTIONDEMO_API AADEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AADEnemyAIController();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	UStateTreeComponent* GetCombatStateTreeComponent() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	AActor* GetCombatTarget() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	bool HasCombatTarget() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	float GetTargetDistance() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	bool IsTargetInAttackRange() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	bool IsControlledEnemyHitReacting() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	bool IsControlledEnemyDead() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	float GetAttackRange() const;

	UFUNCTION(BlueprintPure, Category = "ActionDemo|AI")
	float GetLoseTargetDistance() const;

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|AI")
	void SetCombatTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|AI")
	void ClearCombatTarget();

protected:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void HandleCombatTargetDestroyed(AActor* DestroyedActor);

	bool IsValidCombatTarget(const AActor* Actor) const;
	AADEnemyCharacter* GetControlledEnemy() const;
	void ConfigureFromEnemyConfig();
	void SyncCombatTargetToPawn() const;
	void ValidateCombatTarget();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	TObjectPtr<UStateTreeComponent> CombatStateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|AI")
	TObjectPtr<AActor> CombatTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|AI")
	float AttackRange = 220.0f;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|AI")
	float LoseTargetDistance = 2400.0f;
};

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "ADGA_Target_Lock.generated.h"

class UADTargetingComponent;

/**
 * 锁定目标 Ability。
 * 输入只负责按标签激活该 Ability，实际锁定/解锁仍由 TargetingComponent 维护唯一目标事实。
 */
UCLASS(Blueprintable)
class ACTIONDEMO_API UADGA_Target_Lock : public UADGameplayAbility
{
	GENERATED_BODY()

public:
	UADGA_Target_Lock();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UFUNCTION()
	void HandleTargetChanged(AActor* OldTarget, AActor* NewTarget);

	UADTargetingComponent* ResolveTargetingComponent() const;

	UPROPERTY(Transient)
	TObjectPtr<UADTargetingComponent> ActiveTargetingComponent;
};

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ADDamageExecutionCalculation.generated.h"

/**
 * 默认伤害执行计算。
 * 来源攻击力和目标防御在此处进入公式。
 */
UCLASS()
class ACTIONDEMO_API UADDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UADDamageExecutionCalculation();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

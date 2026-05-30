#include "AbilitySystem/Effects/ADGameplayEffect_Damage.h"

#include "AbilitySystem/Effects/ADDamageExecutionCalculation.h"

UADGameplayEffect_Damage::UADGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition DamageExecution;
	DamageExecution.CalculationClass = UADDamageExecutionCalculation::StaticClass();
	Executions.Add(DamageExecution);
}

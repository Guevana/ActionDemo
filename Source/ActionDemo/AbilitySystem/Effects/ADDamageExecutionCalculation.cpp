#include "AbilitySystem/Effects/ADDamageExecutionCalculation.h"

#include "AbilitySystem/Attributes/ADAttributeSet.h"
#include "Core/Tags/ADGameplayTags.h"

struct FADDamageExecutionStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FADDamageExecutionStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UADAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UADAttributeSet, Defense, Target, false);
	}
};

static const FADDamageExecutionStatics& ADDamageExecutionStatics()
{
	static FADDamageExecutionStatics Statics;
	return Statics;
}

UADDamageExecutionCalculation::UADDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(ADDamageExecutionStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(ADDamageExecutionStatics().DefenseDef);
}

void UADDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackPower = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		ADDamageExecutionStatics().AttackPowerDef,
		EvaluationParameters,
		AttackPower);

	float Defense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		ADDamageExecutionStatics().DefenseDef,
		EvaluationParameters,
		Defense);

	const float ClampedAttackPower = FMath::Max(0.0f, AttackPower);
	if (ClampedAttackPower <= 0.0f)
	{
		return;
	}

	const float DamageScale = FMath::Max(
		0.0f,
		Spec.GetSetByCallerMagnitude(ADGameplayTags::Data_DamageScale, false, 1.0f));
	if (DamageScale <= 0.0f)
	{
		return;
	}

	const float ScaledAttackPower = ClampedAttackPower * DamageScale;
	const float FinalDamage = FMath::Max(1.0f, ScaledAttackPower - FMath::Max(0.0f, Defense));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UADAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-FinalDamage));
}

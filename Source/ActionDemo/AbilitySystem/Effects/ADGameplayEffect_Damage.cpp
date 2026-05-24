#include "AbilitySystem/Effects/ADGameplayEffect_Damage.h"

#include "AbilitySystem/Attributes/ADAttributeSet.h"
#include "Core/Tags/ADGameplayTags.h"

UADGameplayEffect_Damage::UADGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat DamageMagnitude;
	DamageMagnitude.DataTag = ADGameplayTags::Data_Damage;

	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UADAttributeSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageMagnitude);

	Modifiers.Add(HealthModifier);
}

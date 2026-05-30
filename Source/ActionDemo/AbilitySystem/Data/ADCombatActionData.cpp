#include "AbilitySystem/Data/ADCombatActionData.h"

#include "AbilitySystem/Effects/ADGameplayEffect_Damage.h"

UADCombatActionData::UADCombatActionData()
{
	DamageEffectClass = UADGameplayEffect_Damage::StaticClass();
}

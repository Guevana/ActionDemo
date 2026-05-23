#include "AbilitySystem/ADAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "AbilitySystem/Abilities/ADGameplayAbility_AttackBase.h"
#include "GameplayAbilitySpec.h"

bool UADAbilitySystemComponent::TryActivateAbilityByInputTag(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		const UADGameplayAbility* AbilityCDO = Cast<UADGameplayAbility>(AbilitySpec.Ability);
		if (AbilityCDO == nullptr || !AbilityCDO->InputTag.MatchesTagExact(InputTag))
		{
			continue;
		}

		if (AbilitySpec.IsActive() && AbilityCDO->bCancelAbilityOnRepeatedInput)
		{
			CancelAbilityHandle(AbilitySpec.Handle);
			return true;
		}

		if (TryActivateAbility(AbilitySpec.Handle))
		{
			return true;
		}
	}

	return false;
}

bool UADAbilitySystemComponent::TryActivateGrantedAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		return false;
	}

	if (AbilityClass->IsChildOf(UADGameplayAbility_AttackBase::StaticClass()))
	{
		CancelActiveAttackAbilities();
	}

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability == nullptr || AbilitySpec.Ability->GetClass() != AbilityClass)
		{
			continue;
		}

		if (TryActivateAbility(AbilitySpec.Handle))
		{
			return true;
		}
	}

	return false;
}

bool UADAbilitySystemComponent::CancelActiveAttackAbilities()
{
	TArray<FGameplayAbilitySpecHandle> AttackAbilityHandlesToCancel;

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive() || AbilitySpec.Ability == nullptr || !AbilitySpec.Ability->IsA(UADGameplayAbility_AttackBase::StaticClass()))
		{
			continue;
		}

		AttackAbilityHandlesToCancel.Add(AbilitySpec.Handle);
	}

	for (const FGameplayAbilitySpecHandle& AbilityHandle : AttackAbilityHandlesToCancel)
	{
		CancelAbilityHandle(AbilityHandle);
	}

	return !AttackAbilityHandlesToCancel.IsEmpty();
}

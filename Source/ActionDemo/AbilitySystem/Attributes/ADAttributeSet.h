#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ADAttributeSet.generated.h"

#define AD_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 基础属性集。
 * 当前先提供生命与体力骨架，后续可继续扩展攻击、韧性、能量等属性。
 */
UCLASS()
class ACTIONDEMO_API UADAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UADAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Attributes")
	FGameplayAttributeData Health;

	AD_ATTRIBUTE_ACCESSORS(UADAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Attributes")
	FGameplayAttributeData MaxHealth;

	AD_ATTRIBUTE_ACCESSORS(UADAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Attributes")
	FGameplayAttributeData Stamina;

	AD_ATTRIBUTE_ACCESSORS(UADAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Attributes")
	FGameplayAttributeData MaxStamina;

	AD_ATTRIBUTE_ACCESSORS(UADAttributeSet, MaxStamina)
};

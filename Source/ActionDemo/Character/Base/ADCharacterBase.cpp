#include "Character/Base/ADCharacterBase.h"

#include "AbilitySystem/ADAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ADAttributeSet.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Combat/ADHitDetectionComponent.h"
#include "Components/Input/ADAbilityQueueComponent.h"
#include "Components/Target/ADTargetingComponent.h"

AADCharacterBase::AADCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UADAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CombatComponent = CreateDefaultSubobject<UADCombatComponent>(TEXT("CombatComponent"));
	AbilityQueueComponent = CreateDefaultSubobject<UADAbilityQueueComponent>(TEXT("AbilityQueueComponent"));
	TargetingComponent = CreateDefaultSubobject<UADTargetingComponent>(TEXT("TargetingComponent"));
	HitDetectionComponent = CreateDefaultSubobject<UADHitDetectionComponent>(TEXT("HitDetectionComponent"));

	AttributeSet = CreateDefaultSubobject<UADAttributeSet>(TEXT("AttributeSet"));
}

void AADCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilityActorInfo();
}

UAbilitySystemComponent* AADCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UADAbilitySystemComponent* AADCharacterBase::GetADAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UADCombatComponent* AADCharacterBase::GetCombatComponent() const
{
	return CombatComponent;
}

UADAbilityQueueComponent* AADCharacterBase::GetAbilityQueueComponent() const
{
	return AbilityQueueComponent;
}

UADTargetingComponent* AADCharacterBase::GetTargetingComponent() const
{
	return TargetingComponent;
}

UADHitDetectionComponent* AADCharacterBase::GetHitDetectionComponent() const
{
	return HitDetectionComponent;
}

void AADCharacterBase::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

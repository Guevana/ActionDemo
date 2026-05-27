#include "Character/Base/ADCharacterBase.h"

#include "AbilitySystem/ADAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ADAttributeSet.h"
#include "Core/Tags/ADGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Combat/ADHitDetectionComponent.h"
#include "Components/Equipment/ADWeaponManagerComponent.h"
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
	WeaponManagerComponent = CreateDefaultSubobject<UADWeaponManagerComponent>(TEXT("WeaponManagerComponent"));

	AttributeSet = CreateDefaultSubobject<UADAttributeSet>(TEXT("AttributeSet"));
}

void AADCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilityActorInfo();
	BindAttributeDelegates();
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

UADWeaponManagerComponent* AADCharacterBase::GetWeaponManagerComponent() const
{
	return WeaponManagerComponent;
}

UADAttributeSet* AADCharacterBase::GetADAttributeSet() const
{
	return AttributeSet;
}

AADWeaponBase* AADCharacterBase::GetEquippedWeapon() const
{
	return WeaponManagerComponent != nullptr ? WeaponManagerComponent->GetEquippedWeapon() : nullptr;
}

void AADCharacterBase::SetEquippedWeapon(AADWeaponBase* InWeapon)
{
	if (WeaponManagerComponent != nullptr)
	{
		WeaponManagerComponent->SetEquippedWeapon(InWeapon);
	}
}

bool AADCharacterBase::IsDead() const
{
	return bIsDead;
}

bool AADCharacterBase::IsHitReacting() const
{
	return AbilitySystemComponent != nullptr && AbilitySystemComponent->HasMatchingGameplayTag(ADGameplayTags::State_Hit_React);
}

void AADCharacterBase::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AADCharacterBase::BindAttributeDelegates()
{
	if (bAttributeDelegatesBound || AbilitySystemComponent == nullptr || AttributeSet == nullptr)
	{
		return;
	}

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UADAttributeSet::GetHealthAttribute())
		.AddUObject(this, &AADCharacterBase::HandleHealthChanged);
	bAttributeDelegatesBound = true;
}

void AADCharacterBase::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	if (!bIsDead && ChangeData.NewValue <= 0.0f)
	{
		HandleDeath();
	}
}

void AADCharacterBase::HandleDeath_Implementation()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->CancelAllAbilities();
		AbilitySystemComponent->AddLooseGameplayTag(ADGameplayTags::State_Dead);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	if (AController* OwnerController = GetController())
	{
		OwnerController->StopMovement();
	}

	OnCharacterDeath.Broadcast(this);
}

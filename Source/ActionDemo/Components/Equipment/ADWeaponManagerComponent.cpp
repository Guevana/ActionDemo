#include "Components/Equipment/ADWeaponManagerComponent.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Equipment/ADWeaponBase.h"

UADWeaponManagerComponent::UADWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UADWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BindWeaponOwner(EquippedWeapon);
}

void UADWeaponManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearWeaponOwner(EquippedWeapon);

	Super::EndPlay(EndPlayReason);
}

AADCharacterBase* UADWeaponManagerComponent::GetOwningCharacter() const
{
	return Cast<AADCharacterBase>(GetOwner());
}

AADWeaponBase* UADWeaponManagerComponent::GetEquippedWeapon() const
{
	return EquippedWeapon;
}

USkeletalMeshComponent* UADWeaponManagerComponent::GetEquippedWeaponMeshComponent() const
{
	return IsValid(EquippedWeapon) ? EquippedWeapon->GetWeaponMeshComponent() : nullptr;
}

bool UADWeaponManagerComponent::HasEquippedWeapon() const
{
	return IsValid(EquippedWeapon);
}

void UADWeaponManagerComponent::SetEquippedWeapon(AADWeaponBase* InWeapon)
{
	if (EquippedWeapon == InWeapon)
	{
		BindWeaponOwner(EquippedWeapon);
		return;
	}

	AADWeaponBase* OldWeapon = EquippedWeapon;
	ClearWeaponOwner(OldWeapon);

	EquippedWeapon = InWeapon;
	BindWeaponOwner(EquippedWeapon);

	OnEquippedWeaponChanged.Broadcast(OldWeapon, EquippedWeapon);
}

void UADWeaponManagerComponent::ClearEquippedWeapon()
{
	SetEquippedWeapon(nullptr);
}

void UADWeaponManagerComponent::BindWeaponOwner(AADWeaponBase* Weapon) const
{
	AADCharacterBase* OwningCharacter = GetOwningCharacter();
	if (!IsValid(Weapon) || OwningCharacter == nullptr)
	{
		return;
	}

	Weapon->SetOwningCharacter(OwningCharacter);
}

void UADWeaponManagerComponent::ClearWeaponOwner(AADWeaponBase* Weapon) const
{
	const AADCharacterBase* OwningCharacter = GetOwningCharacter();
	if (!IsValid(Weapon) || OwningCharacter == nullptr || Weapon->GetOwningCharacter() != OwningCharacter)
	{
		return;
	}

	Weapon->SetOwningCharacter(nullptr);
}

#include "Equipment/ADWeaponBase.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"

AADWeaponBase::AADWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	SetRootComponent(WeaponMeshComponent);
}

USkeletalMeshComponent* AADWeaponBase::GetWeaponMeshComponent() const
{
	return WeaponMeshComponent;
}

AADCharacterBase* AADWeaponBase::GetOwningCharacter() const
{
	return OwningCharacter;
}

void AADWeaponBase::SetOwningCharacter(AADCharacterBase* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	SetOwner(InOwningCharacter);
}

#include "Components/Equipment/ADWeaponManagerComponent.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Equipment/ADWeaponBase.h"

UADWeaponManagerComponent::UADWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UADWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(EquippedWeapon))
	{
		FADWeaponSpawnConfig ExistingWeaponConfig;
		RegisterWeaponInstance(EquippedWeapon, ExistingWeaponConfig, false);
		SetEquippedWeapon(EquippedWeapon);
	}

	if (bSpawnWeaponsOnBeginPlay)
	{
		SpawnConfiguredWeapons();
	}

	if (!HasEquippedWeapon() && !WeaponInstances.IsEmpty())
	{
		if (!EquipWeaponByIndex(InitialWeaponIndex))
		{
			EquipWeaponByDirection(1);
		}
	}

	RefreshWeaponActivationStates();
}

void UADWeaponManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (int32 WeaponIndex = 0; WeaponIndex < WeaponInstances.Num(); ++WeaponIndex)
	{
		AADWeaponBase* Weapon = WeaponInstances[WeaponIndex];
		ClearWeaponOwner(Weapon);
		if (bDestroySpawnedWeaponsOnEndPlay && bGeneratedWeaponInstances.IsValidIndex(WeaponIndex) && bGeneratedWeaponInstances[WeaponIndex] && IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}

	WeaponInstances.Reset();
	WeaponInstanceConfigs.Reset();
	bGeneratedWeaponInstances.Reset();
	EquippedWeapon = nullptr;
	EquippedWeaponIndex = INDEX_NONE;
	bConfiguredWeaponsSpawned = false;

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

int32 UADWeaponManagerComponent::GetEquippedWeaponIndex() const
{
	return EquippedWeaponIndex;
}

int32 UADWeaponManagerComponent::GetWeaponInstanceCount() const
{
	return WeaponInstances.Num();
}

AADWeaponBase* UADWeaponManagerComponent::GetWeaponInstanceAtIndex(int32 WeaponIndex) const
{
	return WeaponInstances.IsValidIndex(WeaponIndex) ? WeaponInstances[WeaponIndex] : nullptr;
}

bool UADWeaponManagerComponent::SpawnConfiguredWeapons()
{
	if (bConfiguredWeaponsSpawned)
	{
		return false;
	}

	bConfiguredWeaponsSpawned = true;
	bool bSpawnedAnyWeapon = false;

	for (const FADWeaponSpawnConfig& WeaponConfig : WeaponSpawnConfigs)
	{
		if (SpawnWeaponFromConfig(WeaponConfig) != nullptr)
		{
			bSpawnedAnyWeapon = true;
		}
	}

	if (!HasEquippedWeapon() && bSpawnedAnyWeapon)
	{
		if (!EquipWeaponByIndex(InitialWeaponIndex))
		{
			EquipWeaponByDirection(1);
		}
	}

	return bSpawnedAnyWeapon;
}

AADWeaponBase* UADWeaponManagerComponent::SpawnWeaponFromConfig(const FADWeaponSpawnConfig& WeaponConfig)
{
	AADCharacterBase* OwningCharacter = GetOwningCharacter();
	UWorld* World = GetWorld();
	if (OwningCharacter == nullptr || World == nullptr)
	{
		return nullptr;
	}

	if (WeaponConfig.WeaponClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Weapon spawn skipped: WeaponClass is null. Owner=%s"), *GetNameSafe(OwningCharacter));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwningCharacter;
	SpawnParameters.Instigator = OwningCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AADWeaponBase* SpawnedWeapon = World->SpawnActor<AADWeaponBase>(
		WeaponConfig.WeaponClass,
		OwningCharacter->GetActorTransform(),
		SpawnParameters);

	if (!IsValid(SpawnedWeapon))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ActionDemo] Weapon spawn failed. Owner=%s WeaponClass=%s"),
			*GetNameSafe(OwningCharacter),
			*GetNameSafe(WeaponConfig.WeaponClass.Get()));
		return nullptr;
	}

	RegisterWeaponInstance(SpawnedWeapon, WeaponConfig, true);
	AttachWeaponToCharacter(SpawnedWeapon, WeaponConfig);
	SetWeaponActive(SpawnedWeapon, false);

	return SpawnedWeapon;
}

void UADWeaponManagerComponent::SetEquippedWeapon(AADWeaponBase* InWeapon)
{
	if (EquippedWeapon == InWeapon)
	{
		BindWeaponOwner(EquippedWeapon);
		if (IsValid(EquippedWeapon))
		{
			const int32 WeaponIndex = FindWeaponIndex(EquippedWeapon);
			EquippedWeaponIndex = WeaponIndex;
			const FADWeaponSpawnConfig WeaponConfig = WeaponInstanceConfigs.IsValidIndex(WeaponIndex) ? WeaponInstanceConfigs[WeaponIndex] : FADWeaponSpawnConfig();
			AttachWeaponToCharacter(EquippedWeapon, WeaponConfig);
		}
		RefreshWeaponActivationStates();
		return;
	}

	AADWeaponBase* OldWeapon = EquippedWeapon;
	if (IsValid(OldWeapon) && !IsManagedWeapon(OldWeapon))
	{
		ClearWeaponOwner(OldWeapon);
	}

	EquippedWeapon = InWeapon;
	EquippedWeaponIndex = FindWeaponIndex(EquippedWeapon);
	BindWeaponOwner(EquippedWeapon);
	if (IsValid(EquippedWeapon))
	{
		const FADWeaponSpawnConfig WeaponConfig =
			WeaponInstanceConfigs.IsValidIndex(EquippedWeaponIndex) ? WeaponInstanceConfigs[EquippedWeaponIndex] : FADWeaponSpawnConfig();
		AttachWeaponToCharacter(EquippedWeapon, WeaponConfig);
	}

	RefreshWeaponActivationStates();

	OnEquippedWeaponChanged.Broadcast(OldWeapon, EquippedWeapon);
}

void UADWeaponManagerComponent::ClearEquippedWeapon()
{
	SetEquippedWeapon(nullptr);
}

bool UADWeaponManagerComponent::EquipWeaponByIndex(int32 WeaponIndex)
{
	if (!WeaponInstances.IsValidIndex(WeaponIndex) || !IsValid(WeaponInstances[WeaponIndex]))
	{
		return false;
	}

	SetEquippedWeapon(WeaponInstances[WeaponIndex]);
	return HasEquippedWeapon() && EquippedWeaponIndex == WeaponIndex;
}

bool UADWeaponManagerComponent::EquipNextWeapon()
{
	return EquipWeaponByDirection(1);
}

bool UADWeaponManagerComponent::EquipPreviousWeapon()
{
	return EquipWeaponByDirection(-1);
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

void UADWeaponManagerComponent::RegisterWeaponInstance(AADWeaponBase* Weapon, const FADWeaponSpawnConfig& WeaponConfig, bool bGeneratedByManager)
{
	if (!IsValid(Weapon))
	{
		return;
	}

	const int32 ExistingIndex = FindWeaponIndex(Weapon);
	if (ExistingIndex != INDEX_NONE)
	{
		if (WeaponInstanceConfigs.IsValidIndex(ExistingIndex))
		{
			WeaponInstanceConfigs[ExistingIndex] = WeaponConfig;
		}
		if (bGeneratedWeaponInstances.IsValidIndex(ExistingIndex))
		{
			bGeneratedWeaponInstances[ExistingIndex] = bGeneratedWeaponInstances[ExistingIndex] || bGeneratedByManager;
		}
		BindWeaponOwner(Weapon);
		return;
	}

	WeaponInstances.Add(Weapon);
	WeaponInstanceConfigs.Add(WeaponConfig);
	bGeneratedWeaponInstances.Add(bGeneratedByManager);
	BindWeaponOwner(Weapon);
}

bool UADWeaponManagerComponent::IsManagedWeapon(const AADWeaponBase* Weapon) const
{
	return FindWeaponIndex(Weapon) != INDEX_NONE;
}

int32 UADWeaponManagerComponent::FindWeaponIndex(const AADWeaponBase* Weapon) const
{
	if (!IsValid(Weapon))
	{
		return INDEX_NONE;
	}

	for (int32 WeaponIndex = 0; WeaponIndex < WeaponInstances.Num(); ++WeaponIndex)
	{
		if (WeaponInstances[WeaponIndex] == Weapon)
		{
			return WeaponIndex;
		}
	}

	return INDEX_NONE;
}

bool UADWeaponManagerComponent::EquipWeaponByDirection(int32 Direction)
{
	if (WeaponInstances.IsEmpty())
	{
		return false;
	}

	const int32 StepDirection = Direction >= 0 ? 1 : -1;
	const int32 StartIndex = EquippedWeaponIndex != INDEX_NONE ? EquippedWeaponIndex : (StepDirection > 0 ? -1 : WeaponInstances.Num());

	for (int32 Step = 1; Step <= WeaponInstances.Num(); ++Step)
	{
		const int32 CandidateIndex = (StartIndex + StepDirection * Step + WeaponInstances.Num()) % WeaponInstances.Num();
		if (WeaponInstances.IsValidIndex(CandidateIndex) && IsValid(WeaponInstances[CandidateIndex]))
		{
			return EquipWeaponByIndex(CandidateIndex);
		}
	}

	return false;
}

void UADWeaponManagerComponent::AttachWeaponToCharacter(AADWeaponBase* Weapon, const FADWeaponSpawnConfig& WeaponConfig) const
{
	AADCharacterBase* OwningCharacter = GetOwningCharacter();
	if (!IsValid(Weapon) || OwningCharacter == nullptr)
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh();
	if (CharacterMesh == nullptr)
	{
		Weapon->AttachToActor(OwningCharacter, FAttachmentTransformRules::KeepRelativeTransform);
		Weapon->SetActorRelativeTransform(ResolveAttachRelativeTransform(WeaponConfig));
		return;
	}

	Weapon->AttachToComponent(
		CharacterMesh,
		FAttachmentTransformRules::KeepRelativeTransform,
		ResolveAttachSocketName(WeaponConfig));
	Weapon->SetActorRelativeTransform(ResolveAttachRelativeTransform(WeaponConfig));
}

void UADWeaponManagerComponent::RefreshWeaponActivationStates() const
{
	for (AADWeaponBase* Weapon : WeaponInstances)
	{
		SetWeaponActive(Weapon, Weapon == EquippedWeapon);
	}

	if (IsValid(EquippedWeapon) && !IsManagedWeapon(EquippedWeapon))
	{
		SetWeaponActive(EquippedWeapon, true);
	}
}

void UADWeaponManagerComponent::SetWeaponActive(AADWeaponBase* Weapon, bool bActive) const
{
	if (!IsValid(Weapon))
	{
		return;
	}

	if (bActive || bHideUnequippedWeapons)
	{
		Weapon->SetActorHiddenInGame(!bActive);
	}

	Weapon->SetActorEnableCollision(bActive && bEnableEquippedWeaponCollision);
}

FName UADWeaponManagerComponent::ResolveAttachSocketName(const FADWeaponSpawnConfig& WeaponConfig) const
{
	return WeaponConfig.AttachSocketName.IsNone() ? DefaultAttachSocketName : WeaponConfig.AttachSocketName;
}

FTransform UADWeaponManagerComponent::ResolveAttachRelativeTransform(const FADWeaponSpawnConfig& WeaponConfig) const
{
	return WeaponConfig.AttachRelativeTransform.Equals(FTransform::Identity)
		? DefaultAttachRelativeTransform
		: WeaponConfig.AttachRelativeTransform;
}

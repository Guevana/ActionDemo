#include "Character/Enemy/ADEnemyCharacter.h"

#include "AbilitySystem/ADAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "AI/ADEnemyAIController.h"
#include "Character/Enemy/ADEnemyConfigData.h"
#include "GameplayAbilitySpec.h"

AADEnemyCharacter::AADEnemyCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AADEnemyAIController::StaticClass();
}

void AADEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GrantStartupAbilitiesFromConfig();
}

void AADEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeAbilityActorInfo();
	GrantStartupAbilitiesFromConfig();
}

UADEnemyConfigData* AADEnemyCharacter::GetEnemyConfig() const
{
	return EnemyConfig;
}

AADCharacterBase* AADEnemyCharacter::GetLastHitInstigator() const
{
	return LastHitInstigator;
}

void AADEnemyCharacter::ReceiveCombatHit_Implementation(const FADCombatHitEventData& HitData)
{
	LastReceivedHit = HitData;
	LastHitInstigator = HitData.InstigatorCharacter;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[ActionDemo] Enemy hit received. Enemy=%s Instigator=%s Action=%s"),
		*GetName(),
		LastHitInstigator != nullptr ? *LastHitInstigator->GetName() : TEXT("None"),
		*HitData.SourceActionTag.ToString());

	OnEnemyHitReceived.Broadcast(HitData);
}

void AADEnemyCharacter::HandleDeath_Implementation()
{
	const bool bWasDead = IsDead();
	Super::HandleDeath_Implementation();

	if (!bWasDead && IsDead())
	{
		OnEnemyDeath.Broadcast(this);
	}
}

void AADEnemyCharacter::GrantStartupAbilitiesFromConfig()
{
	if (bStartupAbilitiesGranted || !HasAuthority() || GetADAbilitySystemComponent() == nullptr)
	{
		return;
	}

	TSet<UClass*> GrantedAbilityClasses;
	auto GrantAbility = [this, &GrantedAbilityClasses](const TSubclassOf<UADGameplayAbility>& AbilityClass)
	{
		UClass* AbilityClassObject = AbilityClass.Get();
		if (AbilityClassObject == nullptr || GrantedAbilityClasses.Contains(AbilityClassObject))
		{
			return;
		}

		GrantedAbilityClasses.Add(AbilityClassObject);
		FGameplayAbilitySpec AbilitySpec(AbilityClassObject, 1, INDEX_NONE, this);
		GetADAbilitySystemComponent()->GiveAbility(AbilitySpec);
	};

	if (EnemyConfig != nullptr)
	{
		for (const TSubclassOf<UADGameplayAbility>& AbilityClass : EnemyConfig->StartupAbilities)
		{
			GrantAbility(AbilityClass);
		}
	}

	bStartupAbilitiesGranted = true;
}

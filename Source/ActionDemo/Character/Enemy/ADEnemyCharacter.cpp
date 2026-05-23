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

void AADEnemyCharacter::GrantStartupAbilitiesFromConfig()
{
	if (bStartupAbilitiesGranted || !HasAuthority() || EnemyConfig == nullptr || GetADAbilitySystemComponent() == nullptr)
	{
		return;
	}

	for (const TSubclassOf<UADGameplayAbility>& AbilityClass : EnemyConfig->StartupAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
		GetADAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}

	bStartupAbilitiesGranted = true;
}

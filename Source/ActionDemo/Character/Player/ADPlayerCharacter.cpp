#include "Character/Player/ADPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/Combat/ADCombatComponent.h"
#include "Components/Input/ADAbilityQueueComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/SpringArmComponent.h"

AADPlayerCharacter::AADPlayerCharacter()
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = CameraBoomLength;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CombatStateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("CombatStateTreeComponent"));
}

void AADPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CameraBoom->TargetArmLength = CameraBoomLength;

	if (GetCombatComponent() != nullptr)
	{
		GetCombatComponent()->OnCancelWindowOpened.AddDynamic(this, &AADPlayerCharacter::HandleCancelWindowOpened);
	}
}

void AADPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeAbilityActorInfo();
}

UStateTreeComponent* AADPlayerCharacter::GetCombatStateTreeComponent() const
{
	return CombatStateTreeComponent;
}

void AADPlayerCharacter::HandleCancelWindowOpened()
{
	UADAbilityQueueComponent* QueueComponent = GetAbilityQueueComponent();
	if (QueueComponent == nullptr || CombatStateTreeComponent == nullptr || !CombatStateTreeComponent->IsRunning())
	{
		return;
	}

	const FGameplayTag BufferedInputTag = QueueComponent->GetBufferedInputTag();
	if (QueueComponent->HasBufferedInput() && BufferedInputTag.IsValid())
	{
		CombatStateTreeComponent->SendStateTreeEvent(BufferedInputTag);
	}
}

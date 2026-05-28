#include "Game/ADPlayerController.h"

#include "AbilitySystem/ADAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ADGameplayAbility.h"
#include "Character/Base/ADCharacterBase.h"
#include "Character/Player/ADPlayerCharacter.h"
#include "Components/Input/ADAbilityQueueComponent.h"
#include "Components/StateTreeComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/Data/ADInputConfigData.h"
#include "InputAction.h"
#include "Math/RotationMatrix.h"

TArray<FString> AADPlayerController::GetRemoteFunctionNames()
{
	TArray<FString> FunctionNames;

	for (TFieldIterator<UFunction> It(AADPlayerController::StaticClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		const FString FunctionName = It->GetName();
		if (FunctionName.StartsWith(TEXT("Ability_")) && IsValidConfiguredInputFunction(*It))
		{
			FunctionNames.Add(FunctionName);
		}
	}

	FunctionNames.Sort();
	return FunctionNames;
}

AADPlayerController::AADPlayerController()
{
	bReplicates = true;
}

void AADPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	BindConfiguredInputActions();
}

void AADPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BindConfiguredInputActions();
	InitializeConfiguredInputMappings();
	GrantStartupAbilitiesFromConfig();
}

void AADPlayerController::SetPawn(APawn* InPawn)
{
	APawn* PreviousPawn = GetPawn();
	Super::SetPawn(InPawn);

	if (PreviousPawn != InPawn)
	{
		SetFreeMovementMode();
		bStartupAbilitiesGranted = false;

		if (InputConfig == nullptr)
		{
			bInputActionsBound = false;
			bInputMappingsInitialized = false;
		}
	}

	BindConfiguredInputActions();
	InitializeConfiguredInputMappings();
}

void AADPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	bStartupAbilitiesGranted = false;
}

void AADPlayerController::SetFreeMovementMode()
{
	PlayerMovementMode = EADPlayerMovementMode::Free;
	LockOnMovementModeParams = FADLockOnMovementModeParams();
}

void AADPlayerController::SetLockOnMovementMode(AActor* TargetActor, float MinTargetDistance, bool bAllowForwardMoveAtCloseRange)
{
	if (!IsValidLockOnMovementTarget(TargetActor))
	{
		SetFreeMovementMode();
		return;
	}

	PlayerMovementMode = EADPlayerMovementMode::LockOn;
	LockOnMovementModeParams.TargetActor = TargetActor;
	LockOnMovementModeParams.MinTargetDistance = FMath::Max(0.0f, MinTargetDistance);
	LockOnMovementModeParams.bAllowForwardMoveAtCloseRange = bAllowForwardMoveAtCloseRange;
}

EADPlayerMovementMode AADPlayerController::GetPlayerMovementMode() const
{
	return PlayerMovementMode;
}

void AADPlayerController::BindConfiguredInputActions()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	UADInputConfigData* ResolvedInputConfig = GetResolvedInputConfig();
	if (bInputActionsBound || EnhancedInputComponent == nullptr || ResolvedInputConfig == nullptr)
	{
		return;
	}

	for (const FADAbilityInputBinding& Binding : ResolvedInputConfig->AbilityInputBindings)
	{
		if (Binding.InputAction == nullptr || Binding.FunctionNameToCall.IsNone())
		{
			continue;
		}

		UFunction* Function = FindFunction(Binding.FunctionNameToCall);
		if (!IsValidConfiguredInputFunction(Function))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ActionDemo] Input binding function signature is invalid, skipped: %s"), *Binding.FunctionNameToCall.ToString());
			continue;
		}

		EnhancedInputComponent->BindAction(
			Binding.InputAction,
			Binding.TriggerEvent,
			this,
			Binding.FunctionNameToCall);
	}

	bInputActionsBound = true;
}

void AADPlayerController::Ability_HandleLightAttackAbilityInput(
	FInputActionValue ActionValue,
	float ElapsedTime,
	float TriggeredTime,
	const UInputAction* SourceAction)
{
	if (SourceAction == nullptr)
	{
		return;
	}

	const FADAbilityInputBinding* Binding = FindInputBindingByAction(SourceAction);
	AADPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter();
	if (Binding == nullptr || !Binding->InputTag.IsValid() || PlayerCharacter == nullptr)
	{
		return;
	}

	if (PlayerCharacter->GetAbilityQueueComponent() != nullptr)
	{
		PlayerCharacter->GetAbilityQueueComponent()->BufferInputTag(Binding->InputTag);
	}

	if (UStateTreeComponent* CombatStateTreeComponent = PlayerCharacter->GetCombatStateTreeComponent())
	{
		if (CombatStateTreeComponent->IsRunning())
		{
			CombatStateTreeComponent->SendStateTreeEvent(Binding->InputTag);
		}
	}
}

void AADPlayerController::Ability_HandleTargetLockAbilityInput(
	FInputActionValue ActionValue,
	float ElapsedTime,
	float TriggeredTime,
	const UInputAction* SourceAction)
{
	if (SourceAction == nullptr)
	{
		return;
	}

	const FADAbilityInputBinding* Binding = FindInputBindingByAction(SourceAction);
	AADPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter();
	if (Binding == nullptr || !Binding->InputTag.IsValid() || PlayerCharacter == nullptr)
	{
		return;
	}

	if (PlayerCharacter->GetADAbilitySystemComponent() != nullptr)
	{
		PlayerCharacter->GetADAbilitySystemComponent()->TryActivateAbilityByInputTag(Binding->InputTag);
	}
}

void AADPlayerController::Ability_HandleMoveInput(
	FInputActionValue ActionValue, 
	float ElapsedTime, 
	float TriggeredTime,
	const UInputAction* SourceAction)
{
	AADPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter();
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	const FVector2D MovementInput = ActionValue.Get<FVector2D>();
	if (MovementInput.IsNearlyZero())
	{
		return;
	}

	switch (PlayerMovementMode)
	{
	case EADPlayerMovementMode::LockOn:
		if (TryHandleLockOnMoveInput(PlayerCharacter, MovementInput))
		{
			return;
		}
		break;

	case EADPlayerMovementMode::Free:
	default:
		break;
	}

	HandleFreeMoveInput(PlayerCharacter, MovementInput);
}

void AADPlayerController::HandleFreeMoveInput(AADPlayerCharacter* PlayerCharacter, const FVector2D& MovementInput)
{
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	const FRotator CurrentControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, CurrentControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	PlayerCharacter->AddMovementInput(ForwardDirection, MovementInput.Y);
	PlayerCharacter->AddMovementInput(RightDirection, MovementInput.X);
}

bool AADPlayerController::TryHandleLockOnMoveInput(AADPlayerCharacter* PlayerCharacter, const FVector2D& MovementInput)
{
	if (PlayerCharacter == nullptr)
	{
		return false;
	}

	AActor* TargetActor = LockOnMovementModeParams.TargetActor;
	if (!IsValidLockOnMovementTarget(TargetActor))
	{
		SetFreeMovementMode();
		return false;
	}

	FVector ToTarget = TargetActor->GetActorLocation() - PlayerCharacter->GetActorLocation();
	ToTarget.Z = 0.0f;

	if (ToTarget.IsNearlyZero())
	{
		return true;
	}

	const float TargetDistance = ToTarget.Size();
	const FVector ForwardToTarget = ToTarget / TargetDistance;
	const FVector RightAroundTarget = FVector::CrossProduct(FVector::UpVector, ForwardToTarget).GetSafeNormal();
	const bool bIsForwardInput = MovementInput.Y > 0.0f;
	const bool bCanMoveForward = !bIsForwardInput ||
		LockOnMovementModeParams.bAllowForwardMoveAtCloseRange ||
		TargetDistance > LockOnMovementModeParams.MinTargetDistance;

	if (bCanMoveForward)
	{
		PlayerCharacter->AddMovementInput(ForwardToTarget, MovementInput.Y);
	}

	PlayerCharacter->AddMovementInput(RightAroundTarget, MovementInput.X);
	return true;
}

bool AADPlayerController::IsValidLockOnMovementTarget(const AActor* TargetActor) const
{
	const AADCharacterBase* TargetCharacter = Cast<AADCharacterBase>(TargetActor);
	return IsValid(TargetActor) && TargetActor != GetPawn() && TargetCharacter != nullptr && !TargetCharacter->IsDead();
}

void AADPlayerController::Ability_HandleLookInput(FInputActionValue ActionValue, 
	float ElapsedTime,
	float TriggeredTime,
	const UInputAction* SourceAction)
{	AADPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter();
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	const FVector2D LookInput = ActionValue.Get<FVector2D>();
	AddYawInput(LookInput.X);
	AddPitchInput(LookInput.Y);
}

void AADPlayerController::InitializeConfiguredInputMappings()
{
	UADInputConfigData* ResolvedInputConfig = GetResolvedInputConfig();
	if (bInputMappingsInitialized || ResolvedInputConfig == nullptr)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputSubsystem == nullptr)
	{
		return;
	}

	for (const FADInputMappingContextEntry& Entry : ResolvedInputConfig->MappingContexts)
	{
		if (Entry.MappingContext != nullptr)
		{
			InputSubsystem->AddMappingContext(Entry.MappingContext, Entry.Priority);
		}
	}

	bInputMappingsInitialized = true;
}

void AADPlayerController::GrantStartupAbilitiesFromConfig()
{
	AADPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter();
	UADInputConfigData* ResolvedInputConfig = GetResolvedInputConfig();
	if (bStartupAbilitiesGranted || !HasAuthority() || ResolvedInputConfig == nullptr || PlayerCharacter == nullptr || PlayerCharacter->GetADAbilitySystemComponent() == nullptr)
	{
		return;
	}

	for (const TSubclassOf<UADGameplayAbility>& AbilityClass : ResolvedInputConfig->StartupAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
		PlayerCharacter->GetADAbilitySystemComponent()->GiveAbility(AbilitySpec);
	}

	bStartupAbilitiesGranted = true;
}

UADInputConfigData* AADPlayerController::GetResolvedInputConfig() const
{
	if (InputConfig != nullptr)
	{
		return InputConfig;
	}
	
	return nullptr;
}

const FADAbilityInputBinding* AADPlayerController::FindInputBindingByAction(const UInputAction* InputAction) const
{
	UADInputConfigData* ResolvedInputConfig = GetResolvedInputConfig();
	if (ResolvedInputConfig == nullptr || InputAction == nullptr)
	{
		return nullptr;
	}

	for (const FADAbilityInputBinding& Binding : ResolvedInputConfig->AbilityInputBindings)
	{
		if (Binding.InputAction == InputAction)
		{
			return &Binding;
		}
	}

	return nullptr;
}

AADPlayerCharacter* AADPlayerController::GetControlledPlayerCharacter() const
{
	return Cast<AADPlayerCharacter>(GetPawn());
}

bool AADPlayerController::IsValidConfiguredInputFunction(const UFunction* Function)
{
	if (Function == nullptr)
	{
		return false;
	}

	TArray<const FProperty*> Params;
	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		if (!It->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			Params.Add(*It);
		}
	}

	if (Params.Num() != 4)
	{
		return false;
	}

	const FStructProperty* ActionValueProperty = CastField<FStructProperty>(Params[0]);
	const FFloatProperty* ElapsedTimeProperty = CastField<FFloatProperty>(Params[1]);
	const FFloatProperty* TriggeredTimeProperty = CastField<FFloatProperty>(Params[2]);
	const FObjectPropertyBase* SourceActionProperty = CastField<FObjectPropertyBase>(Params[3]);

	return ActionValueProperty != nullptr &&
		ActionValueProperty->Struct == FInputActionValue::StaticStruct() &&
		ElapsedTimeProperty != nullptr &&
		TriggeredTimeProperty != nullptr &&
		SourceActionProperty != nullptr &&
		SourceActionProperty->PropertyClass != nullptr &&
		SourceActionProperty->PropertyClass->IsChildOf(UInputAction::StaticClass());
}

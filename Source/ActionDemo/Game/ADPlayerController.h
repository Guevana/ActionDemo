#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "ADPlayerController.generated.h"

class UADInputConfigData;
class UInputAction;
struct FADAbilityInputBinding;

UENUM(BlueprintType)
enum class EADPlayerMovementMode : uint8
{
	Free,
	LockOn
};

USTRUCT(BlueprintType)
struct FADLockOnMovementModeParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Movement")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Movement")
	float MinTargetDistance = 80.0f;

	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Movement")
	bool bAllowForwardMoveAtCloseRange = false;
};

/**
 * 玩家控制器。
 * 输入来源、输入映射注入和玩家战斗输入分发统一放在控制器层处理。
 */
UCLASS()
class ACTIONDEMO_API AADPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AADPlayerController();

	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Movement")
	void SetFreeMovementMode();

	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Movement")
	void SetLockOnMovementMode(AActor* TargetActor, float MinTargetDistance = 80.0f, bool bAllowForwardMoveAtCloseRange = false);

	UFUNCTION(BlueprintPure, Category = "ActionDemo|Movement")
	EADPlayerMovementMode GetPlayerMovementMode() const;

protected:
	/** 玩家输入配置资产。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Config")
	TObjectPtr<UADInputConfigData> InputConfig;

	/** 轻攻击 Ability 输入触发。 */
	UFUNCTION()
	void Ability_HandleLightAttackAbilityInput(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime, const UInputAction* SourceAction);

	/** 锁定目标 Ability 输入触发。 */
	UFUNCTION()
	void Ability_HandleTargetLockAbilityInput(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime, const UInputAction* SourceAction);

	UFUNCTION()
	void Ability_HandleMoveInput(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime, const UInputAction* SourceAction);
	
	UFUNCTION()
	void Ability_HandleLookInput(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime, const UInputAction* SourceAction);

	void BindConfiguredInputActions();
	void InitializeConfiguredInputMappings();
	void GrantStartupAbilitiesFromConfig();

	void HandleFreeMoveInput(class AADPlayerCharacter* PlayerCharacter, const FVector2D& MovementInput);
	bool TryHandleLockOnMoveInput(class AADPlayerCharacter* PlayerCharacter, const FVector2D& MovementInput);
	bool IsValidLockOnMovementTarget(const AActor* TargetActor) const;

	UADInputConfigData* GetResolvedInputConfig() const;
	const FADAbilityInputBinding* FindInputBindingByAction(const UInputAction* InputAction) const;
	class AADPlayerCharacter* GetControlledPlayerCharacter() const;
	static bool IsValidConfiguredInputFunction(const UFunction* Function);

	/** 供数据资产 GetOptions 使用，收集合法的 Ability_ 输入处理函数。 */
	UFUNCTION()
	static TArray<FString> GetRemoteFunctionNames();

	bool bInputActionsBound = false;
	bool bStartupAbilitiesGranted = false;
	bool bInputMappingsInitialized = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ActionDemo|Movement")
	EADPlayerMovementMode PlayerMovementMode = EADPlayerMovementMode::Free;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "ActionDemo|Movement")
	FADLockOnMovementModeParams LockOnMovementModeParams;
};

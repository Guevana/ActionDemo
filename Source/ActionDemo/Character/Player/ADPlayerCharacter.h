#pragma once

#include "CoreMinimal.h"
#include "Character/Base/ADCharacterBase.h"
#include "ADPlayerCharacter.generated.h"

class UADInputConfigData;
class UADLockOnControlComponent;
class UCameraComponent;
class UStateTreeComponent;
class USpringArmComponent;

/**
 * 玩家角色基类。
 * 保持实体装配与生命周期职责，玩家输入绑定与分发统一由 PlayerController 处理。
 */
UCLASS()
class ACTIONDEMO_API AADPlayerCharacter : public AADCharacterBase
{
	GENERATED_BODY()

public:
	AADPlayerCharacter();

	UFUNCTION(BlueprintPure, Category = "ActionDemo|StateTree")
	UStateTreeComponent* GetCombatStateTreeComponent() const;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	/** 取消窗口开启时，将仍然有效的预输入重新发送给 StateTree。 */
	UFUNCTION()
	void HandleCancelWindowOpened();

	/** 默认相机弹簧臂。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** 默认跟随相机。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** 玩家战斗 StateTree 组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|StateTree")
	TObjectPtr<UStateTreeComponent> CombatStateTreeComponent;

	/** 锁定状态下的玩家朝向控制。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionDemo|Target")
	TObjectPtr<UADLockOnControlComponent> LockOnControlComponent;

	/** 相机臂长，支持蓝图子类微调。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Camera")
	float CameraBoomLength = 350.0f;
};

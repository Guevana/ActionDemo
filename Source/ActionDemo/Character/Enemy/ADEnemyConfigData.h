#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ADEnemyConfigData.generated.h"

class UADGameplayAbility;

/**
 * 敌人首版通用配置。
 * 蓝图敌人通过该资产配置启动 Ability 和基础 AI 感知/战斗距离。
 */
UCLASS(BlueprintType)
class ACTIONDEMO_API UADEnemyConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Ability")
	TArray<TSubclassOf<UADGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|AI", meta = (ClampMin = "0.0"))
	float AttackRange = 220.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|AI", meta = (ClampMin = "0.0"))
	float LoseTargetDistance = 2400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Perception", meta = (ClampMin = "0.0"))
	float SightRadius = 1800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Perception", meta = (ClampMin = "0.0"))
	float LoseSightRadius = 2200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Perception", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionAngleDegrees = 75.0f;
};

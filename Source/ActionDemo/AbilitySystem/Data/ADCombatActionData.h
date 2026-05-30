#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ADCombatActionData.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * 动作配置数据资产。
 * 描述攻击动作的动画、伤害和锁定修正参数；连段跳转由 StateTree 配置。
 */
UCLASS(BlueprintType)
class ACTIONDEMO_API UADCombatActionData : public UDataAsset
{
	GENERATED_BODY()

public:
	UADCombatActionData();

	/** 攻击动作对应的 Montage。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 攻击总时长，默认用于自动结束 Ability(一般需长于 Montage 长度)。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|Animation", meta = (ClampMin = "0.0"))
	float DefaultAttackDuration = 0.35f;

	/** 无 Montage 或 Montage 未启动时，是否使用 DefaultAttackDuration 兜底结束 Ability。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|Animation")
	bool bAutoEndAbility = true;

	/** 是否在激活后自动播放 Montage。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|Animation")
	bool bAutoPlayMontage = true;

	/** 连段取消旧 Ability 时 Montage 的淡出时间。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|Animation", meta = (ClampMin = "0.0"))
	float CancelMontageBlendOutTime = 0.08f;

	/** 命中后由目标侧受击 Ability 应用的伤害 GameplayEffect。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 当前动作参与伤害公式的攻击力系数。最终伤害为 AttackPower * DamageScale - Defense。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|Damage", meta = (ClampMin = "0.0"))
	float DamageScale = 1.0f;

	/** Commit 成功后是否刷新锁定目标对应的 Motion Warping Target。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|MotionWarping")
	bool bUpdateLockOnWarpTargetOnCommit = true;

	/** 是否使用当前锁定目标作为攻击 Motion Warping Target。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|MotionWarping")
	bool bUseLockedTargetForWarping = true;

	/** 攻击 Motion Warping Target 名称，需要与 Montage Notify 中的 Warp Target Name 对齐。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|MotionWarping")
	FName LockOnWarpTargetName = TEXT("AttackTarget");

	/** 沿攻击者指向锁定目标方向的目标点偏移。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|MotionWarping")
	float LockOnWarpTargetForwardOffset = 0.0f;

	/** 沿攻击者面向锁定目标时右方向的目标点偏移。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|MotionWarping")
	float LockOnWarpTargetRightOffset = 0.0f;

	/** 世界 Up 方向目标点偏移。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Combat|MotionWarping")
	float LockOnWarpTargetUpOffset = 0.0f;
};

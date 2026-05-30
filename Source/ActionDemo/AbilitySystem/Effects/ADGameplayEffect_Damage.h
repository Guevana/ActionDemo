#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ADGameplayEffect_Damage.generated.h"

/**
 * 默认即时伤害 GE。
 * 通过执行计算结合来源攻击力与目标防御修改 Health。
 */
UCLASS()
class ACTIONDEMO_API UADGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UADGameplayEffect_Damage();
};

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ADGameplayEffect_Damage.generated.h"

/**
 * 默认即时伤害 GE。
 * 通过 SetByCaller(Data.Damage) 修改 Health；调用方传入负值即扣血。
 */
UCLASS()
class ACTIONDEMO_API UADGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UADGameplayEffect_Damage();
};

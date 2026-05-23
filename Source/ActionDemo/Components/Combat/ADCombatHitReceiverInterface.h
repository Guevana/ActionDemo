#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Components/Combat/ADCombatHitTypes.h"
#include "ADCombatHitReceiverInterface.generated.h"

UINTERFACE(BlueprintType)
class ACTIONDEMO_API UADCombatHitReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可接收标准近战命中数据的对象接口。
 * GAS GameplayEvent 仍是能力侧入口，本接口用于角色或组件同步消费同一份命中上下文。
 */
class ACTIONDEMO_API IADCombatHitReceiverInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ActionDemo|Hit")
	void ReceiveCombatHit(const FADCombatHitEventData& HitData);
};

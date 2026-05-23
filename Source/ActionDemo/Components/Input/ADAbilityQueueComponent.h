#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ADAbilityQueueComponent.generated.h"

/**
 * 输入缓冲与 Ability 排队组件。
 * 当前先提供最小化缓存能力，后续补充时间窗与优先级策略。
 */
UCLASS(ClassGroup=(ActionDemo), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class ACTIONDEMO_API UADAbilityQueueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UADAbilityQueueComponent();

	/** 最近一次缓存的输入标签。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Input")
	FGameplayTag BufferedInputTag;

	/** 输入缓冲有效时间，超过该时间的预输入不会继续触发连段。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionDemo|Input", meta = (ClampMin = "0.0"))
	float BufferedInputLifetime = 0.35f;

	/** 缓存当前输入。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Input")
	void BufferInputTag(const FGameplayTag& InputTag);

	/** 清空当前输入缓存。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Input")
	void ClearBufferedInput();

	/** 读取当前缓存输入。 */
	UFUNCTION(BlueprintPure, Category = "ActionDemo|Input")
	FGameplayTag GetBufferedInputTag() const;

	/** 是否存在可消费的输入缓存。 */
	UFUNCTION(BlueprintPure, Category = "ActionDemo|Input")
	bool HasBufferedInput() const;

	/** 当前缓存是否仍在有效时间内。 */
	UFUNCTION(BlueprintPure, Category = "ActionDemo|Input")
	bool IsBufferedInputFresh() const;

	/** 读取并消费当前输入缓存。 */
	UFUNCTION(BlueprintCallable, Category = "ActionDemo|Input")
	bool ConsumeBufferedInput(FGameplayTag& OutInputTag);

protected:
	/** 最近一次写入输入缓存的世界时间。 */
	UPROPERTY(BlueprintReadOnly, Category = "ActionDemo|Input")
	float BufferedInputTime = -1.0f;
};

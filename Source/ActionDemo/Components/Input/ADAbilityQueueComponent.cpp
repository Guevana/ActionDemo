#include "Components/Input/ADAbilityQueueComponent.h"

UADAbilityQueueComponent::UADAbilityQueueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UADAbilityQueueComponent::BufferInputTag(const FGameplayTag& InputTag)
{
	BufferedInputTag = InputTag;
	BufferedInputTime = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : -1.0f;
}

void UADAbilityQueueComponent::ClearBufferedInput()
{
	BufferedInputTag = FGameplayTag();
	BufferedInputTime = -1.0f;
}

FGameplayTag UADAbilityQueueComponent::GetBufferedInputTag() const
{
	return IsBufferedInputFresh() ? BufferedInputTag : FGameplayTag();
}

bool UADAbilityQueueComponent::HasBufferedInput() const
{
	return BufferedInputTag.IsValid() && IsBufferedInputFresh();
}

bool UADAbilityQueueComponent::IsBufferedInputFresh() const
{
	if (!BufferedInputTag.IsValid())
	{
		return false;
	}

	if (BufferedInputLifetime <= 0.0f)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr || BufferedInputTime < 0.0f)
	{
		return false;
	}

	return World->GetTimeSeconds() - BufferedInputTime <= BufferedInputLifetime;
}

bool UADAbilityQueueComponent::ConsumeBufferedInput(FGameplayTag& OutInputTag)
{
	if (!HasBufferedInput())
	{
		ClearBufferedInput();
		return false;
	}

	OutInputTag = BufferedInputTag;
	ClearBufferedInput();
	return true;
}

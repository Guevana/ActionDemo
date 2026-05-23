#include "Tree/StateTree/Conditions/ADStateTreeCondition_BufferedInputMatchesTag.h"

#include "Character/Base/ADCharacterBase.h"
#include "Components/Input/ADAbilityQueueComponent.h"
#include "Tree/StateTree/ADStateTreeContextHelpers.h"
#include "StateTreeExecutionContext.h"

bool UADStateTreeCondition_BufferedInputMatchesTag::TestCondition(FStateTreeExecutionContext& Context) const
{
	const AADCharacterBase* Character = ADStateTreeContextHelpers::ResolveCharacter(GetOwnerActor(Context));
	if (Character == nullptr || Character->GetAbilityQueueComponent() == nullptr || !ExpectedInputTag.IsValid())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[ActionDemo] BufferedInputMatchesTag 条件测试失败：角色、输入队列或输入标签无效。Character为空:%d AbilityQueue为空:%d"),
			Character == nullptr,
			Character == nullptr || Character->GetAbilityQueueComponent() == nullptr);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[ActionDemo] BufferedInputTag = %s "), *Character->GetAbilityQueueComponent()->GetBufferedInputTag().ToString());
	return Character->GetAbilityQueueComponent()->GetBufferedInputTag().MatchesTagExact(ExpectedInputTag);
}

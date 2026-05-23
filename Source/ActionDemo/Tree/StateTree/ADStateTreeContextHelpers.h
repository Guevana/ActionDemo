#pragma once

#include "CoreMinimal.h"
#include "Character/Base/ADCharacterBase.h"
#include "GameFramework/Controller.h"

namespace ADStateTreeContextHelpers
{
	inline AADCharacterBase* ResolveCharacter(AActor* OwnerActor)
	{
		if (AADCharacterBase* Character = Cast<AADCharacterBase>(OwnerActor))
		{
			return Character;
		}

		if (AController* Controller = Cast<AController>(OwnerActor))
		{
			return Cast<AADCharacterBase>(Controller->GetPawn());
		}

		return nullptr;
	}

	inline const AADCharacterBase* ResolveCharacter(const AActor* OwnerActor)
	{
		if (const AADCharacterBase* Character = Cast<AADCharacterBase>(OwnerActor))
		{
			return Character;
		}

		if (const AController* Controller = Cast<AController>(OwnerActor))
		{
			return Cast<AADCharacterBase>(Controller->GetPawn());
		}

		return nullptr;
	}
}

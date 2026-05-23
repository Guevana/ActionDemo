#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ADGameModeBase.generated.h"

/**
 * 项目默认 GameMode。
 * 当前用于保证玩家能直接生成到 C++ 玩家角色，便于验证战斗闭环。
 */
UCLASS()
class ACTIONDEMO_API AADGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AADGameModeBase();
};

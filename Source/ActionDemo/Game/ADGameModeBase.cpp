#include "Game/ADGameModeBase.h"

#include "Character/Player/ADPlayerCharacter.h"
#include "Game/ADPlayerController.h"

AADGameModeBase::AADGameModeBase()
{
	DefaultPawnClass = AADPlayerCharacter::StaticClass();
	PlayerControllerClass = AADPlayerController::StaticClass();
}

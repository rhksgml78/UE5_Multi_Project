#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "PlayerGameMode.generated.h"

/*
플레이어 게임모드는 게임에서 플레이어의 탈락 등 관리
*/

UCLASS()
class SKH_MULTISHOOTING_API APlayerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class APlayerCharacter* ElimmedCharacter, class AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController);
	
	
};

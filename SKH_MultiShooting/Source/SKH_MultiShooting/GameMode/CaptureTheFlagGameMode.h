#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"

#include "CaptureTheFlagGameMode.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
	
public:
	// 점수 계산
	virtual void PlayerEliminated(class APlayerCharacter* ElimmedCharacter, class AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController) override;

	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};

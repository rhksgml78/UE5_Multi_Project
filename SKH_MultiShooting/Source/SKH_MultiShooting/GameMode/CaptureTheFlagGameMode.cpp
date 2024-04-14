#include "CaptureTheFlagGameMode.h"
#include "SKH_MultiShooting/Weapon/Flag.h"
#include "SKH_MultiShooting/CaptureTheFlag/FlagZone.h"
#include "SKH_MultiShooting/GameState/PlayerGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
	// 상속받은 팀모드가아닌 개인모드의 점수계산을 호출
	APlayerGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);


}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() == Zone->Team;
	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(GameState);
	if (PlayerGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			PlayerGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			PlayerGameState->RedTeamScores();
		}
	}
}

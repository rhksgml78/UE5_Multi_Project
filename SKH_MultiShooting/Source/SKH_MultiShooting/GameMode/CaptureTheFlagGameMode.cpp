#include "CaptureTheFlagGameMode.h"
#include "SKH_MultiShooting/Weapon/Flag.h"
#include "SKH_MultiShooting/CaptureTheFlag/FlagZone.h"
#include "SKH_MultiShooting/GameState/PlayerGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
	// ��ӹ��� ����尡�ƴ� ���θ���� ��������� ȣ��
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

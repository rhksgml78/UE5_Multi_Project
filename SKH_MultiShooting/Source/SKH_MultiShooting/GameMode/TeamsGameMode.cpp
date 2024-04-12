#include "TeamsGameMode.h"
#include "SKH_MultiShooting/GameState/PlayerGameState.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	// 부모클래스의 변수를 재설정
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	// 새로운 플레이어가 접속 했을 경우
	Super::PostLogin(NewPlayer);

	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	if (PlayerGameState)
	{
		AFirstPlayerState* PlayerState = NewPlayer->GetPlayerState<AFirstPlayerState>();

		// 만일 지정 플레이어가 어느팀에도 속해있지 않다면
		if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			// 게임스테이트에서 확인된 각팀의 인원수를 비교하여 적은쪽 팀에 넣는다.
			if (PlayerGameState->BlueTeam.Num() >= PlayerGameState->RedTeam.Num())
			{
				PlayerGameState->RedTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				PlayerGameState->BlueTeam.AddUnique(PlayerState);
				PlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	// 게임스테이트 포인터
	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	// 플레이어스테이트 포인터
	AFirstPlayerState* PlayerState = Exiting->GetPlayerState<AFirstPlayerState>();

	if (PlayerGameState && PlayerState)
	{
		// 게임스테이트의 배열을 확인해서 떠나는 플레이어를 제거한다
		if (PlayerGameState->RedTeam.Contains(PlayerState))
		{
			PlayerGameState->RedTeam.Remove(PlayerState);
		}
		if (PlayerGameState->BlueTeam.Contains(PlayerState))
		{
			PlayerGameState->BlueTeam.Remove(PlayerState);
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AFirstPlayerState* AttackerPState = Attacker->GetPlayerState<AFirstPlayerState>();
	AFirstPlayerState* VictimPState = Victim->GetPlayerState<AFirstPlayerState>();
	
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;

	if (AttackerPState == VictimPState) 
	{
		return BaseDamage;
	}

	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		// 같은팀일경우 데미지 0반환
		return 0.f;
	}

	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
	// 슈퍼버전에서는 단순히 개인플레이어의 점수를 계산하고 HUD를 업데이트한다.
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	// 이후 해당 클래스에서는 팀점수의 계산을 실행한다.
	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	AFirstPlayerState* AttackerPlayerState = AttackerController ? Cast<AFirstPlayerState>(AttackerController->PlayerState) : nullptr;

	AFirstPlayerState* VictimPlayerState = VictimController ? Cast<AFirstPlayerState>(VictimController->PlayerState) : nullptr;

	// 게임상태와 공격한 플레이어의 상태가 검증되고
	if (PlayerGameState && AttackerPlayerState && VictimPlayerState)
	{
		if (AttackerPlayerState != VictimPlayerState)
		{
			// 공격한 플레이어의 팀 상태에따라 해당팀의 점수에 점수를 더한다.
			if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
			{
				PlayerGameState->BlueTeamScores();
			}
			if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
			{
				PlayerGameState->RedTeamScores();
			}
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	// 대기중이던 플레이어들이 일괄적으로 게임에 참가할 경우
	Super::HandleMatchHasStarted();

	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	if (PlayerGameState)
	{
		// 게임스테이트의 모든 플레이어 배열
		for (auto PState : PlayerGameState->PlayerArray)
		{
			// 플레이어스테이트의 포인터에 저장 이때 게임스테이트의->플레이어배열에는 플레이어 스테이트가 있으므로 캐스팅
			AFirstPlayerState* PlayerState = Cast<AFirstPlayerState>(PState.Get());

			// 만일 지정 플레이어가 어느팀에도 속해있지 않다면
			if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				// 게임스테이트에서 확인된 각팀의 인원수를 비교하여 적은쪽 팀에 넣는다.
				if (PlayerGameState->BlueTeam.Num() >= PlayerGameState->RedTeam.Num())
				{
					PlayerGameState->RedTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					PlayerGameState->BlueTeam.AddUnique(PlayerState);
					PlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

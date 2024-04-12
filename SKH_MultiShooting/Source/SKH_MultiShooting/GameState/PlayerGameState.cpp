#include "PlayerGameState.h"
#include "Net/UnrealNetwork.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"

void APlayerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 복제용 함수
	DOREPLIFETIME(APlayerGameState, TopScoringPlayers);
	DOREPLIFETIME(APlayerGameState, RedTeamScore);
	DOREPLIFETIME(APlayerGameState, BlueTeamScore);
}

void APlayerGameState::UpdateTopScore(AFirstPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		// 배열에 아무도 없을 경우 함수를 호출한 플레리어 추가
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		// 최다득점이 여러명일 경우 중복되지않도록 배열에 추가
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		// 만일 호출한 플레이어의 점수가 최고점보다 높다면 현재 리스트를 비워주고 갱신한다.
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void APlayerGameState::RedTeamScores()
{
	// 서버에서는 변수의 실값을 업데이트하고 플레이어(개인)의 HUD를 업데이트
	++RedTeamScore;

	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APlayerGameState::BlueTeamScores()
{
	// 서버에서는 변수의 실값을 업데이트하고 플레이어(개인)의 HUD를 업데이트
	++BlueTeamScore;

	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void APlayerGameState::OnRep_RedTeamScore()
{
	// 변수가 업데이트될때 클라이언트는 플레이어(개인)의 HUD를 업데이트
	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APlayerGameState::OnRep_BlueTeamScore()
{
	// 변수가 업데이트될때 클라이언트는 플레이어(개인)의 HUD를 업데이트
	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

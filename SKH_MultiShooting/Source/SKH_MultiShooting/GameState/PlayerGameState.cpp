#include "PlayerGameState.h"
#include "Net/UnrealNetwork.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"

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

void APlayerGameState::OnRep_RedTeamScore()
{

}

void APlayerGameState::OnRep_BlueTeamScore()
{

}

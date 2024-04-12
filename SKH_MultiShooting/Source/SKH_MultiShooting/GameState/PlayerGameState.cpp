#include "PlayerGameState.h"
#include "Net/UnrealNetwork.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"

void APlayerGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// ������ �Լ�
	DOREPLIFETIME(APlayerGameState, TopScoringPlayers);
	DOREPLIFETIME(APlayerGameState, RedTeamScore);
	DOREPLIFETIME(APlayerGameState, BlueTeamScore);
}

void APlayerGameState::UpdateTopScore(AFirstPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		// �迭�� �ƹ��� ���� ��� �Լ��� ȣ���� �÷����� �߰�
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		// �ִٵ����� �������� ��� �ߺ������ʵ��� �迭�� �߰�
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		// ���� ȣ���� �÷��̾��� ������ �ְ������� ���ٸ� ���� ����Ʈ�� ����ְ� �����Ѵ�.
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void APlayerGameState::RedTeamScores()
{
	// ���������� ������ �ǰ��� ������Ʈ�ϰ� �÷��̾�(����)�� HUD�� ������Ʈ
	++RedTeamScore;

	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APlayerGameState::BlueTeamScores()
{
	// ���������� ������ �ǰ��� ������Ʈ�ϰ� �÷��̾�(����)�� HUD�� ������Ʈ
	++BlueTeamScore;

	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void APlayerGameState::OnRep_RedTeamScore()
{
	// ������ ������Ʈ�ɶ� Ŭ���̾�Ʈ�� �÷��̾�(����)�� HUD�� ������Ʈ
	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APlayerGameState::OnRep_BlueTeamScore()
{
	// ������ ������Ʈ�ɶ� Ŭ���̾�Ʈ�� �÷��̾�(����)�� HUD�� ������Ʈ
	AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(GetWorld()->GetFirstPlayerController());

	if (PlayerController)
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

#include "TeamsGameMode.h"
#include "SKH_MultiShooting/GameState/PlayerGameState.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	// �θ�Ŭ������ ������ �缳��
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	// ���ο� �÷��̾ ���� ���� ���
	Super::PostLogin(NewPlayer);

	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	if (PlayerGameState)
	{
		AFirstPlayerState* PlayerState = NewPlayer->GetPlayerState<AFirstPlayerState>();

		// ���� ���� �÷��̾ ��������� �������� �ʴٸ�
		if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			// ���ӽ�����Ʈ���� Ȯ�ε� ������ �ο����� ���Ͽ� ������ ���� �ִ´�.
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
	// ���ӽ�����Ʈ ������
	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	// �÷��̾����Ʈ ������
	AFirstPlayerState* PlayerState = Exiting->GetPlayerState<AFirstPlayerState>();

	if (PlayerGameState && PlayerState)
	{
		// ���ӽ�����Ʈ�� �迭�� Ȯ���ؼ� ������ �÷��̾ �����Ѵ�
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
		// �������ϰ�� ������ 0��ȯ
		return 0.f;
	}

	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
	// ���۹��������� �ܼ��� �����÷��̾��� ������ ����ϰ� HUD�� ������Ʈ�Ѵ�.
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	// ���� �ش� Ŭ���������� �������� ����� �����Ѵ�.
	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	AFirstPlayerState* AttackerPlayerState = AttackerController ? Cast<AFirstPlayerState>(AttackerController->PlayerState) : nullptr;

	AFirstPlayerState* VictimPlayerState = VictimController ? Cast<AFirstPlayerState>(VictimController->PlayerState) : nullptr;

	// ���ӻ��¿� ������ �÷��̾��� ���°� �����ǰ�
	if (PlayerGameState && AttackerPlayerState && VictimPlayerState)
	{
		if (AttackerPlayerState != VictimPlayerState)
		{
			// ������ �÷��̾��� �� ���¿����� �ش����� ������ ������ ���Ѵ�.
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
	// ������̴� �÷��̾���� �ϰ������� ���ӿ� ������ ���
	Super::HandleMatchHasStarted();

	APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

	if (PlayerGameState)
	{
		// ���ӽ�����Ʈ�� ��� �÷��̾� �迭
		for (auto PState : PlayerGameState->PlayerArray)
		{
			// �÷��̾����Ʈ�� �����Ϳ� ���� �̶� ���ӽ�����Ʈ��->�÷��̾�迭���� �÷��̾� ������Ʈ�� �����Ƿ� ĳ����
			AFirstPlayerState* PlayerState = Cast<AFirstPlayerState>(PState.Get());

			// ���� ���� �÷��̾ ��������� �������� �ʴٸ�
			if (PlayerState && PlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				// ���ӽ�����Ʈ���� Ȯ�ε� ������ �ο����� ���Ͽ� ������ ���� �ִ´�.
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

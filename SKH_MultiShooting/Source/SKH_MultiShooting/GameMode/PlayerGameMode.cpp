#include "PlayerGameMode.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"
#include "SKH_MultiShooting/GameState/PlayerGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}


APlayerGameMode::APlayerGameMode()
{
	bDelayedStart = true; 
}

void APlayerGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void APlayerGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	// ��� �÷��̾� ��Ʈ�ѷ��� Ȯ���ϴ� �ݺ���
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(*It);
		if (PlayerController)
		{
			// ��ӹ޴� MatchState ����
			PlayerController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

void APlayerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		// ���ӽ��� �ð��� ����Ÿ�� �ð����� ���̰� 0���� �۾������ ���ӻ��¸� StartMatch�� ����
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			// MatchState::InProgress �� �������
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		// ���ð�+���ӽð�+���α׷�������۽ð����� ���α׷��� �����ð��� ��� ���ְ� �̶� ī��Ʈ�ٿ�ð��� 0�� �ɶ� ������ ���� ��,Cooldown ���·� ��ġ�� �����Ѵ�.
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		// ������ ������ Cooldown ��ġ ���¿��� ������ �ð��� ���� ī��Ʈ�ٿ��� 0�̵ɶ� ����÷��̾�� RestartGame ��, ���� ������ �÷������� ������ ���� �ٽ� �ε��Ѵ�. ����÷��̾� ��ü�� �ı��ϰ� �ٽû���. �ʱⰪ���� �ٽü��õ�
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void APlayerGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
	AFirstPlayerState* AttackerPlayerState = AttackerController ? Cast<AFirstPlayerState>(AttackerController->PlayerState) : nullptr;

	AFirstPlayerState* VictimPlayerState = VictimController ? Cast<AFirstPlayerState>(VictimController->PlayerState) : nullptr;

	APlayerGameState* PlayerGameState = GetGameState<APlayerGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && PlayerGameState)
	{
		//�ӽù迭�� �÷��̾��� ���� ���� ����Ʈ�� �����Ѵ�.
		TArray<AFirstPlayerState*> PlayersCurrentlyInTheLead;

		for (auto LeadPlayer : PlayerGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		// �÷��̾ �ڻ��̾ƴ� �ٸ� �÷��̾ óġ������ 1���� �߰� ��Ų��.
		AttackerPlayerState->AddToScore(1.f);

		// �ִ���� ����(�ش��÷��̾ Ż���ñ� �÷��̾�)
		PlayerGameState->UpdateTopScore(AttackerPlayerState);

		if (PlayerGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			APlayerCharacter* Leader = Cast<APlayerCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}


		// �÷��̾��� ������ ���ŵǰ��� �迭�� ����Ǿ��ִ� �÷��̾���� �ִٵ��� ����Ʈ�� ��Ȱ��ȭ�Ѵ�.
		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{

			if (!PlayerGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				APlayerCharacter* Loser = Cast<APlayerCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}

	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	// ����� ĳ���Ϳ� �����Ų ĳ���͸� �Ű������� �ް� ����� ĳ���͸� Ż�� ó���� �� �ֵ���

	if (ElimmedCharacter)
	{
		// ���ݴ��Ͽ� ó���� �÷��̾�� Elim �Ű������� false ����.
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFirstPlayerController* FirstPlayer = Cast<AFirstPlayerController>(*It);
		if (FirstPlayer && AttackerPlayerState && VictimPlayerState)
		{
			FirstPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void APlayerGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		// ���ǻ��¸� �����ϰ� �ʱ�ȭ ��Ų��
		ElimmedCharacter->Reset();
		// ���� ����
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		// ���ӻ��ִ� ��� �÷��̾� ��ŸƮ ������ �迭�� �����ϰ� �ش�迭�� ������ ��ü���� �÷��̾ ���� ��Ų��.
		TArray<AActor*> PlayerStartsPoint;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartsPoint);
		int32 Selection = FMath::RandRange(0, PlayerStartsPoint.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStartsPoint[Selection]);
	}
}

void APlayerGameMode::PlayerLeftGame(class AFirstPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;

	APlayerGameState* PlayerGameState = GetGameState<APlayerGameState>();
	if (PlayerGameState && PlayerGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		// �÷��̾ ������ ��� �÷��̾��� ������Ͽ��� �ش��÷��̾ �����ؾ��Ѵ�. ���������ʰ� ������� ���߿� nullptr �� ���������Ƿ�.
		PlayerGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	APlayerCharacter* CharacterLeaving = Cast<APlayerCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		// ������ �÷��̾�� Elim�� �����Ų��.
		CharacterLeaving->Elim(true);
	}
}

float APlayerGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

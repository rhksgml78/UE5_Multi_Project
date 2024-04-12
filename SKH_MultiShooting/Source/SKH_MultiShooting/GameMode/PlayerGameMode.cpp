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

	// 모든 플레이어 컨트롤러를 확인하는 반복문
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFirstPlayerController* PlayerController = Cast<AFirstPlayerController>(*It);
		if (PlayerController)
		{
			// 상속받는 MatchState 변수
			PlayerController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

void APlayerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		// 게임실행 시간이 웜업타임 시간과의 차이가 0보다 작아질경우 게임상태를 StartMatch로 변경
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			// MatchState::InProgress 로 변경실행
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		// 대기시간+게임시간+프로그램실행시작시간에서 프로그램의 가동시간을 계속 빼주고 이때 카운트다운시간이 0이 될때 게임의 종료 즉,Cooldown 상태로 매치를 변경한다.
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		// 게임이 종료후 Cooldown 매치 상태에서 지정한 시간이 지나 카운트다운이 0이될때 모든플레이어는 RestartGame 즉, 현재 게임을 플레이중인 레벨을 전부 다시 로드한다. 모든플레이어 객체를 파괴하고 다시생성. 초기값으로 다시세팅됨
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
		//임시배열에 플레이어의 점수 순서 리스트를 저장한다.
		TArray<AFirstPlayerState*> PlayersCurrentlyInTheLead;

		for (auto LeadPlayer : PlayerGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		// 플레이어가 자살이아닌 다른 플레이어를 처치했을때 1점을 추가 시킨다.
		AttackerPlayerState->AddToScore(1.f);

		// 최대득점 갱신(해당플레이어를 탈락시긴 플레이어)
		PlayerGameState->UpdateTopScore(AttackerPlayerState);

		if (PlayerGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			APlayerCharacter* Leader = Cast<APlayerCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}


		// 플레이어의 득점이 갱신되고나서 배열에 저장되어있던 플레이어들의 최다득점 이펙트를 비활성화한다.
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
	// 사망한 캐릭터와 사망시킨 캐릭터를 매개변수로 받고 사망한 캐릭터를 탈락 처리할 수 있도록

	if (ElimmedCharacter)
	{
		// 공격당하여 처리된 플레이어는 Elim 매개변수로 false 전달.
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
		// 빙의상태를 해제하고 초기화 시킨뒤
		ElimmedCharacter->Reset();
		// 제거 진행
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		// 게임상에있는 모든 플레이어 스타트 지점을 배열에 저장하고 해당배열중 랜덤한 객체에서 플레이어를 스폰 시킨다.
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
		// 플레이어가 떠날때 모든 플레이어의 점수목록에서 해당플레이어를 제거해야한다. 제거하지않고 떠날경우 나중에 nullptr 이 남아있으므로.
		PlayerGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	APlayerCharacter* CharacterLeaving = Cast<APlayerCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		// 떠나는 플레이어는 Elim을 실행시킨다.
		CharacterLeaving->Elim(true);
	}
}

float APlayerGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

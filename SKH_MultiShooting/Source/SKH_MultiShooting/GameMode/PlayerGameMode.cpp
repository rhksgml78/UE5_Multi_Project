#include "PlayerGameMode.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"

APlayerGameMode::APlayerGameMode()
{
	bDelayedStart = true; 
}

void APlayerGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void APlayerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		// 게임실행 시간이 웜업타임 시간과의 차이가 0보다 작아질경우 게임상태를 StartMatch로 변경
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime < 0.f)
		{
			// MatchState::InProgress
			StartMatch();
		}
	}
}

void APlayerGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
	AFirstPlayerState* AttackerPlayerState = AttackerController ? Cast<AFirstPlayerState>(AttackerController->PlayerState) : nullptr;

	AFirstPlayerState* VictimPlayerState = VictimController ? Cast<AFirstPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		// 플레이어가 자살이아닌 다른 플레이어를 처치했을때 1점을 추가 시킨다.
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	// 사망한 캐릭터와 사망시킨 캐릭터를 매개변수로 받고 사망한 캐릭터를 탈락 처리할 수 있도록

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
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

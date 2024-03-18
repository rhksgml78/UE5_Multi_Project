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
		// ���ӽ��� �ð��� ����Ÿ�� �ð����� ���̰� 0���� �۾������ ���ӻ��¸� StartMatch�� ����
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
		// �÷��̾ �ڻ��̾ƴ� �ٸ� �÷��̾ óġ������ 1���� �߰� ��Ų��.
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	// ����� ĳ���Ϳ� �����Ų ĳ���͸� �Ű������� �ް� ����� ĳ���͸� Ż�� ó���� �� �ֵ���

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
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

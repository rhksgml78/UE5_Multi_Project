#include "PlayerGameMode.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void APlayerGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
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

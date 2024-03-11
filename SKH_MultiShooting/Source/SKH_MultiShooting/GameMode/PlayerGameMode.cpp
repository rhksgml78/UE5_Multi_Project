#include "PlayerGameMode.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void APlayerGameMode::PlayerEliminated(APlayerCharacter* ElimmedCharacter, AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController)
{
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

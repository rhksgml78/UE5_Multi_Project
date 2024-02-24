#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// ���ӿ� ������ �ο� Ȯ�� ����
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/ Game / Map / MainPlayerMap?listen"));
		}
	}

}

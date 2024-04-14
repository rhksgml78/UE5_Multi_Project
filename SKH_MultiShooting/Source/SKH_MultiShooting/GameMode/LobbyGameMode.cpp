#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 게임에 참가한 인원 확인 가능
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	// 서브시스템에 접근하여 데이터 확인
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

		// 검증
		check(Subsystem);

		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;

				FString MatchType = Subsystem->DesiredMatchType;
				if (MatchType == "FreeForAll")
				{
					// 만일 플레이할 맵이 각각 다르다면 엔진에서 생성한 맵의 이름으로 수정해야 한다.
					World->ServerTravel(FString("/Game/Maps/MainPlayerMap?listen"));
				}
				else if (MatchType == "Teams")
				{
					// 만일 플레이할 맵이 각각 다르다면 엔진에서 생성한 맵의 이름으로 수정해야 한다.
					World->ServerTravel(FString("/Game/Maps/MainPlayerMap?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					// 만일 플레이할 맵이 각각 다르다면 엔진에서 생성한 맵의 이름으로 수정해야 한다.
					World->ServerTravel(FString("/Game/Maps/MainPlayerMap?listen"));
				}
			}
		}
	}
}

#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// ���ӿ� ������ �ο� Ȯ�� ����
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	// ����ý��ۿ� �����Ͽ� ������ Ȯ��
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

		// ������ if���ƴ� check�� ����Ͽ� ��ġŸ���� ��ġ�������� ��� ���α׷� ��������
		check(Subsystem);

		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;

				FString MatchType = Subsystem->DesiredMatchType;
				if (MatchType == "FreeForAll") // ������ 
				{
					// ���� �÷����� ���� ���� �ٸ��ٸ� �������� ������ ���� �̸����� �����ؾ� �Ѵ�.
					World->ServerTravel(FString("/Game/Maps/Map_SinglePlay?listen"));
				}
				else if (MatchType == "Teams") // ����
				{
					// ���� �÷����� ���� ���� �ٸ��ٸ� �������� ������ ���� �̸����� �����ؾ� �Ѵ�.
					World->ServerTravel(FString("/Game/Maps/Map_TeamPlay?listen"));
				}
				else if (MatchType == "CaptureTheFlag") // ���� ��߻���
				{
					// ���� �÷����� ���� ���� �ٸ��ٸ� �������� ������ ���� �̸����� �����ؾ� �Ѵ�.
					World->ServerTravel(FString("/Game/Maps/Map_TeamFlag?listen"));
				}
			}
		}
	}
}

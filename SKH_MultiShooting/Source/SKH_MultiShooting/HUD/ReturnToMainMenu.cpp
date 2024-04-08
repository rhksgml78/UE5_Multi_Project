#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"


bool UReturnToMainMenu::Initialize()
{
	// 자동 초기화함수

	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (ReturnButton)
	{
		// 버튼 클릭이벤트에 바인딩
		ReturnButton->OnClicked.AddDynamic(this, &ThisClass::ReturnButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			// 세션이 파괴될때 실행할 함수 바인딩
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	// 플레이어의 세션을 파괴하고 메뉴로 돌아가야 한다.
	if (MultiplayerSessionsSubsystem)
	{
		// 세션서브시스템의 디스트로이세션을 실행하면 MultiplayerOnDestroySessionComplete가 호출될때 바인딩된 OnDestroySession 함수가 실행된다.
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	// 서버 클라이언트 모두에서 호출될 수 있기 떄문에 경우를 나누어야함
	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			// 서버의 게임모드 베이스에잇는 함수 실행
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			// 서버가 아닌 클라이언트일경우 컨트롤러에 접근한다.
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				// 언리얼엔진에서는 ClientReturnToMainMenu 보다는 ClientReturnToMainMenuWithTextReason를 권장한다고함.
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}
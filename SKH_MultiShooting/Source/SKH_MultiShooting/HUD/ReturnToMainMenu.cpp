#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"


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
			// 메뉴위젯이 생성될경우 마우스 커서가 활성화되고 작동해야함
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (ReturnButton && !
		ReturnButton->OnClicked.IsBound())
	{
		// 버튼 클릭이벤트에 바인딩 (한번만)
		ReturnButton->OnClicked.AddDynamic(this, &ThisClass::ReturnButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			// 서브시스템에 이미 바인딩이 된 작업이있으므로 해당 바인딩을 지우고 다시 바인딩
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.Clear();

			//MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &ThisClass::OnDestroySession);

			//조건문에 &&!MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound() 제외
			
			// 세션이 파괴될때 실행할 함수 바인딩 (한번만) 반환값은 bool형
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
			// 마우스커서를 비활성화하고 위젯과 상호작용하지 않도록 재설정
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && 
		ReturnButton->OnClicked.IsBound())
	{
		// 버튼 클릭이벤트 바인딩 해제
		ReturnButton->OnClicked.RemoveDynamic(this, &ThisClass::ReturnButtonClicked);
	}

	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		// 세션 바인딩 해제
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &ThisClass::OnDestroySession);
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
			// 서버가 아닌 클라이언트일경우 각플레이어의 컨트롤러에 접근해야 한다.
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				// 언리얼엔진에서는 ClientReturnToMainMenu 보다는 ClientReturnToMainMenuWithTextReason를 권장한다고함.
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	// 버튼이 클릭되면 버튼을 비활성화 하고
	ReturnButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
		// 월드상의 플레이어(본인)의 컨트롤러에 접근한다.
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();

		if (FirstPlayerController)
		{
			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FirstPlayerController->GetPawn());

			if (PlayerCharacter)
			{
				// 서버RPC 함수를 실행한다.
				PlayerCharacter->ServerLeavGame();

				// 플레이어 캐릭터의 델리게이트 OnLeftGame에 함수를 바인딩 시킨다. 이후 플레이어가 Elim타이머가 끝났을때 Broadcast를 실행하면서 OnPlayerLeftGame이 호출된다.
				PlayerCharacter->OnLeftGame.AddDynamic(this, &ThisClass::OnPlayerLeftGame);
			}
			else
			{
				// 플레이어가 nullptr 즉 캐스팅이 안된경우 버튼을 다시 활성화 한다.
				ReturnButton->SetIsEnabled(true);
			}
		}
	}

}

void UReturnToMainMenu::OnPlayerLeftGame()
{
	// 플레이어의 세션을 파괴하고 메뉴로 돌아가야 한다.
	if (MultiplayerSessionsSubsystem)
	{
		// 세션서브시스템의 디스트로이세션을 실행하면 MultiplayerOnDestroySessionComplete가 호출될때 바인딩된 OnDestroySession 함수가 실행된다.
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

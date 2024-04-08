#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"


bool UReturnToMainMenu::Initialize()
{
	// �ڵ� �ʱ�ȭ�Լ�

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
			// �޴������� �����ɰ�� ���콺 Ŀ���� Ȱ��ȭ�ǰ� �۵��ؾ���
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (ReturnButton && !
		ReturnButton->OnClicked.IsBound())
	{
		// ��ư Ŭ���̺�Ʈ�� ���ε� (�ѹ���)
		ReturnButton->OnClicked.AddDynamic(this, &ThisClass::ReturnButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem && 
			!MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
		{
			// ������ �ı��ɶ� ������ �Լ� ���ε� (�ѹ���) ��ȯ���� bool��
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
			// ���콺Ŀ���� ��Ȱ��ȭ�ϰ� ������ ��ȣ�ۿ����� �ʵ��� �缳��
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && 
		ReturnButton->OnClicked.IsBound())
	{
		// ��ư Ŭ���̺�Ʈ ���ε� ����
		ReturnButton->OnClicked.RemoveDynamic(this, &ThisClass::ReturnButtonClicked);
	}

	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		// ���� ���ε� ����
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &ThisClass::OnDestroySession);
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	// �÷��̾��� ������ �ı��ϰ� �޴��� ���ư��� �Ѵ�.
	if (MultiplayerSessionsSubsystem)
	{
		// ���Ǽ���ý����� ��Ʈ���̼����� �����ϸ� MultiplayerOnDestroySessionComplete�� ȣ��ɶ� ���ε��� OnDestroySession �Լ��� ����ȴ�.
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

	// ���� Ŭ���̾�Ʈ ��ο��� ȣ��� �� �ֱ� ������ ��츦 ���������
	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			// ������ ���Ӹ�� ���̽����մ� �Լ� ����
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			// ������ �ƴ� Ŭ���̾�Ʈ�ϰ�� ���÷��̾��� ��Ʈ�ѷ��� �����ؾ� �Ѵ�.
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				// �𸮾��������� ClientReturnToMainMenu ���ٴ� ClientReturnToMainMenuWithTextReason�� �����Ѵٰ���.
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}
#include "FirstPlayerController.h"
#include "SKH_MultiShooting/HUD/PlayerHUD.h"
#include "SKH_MultiShooting/HUD/PlayerOverlay.h"
#include "SKH_MultiShooting/HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "SKH_MultiShooting/GameMode/PlayerGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "SKH_MultiShooting/PlayerComponents/CombatComponent.h"
#include "SKH_MultiShooting/GameState/PlayerGameState.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "SKH_MultiShooting/HUD/ReturnToMainMenu.h"

void AFirstPlayerController::BeginPlay()
{
	Super::BeginPlay();
	PlayerHUD = Cast<APlayerHUD>(GetHUD());
	ServerCheckMatchState();
}

void AFirstPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// ��� "Net/UnrealNetwork.h" �߰� �ʿ�
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirstPlayerController, MatchState);
}

void AFirstPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	// �÷��̾��� �Է�������Ʈ�� Ȱ��ȭ �Ǿ����� Ű�Է� ���ε�.
	InputComponent->BindAction("Quit", IE_Pressed, this, &ThisClass::ShowReturnToMainMenu);
}

void AFirstPlayerController::ShowReturnToMainMenu()
{
	// ���θ޴� ���� Ȱ��ȭ
	if (ReturnToMainMenuWidget == nullptr) return;

	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		// �Լ�������ɶ�(Ű�Է���������) True & False ����.
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			// �޴����� ON
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			// �޴����� OFF
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AFirstPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// HUD�� �ð��� ����
	SetHUDTime();

	// �������� ����ȭ �ð� üũ 
	CheckTimeSync(DeltaTime);
	PollInit();

	// �÷��̾��� Ping�� üũ�ϰ� ���� �ִϸ��̼��� ���
	CheckPing(DeltaTime);
}

void AFirstPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			// GetPing�� ����� ���̱⶧���� 1/4 ������ ���� ��Եȴ�.(uint8) ������ uint32�� ���� ����� 4���� ������ ��´�.
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRuningTime = 0.f; // �ʱ�ȭ

				// ������ ����
				ServerReportPingStatus(true);
			}
			else
			{
				// ������ ����
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0;
	}

	bool bHighPingAnimationPlaying = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->HighPingAnimation &&
		PlayerHUD->PlayerOverlay->IsAnimationPlaying(PlayerHUD->PlayerOverlay->HighPingAnimation);

	// �λ������� �ִϸ��̼��� ������̶��
	if (bHighPingAnimationPlaying)
	{
		// ����ð��� �����ϰ�
		PingAnimationRuningTime += DeltaTime;

		if (PingAnimationRuningTime > HighPingDuration)
		{
			// ���ؽð��� �ʰ��Ͽ������ �ִϸ��̼� ����
			StopHighPingWarning();
		}
	}
}

void AFirstPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AFirstPlayerController::CheckTimeSync(float DeltaTime)
{
	// ������ Ŭ���̾�Ʈ�� �ð� ����ȭ
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		// ����ȭ �ð��� ��Ӵ����ִٰ� ������ �ð��� �Ѿ����� ����ȭ�� �ѹ��ϰ� 0���� �ʱ�ȭ
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AFirstPlayerController::HighPingWarning()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->HighPingImage &&
		PlayerHUD->PlayerOverlay->HighPingText &&
		PlayerHUD->PlayerOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		PlayerHUD->PlayerOverlay->HighPingImage->SetOpacity(1.f);
		PlayerHUD->PlayerOverlay->HighPingText->SetOpacity(1.f);
		PlayerHUD->PlayerOverlay->PlayAnimation(
			PlayerHUD->PlayerOverlay->HighPingAnimation,
			0.f,
			5
			);
	}
}

void AFirstPlayerController::StopHighPingWarning()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->HighPingImage &&
		PlayerHUD->PlayerOverlay->HighPingText &&
		PlayerHUD->PlayerOverlay->HighPingAnimation;

	if (bHUDValid)
	{
		PlayerHUD->PlayerOverlay->HighPingImage->SetOpacity(0.f);
		PlayerHUD->PlayerOverlay->HighPingText->SetOpacity(0.f);
		if (PlayerHUD->PlayerOverlay->IsAnimationPlaying(PlayerHUD->PlayerOverlay->HighPingAnimation))
		{
			PlayerHUD->PlayerOverlay->StopAnimation(PlayerHUD->PlayerOverlay->HighPingAnimation);
		}
	}
}

void AFirstPlayerController::ServerCheckMatchState_Implementation()
{
	// �������� �����ϴ� �۾�
	APlayerGameMode* GameMode = Cast<APlayerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();

		// Ŭ���̾�Ʈ�� ���� ����
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void AFirstPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	// Ŭ���̾�Ʈ���� �����ϴ� �۾�
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (PlayerHUD && MatchState == MatchState::WaitingToStart)
	{
		// ����� �ȳ� ���� ����
		PlayerHUD->AddAnnouncement();
	}
}

void AFirstPlayerController::ServerRequestServerTime_Implementation(float TimOfClientRequest)
{
	// ������ ���� �ð� ���� ����
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();

	ClientReportServerTime(TimOfClientRequest, ServerTimeOfReceipt);
}

void AFirstPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	// ��û���ۿ��ҿ��� �ð��� ����Ѵ�.
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	
	// ��û���� �� Ÿ��
	SingleTripTime = 0.5f * RoundTripTime;

	// Ŭ���̾�Ʈ�� ������ ��û�� ������ �������� Ŭ���̾�Ʈ�� ȸ���ϴ� �ð��� Ŭ���̾�Ʈ�� ��û~ȸ�� �� �ѽð��� ������ ���� �ð����� �� �� �ִ�.
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();

}

float AFirstPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		// �����ϰ�� ������ �ð��� �״�� ��ȯ
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		// Ŭ���̾�Ʈ�ϰ�� ���̰��� ���� �ð��� ��ȯ
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

void AFirstPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AFirstPlayerController::OnPossess(APawn* InPawn)
{
	// �÷��̾ �ٽ� �����ɋ�(�����) �ڵ����� ���� �Ǹ鼭 �ѹ� ������ �ȴ�.

	Super::OnPossess(InPawn);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(InPawn);

	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
	}
}

void AFirstPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD && 
		PlayerHUD->PlayerOverlay && 
		PlayerHUD->PlayerOverlay->HealthBar && 
		PlayerHUD->PlayerOverlay->HealthText;


	if (bHUDValid)
	{
		// ü�¹� ������ ����
		const float HealthPercent = Health / MaxHealth;
		PlayerHUD->PlayerOverlay->HealthBar->SetPercent(HealthPercent);

		// ü�¹� �ؽ�Ʈ ����(��Ʈ������ ������ �ؽ�Ʈ��)
		if (Health > 0)
		{
			FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
			PlayerHUD->PlayerOverlay->HealthText->SetText(FText::FromString(HealthText));
		}
		else
		{
			FString HealthText = FString::Printf(TEXT("Death!"));
			PlayerHUD->PlayerOverlay->HealthText->SetText(FText::FromString(HealthText));
		}
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AFirstPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->ShieldBar &&
		PlayerHUD->PlayerOverlay->ShieldText;


	if (bHUDValid)
	{
		// ü�¹� ������ ����
		const float ShieldPercent = Shield / MaxShield;
		PlayerHUD->PlayerOverlay->ShieldBar->SetPercent(ShieldPercent);

		// ü�¹� �ؽ�Ʈ ����(��Ʈ������ ������ �ؽ�Ʈ��)
		/*
		�ؽ�Ʈ ���� [���罯��/�ִ뽯��]
				FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		PlayerHUD->PlayerOverlay->ShieldText->SetText(FText::FromString(ShieldText));
		*/

		if (Shield > 0)
		{
			// �ؽ�Ʈ ���� Armor : ���� ����
			FString ShieldText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Shield));
			PlayerHUD->PlayerOverlay->ShieldText->SetText(FText::FromString(ShieldText));
		}
		else
		{
			// �ؽ�Ʈ ���� Armor : ���� ����
			FString ShieldText = FString::Printf(TEXT("Broken!"));
			PlayerHUD->PlayerOverlay->ShieldText->SetText(FText::FromString(ShieldText));
		}

	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void AFirstPlayerController::SetHUDScore(float Score)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score) );
		PlayerHUD->PlayerOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AFirstPlayerController::SetHUDDefeats(int32 Defeats)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		PlayerHUD->PlayerOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AFirstPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->PlayerOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AFirstPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->PlayerOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}
void AFirstPlayerController::PlayDefeatsAnimation()
{
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay;
	if (bHUDValid)
	{
		// ����� HUD �ִϸ��̼� BP���� �÷���
		PlayerHUD->PlayerOverlay->PlayDeafeats();
	}
}

void AFirstPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			// �ð��� - ǥ�Ⱑ ���� �ʵ��� 0���� �۾������ FText() ����ִ� �ؽ�Ʈ�� ����Ѵ�. ������ �۾��� ���� �ʵ��� ���� reutrn ����
			PlayerHUD->PlayerOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);

		// 2�ڸ����� ����
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->PlayerOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFirstPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->Announcement &&
		PlayerHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			// �ð��� - ǥ�Ⱑ ���� �ʵ��� 0���� �۾������ FText() ����ִ� �ؽ�Ʈ�� ����Ѵ�. ������ �۾��� ���� �ʵ��� ���� reutrn ����
			PlayerHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);

		// 2�ڸ����� ����
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AFirstPlayerController::SetHUDGrenades(int32 Grenades)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->GrenadeAmount;

	if (bHUDValid)
	{
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
		PlayerHUD->PlayerOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void AFirstPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondLeft = FMath::CeilToInt(TimeLeft);

	// ������ ��� ����
	//if (HasAuthority())
	//{
	//	PlayerGameMode = PlayerGameMode == nullptr ? Cast<APlayerGameMode>(UGameplayStatics::GetGameMode(this)) : PlayerGameMode;
	//	if (PlayerGameMode)
	//	{
	//		SecondLeft = FMath::CeilToInt(PlayerGameMode->GetCountdownTime() + LevelStartingTime);
	//	}
	//}

	if (CountDownInt != SecondLeft)
	{
		if (MatchState == MatchState::WaitingToStart ||
			MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}		
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountDownInt = SecondLeft;
}

void AFirstPlayerController::PollInit()
{
	if (PlayerOverlay == nullptr)
	{
		if (PlayerHUD && PlayerHUD->PlayerOverlay)
		{
			PlayerOverlay = PlayerHUD->PlayerOverlay;
			if (PlayerOverlay)
			{
				// �� HUD ������Ʈ�� �ʱ�ȭ�� �̷�������� Ȯ���Ŀ� �������� ������Ʈ�� ����ȴ�.
				if (bInitializeHealth)
				{
					SetHUDHealth(HUDHealth, HUDMaxHealth);
				}
				if (bInitializeShield)
				{
					SetHUDShield(HUDShield, HUDMaxShield);
				}
				if (bInitializeScore)
				{
					SetHUDScore(HUDScore);
				}
				if (bInitializeDefeats)
				{
					SetHUDDefeats(HUDDefeats);
				}
				if (bInitializeWeaponAmmo)
				{
					SetHUDWeaponAmmo(HUDWeaponAmmo);
				}
				if (bInitializeCarriedAmmo)
				{
					SetHUDCarriedAmmo(HUDCarriedAmmo);
				}

				APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
				if (PlayerCharacter && PlayerCharacter->GetCombat())
				{
					if (bInitializeGrenades)
					{
						SetHUDGrenades(PlayerCharacter->GetCombat()->GetGrenades());
					}
				}
			}
		}
	}
}

void AFirstPlayerController::OnMatchStateSet(FName State)
{
	// �÷��̾� ���Ӹ�� Ŭ�������� �ش� �Լ��� ȣ���Ͽ� ��ġ ������Ʈ�� �����ϰ�����. �������� ������ �����̱� ������ OnRep_MatchState �Լ��� ����

	// ���� ����
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AFirstPlayerController::OnRep_MatchState()
{
	// Ŭ���̾�Ʈ ����
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AFirstPlayerController::HandleMatchHasStarted()
{
	// ���ӽ����
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		// �÷��̾��� UI�����ϱ�
		if (PlayerHUD->PlayerOverlay == nullptr)
		{
			PlayerHUD->AddCharacterOverlay();
		}
		if (PlayerHUD->Announcement)
		{
			// ����� �ȳ� ������ �����
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFirstPlayerController::HandleCooldown()
{
	// ���������� ��ٿ� ���°� ������
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		// �÷��̾��� UI�����ϱ�
		PlayerHUD->PlayerOverlay->RemoveFromParent();
		
		bool bHUDValid = PlayerHUD->Announcement &&
			PlayerHUD->Announcement->AnnouncementText &&
			PlayerHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			// ����� �ȳ� ���� �ٽ� ǥ���ϱ�
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Visible);

			FString AnnouncementText("NEWGAME LOADING");

			PlayerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

			AFirstPlayerState* FirstPlayerState = GetPlayerState<AFirstPlayerState>();

			if (PlayerGameState && FirstPlayerState)
			{
				TArray<AFirstPlayerState*> TopPlayers = PlayerGameState->TopScoringPlayers;

				FString InfoTextString;

				if (TopPlayers.Num() == 0)
				{
					// �ִ� �����ڰ� ������� (���ӽ����� �״����� �÷��̾ ������ �������)
					InfoTextString = FString("No Winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == FirstPlayerState)
				{
					// ���� �÷������� �÷��̾ �ִٵ������� ��� ���� �÷��̾�� �������� ǥ����
					InfoTextString = FString("You Win!");
				}
				else if (TopPlayers.Num() == 1)
				{
					// �����÷��̾�ƴ� �ٸ� �������� �÷��̾ �ִ� ������ �ܵ����� �¸��Ͽ������ �ش� �÷��̾��� �̸��� ����Ѵ�.
					InfoTextString = FString::Printf(TEXT("Winner\n%s"), *TopPlayers[0]->GetPlayerName());

				}
				else if (TopPlayers.Num() > 1)
				{
					// �ִٵ����� �������� ���
					InfoTextString = FString("TopScores\n");

					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}

				// UI�� INFO���� ��µǴ� �ؽ�Ʈ ����
				PlayerHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));

			}
		}
	}
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	if (PlayerCharacter && PlayerCharacter->GetCombat())
	{
		// �÷��̾� ĳ������ �Է��� �Ұ����ϰ� �ϴ� ���� ����
		PlayerCharacter->bDisableGameplay = true;

		// ĳ���Ͱ� �߻��ϰ� �ִµ��� ��ٿ� ���°� �ɰ�� �Ѿ��� �� ������������ �߻���ä�� �ְԵǹǷ� ������Ʈ�� �����Ͽ� �߻縦 �ߴ� ��Ų��.
		PlayerCharacter->GetCombat()->FireButtonPressed(false);
	}
}

void AFirstPlayerController::SetSpeedUi(bool isVisible)
{
	if (PlayerOverlay)
	{
		if (isVisible == true)
		{
			PlayerOverlay->SpeedUpText->SetVisibility(ESlateVisibility::Visible);
			PlayerOverlay->SpeedUpUi->SetVisibility(ESlateVisibility::Visible);
		}
		else if (isVisible == false)
		{
			PlayerOverlay->SpeedUpText->SetVisibility(ESlateVisibility::Hidden);
			PlayerOverlay->SpeedUpUi->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFirstPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	// ������ Ŭ���̾�Ʈ�� ���� ����
	ClientElimAnnouncement(Attacker, Victim);
}

void AFirstPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	// ��� Ŭ���̾�Ʈ�� �����ȴ�.
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

		if (PlayerHUD)
		{
			// ���� �ٸ��÷��̾ óġ
			if (Attacker == Self && Victim != Self)
			{
				PlayerHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}

			// �ٸ��÷��̾ ���� óġ
			if (Attacker != Self && Victim == Self)
			{
				PlayerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}

			// ���� ���� óġ
			if (Attacker == Self && Attacker == Victim)
			{
				PlayerHUD->AddElimAnnouncement("You", "Self");
				return;
			}

			// �ٸ��÷��̾ ���� ������ óġ
			if (Attacker != Self && Attacker == Victim)
			{
				PlayerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Self");
				return;
			}

			// �÷��̾� �̿��� �÷��̾�鳢�� óġ
			PlayerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}
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

void AFirstPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();

	// �������� ����ȭ �ð� üũ 
	CheckTimeSync(DeltaTime);
	PollInit();

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

	// Ŭ���̾�Ʈ�� ������ ��û�� ������ �������� Ŭ���̾�Ʈ�� ȸ���ϴ� �ð��� Ŭ���̾�Ʈ�� ��û~ȸ�� �� �ѽð��� ������ ���� �ð����� �� �� �ִ�.
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);

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
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PlayerHUD->PlayerOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializePlayerOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
		bInitializePlayerOverlay = true;
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
		bInitializePlayerOverlay = true;
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
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
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
		PlayerHUD->AddCharacterOverlay();
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
			FString InfomationText("BEST PLAYER");

			PlayerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			PlayerHUD->Announcement->InfoText->SetText(FText::FromString(InfomationText));
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

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
	// 헤더 "Net/UnrealNetwork.h" 추가 필요
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirstPlayerController, MatchState);
}

void AFirstPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();

	// 매프레임 동기화 시간 체크 
	CheckTimeSync(DeltaTime);
	PollInit();

}

void AFirstPlayerController::CheckTimeSync(float DeltaTime)
{
	// 서버와 클라이언트의 시간 동기화
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		// 동기화 시간을 계속더해주다가 지정한 시간을 넘었을때 동기화를 한번하고 0으로 초기화
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AFirstPlayerController::ServerCheckMatchState_Implementation()
{
	// 서버에서 진행하는 작업
	APlayerGameMode* GameMode = Cast<APlayerGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();

		// 클라이언트에 정보 전달
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void AFirstPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	// 클라이언트에서 진행하는 작업
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (PlayerHUD && MatchState == MatchState::WaitingToStart)
	{
		// 대기중 안내 위젯 생성
		PlayerHUD->AddAnnouncement();
	}
}

void AFirstPlayerController::ServerRequestServerTime_Implementation(float TimOfClientRequest)
{
	// 서버의 현재 시간 값을 저장
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();

	ClientReportServerTime(TimOfClientRequest, ServerTimeOfReceipt);
}

void AFirstPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	// 요청전송에소요한 시간을 계산한다.
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;

	// 클라이언트가 서버에 요청을 보내고 서버에서 클라이언트로 회신하는 시간은 클라이언트가 요청~회신 한 총시간을 반으로 나눈 시간으로 볼 수 있다.
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();

}

float AFirstPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		// 서버일경우 서버의 시간을 그대로 반환
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		// 클라이언트일경우 차이값을 더한 시간을 반환
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
	// 플레이어가 다시 생성될떄(사망후) 자동으로 빙의 되면서 한번 실행이 된다.

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
		// 체력바 게이지 설정
		const float HealthPercent = Health / MaxHealth;
		PlayerHUD->PlayerOverlay->HealthBar->SetPercent(HealthPercent);

		// 체력바 텍스트 설정(스트링으로 연산후 텍스트로)
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
		// 사망시 HUD 애니메이션 BP에서 플레이
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
			// 시간이 - 표기가 되지 않도록 0보다 작아질경우 FText() 비어있는 텍스트를 출력한다. 이후의 작업을 하지 않도록 빠른 reutrn 실행
			PlayerHUD->PlayerOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);

		// 2자리수로 제한
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
			// 시간이 - 표기가 되지 않도록 0보다 작아질경우 FText() 비어있는 텍스트를 출력한다. 이후의 작업을 하지 않도록 빠른 reutrn 실행
			PlayerHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);

		// 2자리수로 제한
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

	// 서버일 경우 실행
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
	// 플레이어 게임모드 클래스에서 해당 함수를 호출하여 매치 스테이트를 변경하고있음. 변경직후 복제된 변수이기 때문에 OnRep_MatchState 함수가 실행

	// 서버 실행
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
	// 클라이언트 실행
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
	// 게임실행시
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		// 플레이어의 UI생성하기
		PlayerHUD->AddCharacterOverlay();
		if (PlayerHUD->Announcement)
		{
			// 대기중 안내 위젯은 숨기기
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFirstPlayerController::HandleCooldown()
{
	// 게임종료후 쿨다운 상태가 들어갔을때
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		// 플레이어의 UI제거하기
		PlayerHUD->PlayerOverlay->RemoveFromParent();
		
		bool bHUDValid = PlayerHUD->Announcement &&
			PlayerHUD->Announcement->AnnouncementText &&
			PlayerHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			// 대기중 안내 위젯 다시 표시하기
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
		// 플래이어 캐릭터의 입력이 불가능하게 하는 변수 지정
		PlayerCharacter->bDisableGameplay = true;

		// 캐릭터가 발사하고 있는동안 쿨다운 상태가 될경우 총알이 다 떨어질때까지 발사한채로 있게되므로 컴포넌트에 접근하여 발사를 중단 시킨다.
		PlayerCharacter->GetCombat()->FireButtonPressed(false);
	}
}

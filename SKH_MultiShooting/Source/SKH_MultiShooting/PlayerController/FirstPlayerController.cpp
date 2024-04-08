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
	// 헤더 "Net/UnrealNetwork.h" 추가 필요
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirstPlayerController, MatchState);
}

void AFirstPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	// 플레이어의 입력컴포넌트가 활성화 되었을때 키입력 바인딩.
	InputComponent->BindAction("Quit", IE_Pressed, this, &ThisClass::ShowReturnToMainMenu);
}

void AFirstPlayerController::ShowReturnToMainMenu()
{
	// 메인메뉴 위젯 활성화
	if (ReturnToMainMenuWidget == nullptr) return;

	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		// 함수가실행될때(키입력이있을때) True & False 스왑.
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			// 메뉴위젯 ON
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			// 메뉴위젯 OFF
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AFirstPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// HUD의 시간을 세팅
	SetHUDTime();

	// 매프레임 동기화 시간 체크 
	CheckTimeSync(DeltaTime);
	PollInit();

	// 플레이어의 Ping을 체크하고 관련 애니메이션을 재생
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
			// GetPing은 압축된 값이기때문에 1/4 정도의 값을 얻게된다.(uint8) 때문에 uint32의 값을 얻고자 4배의 값으로 얻는다.
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRuningTime = 0.f; // 초기화

				// 서버로 전송
				ServerReportPingStatus(true);
			}
			else
			{
				// 서버로 전송
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0;
	}

	bool bHighPingAnimationPlaying = PlayerHUD &&
		PlayerHUD->PlayerOverlay &&
		PlayerHUD->PlayerOverlay->HighPingAnimation &&
		PlayerHUD->PlayerOverlay->IsAnimationPlaying(PlayerHUD->PlayerOverlay->HighPingAnimation);

	// 핑상태주의 애니메이션이 재생중이라면
	if (bHighPingAnimationPlaying)
	{
		// 재생시간을 측정하고
		PingAnimationRuningTime += DeltaTime;

		if (PingAnimationRuningTime > HighPingDuration)
		{
			// 기준시간을 초과하였을경우 애니메이션 정지
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
	// 서버와 클라이언트의 시간 동기화
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		// 동기화 시간을 계속더해주다가 지정한 시간을 넘었을때 동기화를 한번하고 0으로 초기화
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
	
	// 요청전송 편도 타임
	SingleTripTime = 0.5f * RoundTripTime;

	// 클라이언트가 서버에 요청을 보내고 서버에서 클라이언트로 회신하는 시간은 클라이언트가 요청~회신 한 총시간을 반으로 나눈 시간으로 볼 수 있다.
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;

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
		// 체력바 게이지 설정
		const float ShieldPercent = Shield / MaxShield;
		PlayerHUD->PlayerOverlay->ShieldBar->SetPercent(ShieldPercent);

		// 체력바 텍스트 설정(스트링으로 연산후 텍스트로)
		/*
		텍스트 형식 [현재쉴드/최대쉴드]
				FString ShieldText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		PlayerHUD->PlayerOverlay->ShieldText->SetText(FText::FromString(ShieldText));
		*/

		if (Shield > 0)
		{
			// 텍스트 형식 Armor : 현재 쉴드
			FString ShieldText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Shield));
			PlayerHUD->PlayerOverlay->ShieldText->SetText(FText::FromString(ShieldText));
		}
		else
		{
			// 텍스트 형식 Armor : 현재 쉴드
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
				// 각 HUD 업데이트는 초기화가 이루어졌는지 확인후에 매프레임 업데이트가 진행된다.
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
		if (PlayerHUD->PlayerOverlay == nullptr)
		{
			PlayerHUD->AddCharacterOverlay();
		}
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

			PlayerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			APlayerGameState* PlayerGameState = Cast<APlayerGameState>(UGameplayStatics::GetGameState(this));

			AFirstPlayerState* FirstPlayerState = GetPlayerState<AFirstPlayerState>();

			if (PlayerGameState && FirstPlayerState)
			{
				TArray<AFirstPlayerState*> TopPlayers = PlayerGameState->TopScoringPlayers;

				FString InfoTextString;

				if (TopPlayers.Num() == 0)
				{
					// 최다 득점자가 없을경우 (게임시작후 그누구도 플레이어를 죽이지 않을경우)
					InfoTextString = FString("No Winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == FirstPlayerState)
				{
					// 현재 플레이중인 플레이어가 최다득점자일 경우 현재 플레이어에게 위너임을 표시함
					InfoTextString = FString("You Win!");
				}
				else if (TopPlayers.Num() == 1)
				{
					// 현재플레이어가아닌 다른 누군가의 플레이어가 최다 득점을 단독으로 승리하였을경우 해당 플레이어의 이름을 출력한다.
					InfoTextString = FString::Printf(TEXT("Winner\n%s"), *TopPlayers[0]->GetPlayerName());

				}
				else if (TopPlayers.Num() > 1)
				{
					// 최다득점이 여러명일 경우
					InfoTextString = FString("TopScores\n");

					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}

				// UI의 INFO참에 출력되는 텍스트 편집
				PlayerHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));

			}
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
	// 서버가 클라이언트에 복제 실행
	ClientElimAnnouncement(Attacker, Victim);
}

void AFirstPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	// 모든 클라이언트에 복제된다.
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

		if (PlayerHUD)
		{
			// 내가 다른플레이어를 처치
			if (Attacker == Self && Victim != Self)
			{
				PlayerHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}

			// 다른플레이어가 나를 처치
			if (Attacker != Self && Victim == Self)
			{
				PlayerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}

			// 내가 나를 처치
			if (Attacker == Self && Attacker == Victim)
			{
				PlayerHUD->AddElimAnnouncement("You", "Self");
				return;
			}

			// 다른플레이어가 본인 스스로 처치
			if (Attacker != Self && Attacker == Victim)
			{
				PlayerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Self");
				return;
			}

			// 플레이어 이외의 플레이어들끼리 처치
			PlayerHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}
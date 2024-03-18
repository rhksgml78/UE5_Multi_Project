#include "FirstPlayerController.h"
#include "SKH_MultiShooting/HUD/PlayerHUD.h"
#include "SKH_MultiShooting/HUD/PlayerOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"

void AFirstPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//PrimaryActorTick.bCanEverTick = true;

	PlayerHUD = Cast<APlayerHUD>(GetHUD());

}

void AFirstPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
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
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);

		// 2자리수로 제한
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->PlayerOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFirstPlayerController::SetHUDTime()
{
	uint32 SecondLeft = FMath::CeilToInt(MatchTime - GetWorld()->GetTimeSeconds());
	if (CountDownInt != SecondLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetWorld()->GetTimeSeconds());
	}
	CountDownInt = SecondLeft;
}
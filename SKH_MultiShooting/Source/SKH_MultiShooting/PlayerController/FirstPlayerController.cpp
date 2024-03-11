#include "FirstPlayerController.h"
#include "SKH_MultiShooting/HUD/PlayerHUD.h"
#include "SKH_MultiShooting/HUD/PlayerOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AFirstPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());

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

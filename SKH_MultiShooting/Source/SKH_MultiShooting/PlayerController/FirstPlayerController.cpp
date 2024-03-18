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

	// �������� ����ȭ �ð� üũ 
	CheckTimeSync(DeltaTime);

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
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);

		// 2�ڸ����� ����
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->PlayerOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFirstPlayerController::SetHUDTime()
{
	uint32 SecondLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountDownInt != SecondLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountDownInt = SecondLeft;
}
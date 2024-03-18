#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FirstPlayerController.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AFirstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void PlayDefeatsAnimation();
	void SetHUDMatchCountdown(float CountdownTime);

	// 플레이어 빙의시 바로 한번 업데이트
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

private:

	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	float MatchTime = 120.f;
	uint32 CountDownInt = 0;
	
};

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SKH_MultiShooting/Weapon/WeaponTypes.h"

#include "PlayerOverlay.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API UPlayerOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GrenadeAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SpeedUpText;

	UPROPERTY(meta = (BindWidget))
	class UImage* SpeedUpUi;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HighPingText;
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* HighPingAnimation;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RedTeamScore;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	class UImage* RedBlueFlag;

	UFUNCTION(BlueprintNativeEvent, Category = "BluePrintEvent")
	void PlayDeafeats();

	void PlayDeafeats_Implementation();
protected:

private:

};


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

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
	class UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponAmmoAmount;
	

	UFUNCTION(BlueprintNativeEvent, Category = "BluePrintEvent")
	void PlayDeafeats();
	void PlayDeafeats_Implementation();

protected:

private:

};

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "OverheadWidget.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	UFUNCTION(BlueprintCallable)
	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

protected:
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* World) override;
	
};

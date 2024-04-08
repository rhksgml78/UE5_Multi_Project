#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ElimAnnouncement.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;

	void SetElimAnnouncementText(FString AttackerName, FString VictimName);
	
};

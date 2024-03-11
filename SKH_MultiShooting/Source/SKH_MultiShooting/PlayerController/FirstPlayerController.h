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

protected:
	virtual void BeginPlay() override;

private:
	class APlayerHUD* PlayerHUD;
	
};

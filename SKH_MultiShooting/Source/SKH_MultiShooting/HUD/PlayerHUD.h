#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "PlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrossHairCenter;
	class UTexture2D* CrossHairLeft;
	class UTexture2D* CrossHairRight;
	class UTexture2D* CrossHairTop;
	class UTexture2D* CrossHairBottom;
};

UCLASS()
class SKH_MULTISHOOTING_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	// 드로우HUD 함수는 매프레임 호출되는 함수
	virtual void DrawHUD() override;

private:
	FHUDPackage HUDPackage;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }


};

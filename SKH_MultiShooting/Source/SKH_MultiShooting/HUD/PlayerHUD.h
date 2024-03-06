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
	// ��ο�HUD �Լ��� �������� ȣ��Ǵ� �Լ�
	virtual void DrawHUD() override;

private:
	FHUDPackage HUDPackage;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }


};

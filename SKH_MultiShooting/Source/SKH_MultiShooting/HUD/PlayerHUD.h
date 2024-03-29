#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "PlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	// 크로스헤어의 각 부분에 그려지는 텍스처
	class UTexture2D* CrossHairCenter;
	class UTexture2D* CrossHairLeft;
	class UTexture2D* CrossHairRight;
	class UTexture2D* CrossHairTop;
	class UTexture2D* CrossHairBottom;

	// 크로스헤어를 벌어지게하는 변수
	float CrosshairSpread;

	// 크로스헤어의 색상 변수(디폴트 흰색)
	FLinearColor CrosshairsColor = FLinearColor::White;
};

UCLASS()
class SKH_MULTISHOOTING_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	// 드로우HUD 함수는 매프레임 호출되는 함수
	virtual void DrawHUD() override;
	
	// 캐릭터의 위젯을 설정할 수 있는 변수(BP에서 지정 해줘야함)
	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	// 플레이어 체력바
	UPROPERTY()
	class UPlayerOverlay* PlayerOverlay;

	// 플레이어의 오버레이UI를 생성하는 함수
	void AddCharacterOverlay();

	// 대기실 오버레이
	UPROPERTY(EditAnywhere, Category = "Player Announcement")
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UAnnouncement* Announcement;

	void AddAnnouncement();

protected:
	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	void DrawCrosshiar(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);

	// 크로스헤어의 퍼짐 최대치
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 15.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};

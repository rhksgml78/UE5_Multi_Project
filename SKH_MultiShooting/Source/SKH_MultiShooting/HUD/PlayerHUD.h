#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "PlayerHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	// ũ�ν������ �� �κп� �׷����� �ؽ�ó
	class UTexture2D* CrossHairCenter;
	class UTexture2D* CrossHairLeft;
	class UTexture2D* CrossHairRight;
	class UTexture2D* CrossHairTop;
	class UTexture2D* CrossHairBottom;

	// ũ�ν��� ���������ϴ� ����
	float CrosshairSpread;

	// ũ�ν������ ���� ����(����Ʈ ���)
	FLinearColor CrosshairsColor = FLinearColor::White;
};

UCLASS()
class SKH_MULTISHOOTING_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	// ��ο�HUD �Լ��� �������� ȣ��Ǵ� �Լ�
	virtual void DrawHUD() override;
	
	// ĳ������ ������ ������ �� �ִ� ����(BP���� ���� �������)
	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	// �÷��̾� ü�¹�
	UPROPERTY()
	class UPlayerOverlay* PlayerOverlay;

	// �÷��̾��� ��������UI�� �����ϴ� �Լ�
	void AddCharacterOverlay();

	// ���� ��������
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

	// ũ�ν������ ���� �ִ�ġ
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 15.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};

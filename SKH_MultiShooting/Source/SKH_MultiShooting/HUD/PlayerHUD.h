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
	UPROPERTY()
	class UTexture2D* CrossHairCenter;

	UPROPERTY()
	class UTexture2D* CrossHairLeft;

	UPROPERTY()
	class UTexture2D* CrossHairRight;

	UPROPERTY()
	class UTexture2D* CrossHairTop;

	UPROPERTY()
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
	void AddElimAnnouncement(FString Attacker, FString Victim);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class APlayerController* OwningPlayer;

	FHUDPackage HUDPackage;

	void DrawCrosshiar(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);

	// ũ�ν������ ���� �ִ�ġ
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 15.f;

	// �÷��̾� Ż�� ó�� ����
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 3.f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};

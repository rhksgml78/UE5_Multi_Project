#include "PlayerHUD.h"
#include "GameFramework/PlayerController.h"
#include "PlayerOverlay.h"
#include "Announcement.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerHUD::AddCharacterOverlay()
{
	// 위젯 생성시키기
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && CharacterOverlayClass)
	{
		PlayerOverlay = CreateWidget<UPlayerOverlay>(PlayerController, CharacterOverlayClass);
		PlayerOverlay->AddToViewport();
	}
}

void APlayerHUD::AddAnnouncement()
{
	// 위젯 생성시키기
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void APlayerHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;

	if (OwningPlayer && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			// BP에서 생성한 위젯 애니메이션 재생
			ElimAnnouncementWidget->SlideAnimStart();

			for (UElimAnnouncement* Msg : ElimMessages)
			{
				// 배열의 갯수만큼 일정한 간격으로 아래(+Y축)로 롤업.
				if (Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					
					FVector2D Position = CanvasSlot->GetPosition();
					FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y + CanvasSlot->GetSize().Y);
					CanvasSlot->SetPosition(NewPosition);
				}
			}

			// 배열에 저장
			ElimMessages.Add(ElimAnnouncementWidget);

			// 자동으로 위젯을 처리할 타이머 생성
			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			
			// 타이머델리게이트에 함수와 함수의 매개변수를 바인딩
			ElimMsgDelegate.BindUFunction(
				this, // 오브젝트
				FName("ElimAnnouncementTimerFinished"), // 함수명
				ElimAnnouncementWidget); // 매개변수타입

			// 타이머 세팅 단순히 함수를 호출하는것이아닌 델리게이트에 바인딩한 함수를 호출하여 실행하는 것
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer, // 핸들
				ElimMsgDelegate, // 델리게이트
				ElimAnnouncementTime, // 딜레이
				false); // 반복
		}
	}
}

void APlayerHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		ElimMessages.Remove(MsgToRemove);
		MsgToRemove->RemoveFromParent();
	}
}

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	// 크로스 헤어 그리기
	
	// 우선 화면의 사이즈
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		// 화면의 중심 위치
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		// 크로스헤어 조준점 퍼짐 동적으로 변경되어야함. 사격실행시 벌어지고 조금씩 줄어들도록 CombatComponent 클래스에서 업데이트
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrossHairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshiar(HUDPackage.CrossHairCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrossHairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshiar(HUDPackage.CrossHairLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrossHairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshiar(HUDPackage.CrossHairRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrossHairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshiar(HUDPackage.CrossHairTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrossHairBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshiar(HUDPackage.CrossHairBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}

}

void APlayerHUD::DrawCrosshiar(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor)
{
	// 현재 텍스처의 좌표는 왼쪽상단(0,0)에서 우측하단(1,1)이기 때문에 텍스처 자체의 중심을 계산해야함.

	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairsColor
	);

}

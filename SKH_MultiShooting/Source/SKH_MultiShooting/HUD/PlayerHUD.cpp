#include "PlayerHUD.h"

void APlayerHUD::DrawHUD()
{
	Super::DrawHUD();

	// ũ�ν� ��� �׸���
	
	// �켱 ȭ���� ������
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		// ȭ���� �߽� ��ġ
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		// ũ�ν���� ������ ���� �������� ����Ǿ����. ��ݽ���� �������� ���ݾ� �پ�鵵�� CombatComponent Ŭ�������� ������Ʈ
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrossHairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshiar(HUDPackage.CrossHairCenter, ViewportCenter, Spread);
		}
		if (HUDPackage.CrossHairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshiar(HUDPackage.CrossHairLeft, ViewportCenter, Spread);
		}
		if (HUDPackage.CrossHairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshiar(HUDPackage.CrossHairRight, ViewportCenter, Spread);
		}
		if (HUDPackage.CrossHairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshiar(HUDPackage.CrossHairTop, ViewportCenter, Spread);
		}
		if (HUDPackage.CrossHairBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshiar(HUDPackage.CrossHairBottom, ViewportCenter, Spread);
		}
	}

}

void APlayerHUD::DrawCrosshiar(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	// ���� �ؽ�ó�� ��ǥ�� ���ʻ��(0,0)���� �����ϴ�(1,1)�̱� ������ �ؽ�ó ��ü�� �߽��� ����ؾ���.

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
		FLinearColor::White
	);

}

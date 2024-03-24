#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"

#include "Shotgun.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
	
public:
	// ¹ß»ç¿ë ÇÔ¼ö
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10; //¼¦°Ç ÅºÆÛÁü °¹¼ö

};

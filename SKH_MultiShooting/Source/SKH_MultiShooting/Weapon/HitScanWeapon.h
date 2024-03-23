#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"

#include "HitScanWeapon.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	// 발사용 함수
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* HitSound;

};

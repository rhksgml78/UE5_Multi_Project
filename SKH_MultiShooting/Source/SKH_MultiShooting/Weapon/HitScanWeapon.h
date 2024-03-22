#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"

#include "HitScanWeapon.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	// �߻�� �Լ�
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

};

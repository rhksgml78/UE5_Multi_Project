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

protected:
	// ���� ��ź���� ���
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

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

	/*
	ź���� Ʈ���̽�
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

};

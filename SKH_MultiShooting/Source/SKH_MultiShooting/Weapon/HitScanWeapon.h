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

protected:
	// 원형 착탄범위 계산
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

private:

	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	class USoundCue* FireSound;

	/*
	탄퍼짐 트레이스
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

};

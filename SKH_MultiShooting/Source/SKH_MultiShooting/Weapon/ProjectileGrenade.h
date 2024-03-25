#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"

#include "ProjectileGrenade.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	// 프로젝타일무브먼트 컴포넌트의 델리게이트 파라미터2개 사용
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
	class USoundCue* BounceSound;

	int32 BounceCount = 0;

	UPROPERTY(EditAnywhere)
	int32 MaxBounceSound = 3;

	// 한번 바운드 하고 타이머 세팅하기
	bool bTimerSet = false;
};

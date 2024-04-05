#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"

#include "ProjectileWeapon.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;
	
private:
	// 서버에서 사용할 복제할 투사체
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

	// 클라이언트에서 즉시 발사하고 발사경로를 계산할 투사체 (복제x)
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ServerSideRewindProjectileClass;
};

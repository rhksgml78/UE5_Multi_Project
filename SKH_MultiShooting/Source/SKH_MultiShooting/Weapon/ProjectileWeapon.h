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
	// �������� ����� ������ ����ü
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

	// Ŭ���̾�Ʈ���� ��� �߻��ϰ� �߻��θ� ����� ����ü (����x)
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ServerSideRewindProjectileClass;
};

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

	// ������Ÿ�Ϲ����Ʈ ������Ʈ�� ��������Ʈ �Ķ����2�� ���
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
	class USoundCue* BounceSound;

	int32 BounceCount = 0;

	UPROPERTY(EditAnywhere)
	int32 MaxBounceSound = 3;

	// �ѹ� �ٿ�� �ϰ� Ÿ�̸� �����ϱ�
	bool bTimerSet = false;
};

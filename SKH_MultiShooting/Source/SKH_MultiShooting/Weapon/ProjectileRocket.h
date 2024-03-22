#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"

#include "ProjectileRocket.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	// Ÿ�̹� �ݹ� �Լ�
	void DestroyTimerFinished();

	// ���̾ư��� ����Ʈ
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	// ���̾�ī�� ����Ʈ�� ��Ʈ���� ������Ʈ
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	// ���� ����
	UPROPERTY(EditAnywhere)
	class USoundCue* ProjectileLoop;

	UPROPERTY()
	class UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	class USoundAttenuation* LoopingSoundAttenuation;

	// ������ �ٸ� �Ѿ˰��ٸ� Ŀ���� �����Ʈ�� ���
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

};

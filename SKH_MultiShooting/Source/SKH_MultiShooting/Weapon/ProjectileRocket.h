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

	// 타이버 콜백 함수
	void DestroyTimerFinished();

	// 나이아가라 이펙트
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	// 나이아카라 이펙트를 컨트롤할 컴포넌트
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	// 사운드 관련
	UPROPERTY(EditAnywhere)
	class USoundCue* ProjectileLoop;

	UPROPERTY()
	class UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	class USoundAttenuation* LoopingSoundAttenuation;

	// 로켓은 다른 총알과다른 커스텀 무브먼트를 사용
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

};

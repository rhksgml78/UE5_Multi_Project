#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Projectile.generated.h"

// 총기류의 발사체(총알) 클래스

UCLASS()
class SKH_MULTISHOOTING_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	// 파괴 함수
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	// 피격이벤트(바인딩해야함)
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 데미지 관련
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* StaticMeshComponent;

private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

public:	

	// 파티클 재생 함수
	void SpawnParticleEffects();
};

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

	// 서버측 되감기
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	// 넷퀀타이즈100은 일반보다 소수점2자리수 더 정확
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	// 데미지 관련
	float Damage = 20.f;

protected:
	virtual void BeginPlay() override;

	// 타이머 세팅 함수
	void StartDestroyTimer();

	// 타이머 콜백 함수
	void DestroyTimerFinished();

	// 피격이벤트(바인딩해야함)
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 트레일(나이아가라이펙트)생성
	void SpanwTrailSystem();

	// 범위 공격
	void ExplodeDamage();

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	// 충돌박스
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// 나이아가라 이펙트
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	// 나이아카라 이펙트를 컨트롤할 컴포넌트
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	// 범위 공격에 관련된 변수
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

public:	
	// 파티클 재생 함수
	void SpawnParticleEffects();

};

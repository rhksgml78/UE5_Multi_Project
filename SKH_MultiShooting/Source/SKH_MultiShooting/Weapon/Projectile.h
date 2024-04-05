#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Projectile.generated.h"

// �ѱ���� �߻�ü(�Ѿ�) Ŭ����

UCLASS()
class SKH_MULTISHOOTING_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	// �ı� �Լ�
	virtual void Destroyed() override;

	// ������ �ǰ���
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	// ����Ÿ����100�� �Ϲݺ��� �Ҽ���2�ڸ��� �� ��Ȯ
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	// ������ ����
	float Damage = 20.f;

protected:
	virtual void BeginPlay() override;

	// Ÿ�̸� ���� �Լ�
	void StartDestroyTimer();

	// Ÿ�̸� �ݹ� �Լ�
	void DestroyTimerFinished();

	// �ǰ��̺�Ʈ(���ε��ؾ���)
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Ʈ����(���̾ư�������Ʈ)����
	void SpanwTrailSystem();

	// ���� ����
	void ExplodeDamage();

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	// �浹�ڽ�
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	// ���̾ư��� ����Ʈ
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	// ���̾�ī�� ����Ʈ�� ��Ʈ���� ������Ʈ
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	// ���� ���ݿ� ���õ� ����
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
	// ��ƼŬ ��� �Լ�
	void SpawnParticleEffects();

};

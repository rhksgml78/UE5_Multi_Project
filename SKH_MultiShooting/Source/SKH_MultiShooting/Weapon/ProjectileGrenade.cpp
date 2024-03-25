#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	// ������Ÿ���� �⺻ ������ �θ�Ŭ�������� �ϰ������� �׷�����Ʈ �ʿ����� �ٿ ������ ���� ���ش�.
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();
	SpanwTrailSystem();

	// �ٿ�� ���ε�
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &ThisClass::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	BounceCount++;
	if (!bTimerSet)
	{
		bTimerSet = true;
		StartDestroyTimer();

	}

	if (BounceSound && BounceCount < MaxBounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::Destroyed()
{
	// ��缱���� ���� �������� �����ϰ� �θ��� ��Ʈ���̸� ȣ���Ѵ�.
	ExplodeDamage();
	Super::Destroyed();


}
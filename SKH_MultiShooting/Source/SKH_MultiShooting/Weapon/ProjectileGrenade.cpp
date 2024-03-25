#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	// 프로젝타일의 기본 설정은 부모클래스에서 하고있으며 그레네이트 쪽에서는 바운스 설정만 따로 해준다.
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();
	SpanwTrailSystem();

	// 바운딩 바인드
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
	// 방사선상의 범위 데미지를 전달하고 부모의 디스트로이를 호출한다.
	ExplodeDamage();
	Super::Destroyed();


}
#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // 월드상에 위치한 오브젝트
				Damage, // 기본 데미지
				10.f, // 최소 데미지
				GetActorLocation(), // 생성위치
				200.f, // 최소 범위
				500.f, // 최대 범위
				1.f, // 데미지 Falloff
				UDamageType::StaticClass(), // 데미지타입클래스
				TArray<AActor*>(), // 예외처리(Empty/없음)
				this, // 데미지 커서(원인)
				FiringController // 소유주의 컨트롤러
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

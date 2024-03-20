#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 로켓은 파괴될때 주변에 스플래쉬 형태로 데미지를 준다

	// 발사된 투사체는 발사한 무기의 소유자(Instigator)를 알수있다.
	APawn* FiringPawn =	GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			// 방사선상의 데미지를 준다
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // 월드상의 오브젝트 기준
				Damage, // 기본 데미지
				10.f, // 최소 데미지
				GetActorLocation(), //발생위치
				200.f, // 내부반경
				500.f, // 외부반경
				1.f, // 데미지폴오프
				UDamageType::StaticClass(), // 데미지타입
				TArray<AActor*>(), // 예외처리할 클래스(없음) 본인도 손상을 입는 옵션으로 예외처리를 비워둠
				this, // 데미지의 원인
				FiringController // 선동한 컨트롤러
			);
		}
	}
	
	// 부모의 Onhit 에서는 파괴하면서 파티클과 사운드를 재생시킨다.
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

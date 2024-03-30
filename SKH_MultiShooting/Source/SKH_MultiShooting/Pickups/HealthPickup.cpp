#include "HealthPickup.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	// 서버와 클라이언트 모두에 존재하기때문에 복제되어야함
	bReplicates = true;

}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		// 플레이어의 버프컴포넌트에 접근
		UBuffComponent* Buff = PlayerCharacter->GetBuff();
		if (Buff)
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}

	// 디스트로이시 해당클래스에서 정의하지 않았으므로 부모클래스의 것이 호출되고 Destroyed 함수의 사운드 재생이 출력되고 디스트로이 실행.
	Destroy();
}
#include "AmmoPickup.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		UCombatComponent* Combat = PlayerCharacter->GetCombat();
		if (Combat)
		{
			// 해당아이템의 무기타입에 따라 맵에 저장되는 값을 추가
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}

	// 디스트로이시 해당클래스에서 정의하지 않았으므로 부모클래스의 것이 호출되고 Destroyed 함수의 사운드 재생이 출력되고 디스트로이 실행.
	Destroy();
}

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
			// �ش�������� ����Ÿ�Կ� ���� �ʿ� ����Ǵ� ���� �߰�
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}

	// ��Ʈ���̽� �ش�Ŭ�������� �������� �ʾ����Ƿ� �θ�Ŭ������ ���� ȣ��ǰ� Destroyed �Լ��� ���� ����� ��µǰ� ��Ʈ���� ����.
	Destroy();
}

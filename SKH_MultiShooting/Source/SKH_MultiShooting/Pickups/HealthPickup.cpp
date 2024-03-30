#include "HealthPickup.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	// ������ Ŭ���̾�Ʈ ��ο� �����ϱ⶧���� �����Ǿ����
	bReplicates = true;

}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		// �÷��̾��� ����������Ʈ�� ����
		UBuffComponent* Buff = PlayerCharacter->GetBuff();
		if (Buff)
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}

	// ��Ʈ���̽� �ش�Ŭ�������� �������� �ʾ����Ƿ� �θ�Ŭ������ ���� ȣ��ǰ� Destroyed �Լ��� ���� ����� ��µǰ� ��Ʈ���� ����.
	Destroy();
}
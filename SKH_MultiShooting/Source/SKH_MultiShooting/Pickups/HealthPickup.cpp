#include "HealthPickup.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AHealthPickup::AHealthPickup()
{
	// ������ Ŭ���̾�Ʈ ��ο� �����ϱ⶧���� �����Ǿ����
	bReplicates = true;

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);

}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{

	}

	// ��Ʈ���̽� �ش�Ŭ�������� �������� �ʾ����Ƿ� �θ�Ŭ������ ���� ȣ��ǰ� Destroyed �Լ��� ���� ����� ��µǰ� ��Ʈ���� ����.
	Destroy();
}

void AHealthPickup::Destroyed()
{

	// �������� �÷��̾ ȹ���Ͽ� �Ҹ�Ǿ����� ����Ʈ ����
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	Super::Destroyed();
}
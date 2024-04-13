#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	// ���Ŭ������ ����ƽ�Žø��� ����� ���̱⶧���� �ش�޽ø� ��Ʈ�������Ѵ�.
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// �浹�� ����� ��ü�� �Ⱦ� ������ ���̷�Ż�޽ÿ� �پ��ֱ� ������ �ٽ� ����ƽ�޽�(��Ʈ)�� ����
	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);


}

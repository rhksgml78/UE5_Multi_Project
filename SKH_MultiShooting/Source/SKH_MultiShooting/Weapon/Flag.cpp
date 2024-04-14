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

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	// ����ġ �Ǿ��ִ� ���⸦ ����� ����߷��α�
	FDetachmentTransformRules DetechRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetechRules);

	// ������ ����
	SetOwner(nullptr);

	// �÷��̾�� ��Ʈ�ѷ��� ����
	PlayerOwnerCharacter = nullptr;
	PlayerOwnerController = nullptr;
}

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ������ ���� OFF
	EnableCustomDepth(false);
}

void AFlag::OnDroped()
{
	if (HasAuthority())
	{
		// �������� �۾��� ��
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// ������ ���� ON
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

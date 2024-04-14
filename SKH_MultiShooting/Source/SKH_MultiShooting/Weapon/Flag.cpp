#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"

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

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	// ����� �ʱ� ��ġ ����
	InitialTransform = GetActorTransform();
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

void AFlag::ReSetFlag()
{
	// �������ִ� �÷��̾� �ʱ�ȭ
	APlayerCharacter* FlagBearer = Cast<APlayerCharacter>(GetOwner());
	if (FlagBearer)
	{
		// �÷��̾��� ����� ����ִ� ���¸� �ʱ�ȭ�Ѵ�.(������ Ŭ�� ��� ����)
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
	}

	// ������ �۾��� Ŭ���̾�Ʈ���� �� �ʿ䰡 ����. ���������� �������ֵ�����.
	if (!HasAuthority()) return;

	// �÷��̾�Լ� ����߸���
	FDetachmentTransformRules DetechRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetechRules);
	SetWeaponState(EWeaponState::EWS_Initial);

	// �ʱ� ��ġ,ȸ�������� �ǵ�����
	SetActorTransform(InitialTransform);

	// ��ġ�� �̵���Ų�ڿ� �浹Ÿ���� �ٲپ���� 2ȸ ī���ÿ����� ��������.
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

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
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	// ������ ���� OFF
	//EnableCustomDepth(false);
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
	//FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	//FlagMesh->MarkRenderStateDirty();
	//EnableCustomDepth(true);
}

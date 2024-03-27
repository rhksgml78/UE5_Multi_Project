#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	// �������� ���� Ŭ���̾�Ʈ ��ο��Լ� ��ȣ�ۿ��ϱ� ������ �����Ǿ���Ѵ�
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(100.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Ŀ���� ���� ���ٽ� �� ����
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	PickupMesh->MarkRenderStateDirty();
	PickupMesh->SetRenderCustomDepth(true);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	// ��ü �浹�ڽ��� �������̺�Ʈ�� ���ε�. ��, ���������� �������
	if (HasAuthority())
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}
}


void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APickup::Destroyed()
{
	Super::Destroyed();
	// �÷��̾ �������� ȹ��(�ı�)�Ͽ����� ȿ���� ����� �ش� ���� �ı� ����
	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickUpSound,
			GetActorLocation()
		);
	}
}


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "SKH_MultiShooting/Weapon/WeaponTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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
	PickupMesh->SetRelativeScale3D(FVector(3.f, 3.f, 3.f));

	// Ŀ���� ���� ���ٽ� �� ����
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	PickupMesh->MarkRenderStateDirty();
	PickupMesh->SetRenderCustomDepth(true);

	EffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectComponent"));
	EffectComponent->SetupAttachment(PickupMesh);
	EffectComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	// ��ü �浹�ڽ��� �������̺�Ʈ�� Ÿ�̸ӷ� ���ε�. ��, ���������� �������, ��� ���ε��ϸ� �������� �԰� �÷��̾ ����ġ�� �״��������� �÷��̾���� �������ڸ��� �������� �߰������� �ȵǱ⶧���� ������ ������ �Ͻ����� �ð����� �ΰ��� ���ε��Ѵ�.
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&ThisClass::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}


void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// �ڽ�Ŭ�������� �������Ͽ� �����
}

void APickup::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
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
}


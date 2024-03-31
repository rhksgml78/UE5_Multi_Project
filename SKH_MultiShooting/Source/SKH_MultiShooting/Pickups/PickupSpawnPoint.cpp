#include "PickupSpawnPoint.h"
#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	// C��Ÿ�� ĳ������ ���Ͽ� �Ű������� nullptr �� ���� �ܼ��� ���ε��� �Ķ���͸� �����ϱ� ���� ����
	StartSpawnPickupTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::SpawnPickup()
{
	// ������ Ŭ���������� �迭������ŭ �߿��� �������� ����
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses-1);

		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		// ������ Ŭ���� ��������Ʈ ���ε�
		if (HasAuthority() && SpawnedPickup)
		{
			// ������ Ŭ������ �ı��ɶ� �ٽ� �����Ǵ� Ÿ�̸Ӱ� �缼�õȴ�. �����ؾ������� ���ε��� �Լ��� UFUNCTION ��ũ�ο����Ѵ�.
			SpawnedPickup->OnDestroyed.AddDynamic(this, &ThisClass::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	// �������� ������ ��Ŭ���̾�Ʈ���ƴ� ���������� ����
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);

	GetWorldTimerManager().SetTimer(
		SpawnPicupTimer,
		this,
		&ThisClass::SpawnPickupTimerFinished,
		SpawnTime
	);
}


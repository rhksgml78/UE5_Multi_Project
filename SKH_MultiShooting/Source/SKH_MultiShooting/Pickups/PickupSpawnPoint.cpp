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

	// C스타일 캐스팅을 통하여 매개변수를 nullptr 로 설정 단순히 바인딩용 파라미터를 충족하기 위한 조건
	StartSpawnPickupTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::SpawnPickup()
{
	// 지정된 클래스형식의 배열갯수만큼 중에서 랜덤으로 스폰
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses-1);

		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		// 생성된 클래스 델리게이트 바인딩
		if (HasAuthority() && SpawnedPickup)
		{
			// 생성된 클래스가 파괴될때 다시 스폰되는 타이머가 재세팅된다. 주의해야할점은 바인딩할 함수가 UFUNCTION 매크로여야한다.
			SpawnedPickup->OnDestroyed.AddDynamic(this, &ThisClass::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	// 아이템의 스폰은 각클라이언트가아닌 서버에서만 진행
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


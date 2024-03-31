#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "PickupSpawnPoint.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// 생성할 아이템의 타입을 엔진BP에서 지정설정할수있도록 한다.
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	// 생성된 아이템이 파괴되었는지 확인할 수 있어야 다음 스폰을 실행 할 수 있다.
	class APickup* SpawnedPickup;

	// 스폰 생성용 함수
	void SpawnPickup();

	// 스폰 타이밍 조절용 타이머
	void SpawnPickupTimerFinished();

	// 생성된 아이템클래스와 델리게이트 바인딩을 해야하기때문에 매개변수가  필요하다. UFUNCTION 매크로 지정필수!
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:	
	FTimerHandle SpawnPicupTimer;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax;

};

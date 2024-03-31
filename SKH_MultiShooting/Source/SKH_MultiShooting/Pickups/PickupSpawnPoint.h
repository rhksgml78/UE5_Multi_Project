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

	// ������ �������� Ÿ���� ����BP���� ���������Ҽ��ֵ��� �Ѵ�.
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	// ������ �������� �ı��Ǿ����� Ȯ���� �� �־�� ���� ������ ���� �� �� �ִ�.
	class APickup* SpawnedPickup;

	// ���� ������ �Լ�
	void SpawnPickup();

	// ���� Ÿ�̹� ������ Ÿ�̸�
	void SpawnPickupTimerFinished();

	// ������ ������Ŭ������ ��������Ʈ ���ε��� �ؾ��ϱ⶧���� �Ű�������  �ʿ��ϴ�. UFUNCTION ��ũ�� �����ʼ�!
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:	
	FTimerHandle SpawnPicupTimer;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin;

	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax;

};

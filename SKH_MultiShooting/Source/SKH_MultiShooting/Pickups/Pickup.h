#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Pickup.generated.h"

// 플레이어와 상호작용할 아이템 상위 클래스

UCLASS()
class SKH_MULTISHOOTING_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	// 구체 콜리전 오버랩 이벤트와 바인딩할 함수
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickUpSound;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* Effect;

public:	


};

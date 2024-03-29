#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"

#include "HealthPickup.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AHealthPickup : public APickup
{
	GENERATED_BODY()
	
public:
	AHealthPickup();

protected:
	// 구체 콜리전 오버랩 이벤트와 바인딩할 함수
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

private:

	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;
};

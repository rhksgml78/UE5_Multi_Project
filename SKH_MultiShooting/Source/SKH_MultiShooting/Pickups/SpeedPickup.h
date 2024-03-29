#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"

#include "SpeedPickup.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API ASpeedPickup : public APickup
{
	GENERATED_BODY()

public:
	ASpeedPickup();

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
	// 서있는 상태에서의 이동속도 값
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1200.f;

	// 앉은 상태의 이동속도 값
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 600.f;

	// 속도상승 유지 시간
	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 10.f;

};

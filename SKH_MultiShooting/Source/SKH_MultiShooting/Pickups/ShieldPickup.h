#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"

#include "ShieldPickup.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AShieldPickup : public APickup
{
	GENERATED_BODY()

public:
	AShieldPickup();

protected:
	// ��ü �ݸ��� ������ �̺�Ʈ�� ���ε��� �Լ�
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
	float ShieldReplenishAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float  ShieldReplenish = 5.f;
};

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
	// ���ִ� ���¿����� �̵��ӵ� ��
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1200.f;

	// ���� ������ �̵��ӵ� ��
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 600.f;

	// �ӵ���� ���� �ð�
	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 10.f;

};

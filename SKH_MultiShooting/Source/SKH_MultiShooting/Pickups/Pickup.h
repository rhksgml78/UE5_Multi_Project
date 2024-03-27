#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Pickup.generated.h"

// �÷��̾�� ��ȣ�ۿ��� ������ ���� Ŭ����

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

	// ��ü �ݸ��� ������ �̺�Ʈ�� ���ε��� �Լ�
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

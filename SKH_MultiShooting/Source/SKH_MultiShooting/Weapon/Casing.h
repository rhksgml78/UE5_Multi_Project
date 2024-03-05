#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Casing.generated.h"

/*
�ش�Ŭ������ ��������X
������ �߻��� ź�Ǹ� ���̵��� �÷��̿� ���θ� ���δ�.
*/

UCLASS()
class SKH_MULTISHOOTING_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

protected:
	virtual void BeginPlay() override;

	// �ǰ��̺�Ʈ(���ε��ؾ���) �ٴ��̳� ��� �浹������ ��ü�� �ı� �ǵ���
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float EjectionImpulse;

	UPROPERTY(EditAnywhere)
	float DestroyTimer;

	UPROPERTY(EditAnywhere)
	class USoundCue* DropSound;

	bool SoundPlayOnce;


};

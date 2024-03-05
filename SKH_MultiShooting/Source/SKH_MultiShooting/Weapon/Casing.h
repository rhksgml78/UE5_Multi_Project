#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Casing.generated.h"

/*
해당클래스는 복제생성X
본인이 발사한 탄피만 보이도록 플레이에 본인만 보인다.
*/

UCLASS()
class SKH_MULTISHOOTING_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

protected:
	virtual void BeginPlay() override;

	// 피격이벤트(바인딩해야함) 바닥이나 어딘가 충돌했을때 객체가 파괴 되도록
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

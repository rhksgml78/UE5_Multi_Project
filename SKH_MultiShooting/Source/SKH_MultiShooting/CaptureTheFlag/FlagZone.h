#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SKH_MultiShooting/PlayerTypes/Team.h"

#include "FlagZone.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam Team;

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
	class USphereComponent* ZoneSphere;

public:	

};

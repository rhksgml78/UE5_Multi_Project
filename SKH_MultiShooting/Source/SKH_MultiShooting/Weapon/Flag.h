#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"

#include "Flag.generated.h"

// 깃발클래스는 무기클래스를 상속받지만 공격 기능은 없을것

UCLASS()
class SKH_MULTISHOOTING_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();
	virtual void Dropped() override;
	void ReSetFlag();

protected:
	virtual void OnEquipped() override;
	virtual void OnDroped() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialLocation() const { return InitialTransform; }
};

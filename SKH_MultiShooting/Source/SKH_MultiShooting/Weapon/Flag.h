#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"

#include "Flag.generated.h"

// ���Ŭ������ ����Ŭ������ ��ӹ����� ���� ����� ������

UCLASS()
class SKH_MULTISHOOTING_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();
	virtual void Dropped() override;

protected:
	virtual void OnEquipped() override;
	virtual void OnDroped() override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;
};

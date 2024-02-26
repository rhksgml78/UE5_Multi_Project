#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKH_MULTISHOOTING_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class APlayerCharacter;

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EquipWeapon(class AWeapon* WeaponToEquip);

	// 복제용 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

private:
	class APlayerCharacter* Character;

	UPROPERTY(Replicated)
	class AWeapon* EquippedWeapon;

public:	

		
};

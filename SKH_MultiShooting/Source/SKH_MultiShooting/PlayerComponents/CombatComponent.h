#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SKH_MultiShooting/HUD/PlayerHUD.h"
#include "SKH_MultiShooting/Weapon/WeaponTypes.h"

#include "CombatComponent.generated.h"

#define TRACE_LENGTH 8000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKH_MULTISHOOTING_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class APlayerCharacter;

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	// �߻�� �Լ�
	void Fire();

	// ����� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	// �浹������
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// ũ�ν���� ����
	void SetHUDCrosshairs(float DeltaTime);

	// ���ε� RPC
	UFUNCTION(Server, Reliable)
	void ServerReload();


private:
	UPROPERTY()
	class APlayerCharacter* Character;

	UPROPERTY()
	class AFirstPlayerController* Controller;

	UPROPERTY()
	class APlayerHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	// HUD�� ũ�ν����
	FHUDPackage HUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairShootingFactor;
	float CrosshairAimFactor;

	// ������ġ
	FVector HitTarget;

	// ����(Aim)�� FOV
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	// ���ӹ߻縦 ���� Ÿ�̸�
	FTimerHandle FireTimer; // Ÿ�̸� �ڵ�
	bool bCanFire = true; // ��ȭ�� �Һ���
	void StartFireTimer(); // �����Լ�
	void FireTimerFinished(); // �ݹ��Լ�

	bool CanFire(); // ź������������ �߻��� �� �ֵ���

	// ������źâ�� �ٷξ�����Ʈ�Ǿ���ϱ⶧���� ������Ʈ Ŭ�������� �����Ѵ�. �÷��̾�� �������� ������ ��� Ÿ���� źâ������ ������ �������� �ʴ´�. ���� �������� ���⿡ ���ؼ��� ���� ������.
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	// ������ Ÿ�Կ����� �����ϴ� ź���� ������ TMap���� ����
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	void InitializeCarriedAmmo();

public:	
	void SetMaxWalkSpeed(float Value);
	
};

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SKH_MultiShooting/HUD/PlayerHUD.h"
#include "SKH_MultiShooting/Weapon/WeaponTypes.h"
#include "SKH_MultiShooting/PlayerTypes/CombatState.h"
#include "SKH_MultiShooting/Weapon/Projectile.h"

#include "CombatComponent.generated.h"

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
	void SwapWeapon();
	void Reload();
	// �������Ʈ ��Ƽ���̿��� ������ �Լ�
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);

	// ����, ��ź�߻��� Shell ��Ƽ���� ���� �Լ�
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	// ����ź�� ��ô�ϰ� �ٽ� ������ �������·� �ǵ��������� ��Ƽ���� ���� �Լ�
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	// ����ź�� �������� Ÿ�ֿ̹� ����ź�� �����ϰų� �տ� ǥ�õ� ����ź�� ���ߴ� �Լ�
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	// ���ⱳü�� ������ ��Ƽ���̿� ������ �Լ�
	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	// �߰��� ������ ���⸦ ��ü�� ��Ƽ���� ���� �Լ�
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapon();

	// �������� ����ź�� ������ ���� Ȯ��
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	// ������ ȸ�� ����
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	// �����丵 �Լ�
	void DropEquippedWeapon();
	void AttachActorToLefttHand(AActor* ActorToAttach);
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttackActorToBackpack(AActor* ActorToAttach);
	void AttackFlagToLeftHand(AWeapon* Flag);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	//���ε� ���� ����
	bool bLocallyReloading = false;

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	// �߻�� �Լ�
	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireMultiHitScanWeapon();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTarget);

	// ���������� ȣ�� _Implementation ��� �ʿ�
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	// �������� ���÷� ���� ���Ŭ���̾�Ʈ�� ȣ��
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	// ���������� ȣ�� _Implementation ��� �ʿ�
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay);

	// �������� ���÷� ���� ���Ŭ���̾�Ʈ�� ȣ��
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget);

	// �浹������
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// ũ�ν���� ����
	void SetHUDCrosshairs(float DeltaTime);

	// ���ε� RPC
	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload(); // ���� Ŭ���̾�Ʈ ��ο��� ����

	// ���ε嶧 ������ �� �� �ִ� ź�హ���� ����ϴ� �Լ�
	int32 AmountToReload();

	// ����ź ��ô ����
	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

private:
	UPROPERTY()
	class APlayerCharacter* Character;

	UPROPERTY()
	class AFirstPlayerController* Controller;

	UPROPERTY()
	class APlayerHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	class AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

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
	int32 MaxCarriedAmmo = 200;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 1;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 4;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 40;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 4;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 12;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 4;


	// ����ź�� ���� ���� ����
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 3;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 3;

	void UpdateHUDGrenades();

	void InitializeCarriedAmmo();

	// �÷��̾��� ���¸� �����ϴ� ����
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	// ���ε� ������ Ÿ�ֿ̹� ����� �Լ�
	void UpdateAmmoValues();

	// ���� ��ź
	void UpdateShotgunAmmoValues();

	// ��� ����
	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	UFUNCTION()
	void OnRep_HoldingTheFlag();

	UPROPERTY()
	AWeapon* TheFlag;

public:	
	void SetMaxWalkSpeed(float Value);
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapon();
};

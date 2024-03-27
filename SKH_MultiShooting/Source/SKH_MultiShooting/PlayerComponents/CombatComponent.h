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

	// 복제용 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	// 블루프린트 노티파이에서 실행할 함수
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);

	// 샷건, 유탄발사기용 Shell 노티파이 연계 함수
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	// 수류탄을 투척하고 다시 원래의 전투상태로 되돌리기위한 노티파이 연계 함수
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	// 수류탄이 던저지는 타이밍에 수류탄을 스폰하거나 손에 표시된 수류탄을 감추는 함수
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	// 서버에서 수류탄을 던지는 방향 확인
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	// 아이템 회수 관련
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	// 리펙토링 함수
	void DropEquippedWeapon();
	void AttachActorToLefttHand(AActor* ActorToAttach);
	void AttachActorToRightHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound();
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);


protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	// 발사용 함수
	void Fire();

	// 복사용 함수
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	// 충돌판정용
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// 크로스헤어 세팅
	void SetHUDCrosshairs(float DeltaTime);

	// 리로드 RPC
	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload(); // 서버 클라이언트 모두에서 실행

	// 리로드때 재장전 할 수 있는 탄약갯수를 계산하는 함수
	int32 AmountToReload();

	// 수류탄 투척 관련
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

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	// HUD와 크로스헤어
	FHUDPackage HUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairShootingFactor;
	float CrosshairAimFactor;

	// 조준위치
	FVector HitTarget;

	// 조준(Aim)과 FOV
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	// 연속발사를 위한 타이머
	FTimerHandle FireTimer; // 타이머 핸들
	bool bCanFire = true; // 점화식 불변수
	void StartFireTimer(); // 시작함수
	void FireTimerFinished(); // 콜백함수

	bool CanFire(); // 탄약이있을때만 발사할 수 있도록

	// 소지한탄창은 바로업데이트되어야하기때문에 컴포넌트 클래스에서 소지한다. 플레이어는 소유중인 무기의 모든 타입의 탄창갯수를 별도로 저장하지 않는다. 현재 장착중인 무기에 관해서만 값을 가진다.
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	// 무기의 타입에따라 소지하는 탄약의 갯수를 TMap으로 저장
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


	// 수류탄의 소지 갯수 관련
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 2;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 3;

	void UpdateHUDGrenades();

	void InitializeCarriedAmmo();

	// 플레이어의 상태를 지정하는 변수
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	// 리로드 끝나는 타이밍에 실행될 함수
	void UpdateAmmoValues();

	// 샷건 장탄
	void UpdateShotgunAmmoValues();

public:	
	void SetMaxWalkSpeed(float Value);
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
};

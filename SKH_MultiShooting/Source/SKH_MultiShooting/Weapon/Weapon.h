#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "SKH_MultiShooting/PlayerTypes/Team.h"

#include "Weapon.generated.h"

class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan Weapon"),
	EFT_MultiHitScan UMETA(DisplayName = "MultiHitScan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SKH_MULTISHOOTING_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ���ʰ� �ٲ� ����Ǵ� �Լ� ������ (AActor���)
	virtual void OnRep_Owner() override;

	// Ammo HUD ���ÿ� ȣ���Լ�
	void SetHUDAmmo();

	// �߻�� �Լ�(������ �� �� �ֵ���)
	virtual void Fire(const FVector& HitTarget);

	// �÷��̾� ����� ���� ������̱�
	virtual void Dropped();

	// �������� ź�� ���� �ø���
	void AddAmmo(int32 AmmoToAdd);

	// ũ�ν���� �׸���� ����(�������� �ִϸ��̼��� ���ؼ� ���� ������ ����)
	UPROPERTY(EditAnywhere, Category = CrossHairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = CrossHairs)
	class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = CrossHairs)
	class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = CrossHairs)
	class UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = CrossHairs)
	class UTexture2D* CrosshairsBottom;

	// ������ ���� Ÿ�� ����
	UPROPERTY(EditAnywhere)
	EFireType FireType;

	// �߻���� ����
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f; // Ÿ�̸� �� �ֱ�(BP���� ���⸶�� �ٸ��� �����Ͽ� ����ӵ� ���� ����

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true; // ����, ���� ���� ����

	// �÷��̾ ���� ������ ����� ����
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	// ������ �ƿ����� ������ ���� (depth �� ���)
	void EnableCustomDepth(bool bEnable);

	// �⺻������ ���⸦ �ı��ϱ� ���� ���� �÷��̾ SpawnDefaultWeapon �Լ��� ȣ���Ҷ� �ش繫�⸸ true ������ �����Ѵ�.
	bool bDestroyWeapon = false;

	// ź���� ����
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	FVector TraceEndWithScatter(const FVector& HitTarget);

protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();

	// ������¿����� ���� �����丵
	virtual void OnEquipped();
	virtual void OnDroped();
	virtual void OnEquippedSecondary();

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

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class APlayerCharacter* PlayerOwnerCharacter;

	UPROPERTY()
	class AFirstPlayerController* PlayerOwnerController;

	// ��������Ʈ ���ε�
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UWidgetComponent* PickupWidget;

	// ������ ���°��� �����Ͽ� ����� �� �ֵ��� ��ũ�θ� ����Ѵ�.
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon")
	EWeaponState WeaponState;

	// ���� ����Ǹ� ȣ��Ǵ� �Լ�
	UFUNCTION()
	void OnRep_WeaponState();

	// ������ �ִϸ��̼�
	UPROPERTY(EditAnywhere, Category = "Weapon")
	class UAnimationAsset* FireAnimation;

	// ź��
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;


	// �ܽ��� FOV �� ���� ����
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	// źâ ����
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// ó���������� ����������Ʈ�� ���ڸ� ���� ���� Ammo ����
	int32 Sequence = 0;

	// ������ Ÿ���� ����
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	ETeam Team;

public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE ETeam GetTeam() const { return Team; }

	// �ܺο��� ������ źâ�� ����ִ��� ������ �Լ�
	bool IsEmpty();

	// ������ źâ�� ���� á���� Ȯ�� �� �Լ�
	bool IsFull();

	// ���ο��� ź�హ���� ���� �� �ֵ���
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }

};

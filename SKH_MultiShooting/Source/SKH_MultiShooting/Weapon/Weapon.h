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
	// 복제용 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 오너가 바뀔때 실행되는 함수 재정의 (AActor상속)
	virtual void OnRep_Owner() override;

	// Ammo HUD 세팅용 호출함수
	void SetHUDAmmo();

	// 발사용 함수(재정의 할 수 있도록)
	virtual void Fire(const FVector& HitTarget);

	// 플레이어 사망시 무기 떨어뜰이기
	virtual void Dropped();

	// 재장전후 탄약 갯수 늘리기
	void AddAmmo(int32 AmmoToAdd);

	// 크로스헤어 그리기용 변수(벌어지는 애니메이션을 위해서 여러 부위로 나뉨)
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

	// 무기의 공격 타입 지정
	UPROPERTY(EditAnywhere)
	EFireType FireType;

	// 발사관련 변수
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f; // 타이머 콜 주기(BP에서 무기마다 다르게 설정하여 연사속도 조절 가능

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true; // 연사, 점사 무기 구분

	// 플레이어가 무기 장착시 재생할 사운드
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	// 무기의 아웃라인 윤곽선 설정 (depth 값 사용)
	void EnableCustomDepth(bool bEnable);

	// 기본생성된 무기를 파괴하기 위한 변수 플레이어가 SpawnDefaultWeapon 함수를 호출할때 해당무기만 true 값으로 설정한다.
	bool bDestroyWeapon = false;

	// 탄퍼짐 관련
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

	// 무기상태에따른 설정 리펙토링
	virtual void OnEquipped();
	virtual void OnDroped();
	virtual void OnEquippedSecondary();

	// 구체 콜리전 오버랩 이벤트와 바인딩할 함수
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

	// 델리게이트 바인딩
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UWidgetComponent* PickupWidget;

	// 무기의 상태값을 복사하여 사용할 수 있도록 매크로를 사용한다.
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon")
	EWeaponState WeaponState;

	// 값이 변경되면 호출되는 함수
	UFUNCTION()
	void OnRep_WeaponState();

	// 무기의 애니메이션
	UPROPERTY(EditAnywhere, Category = "Weapon")
	class UAnimationAsset* FireAnimation;

	// 탄피
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;


	// 줌시의 FOV 값 조절 관련
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	// 탄창 관련
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// 처리되지않은 서버리퀘스트의 숫자를 위한 변수 Ammo 관련
	int32 Sequence = 0;

	// 무기의 타입을 지정
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

	// 외부에서 접근할 탄창이 비어있는지 학인할 함수
	bool IsEmpty();

	// 장전중 탄창이 가득 찼는지 확인 할 함수
	bool IsFull();

	// 위부에서 탄약갯수를 얻을 수 있도록
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }

};

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

class USphereComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
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

	// 발사용 함수(재정의 할 수 있도록)
	virtual void Fire(const FVector& HitTarget);

	// 플레이어 사망시 무기 떨어뜰이기
	void Dropped();

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

	// 발사관련 변수
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f; // 타이머 콜 주기(BP에서 무기마다 다르게 설정하여 연사속도 조절 가능

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true; // 연사, 점사 무기 구분

protected:
	virtual void BeginPlay() override;

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


public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
};

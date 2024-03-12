#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SKH_MultiShooting/PlayerTypes/TurningInPlace.h"
#include "SKH_MultiShooting/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"

#include "PlayerCharacter.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API APlayerCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 복제용 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	// 몽타주 재생
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();

	// 캐릭터의 움직임이 변할때마다 호출되는 함수(매프레임X)
	virtual void OnRep_ReplicatedMovement() override;

	// 사망시 탈락 처리할 함수
	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

protected:
	virtual void BeginPlay() override;

	// 무브먼트
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// 액션
	virtual void Jump() override;
	void EquipButtonPressed(); //E Key
	void CrouchButtonPressed(); // L,R Shift key
	void AimButtonPressed(); // Right Mouse Button(Down)
	void AimButtonReleased(); // Right UnMouse Button(Up)
	void FireButtonPressed(); // Left Mouse Button(Down)
	void FireButtonReleased(); // Left Mouse Button(Up)

	// 애임오프셋
	void AimOffset(float DeltaTime);

	void CalculateAO_Pitch();

	// 심프록시 턴 함수
	void SimProxiesTurn();

	// 데미지 받을 콜백 함수 꼭 UFUNCTION매크로 사용
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// HUD 체력 업데이트 함수
	void UpdateHudHealth();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = Combat)
	class UCombatComponent* Combat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;


	// 오버랩된 무기를 복사 하며 복사된 값이 변경될때마다 특정 함수를 호출할 수 있도록 사용한다.
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	// 에임오프셋용 변수
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// 플레이어 회전 관련 변수
	ETurningInPlace TurningInplace;
	void TurnInPlace(float DeltaTime);

	// 애니메이션 몽타주 관련
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage;

	// 카메라 가려짐 보안
	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	// 루트본 회전 관련
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	// 속도 관련
	float CalculateSpeed();

	// 파티클 관련
	UPROPERTY(EditAnywhere)
	UParticleSystem* BloodParticles;

	// 플레이어의 체력 관련
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxHealth = 100.f; // 최대 체력치
	
	UPROPERTY(ReplicatedUsing = Onrep_Health, VisibleAnywhere, Category = "Player State")
	float Health = 100.f; // 현재 체력치

	UFUNCTION()
	void OnRep_Health(); // 복제변수가사용될 함수

	class AFirstPlayerController* FirstPlayerController;

	// 캐릭터가 사망(탈락)헀는지 판단할 변수
	bool bElimmed = false;

	// 리스폰을 위한 변수
	FTimerHandle ElimTimer;
	void ElimTimerFinishied(); // 콜백용 함수

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 5.f;
	
	// 디졸브 이펙트 관련
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	// 런타임중 변경할 인스턴스
	UPROPERTY(VisibleAnywhere, Category = Elim)
	TArray<UMaterialInstanceDynamic*> DynamicDissolveMaterialInstances;

	// 머티리얼 인스턴스 BP에서 설정할 것
	UPROPERTY(EditAnywhere, Category = Elim)
	TArray<UMaterialInstance*> DissolveMaterialInstances;

	//UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//UMaterialInstance* DissolveMaterialInstance;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInplace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }

};

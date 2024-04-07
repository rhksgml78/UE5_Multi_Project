#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SKH_MultiShooting/PlayerTypes/TurningInPlace.h"
#include "SKH_MultiShooting/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "SKH_MultiShooting/PlayerTypes/CombatState.h"

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

	// 클라이언트 파괴용 함수
	virtual void Destroyed() override;

	virtual void PostInitializeComponents() override;

	// 몽타주 재생
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayReLoadMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();

	// 캐릭터의 움직임이 변할때마다 호출되는 함수(매프레임X)
	virtual void OnRep_ReplicatedMovement() override;

	// 사망시 탈락 처리할 함수
	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	// 게임의 상태별로 캐릭터의 입력을 제한하기 위한 변수
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	// 스코프 줌인 애니메이션 재생을 위한 변수
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	// HUD 체력 업데이트 함수
	void UpdateHudHealth();

	// HUD 쉴드 업데이트 함수
	void UpdateHudShield();

	// 처음무기 장착시 HUD의 탄창수 업데이트.
	void UpdateHUDAmmo();

	// HUD 스피드업 노출 함수
	void SetSpeedUi(bool isVisible);
	void SetSpeedUpBuff(bool isSpeedUpActive);

	// 기본무기 생성 함수
	void SpawnDefaultWeapon();

	bool bFinishedSwapping = false;

	/*
	서버의 되감기용 박스 충돌체 생성.
	각 충돌체는 단순박스이므로 비용이 크게 높지않다.
	때문에 주요 관절마다 배치.
	박스컴포넌트의 이름은 가능하면 관절과 동일하게.
	총 16개 관절 부위
	*/

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* foot_r;

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
	void ReloadButtonPressed(); // E Key
	void GrenadeButtonPressed(); // G Key

	// 애임오프셋
	void AimOffset(float DeltaTime);

	void CalculateAO_Pitch();

	// 심프록시 턴 함수
	void SimProxiesTurn();

	// 데미지 받을 콜백 함수 꼭 UFUNCTION매크로 사용
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// HUD와 관련된 것들을 초기화 하기위한 함수
	void PollInit();

	// 캐릭터가 사망후 제자리에만 회전할 수 있는 함수
	void RotateInPlace(float DeltaTime);

	// 캐릭터사망시 소지한 아이템 처리 함수
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	// 전투와 관련된 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta =  (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	// 플레이어의 추가 능력치와 관련된 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	// 서버에서 지연보상에 사용할 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;

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

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReLoadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* SwapMontage;

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

	UPROPERTY(ReplicatedUsing = OnRep_SpeedUp, VisibleAnywhere, Category = "Player State")
	bool SpeedUpBuff = false;

	UFUNCTION()
	void OnRep_SpeedUp(); 

	// 플레이어의 체력 관련
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxHealth = 100.f; // 최대 체력치
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player State")
	float Health = 100.f; // 현재 체력치

	UFUNCTION()
	void OnRep_Health(float LastHealth); // 복제변수가사용될 함수

	// 플레이어의 쉴드 관련
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player State")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield); // 복제변수가사용될 함수

	UPROPERTY()
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

	// 플레이어 사망시 추가 이펙트 관련
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class AFirstPlayerState* FirstPlayerState;

	// 수류탄
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* AttachedGrenade;

	// 게임 시작시 기본적으로 들고있을 무기
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	ECombatState GetCombatState() const;
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInplace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }

	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReLoadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE bool GetIsSpeedUpBuff() { return SpeedUpBuff; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
};

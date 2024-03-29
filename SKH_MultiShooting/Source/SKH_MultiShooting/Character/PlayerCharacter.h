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

	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Ŭ���̾�Ʈ �ı��� �Լ�
	virtual void Destroyed() override;

	virtual void PostInitializeComponents() override;

	// ��Ÿ�� ���
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayReLoadMontage();
	void PlayThrowGrenadeMontage();

	// ĳ������ �������� ���Ҷ����� ȣ��Ǵ� �Լ�(��������X)
	virtual void OnRep_ReplicatedMovement() override;

	// ����� Ż�� ó���� �Լ�
	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	// ������ ���º��� ĳ������ �Է��� �����ϱ� ���� ����
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	// ������ ���� �ִϸ��̼� ����� ���� ����
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

protected:
	virtual void BeginPlay() override;

	// �����Ʈ
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// �׼�
	virtual void Jump() override;
	void EquipButtonPressed(); //E Key
	void CrouchButtonPressed(); // L,R Shift key
	void AimButtonPressed(); // Right Mouse Button(Down)
	void AimButtonReleased(); // Right UnMouse Button(Up)
	void FireButtonPressed(); // Left Mouse Button(Down)
	void FireButtonReleased(); // Left Mouse Button(Up)
	void ReloadButtonPressed(); // E Key
	void GrenadeButtonPressed(); // T Key ( G Ű�κ����� ��������)

	// ���ӿ�����
	void AimOffset(float DeltaTime);

	void CalculateAO_Pitch();

	// �����Ͻ� �� �Լ�
	void SimProxiesTurn();

	// ������ ���� �ݹ� �Լ� �� UFUNCTION��ũ�� ���
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// HUD ü�� ������Ʈ �Լ�
	void UpdateHudHealth();

	// HUD�� ���õ� �͵��� �ʱ�ȭ �ϱ����� �Լ�
	void PollInit();

	// ĳ���Ͱ� ����� ���ڸ����� ȸ���� �� �ִ� �Լ�
	void RotateInPlace(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	// ������ ���õ� ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta =  (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	// �÷��̾��� �߰� �ɷ�ġ�� ���õ� ������Ʈ
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	// �������� ���⸦ ���� �ϸ� ����� ���� ����ɶ����� Ư�� �Լ��� ȣ���� �� �ֵ��� ����Ѵ�.
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	// ���ӿ����¿� ����
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// �÷��̾� ȸ�� ���� ����
	ETurningInPlace TurningInplace;
	void TurnInPlace(float DeltaTime);

	// �ִϸ��̼� ��Ÿ�� ����
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

	// ī�޶� ������ ����
	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	// ��Ʈ�� ȸ�� ����
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	// �ӵ� ����
	float CalculateSpeed();

	// �÷��̾��� ü�� ����
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxHealth = 100.f; // �ִ� ü��ġ
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player State")
	float Health = 100.f; // ���� ü��ġ

	UFUNCTION()
	void OnRep_Health(); // �������������� �Լ�

	UPROPERTY()
	class AFirstPlayerController* FirstPlayerController;

	// ĳ���Ͱ� ���(Ż��)������ �Ǵ��� ����
	bool bElimmed = false;

	// �������� ���� ����
	FTimerHandle ElimTimer;
	void ElimTimerFinishied(); // �ݹ�� �Լ�

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 5.f;
	
	// ������ ����Ʈ ����
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	// ��Ÿ���� ������ �ν��Ͻ�
	UPROPERTY(VisibleAnywhere, Category = Elim)
	TArray<UMaterialInstanceDynamic*> DynamicDissolveMaterialInstances;

	// ��Ƽ���� �ν��Ͻ� BP���� ������ ��
	UPROPERTY(EditAnywhere, Category = Elim)
	TArray<UMaterialInstance*> DissolveMaterialInstances;

	//UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//UMaterialInstance* DissolveMaterialInstance;

	// �÷��̾� ����� �߰� ����Ʈ ����
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class AFirstPlayerState* FirstPlayerState;

	// ����ź
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* AttachedGrenade;

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
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReLoadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
};

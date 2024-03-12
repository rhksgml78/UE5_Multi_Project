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

	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	// ��Ÿ�� ���
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();

	// ĳ������ �������� ���Ҷ����� ȣ��Ǵ� �Լ�(��������X)
	virtual void OnRep_ReplicatedMovement() override;

	// ����� Ż�� ó���� �Լ�
	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

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

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = Combat)
	class UCombatComponent* Combat;

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

	// ��ƼŬ ����
	UPROPERTY(EditAnywhere)
	UParticleSystem* BloodParticles;

	// �÷��̾��� ü�� ����
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxHealth = 100.f; // �ִ� ü��ġ
	
	UPROPERTY(ReplicatedUsing = Onrep_Health, VisibleAnywhere, Category = "Player State")
	float Health = 100.f; // ���� ü��ġ

	UFUNCTION()
	void OnRep_Health(); // �������������� �Լ�

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

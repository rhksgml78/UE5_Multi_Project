#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "PlayerCharacter.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

	// �����Ʈ
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// �׼�
	void EquipButtonPressed(); //E Key
	void CrouchButtonPressed(); // L,R Shift key
	void AimButtonPressed(); // Right Mouse
	void AimButtonReleased(); // Right UnMouse

	// ���ӿ�����
	void AimOffset(float DeltaTime);

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
	float AO_Pitch;
	FRotator StartingAimRotation;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
};

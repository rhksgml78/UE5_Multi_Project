#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SKH_MultiShooting/PlayerTypes/TurningInPlace.h"

#include "PlayerAnimInstance.generated.h"

//�ִϸ��̼� �ν��Ͻ������� ���������ַ� ����

UCLASS()
class SKH_MULTISHOOTING_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// �ִϸ��̼� ó�� ������Ʈ �ѹ�
	virtual void NativeInitializeAnimation() override;
	
	// �ִϸ��̼� �ν��Ͻ������� �ش� �Լ��� Tick�� ���� ������ �Ѵ�.
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Charcter, meta = (AllowPrivateAccess = "true"))
	class APlayerCharacter* PlayerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerationg;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	// ���ӿ����¿� Yaw Pitch
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	// FABRIK IK ���� (�޼�. �������� �����̿� �����Ǿ�����) ���⿡ �޼��� ���� ���� �����ΰ� ����ġ�� �����Ұ�
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;


	// �÷��̾� ȸ�� ���� ����
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInplace;

	// �÷��̾��� ������ ȸ�� ����
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	// �����պ����� ��Ʈ���ϰ��ִ� �÷��̾ �����Ǿ� ���̵��� �ϱ� ���� ����
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	// ��Ʈ�� ȸ�� ����
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

};

#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SKH_MultiShooting/Weapon/Weapon.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// �÷��̾� ĳ���������� ĳ����
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// �÷��̾ null�϶� �ٽ��ѹ� ĳ����
	if (PlayerCharacter == nullptr)
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	// ���� ĳ������ �����Ѵٸ� ��ȯ
	if (PlayerCharacter == nullptr) return;

	// ĳ������ �����Ͽ��ٸ� �Ʒ��۾� ����
	FVector Velocity = PlayerCharacter->GetVelocity();
	
	// ĳ������ ���� ���� �����ϰ�
	Velocity.Z = 0.f;

	// ������ ������ ũ�⸦ �ӵ��� �������ش�.
	Speed = Velocity.Size();

	// ĳ���Ͱ� ���߿� �ְų� �������ϰ��
	bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();

	// ���� �����ڸ� ����� ĳ������ ���ӵ��� ���� ����. ĳ������ ���� ��ġ�� 0���� Ŭ��� true ��ȯ, ������� false ��ȯ.
	bIsAccelerationg = PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	// ���⸦ �����ߴ���
	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();

	// FABRIK IK�� ���� ������ �� �ֱ�
	EquippedWeapon = PlayerCharacter->GetEquippedWeapon();

	// ĳ���Ͱ� ��ũ�ȴ���
	bIsCrouched = PlayerCharacter->bIsCrouched;

	// ĳ���Ͱ� �����ϰ��ִ���
	bAiming = PlayerCharacter->IsAiming();

	// ȸ������ ����ȭ. �ν��Ͻ��� ���� �÷��̾� ���ֱ�
	TurningInplace = PlayerCharacter->GetTurningInPlace();

	// ������ ȸ����(Yaw)
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	// -180���� 180������ ������ �ε巴�� ����
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;
	
	// ���� Lean
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	// �÷��̾� Ŭ�������� ���� AO_Yaw ���� �ִ´�.
	AO_Yaw = PlayerCharacter->GetAO_Yaw();
	AO_Pitch = PlayerCharacter->GetAO_Pitch();


	// FABRIK IK
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PlayerCharacter->GetMesh())
	{
		// �޼� ��ġ�� ������ ���� ���� ��ġ(����)����
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPosition;
		FRotator OutRotation;

		PlayerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

		// �޼��� ��ġ�� ȸ������ ����
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}

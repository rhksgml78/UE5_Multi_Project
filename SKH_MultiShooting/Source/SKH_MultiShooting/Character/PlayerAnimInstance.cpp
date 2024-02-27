#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

	// ĳ���Ͱ� ��ũ�ȴ���
	bIsCrouched = PlayerCharacter->bIsCrouched;

	// ĳ���Ͱ� �����ϰ��ִ���
	bAiming = PlayerCharacter->IsAiming();

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
}

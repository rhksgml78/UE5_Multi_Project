#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();

}

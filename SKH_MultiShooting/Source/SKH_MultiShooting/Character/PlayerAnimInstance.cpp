#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SKH_MultiShooting/Weapon/Weapon.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 플레이어 캐릭터형으로 캐스팅
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// 플레이어가 null일때 다시한번 캐스팅
	if (PlayerCharacter == nullptr)
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	// 만일 캐스팅이 실패한다면 반환
	if (PlayerCharacter == nullptr) return;

	// 캐스팅이 성공하였다면 아래작업 진행
	FVector Velocity = PlayerCharacter->GetVelocity();
	
	// 캐릭터의 높이 값은 제외하고
	Velocity.Z = 0.f;

	// 벡터의 순수한 크기를 속도로 지정해준다.
	Speed = Velocity.Size();

	// 캐릭터가 공중에 있거나 낙하중일경우
	bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();

	// 삼항 연산자를 사용한 캐릭터의 가속도에 대한 변수. 캐릭터의 가속 수치가 0보다 클경우 true 반환, 작을경우 false 반환.
	bIsAccelerationg = PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	// 무기를 장착했는지
	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();

	// FABRIK IK용 무기 변수에 값 넣기
	EquippedWeapon = PlayerCharacter->GetEquippedWeapon();

	// 캐릭터가 웅크렸는지
	bIsCrouched = PlayerCharacter->bIsCrouched;

	// 캐릭터가 조준하고있는지
	bAiming = PlayerCharacter->IsAiming();

	// 회전변수 동기화. 인스턴스의 값에 플레이어 값넣기
	TurningInplace = PlayerCharacter->GetTurningInPlace();

	// 애임의 회전값(Yaw)
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	// -180도와 180도간의 보간을 부드럽게 보간
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;
	
	// 기울기 Lean
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	// 플레이어 클래스에서 계산된 AO_Yaw 값을 넣는다.
	AO_Yaw = PlayerCharacter->GetAO_Yaw();
	AO_Pitch = PlayerCharacter->GetAO_Pitch();


	// FABRIK IK
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PlayerCharacter->GetMesh())
	{
		// 왼손 위치의 변수에 무기 소켓 위치(월드)저장
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPosition;
		FRotator OutRotation;

		PlayerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

		// 왼손의 위치와 회전값을 저장
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}

#include "BuffComponent.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	// 호출 점화식을 생성하고 회복될 값과 회복시간을 정하고 총회복량을 기록한다.
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	// 호출 점화식을 생성하고 회복될 값과 회복시간을 정하고 총회복량을 기록한다.
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffJumpZVelocity, float BuffTime)
{
	if (Character == nullptr || 
		Character->GetCharacterMovement() == nullptr) return;

	// 버프시간만큼의 타이머를 설정하고 버프타임종료시 초기화 함수를 호출 한다.
	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&ThisClass::ResetSpeed,
		BuffTime
	);

	// BuffTime 시간동안만큼 속도를 상승 시킨다.
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpZVelocity;
	};

	// 서버와 클라이언트간의 차이를 없애기 위해서 멀티캐스트 RPC 함수를 호출 한다.
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed, BuffJumpZVelocity);
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || 
		Character->GetCharacterMovement() == nullptr) return;

	// 저장해둔 원래의 값으로 초기화
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity;
	};
	if (Character)
	{
		// 스피드UI OFF
		Character->SetSpeedUpBuff(false);
	}
	// 서버와 클라이언트간의 차이를 없애기 위해서 멀티캐스트 RPC 함수를 호출 한다.
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed, InitialJumpZVelocity);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CoruchSpeed, float JumpZVelocity)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CoruchSpeed;
	Character->GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CoruchSpeed, float JumpZVelocity)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CoruchSpeed;
	InitialJumpZVelocity = JumpZVelocity;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || 
		Character == nullptr || 
		Character->IsElimmed()) return;

	// 매프레임 값을 갱신하며 회복한다.
	const float HealThisFrame = HealingRate * DeltaTime;

	Character->SetHealth(FMath::ClampAngle(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));

	// 플레이어의 체력게이지 업데이트
	Character->UpdateHudHealth();

	// 매프레임 회복된 값만큼 회복할 값을 감소 시킨다.
	AmountToHeal -= HealThisFrame;

	// 초기화조건식 회복량이 0이되었거나 플레이어의 체력이 최대치에 도달하였을 경우
	if (AmountToHeal <= 0 || 
		Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield ||
		Character == nullptr ||
		Character->IsElimmed()) return;

	// 매프레임 값을 갱신하며 회복한다.
	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;

	Character->SetShield(FMath::ClampAngle(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));

	// 플레이어의 체력게이지 업데이트
	Character->UpdateHudShield();

	// 매프레임 회복된 값만큼 회복할 값을 감소 시킨다.
	ShieldReplenishAmount -= ReplenishThisFrame;

	// 초기화조건식 회복량이 0이되었거나 플레이어의 체력이 최대치에 도달하였을 경우
	if (ShieldReplenishAmount <= 0 ||
		Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}


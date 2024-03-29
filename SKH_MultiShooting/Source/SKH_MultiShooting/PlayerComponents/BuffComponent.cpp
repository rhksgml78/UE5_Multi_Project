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
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	// ȣ�� ��ȭ���� �����ϰ� ȸ���� ���� ȸ���ð��� �����ϰ� ��ȸ������ ����Ѵ�.
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr || 
		Character->GetCharacterMovement() == nullptr) return;

	// �����ð���ŭ�� Ÿ�̸Ӹ� �����ϰ� ����Ÿ������� �ʱ�ȭ �Լ��� ȣ�� �Ѵ�.
	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&ThisClass::ResetSpeed,
		BuffTime
	);

	// BuffTime �ð����ȸ�ŭ �ӵ��� ��� ��Ų��.
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	};

	// ������ Ŭ���̾�Ʈ���� ���̸� ���ֱ� ���ؼ� ��Ƽĳ��Ʈ RPC �Լ��� ȣ�� �Ѵ�.
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || 
		Character->GetCharacterMovement() == nullptr) return;

	// �����ص� ������ ������ �ʱ�ȭ
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	};
	if (Character)
	{
		// ���ǵ�UI OFF
		Character->SetSpeedUpBuff(false);
	}
	// ������ Ŭ���̾�Ʈ���� ���̸� ���ֱ� ���ؼ� ��Ƽĳ��Ʈ RPC �Լ��� ȣ�� �Ѵ�.
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CoruchSpeed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CoruchSpeed;
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CoruchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CoruchSpeed;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || 
		Character == nullptr || 
		Character->IsElimmed()) return;

	// �������� ���� �����ϸ� ȸ���Ѵ�.
	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::ClampAngle(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));

	// �÷��̾��� ü�°����� ������Ʈ
	Character->UpdateHudHealth();

	// �������� ȸ���� ����ŭ ȸ���� ���� ���� ��Ų��.
	AmountToHeal -= HealThisFrame;


	// �ʱ�ȭ���ǽ� ȸ������ 0�̵Ǿ��ų� �÷��̾��� ü���� �ִ�ġ�� �����Ͽ��� ���
	if (AmountToHeal <= 0 || 
		Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}


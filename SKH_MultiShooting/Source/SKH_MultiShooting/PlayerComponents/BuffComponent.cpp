#include "BuffComponent.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"


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


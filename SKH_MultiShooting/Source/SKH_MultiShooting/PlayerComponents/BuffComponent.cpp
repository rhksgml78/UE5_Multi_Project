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
	// 호출 점화식을 생성하고 회복될 값과 회복시간을 서정하고 총회복량을 기록한다.
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
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


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKH_MULTISHOOTING_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class APlayerCharacter;
	void Heal(float HealAmount, float HealingTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffJumpZVelocity, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CoruchSpeed, float JumpZVelocity);

protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);

private:
	UPROPERTY()
	class APlayerCharacter* Character;

	// ȸ������
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	// �ӵ���� ����
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	float InitialJumpZVelocity;

	// ������ Ŭ���̾�Ʈ���� ������ ���̷� ���Ͽ� �ӵ� ������ �ð��� ������ ���� �� �ֱ⶧���� �̸� �����ϱ� ���Ͽ� ��Ƽĳ��ƮRPC�� ����Ͽ� ��� �����ϰ� �Ѵ�.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CoruchSpeed, float JumpZVelocity);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

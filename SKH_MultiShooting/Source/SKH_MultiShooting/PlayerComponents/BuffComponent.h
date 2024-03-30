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

	// 회복관련
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	// 속도상승 관련
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	float InitialJumpZVelocity;

	// 서버와 클라이언트간의 프레임 차이로 인하여 속도 버프에 시간적 오차가 생길 수 있기때문에 이를 방지하기 위하여 멀티캐스트RPC를 사용하여 모두 동등하게 한다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CoruchSpeed, float JumpZVelocity);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

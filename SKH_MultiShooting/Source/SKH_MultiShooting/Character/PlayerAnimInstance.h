#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

//애니메이션 인스턴스에서는 변수들위주로 선언

UCLASS()
class SKH_MULTISHOOTING_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 애니메이션 처음 업데이트 한번
	virtual void NativeInitializeAnimation() override;
	
	// 애니매이션 인스턴스에서는 해당 함수가 Tick과 같은 역할을 한다.
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Charcter, meta = (AllowPrivateAccess = "true"))
	class APlayerCharacter* PlayerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerationg;
	
};

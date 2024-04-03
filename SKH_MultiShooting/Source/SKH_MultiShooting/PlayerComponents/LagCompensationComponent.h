#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "LagCompensationComponent.generated.h"

/*
해당 컴포넌트는 서버캐릭터가 클라이언트 캐릭터의 정보를 저장하여 대조하고
지연에 관하여 되감기를 사용할 수 있도록 하기위한 컴포넌트

매프레임의 정보를 저장할 수 있어야한다 Frame History
저장할때 단순히 플레이어의 위치만 저장하기보다는 플레이어의 충돌과관련된
정보를 저장해야하며 해당 정보의 디테일은 설정에 따라 다르다.
디테일할 수록 성능에 부담이 되고 비용이 높다.

가장 저렴한것은 캡슐 = 정확도 떨어짐
중간단계는 박스단위 충돌체를 플레이어 관절마다 배치 = 어느정도 정확함
가방 비용이높은것은 메쉬단위 = 정확한 캐릭터형태충돌은 매우비싸다!
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKH_MULTISHOOTING_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:	

		
};

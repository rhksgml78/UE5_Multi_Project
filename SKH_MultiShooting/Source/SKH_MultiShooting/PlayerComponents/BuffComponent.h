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

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class APlayerCharacter* Character;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

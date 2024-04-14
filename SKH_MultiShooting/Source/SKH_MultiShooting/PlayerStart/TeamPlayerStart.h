#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SKH_MultiShooting/PlayerTypes/Team.h"

#include "TeamPlayerStart.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
};

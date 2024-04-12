#pragma once

#include "CoreMinimal.h"
#include "PlayerGameMode.h"

#include "TeamsGameMode.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API ATeamsGameMode : public APlayerGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();

	// 플레이어가 접속할때
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// 플레이어가 떠날때
	virtual void Logout(AController* Exiting) override;

	// 데미지 전달
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;

	// 점수 계산
	virtual void PlayerEliminated(class APlayerCharacter* ElimmedCharacter, class AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController) override;

protected:
	// 매치스테이트가 변경될경우 실행되는 함수 재정의
	virtual void HandleMatchHasStarted() override;

};

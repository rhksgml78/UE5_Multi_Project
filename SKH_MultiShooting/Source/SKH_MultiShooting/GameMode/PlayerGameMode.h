#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "PlayerGameMode.generated.h"

/*
플레이어 게임모드는 게임에서 플레이어의 탈락 등 관리
*/

UCLASS()
class SKH_MULTISHOOTING_API APlayerGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// Game State
	APlayerGameMode();
	virtual void Tick(float DeltaTime) override;

	virtual void PlayerEliminated(class APlayerCharacter* ElimmedCharacter, class AFirstPlayerController* VictimController, AFirstPlayerController* AttackerController);
	
	// 리스폰
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	// 플레이를 시작할 타이머
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; // 대기시간

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

private:
	float CountdownTime = 0.f;
	
};

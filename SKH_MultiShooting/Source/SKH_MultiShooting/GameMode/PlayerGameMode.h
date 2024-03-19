#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "PlayerGameMode.generated.h"

// 커스텀 매치 스테이트
namespace MatchState
{
	// 정해진 경기시간이 종료된후 승자를 표시할 수 있는 상태
	extern SKH_MULTISHOOTING_API const FName Cooldown;
}

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
	float WarmupTime = 10.f; // 대기 시간

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f; // 게임 진행시간

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f; // 게임 시간이 종료되고 쿨타임

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

	// 게임의 매치 상태 재정의
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	
};

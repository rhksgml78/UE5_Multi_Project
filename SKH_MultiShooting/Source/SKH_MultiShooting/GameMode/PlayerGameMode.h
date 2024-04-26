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

	// 플레이어가 게임을 떠날때
	void PlayerLeftGame(class AFirstPlayerState* PlayerLeaving);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	// 게임모드 타이머 변수
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; // 대기 시간 (플레이어 입장 및 맵 로드)

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f; // 실제 게임을 플레이 할 수 있는 시간

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f; // 게임 시간이 종료후 정산을 위한 쿨타임

	float LevelStartingTime = 0.f;

	// 게임모드가 팀전인지 개인전인지 확인하는 변수
	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;

	// 게임의 매치 상태 재정의
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	
};

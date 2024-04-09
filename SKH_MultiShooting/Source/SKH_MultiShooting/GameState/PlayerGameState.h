#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "PlayerGameState.generated.h"

// 최다 득점한 플레이어의 정보를 처리

UCLASS()
class SKH_MULTISHOOTING_API APlayerGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	// 복제용 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(class AFirstPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<class AFirstPlayerState*> TopScoringPlayers;

	// 팀 설정 관련
	TArray<AFirstPlayerState*> RedTeam;
	TArray<AFirstPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();

private:
	// 갱신해줄 최대 득점 변수
	float TopScore = 0.f;
};

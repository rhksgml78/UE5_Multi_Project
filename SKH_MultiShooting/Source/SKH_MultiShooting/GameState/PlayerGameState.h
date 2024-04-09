#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "PlayerGameState.generated.h"

// �ִ� ������ �÷��̾��� ������ ó��

UCLASS()
class SKH_MULTISHOOTING_API APlayerGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(class AFirstPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<class AFirstPlayerState*> TopScoringPlayers;

	// �� ���� ����
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
	// �������� �ִ� ���� ����
	float TopScore = 0.f;
};

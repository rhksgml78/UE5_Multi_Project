#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"

#include "PlayerGameMode.generated.h"

// Ŀ���� ��ġ ������Ʈ
namespace MatchState
{
	// ������ ���ð��� ������� ���ڸ� ǥ���� �� �ִ� ����
	extern SKH_MULTISHOOTING_API const FName Cooldown;
}

/*
�÷��̾� ���Ӹ��� ���ӿ��� �÷��̾��� Ż�� �� ����
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
	
	// ������
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	// �÷��̸� ������ Ÿ�̸�
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; // ��� �ð�

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f; // ���� ����ð�

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f; // ���� �ð��� ����ǰ� ��Ÿ��

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

	// ������ ��ġ ���� ������
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	
};

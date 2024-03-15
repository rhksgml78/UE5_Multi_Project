#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FirstPlayerState.generated.h"

// �÷��̾��� ���� �����͸� �����ϴ� Ŭ����

UCLASS()
class SKH_MULTISHOOTING_API AFirstPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// OnRep �Լ��� Ŭ���̾�Ʈ������ ���� ����ȴ�.
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	// �������� ������Ʈ�� �Լ��� ���� �����.
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

private:

	UPROPERTY()
	class APlayerCharacter* Character;

	UPROPERTY()
	class AFirstPlayerController* Controller;
	
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

};

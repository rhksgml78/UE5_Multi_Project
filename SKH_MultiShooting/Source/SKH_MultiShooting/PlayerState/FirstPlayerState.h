#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FirstPlayerState.generated.h"

// 플레이어의 상태 데이터를 관리하는 클래스

UCLASS()
class SKH_MULTISHOOTING_API AFirstPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	// 복제용 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// OnRep 함수는 클라이언트에서만 복제 실행된다.
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	// 서버에서 업데이트할 함수를 따로 만든다.
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

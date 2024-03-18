#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FirstPlayerController.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AFirstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// HUD의 값 설정
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);

	void PlayDefeatsAnimation();

	// 플레이어 빙의시 바로 한번 업데이트
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

	/*
	*서버와 클라이언트의 시간차이 동기화
	*/
	// 현재 서버의 시간을 요청(클라이언트의 현재 시간기준)
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimOfClientRequest);

	// 클라이언트가 서버에 현재시간을 요청하였을경우 서버의 시간을 얻는다.
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// 클라이언트와 서버간의 차이시간을 저장할 멤버 변수
	float ClientServerDelta = 0.f; 

	// 서버의 동기화된 시간을 얻는 가상함수
	virtual float GetServerTime();

	// 빠른 동기화를 위한 함수
	virtual void ReceivedPlayer() override;

	// 동기화를 얼마나 자주시킬지의 변수
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	// 동기화 시간 체크
	void CheckTimeSync(float DeltaTime);

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();


private:

	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	float MatchTime = 600.f;
	uint32 CountDownInt = 0;
	
};

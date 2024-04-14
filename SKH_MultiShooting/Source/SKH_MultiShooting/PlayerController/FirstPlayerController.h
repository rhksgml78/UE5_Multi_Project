#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FirstPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class SKH_MULTISHOOTING_API AFirstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// HUD의 값 설정
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);

	// 플레이어 사망시 재생할 애니메이션 호출 함수
	void PlayDefeatsAnimation();

	// 복제용 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 플레이어 빙의시 바로 한번 업데이트
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

	// 서버의 동기화된 시간을 얻는 가상함수
	virtual float GetServerTime();

	// 빠른 동기화를 위한 함수
	virtual void ReceivedPlayer() override;

	// 게임의 매치 상태 관련
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);

	// 게임이 종료시간이 되었을때 쿨다운상태에서 실행
	void HandleCooldown();

	// 서버와 클라이언트간에 동기화 예측시간의 반
	float SingleTripTime = 0.f;

	// 핑 델리게이트
	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	// 키입력 바인딩
	virtual void SetupInputComponent() override;

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
	// 동기화를 얼마나 자주시킬지의 변수
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	// 동기화 시간 체크
	void CheckTimeSync(float DeltaTime);

	// 서버의 매치상태를 확인하는 함수
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// 클라이언트가 중간난입 했을때
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	// 인터넷 연결상태에 측정 관련 함수
	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	// 바인딩 콜백 함수
	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	// 게임모드의 상태(팀전)에 관한 복제변수
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	// 게임모드가 개인전일떄
	FString GetInfoText(const TArray<class AFirstPlayerState*>& Players);

	int32 WinnerInfo = 0;
	FString GetWinnerTeamInfoText(int32 WinnerTeam);
	FLinearColor GetWinnerTeamInfoTextColor(int32 WinnerTeam);

	// 게임모드가 팀전일때
	FString GetTeamsInfoText(class APlayerGameState* PlayerGameState);

private:
	// 메인메뉴 위젯
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	UPROPERTY()
	class APlayerGameMode* PlayerGameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountDownInt = 0;

	// 게임의 매치 상태를 확인하기위한 변수 서버와 클라이언트 모두가 알고 있으야하므로 복사 변수로 만든다
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UPlayerOverlay* PlayerOverlay;

	// 플레이어의 HUD 초기값을 설정할 변수들
	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false; // 초기화를 했는지 확인할 변수

	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false; // 초기화를 했는지 확인할 변수

	float HUDScore;
	bool bInitializeScore = false; // 초기화를 했는지 확인할 변수

	int32 HUDDefeats;
	bool bInitializeDefeats = false; // 초기화를 했는지 확인할 변수

	int32 HUDGrenades;
	bool bInitializeGrenades = false; // 초기화를 했는지 확인할 변수

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false; // 초기화를 했는지 확인할 변수

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false; // 초기화를 했는지 확인할 변수

	// 플레이어의 핑을 체크 관련
	float HighPingRunningTime = 0.f; // 애니메이션이 노출된 시간

	float PingAnimationRuningTime = 0.f; // 애니메이션을 노출된 시간

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f; // 애니메이션을 노출할 시간

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f; // 핑을 체크하고자 하는 주기

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f; // 주의 기준치가될 핑 수치

	// 서버에 핑의 상태를 전송할 함수
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

public:
	FORCEINLINE APlayerHUD* GetPlayerHUD() const { return PlayerHUD; }
	void SetSpeedUi(bool isVisible);
};

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
	// HUD�� �� ����
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

	// �÷��̾� ����� ����� �ִϸ��̼� ȣ�� �Լ�
	void PlayDefeatsAnimation();

	// ������ �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// �÷��̾� ���ǽ� �ٷ� �ѹ� ������Ʈ
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

	// ������ ����ȭ�� �ð��� ��� �����Լ�
	virtual float GetServerTime();

	// ���� ����ȭ�� ���� �Լ�
	virtual void ReceivedPlayer() override;

	// ������ ��ġ ���� ����
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);

	// ������ ����ð��� �Ǿ����� ��ٿ���¿��� ����
	void HandleCooldown();

	// ������ Ŭ���̾�Ʈ���� ����ȭ �����ð��� ��
	float SingleTripTime = 0.f;

	// �� ��������Ʈ
	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	// Ű�Է� ���ε�
	virtual void SetupInputComponent() override;

	/*
	*������ Ŭ���̾�Ʈ�� �ð����� ����ȭ
	*/
	// ���� ������ �ð��� ��û(Ŭ���̾�Ʈ�� ���� �ð�����)
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimOfClientRequest);

	// Ŭ���̾�Ʈ�� ������ ����ð��� ��û�Ͽ������ ������ �ð��� ��´�.
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// Ŭ���̾�Ʈ�� �������� ���̽ð��� ������ ��� ����
	float ClientServerDelta = 0.f;
	// ����ȭ�� �󸶳� ���ֽ�ų���� ����
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	// ����ȭ �ð� üũ
	void CheckTimeSync(float DeltaTime);

	// ������ ��ġ���¸� Ȯ���ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// Ŭ���̾�Ʈ�� �߰����� ������
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	// ���ͳ� ������¿� ���� ���� �Լ�
	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	// ���ε� �ݹ� �Լ�
	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	// ���Ӹ���� ����(����)�� ���� ��������
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	// ���Ӹ�尡 �������ϋ�
	FString GetInfoText(const TArray<class AFirstPlayerState*>& Players);

	int32 WinnerInfo = 0;
	FString GetWinnerTeamInfoText(int32 WinnerTeam);
	FLinearColor GetWinnerTeamInfoTextColor(int32 WinnerTeam);

	// ���Ӹ�尡 �����϶�
	FString GetTeamsInfoText(class APlayerGameState* PlayerGameState);

private:
	// ���θ޴� ����
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

	// ������ ��ġ ���¸� Ȯ���ϱ����� ���� ������ Ŭ���̾�Ʈ ��ΰ� �˰� �������ϹǷ� ���� ������ �����
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UPlayerOverlay* PlayerOverlay;

	// �÷��̾��� HUD �ʱⰪ�� ������ ������
	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false; // �ʱ�ȭ�� �ߴ��� Ȯ���� ����

	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false; // �ʱ�ȭ�� �ߴ��� Ȯ���� ����

	float HUDScore;
	bool bInitializeScore = false; // �ʱ�ȭ�� �ߴ��� Ȯ���� ����

	int32 HUDDefeats;
	bool bInitializeDefeats = false; // �ʱ�ȭ�� �ߴ��� Ȯ���� ����

	int32 HUDGrenades;
	bool bInitializeGrenades = false; // �ʱ�ȭ�� �ߴ��� Ȯ���� ����

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false; // �ʱ�ȭ�� �ߴ��� Ȯ���� ����

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false; // �ʱ�ȭ�� �ߴ��� Ȯ���� ����

	// �÷��̾��� ���� üũ ����
	float HighPingRunningTime = 0.f; // �ִϸ��̼��� ����� �ð�

	float PingAnimationRuningTime = 0.f; // �ִϸ��̼��� ����� �ð�

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f; // �ִϸ��̼��� ������ �ð�

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f; // ���� üũ�ϰ��� �ϴ� �ֱ�

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f; // ���� ����ġ���� �� ��ġ

	// ������ ���� ���¸� ������ �Լ�
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

public:
	FORCEINLINE APlayerHUD* GetPlayerHUD() const { return PlayerHUD; }
	void SetSpeedUi(bool isVisible);
};

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FirstPlayerController.generated.h"


UCLASS()
class SKH_MULTISHOOTING_API AFirstPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// HUD�� �� ����
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);

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
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();

	// ������ ����ð��� �Ǿ����� ��ٿ���¿��� ����
	void HandleCooldown();


protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();

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

private:

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
	bool bInitializePlayerOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	
};

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

	void PlayDefeatsAnimation();

	// �÷��̾� ���ǽ� �ٷ� �ѹ� ������Ʈ
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaTime) override;

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

	// ������ ����ȭ�� �ð��� ��� �����Լ�
	virtual float GetServerTime();

	// ���� ����ȭ�� ���� �Լ�
	virtual void ReceivedPlayer() override;

	// ����ȭ�� �󸶳� ���ֽ�ų���� ����
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	// ����ȭ �ð� üũ
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

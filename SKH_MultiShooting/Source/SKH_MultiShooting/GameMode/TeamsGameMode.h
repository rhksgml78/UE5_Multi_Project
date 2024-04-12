#pragma once

#include "CoreMinimal.h"
#include "PlayerGameMode.h"

#include "TeamsGameMode.generated.h"

UCLASS()
class SKH_MULTISHOOTING_API ATeamsGameMode : public APlayerGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();

	// �÷��̾ �����Ҷ�
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// �÷��̾ ������
	virtual void Logout(AController* Exiting) override;

	// ������ ����
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;

protected:
	// ��ġ������Ʈ�� ����ɰ�� ����Ǵ� �Լ� ������
	virtual void HandleMatchHasStarted() override;

};

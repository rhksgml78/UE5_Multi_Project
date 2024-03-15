#include "FirstPlayerState.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Net/UnrealNetwork.h"

void AFirstPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFirstPlayerState, Defeats);
}

void AFirstPlayerState::AddToScore(float ScoreAmount)
{
	// ������ ���� �Լ�
	SetScore(GetScore() + ScoreAmount);

	// ���� �������� nullptr �϶��� ĳ���� �ϵ���
	Character = Character == nullptr ? Cast<APlayerCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AFirstPlayerState::OnRep_Score()
{
	// Ŭ���̾�Ʈ�� ���� �Լ�

	Super::OnRep_Score();

	// ���� �������� nullptr �϶��� ĳ���� �ϵ���
	Character = Character == nullptr ?  Cast<APlayerCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			// �÷��̾� ������Ʈ�� �����ϴ� �⺻ ���� Score
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AFirstPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	// ����ȣ��� �Լ�
	Defeats += DefeatsAmount;

	Character = Character == nullptr ? Cast<APlayerCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AFirstPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<APlayerCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}


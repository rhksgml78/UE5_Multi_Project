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
	// 서버용 실행 함수
	SetScore(GetScore() + ScoreAmount);

	// 삼항 연산으로 nullptr 일때만 캐스팅 하도록
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
	// 클라이언트용 실행 함수

	Super::OnRep_Score();

	// 삼항 연산으로 nullptr 일때만 캐스팅 하도록
	Character = Character == nullptr ?  Cast<APlayerCharacter>(GetPawn()) : Character;

	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;

		if (Controller)
		{
			// 플레이어 스테이트가 제공하는 기본 변수 Score
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AFirstPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	// 서버호출용 함수
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


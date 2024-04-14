#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "SKH_MultiShooting/Weapon/Flag.h"
#include "SKH_MultiShooting/GameMode/CaptureTheFlagGameMode.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);

}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->GetTeam() == Team)
	{
		ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		
		// 서버에서만 게임모드에 접근이 가능하여 점수계산을 실행
		if (GameMode)
		{
			// 점수를 추가한다.
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		// 깃발 초기화
		OverlappingFlag->ReSetFlag();
	}
}



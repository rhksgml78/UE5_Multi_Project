#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	// 샷건은 상속된 FIre를 콜하지 않는대신 AWeapon클래스에있는 Fire 함수를 호출한다.
	//Super::Fire(HitTarget);

	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}

	// 모든 피격계산은 서버에서하기때문에 크라이언트에서 InstigatorController 는 항상 null이다.
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		FVector Start = SocketTransform.GetLocation();
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FVector End = TraceEndWithScatter(Start, HitTarget);
		}
	}
}

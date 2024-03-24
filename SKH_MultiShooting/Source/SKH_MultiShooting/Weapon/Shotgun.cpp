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

		// 산탄에 여러명의 플레이어가 피격하였을 경우를 위한 변수
		TMap<APlayerCharacter*, uint32> HitMap;

		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			// 샷건은 여러 파편에 맞은 횟수를 도합하여 데미지를 합산하고 한번에 전달시킨다.
			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(PlayerCharacter))
				{
					// HitMap 테이블에 플레이어가 이미 있을경우 해당플레이어의 값(int32)을 1증가시킨다.
					HitMap[PlayerCharacter]++;
				}
				else
				{
					// HitMap 테이블에 플레이어가 없을경우 해당플레이어를 테이블에 더하고 플레이어의 값(int32)을 1로 생성한다.
					HitMap.Emplace(PlayerCharacter, 1);
				}
			}

			if (ImpactParticles) // 피격 파티클 재생
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound) // 피격사운드 재생
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					FMath::FRandRange(-.5f, .5f) // 산탄각각에 랜덤값
				);
			}
		}
		for (auto HitPair : HitMap)
		{
			// 피격 반복문이 끝난후 생성한 테이블을 검사한다. 해당 반복문은 샷건에 피격단한 플레이어의 수만큼 반복된다.

			if (InstigatorController)
			{
				if (HitPair.Key && HasAuthority() && InstigatorController)
				{
					// 테이블의 키(플레이어클래스)에게 테이블의 값(피격한횟수)만큼 곱해진 데미지를 전달한다.(샷건의경우 한발 한발의 데미지가 설정되기때문에 낮은 데미지로 지정할 것
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						Damage*HitPair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
	}
}

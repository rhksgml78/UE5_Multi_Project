#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "SKH_MultiShooting/PlayerComponents/LagCompensationComponent.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// 피격횟수만큼의 데미지를 저장하기위한 맵
		TMap<APlayerCharacter*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter)
			{
				if (HitMap.Contains(PlayerCharacter))
				{
					HitMap[PlayerCharacter]++;
				}
				else
				{
					HitMap.Emplace(PlayerCharacter, 1);
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
						FMath::FRandRange(-.5f, .5f)
					);
				}
				if (DebugEndSphere)
				{
					DrawDebugSphere(GetWorld(), FireHit.ImpactPoint, 16.f, 12, FColor::Orange, true);
				}
			}
		}

		// 서버측 되감기에 사용할 임시 변수
		TArray<APlayerCharacter*> HitCharacters;

		for (auto HitPair : HitMap)
		{
			// 피격 반복문이 끝난후 생성한 테이블을 검사한다. 해당 반복문은 샷건에 피격단한 플레이어의 수만큼 반복된다.
			if (HitPair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();

				if (HasAuthority() && bCauseAuthDamage) // 서버 일때
				{
					 // 되감기 설정 OFF
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						Damage * HitPair.Value, // 피격수만큼 데미지
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}

				// 임시배열에 피격당한 플레이어를 추가해준다.
				HitCharacters.Add(HitPair.Key);
			}
		}
		if (!HasAuthority() && bUseServerSideRewind) // 클라 일때
		{
			// 되감기 설정 ON
			PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(OwnerPawn) : PlayerOwnerCharacter;

			PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(InstigatorController) : PlayerOwnerController;

			// 주의할점! 샷건의 탄퍼짐 갯수*플레이어수*피격검사수*데미지계산수 의 상당히 큰 반복문이 필요하기때문에 검사 조건에 PlayerOwnerCharacter->IsLocallyControlled() 를 넣어 클라이언트 한명의 행동에 호출시키는 것이 적절하다. 단순히 클라이언트 조건으로만 검사할경우 한명의 플레이어가 샷건을 쐈을때 모든 플레이어가 실행하기 때문이다.
			if (PlayerOwnerController &&
				PlayerOwnerCharacter &&
				PlayerOwnerCharacter->GetLagCompensation() &&
				PlayerOwnerCharacter->IsLocallyControlled())
			{
				PlayerOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					PlayerOwnerController->GetServerTime() - PlayerOwnerController->SingleTripTime
				);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	// 탄퍼짐 갯수만큼 반복문을 선언하고 매개변수로 들어온 배열로 랜덤으로 설정된 위치를 배열에 넣어준다.
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{

		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}

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

		// �ǰ�Ƚ����ŭ�� �������� �����ϱ����� ��
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

				if (ImpactParticles) // �ǰ� ��ƼŬ ���
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
				if (HitSound) // �ǰݻ��� ���
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

		// ������ �ǰ��⿡ ����� �ӽ� ����
		TArray<APlayerCharacter*> HitCharacters;

		for (auto HitPair : HitMap)
		{
			// �ǰ� �ݺ����� ������ ������ ���̺��� �˻��Ѵ�. �ش� �ݺ����� ���ǿ� �ǰݴ��� �÷��̾��� ����ŭ �ݺ��ȴ�.
			if (HitPair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();

				if (HasAuthority() && bCauseAuthDamage) // ���� �϶�
				{
					 // �ǰ��� ���� OFF
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						Damage * HitPair.Value, // �ǰݼ���ŭ ������
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}

				// �ӽù迭�� �ǰݴ��� �÷��̾ �߰����ش�.
				HitCharacters.Add(HitPair.Key);
			}
		}
		if (!HasAuthority() && bUseServerSideRewind) // Ŭ�� �϶�
		{
			// �ǰ��� ���� ON
			PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(OwnerPawn) : PlayerOwnerCharacter;

			PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(InstigatorController) : PlayerOwnerController;

			// ��������! ������ ź���� ����*�÷��̾��*�ǰݰ˻��*���������� �� ����� ū �ݺ����� �ʿ��ϱ⶧���� �˻� ���ǿ� PlayerOwnerCharacter->IsLocallyControlled() �� �־� Ŭ���̾�Ʈ �Ѹ��� �ൿ�� ȣ���Ű�� ���� �����ϴ�. �ܼ��� Ŭ���̾�Ʈ �������θ� �˻��Ұ�� �Ѹ��� �÷��̾ ������ ������ ��� �÷��̾ �����ϱ� �����̴�.
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

	// ź���� ������ŭ �ݺ����� �����ϰ� �Ű������� ���� �迭�� �������� ������ ��ġ�� �迭�� �־��ش�.
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{

		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}

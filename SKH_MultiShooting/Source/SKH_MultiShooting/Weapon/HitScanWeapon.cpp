#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"
#include "SKH_MultiShooting/PlayerComponents/LagCompensationComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	
	// ��� �ǰݰ���� ���������ϱ⶧���� ũ���̾�Ʈ���� InstigatorController �� �׻� null�̴�.
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
		if (PlayerCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();

			if (HasAuthority() && bCauseAuthDamage)
			{
				// ��弦 ���� ����
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				// ��Ʈ�ѷ��� ���������� ���� ������������ Ŭ���̾�Ʈ������ null�� ��ȯ�Ѵ� ���� �ش� ���ǹ��� �÷��̾� ĳ���� && ���� && ���� �� ���ǹ����� ������ ���������� �ϴ� �۾��� �ȴ�. ������ ����� ���������� �Ѵ�.
				UGameplayStatics::ApplyDamage(
					PlayerCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				// Ŭ���̾�Ʈ������ �����ǰ��⸦ ������ �� �ִ�.
				PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(OwnerPawn) : PlayerOwnerCharacter;

				PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(InstigatorController) : PlayerOwnerController;
				
				if (PlayerOwnerController && 
					PlayerOwnerCharacter &&
					PlayerOwnerCharacter->GetLagCompensation() && 
					PlayerOwnerCharacter->IsLocallyControlled())
				{
					PlayerOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						PlayerCharacter,
						Start,
						HitTarget,
						PlayerOwnerController->GetServerTime() - PlayerOwnerController->SingleTripTime);
				}
			}
	
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		// �ǰ������� ���� ���
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		if (MuzzleFlash)
		{
			// ��ݽ� �ѱ��� ��ƼŬ ����Ʈ
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			// ��� ���� ����Ʈ
			UGameplayStatics::SpawnSoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		// ��ƼŬ�� ������ ����Ʈ���̽��� ������ ���
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}

		// ���� �۾������� �������� ����ƼŬ ����
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				// ������Ʈ������ ������ ����Ʈ�� �Ķ������ Target �̶��� �Ķ������ ���� BeamEnd�� �����Ѵ�.

				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

		if (DebugEndSphere)
		{
			DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		}
	}
}


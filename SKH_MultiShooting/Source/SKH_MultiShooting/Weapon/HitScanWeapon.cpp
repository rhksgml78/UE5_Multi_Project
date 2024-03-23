#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}
	
	// ��� �ǰݰ���� ���������ϱ⶧���� ũ���̾�Ʈ���� InstigatorController �� �׻� null�̴�.
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + ((HitTarget - Start) * 1.25f);

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);

			// ��ƼŬ�� ������ ����Ʈ���̽��� ������ ���
			FVector BeamEnd = End;

			if (FireHit.bBlockingHit)
			{
				// ���� ����Ʈ ����Ʈ�� �ִٸ� �װ����� �ٽ� ����
				BeamEnd = FireHit.ImpactPoint;
				APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());

				if (PlayerCharacter && HasAuthority() && InstigatorController)
				{
					// ��Ʈ�ѷ��� ���������� ���� ������������ Ŭ���̾�Ʈ������ null�� ��ȯ�Ѵ� ���� �ش� ���ǹ��� �÷��̾� ĳ���� && ���� && ���� �� ���ǹ����� ������ ���������� �ϴ� �۾��� �ȴ�. ������ ����� ���������� �Ѵ�.
					UGameplayStatics::ApplyDamage(
						PlayerCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
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
			}
			// ���� �۾������� �������� ����ƼŬ ����
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					SocketTransform
				);
				if (Beam)
				{
					// ������Ʈ������ ������ ����Ʈ�� �Ķ������ Target �̶��� �Ķ������ ���� BeamEnd�� �����Ѵ�.

					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
		if (MuzzleFlash)
		{
			// ��ݽ� �ѱ��� ��ƼŬ ����Ʈ
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
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

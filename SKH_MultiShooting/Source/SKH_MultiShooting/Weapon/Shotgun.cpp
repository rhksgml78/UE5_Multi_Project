#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	// ������ ��ӵ� FIre�� ������ �ʴ´�� AWeaponŬ�������ִ� Fire �Լ��� ȣ���Ѵ�.
	//Super::Fire(HitTarget);

	AWeapon::Fire(HitTarget);

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

		// ��ź�� �������� �÷��̾ �ǰ��Ͽ��� ��츦 ���� ����
		TMap<APlayerCharacter*, uint32> HitMap;

		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			// ������ ���� ���� ���� Ƚ���� �����Ͽ� �������� �ջ��ϰ� �ѹ��� ���޽�Ų��.
			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(PlayerCharacter))
				{
					// HitMap ���̺� �÷��̾ �̹� ������� �ش��÷��̾��� ��(int32)�� 1������Ų��.
					HitMap[PlayerCharacter]++;
				}
				else
				{
					// HitMap ���̺� �÷��̾ ������� �ش��÷��̾ ���̺� ���ϰ� �÷��̾��� ��(int32)�� 1�� �����Ѵ�.
					HitMap.Emplace(PlayerCharacter, 1);
				}
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
					FMath::FRandRange(-.5f, .5f) // ��ź������ ������
				);
			}
		}
		for (auto HitPair : HitMap)
		{
			// �ǰ� �ݺ����� ������ ������ ���̺��� �˻��Ѵ�. �ش� �ݺ����� ���ǿ� �ǰݴ��� �÷��̾��� ����ŭ �ݺ��ȴ�.

			if (InstigatorController)
			{
				if (HitPair.Key && HasAuthority() && InstigatorController)
				{
					// ���̺��� Ű(�÷��̾�Ŭ����)���� ���̺��� ��(�ǰ���Ƚ��)��ŭ ������ �������� �����Ѵ�.(�����ǰ�� �ѹ� �ѹ��� �������� �����Ǳ⶧���� ���� �������� ������ ��
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

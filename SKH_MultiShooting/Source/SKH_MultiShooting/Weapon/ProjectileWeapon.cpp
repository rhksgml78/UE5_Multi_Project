#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);


	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	// �߻�ü �����ϱ� (�߻���ġ �����̸� MuzzleFlash)
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	UWorld* World = GetWorld();

	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// �ѱ��κ��� ũ�ν���� ��Ʈ ���������� ����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TagetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;

		// ���Ⱑ ������ �ǰ��� ��� ON
		if (bUseServerSideRewind)
		{
			// ���� (���Ⱑ ������ �ǰ��⸦ ����Ѵ�)
			if (InstigatorPawn->HasAuthority())
			{
				if (InstigatorPawn->IsLocallyControlled())
				{
					// ����, �÷��̾�� ������ ����ü�� �����Ѵ�. �����̱⶧���� ��¥ ����ü�� �����ؾ��ϱ� ����
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					// ���Ⱑ �������ִ� ���������� ����ü�� ����
					SpawnedProjectile->Damage = Damage;
				}
				else
				{
					// ����, �ٸ� ������ �������� ���� ����ü�� �����Ѵ�. ������ �ǰ��� ��� X
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
			// Ŭ�� (���Ⱑ ������ �ǰ��⸦ ����Ѵ�)
			else
			{
				if (InstigatorPawn->IsLocallyControlled())
				{
					// Ŭ��, �÷��̾�� ������������ ����ü�� �����ϰ� ������ �ǰ��� ��� O
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					// ���Ⱑ �������ִ� ���������� ����ü�� ����
					SpawnedProjectile->Damage = Damage;
				}
				else
				{
					// Ŭ��, �ٸ� ������ �������� ���� ����ü�� �����Ѵ�. ������ �ǰ��� ��� X
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		// ���Ⱑ ������ �ǰ��� ��� OFF
		else
		{
			if (InstigatorPawn->HasAuthority())
			{
				// ����, �÷��̾�� ������ ����ü�� �����Ѵ�. �����̱⶧���� ��¥ ����ü�� �����ؾ��ϱ� ����
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				// ���Ⱑ �������ִ� ���������� ����ü�� ����
				SpawnedProjectile->Damage = Damage;
			}
		}
	}
}

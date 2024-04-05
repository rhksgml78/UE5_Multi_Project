#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);


	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	// 발사체 생성하기 (발사위치 소켓이름 MuzzleFlash)
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	UWorld* World = GetWorld();

	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 총구로부터 크로스헤어 히트 지점까지의 방향
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TagetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;

		// 무기가 서버측 되감기 기능 ON
		if (bUseServerSideRewind)
		{
			// 서버 (무기가 서버측 되감기를 사용한다)
			if (InstigatorPawn->HasAuthority())
			{
				if (InstigatorPawn->IsLocallyControlled())
				{
					// 서버, 플레이어는 복제된 투사체를 스폰한다. 서버이기때문에 진짜 투사체를 생성해야하기 때문
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					// 무기가 가지고있는 데미지값을 투사체에 적용
					SpawnedProjectile->Damage = Damage;
				}
				else
				{
					// 서버, 다른 유저는 복제되지 않은 투사체를 스폰한다. 서버측 되감기 사용 X
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
			// 클라 (무기가 서버측 되감기를 사용한다)
			else
			{
				if (InstigatorPawn->IsLocallyControlled())
				{
					// 클라, 플레이어는 복제되지않은 투사체를 스폰하고 서버측 되감기 사용 O
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					// 무기가 가지고있는 데미지값을 투사체에 적용
					SpawnedProjectile->Damage = Damage;
				}
				else
				{
					// 클라, 다른 유저는 복제되지 않은 투사체를 스폰한다. 서버측 되감기 사용 X
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		// 무기가 서버측 되감기 기능 OFF
		else
		{
			if (InstigatorPawn->HasAuthority())
			{
				// 서버, 플레이어는 복제된 투사체를 스폰한다. 서버이기때문에 진짜 투사체를 생성해야하기 때문
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TagetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				// 무기가 가지고있는 데미지값을 투사체에 적용
				SpawnedProjectile->Damage = Damage;
			}
		}
	}
}

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
	
	// 모든 피격계산은 서버에서하기때문에 크라이언트에서 InstigatorController 는 항상 null이다.
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

			// 파티클의 끝점을 라인트레이스의 끝으로 잡고
			FVector BeamEnd = End;

			if (FireHit.bBlockingHit)
			{
				// 만일 임팩트 포인트가 있다면 그곳으로 다시 저장
				BeamEnd = FireHit.ImpactPoint;
				APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());

				if (PlayerCharacter && HasAuthority() && InstigatorController)
				{
					// 컨트롤러는 서버에서만 값을 가지고있으며 클라이언트에서는 null을 반환한다 따라서 해당 조건문은 플레이어 캐릭터 && 서버 && 서버 의 조건문으로 무조건 서버에서만 하는 작업이 된다. 데미지 계산은 서버에서만 한다.
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
				// 피격했을때 사운드 재생
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint
					);
				}
			}
			// 위의 작업종료후 일직선의 빔파티클 생성
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					SocketTransform
				);
				if (Beam)
				{
					// 컴포넌트를통해 생성한 이펙트의 파라미터중 Target 이란느 파라미터의 값을 BeamEnd로 지정한다.

					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
		if (MuzzleFlash)
		{
			// 사격시 총구의 파티클 이펙트
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			// 사격 사운드 이펙트
			UGameplayStatics::SpawnSoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"

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

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

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
				GetWorld(),
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
		if (MuzzleFlash)
		{
			// 사격시 총구의 파티클 이펙트
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
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

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		// 탄퍼짐을 설정하였을경우 TraceEndWithScatter 계산 함수를 실행하고 아닐경우 타겟으로 일직선지점을 끝으로 계산
		FVector End = bUseScatter? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + ((HitTarget - TraceStart) * 1.25f);

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		// 파티클의 끝점을 라인트레이스의 끝으로 잡고
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			// 만일 임팩트 포인트가 있다면 그곳으로 다시 저장
			BeamEnd = OutHit.ImpactPoint;
		}
		// 위의 작업종료후 일직선의 빔파티클 생성
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
				// 컴포넌트를통해 생성한 이펙트의 파라미터중 Target 이란느 파라미터의 값을 BeamEnd로 지정한다.

				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	// 일정범위내에서 탄퍼짐 구현
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	// 생성된 원형 구역에 랜덤한 지점들을 선택한다
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	/*
	탄퍼짐 확인용 디버그 구체와 라인
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12.f, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12.f, FColor::Green, true);
	DrawDebugLine(GetWorld(), 
		TraceStart, 
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true
		);
	*/

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}


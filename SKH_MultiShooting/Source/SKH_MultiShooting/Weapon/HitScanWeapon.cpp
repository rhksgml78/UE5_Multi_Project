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
		// ź������ �����Ͽ������ TraceEndWithScatter ��� �Լ��� �����ϰ� �ƴҰ�� Ÿ������ ������������ ������ ���
		FVector End = bUseScatter? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + ((HitTarget - TraceStart) * 1.25f);

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
			// ���� ����Ʈ ����Ʈ�� �ִٸ� �װ����� �ٽ� ����
			BeamEnd = OutHit.ImpactPoint;
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
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	// �������������� ź���� ����
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	// ������ ���� ������ ������ �������� �����Ѵ�
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	/*
	ź���� Ȯ�ο� ����� ��ü�� ����
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


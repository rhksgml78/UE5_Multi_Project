#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "SKH_MultiShooting/PlayerComponents/LagCompensationComponent.h"

AProjectileBullet::AProjectileBullet()
{
	//ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//ProjectileMovementComponent->bRotationFollowsVelocity = true;
	//ProjectileMovementComponent->SetIsReplicated(true);

	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/* ����ü ��� �׸���
	FPredictProjectilePathParams PathParms;
	PathParms.bTraceWithChannel = true;
	PathParms.bTraceWithCollision = true;
	PathParms.DrawDebugTime = 5.f;
	PathParms.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParms.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParms.MaxSimTime = 4.f;
	PathParms.ProjectileRadius = 5.f;
	PathParms.SimFrequency = 30.f;
	PathParms.StartLocation = GetActorLocation();
	PathParms.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParms.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParms, PathResult);
	*/
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APlayerCharacter* OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		// �������ǰ��� SSR �� ����ϱ����Ͽ� ���� ����
		AFirstPlayerController* OwnerController = Cast<AFirstPlayerController>(OwnerCharacter->Controller);

		if (OwnerController)
		{
			if (OwnerCharacter->HasAuthority() && 
				!bUseServerSideRewind)
			{
				// ����, SSR��� ����
				
				// ��弦 ����
				const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				
				// ��ӹ��� ��Ʈ �̺�Ʈ ���� Destroy �� ����ǹǷ� ���߿� �����Ų��.
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			
			APlayerCharacter* HitCharacter = Cast<APlayerCharacter>(OtherActor);
			if (bUseServerSideRewind && 
				OwnerCharacter->GetLagCompensation() && 
				OwnerCharacter->IsLocallyControlled() && 
				HitCharacter)
			{
				// SSR ���, �÷��̾� ������ ������Ʈ ����, ������Ʈ�ѷ�

				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter, 
					TraceStart, 
					InitialVelocity, 
					OwnerController->GetServerTime() - OwnerController->SingleTripTime);
			}
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/SKH_MultiShooting.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 클라이언트에 복제 할 것 
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 우선 충돌 판정을 Ignore로 설정하고 아래에서 상세설정할 것
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//  Visiblility 물체는 모두 Block
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	// 월드상 static 물체 모두 Block
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	// 플레이어캐릭터(Pawn)타입 Block
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	// 사용자 지정 타입 Block
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);

}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 발사체가 스폰될경우 바로 실행하도록
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		// 충돌박스의 피격 이벤트에 바인딩
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);

		// 충돌박스가 움직이기시작할때 오너와의 충돌처리를 예외처리 할 수 있다. 때문에 로켓런처의 커스텀무브먼트 사용안해도 괜찮음.
		CollisionBox->IgnoreActorWhenMoving(Owner, true);
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 이벤트를 마치고 해당 액터는 파괴된다. 이때 단순히 Destroy 함수를 호출하면 AActor의 Destroyed 가 호출되는데 이것을 재정의한다.
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ThisClass::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Destroyed()
{
	// Destroyed 함수는 클라이언트에서도 호출되도록 구현 되어져 있다. 때문에 클라이언트에서 도 아래의 기능들이 호출 될 것이다.
	Super::Destroyed();
	SpawnParticleEffects();

}

void AProjectile::SpanwTrailSystem()
{
	if (TrailSystem)
	{
		// 컴포넌트에 연결
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();

	if (FiringPawn && HasAuthority())
	{
		// 피해계산은 서버에서만 진행한다.

		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // 월드상에 위치한 오브젝트
				Damage, // 기본 데미지
				10.f, // 최소 데미지
				GetActorLocation(), // 생성위치
				DamageInnerRadius, // 최소 범위
				DamageOuterRadius, // 최대 범위
				1.f, // 데미지 Falloff
				UDamageType::StaticClass(), // 데미지타입클래스
				TArray<AActor*>(), // 예외처리(Empty/없음)
				this, // 데미지 커서(원인)
				FiringController // 소유주의 컨트롤러
			);
		}
	}
}

void AProjectile::SpawnParticleEffects()
{
	// 파티클과 사운드를 출력시킨다.
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	// 피격 사운드는 나중에 재질에 따라 다른 사운드  출력필요
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}
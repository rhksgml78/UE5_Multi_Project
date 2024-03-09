#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/SKH_MultiShooting.h"

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
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 프로젝타일과 충돌한 액터가 플레이어일때
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		bHitPlayer = true;
		PlayerCharacter->MulticastHit();
	}

	
	// 이벤트를 마치고 해당 액터는 파괴된다. 이때 단순히 Destroy 함수를 호출하면 AActor의 Destroyed 가 호출되는데 이것을 재정의한다.
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	// Destroyed 함수는 클라이언트에서도 호출되도록 구현 되어져 있다. 때문에 클라이언트에서 도 아래의 기능들이 호출 될 것이다.
	Super::Destroyed();

	// 파티클과 사운드를 출력시킨다.
	if (bHitPlayer)
	{
		// 플레이어 피격시
		if (BloodParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodParticles, GetActorTransform());
		}
	}
	else
	{
		// 그외 피격시
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
		}
	}

	// 피격 사운드는 나중에 재질에 따라 다른 사운드  출력필요
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}


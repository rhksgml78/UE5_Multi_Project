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

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Ŭ���̾�Ʈ�� ���� �� �� 
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// �켱 �浹 ������ Ignore�� �����ϰ� �Ʒ����� �󼼼����� ��
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//  Visiblility ��ü�� ��� Block
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	// ����� static ��ü ��� Block
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	// �÷��̾�ĳ����(Pawn)Ÿ�� Block
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	// ����� ���� Ÿ�� Block
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
	
	// �߻�ü�� �����ɰ�� �ٷ� �����ϵ���
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
		// �浹�ڽ��� �ǰ� �̺�Ʈ�� ���ε�
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);

		// �浹�ڽ��� �����̱�����Ҷ� ���ʿ��� �浹ó���� ����ó�� �� �� �ִ�. ������ ���Ϸ�ó�� Ŀ���ҹ����Ʈ �����ص� ������.
		CollisionBox->IgnoreActorWhenMoving(Owner, true);
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// �̺�Ʈ�� ��ġ�� �ش� ���ʹ� �ı��ȴ�. �̶� �ܼ��� Destroy �Լ��� ȣ���ϸ� AActor�� Destroyed �� ȣ��Ǵµ� �̰��� �������Ѵ�.
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	// Destroyed �Լ��� Ŭ���̾�Ʈ������ ȣ��ǵ��� ���� �Ǿ��� �ִ�. ������ Ŭ���̾�Ʈ���� �� �Ʒ��� ��ɵ��� ȣ�� �� ���̴�.
	Super::Destroyed();
	SpawnParticleEffects();

}

void AProjectile::SpawnParticleEffects()
{
	// ��ƼŬ�� ���带 ��½�Ų��.
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	// �ǰ� ����� ���߿� ������ ���� �ٸ� ����  ����ʿ�
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}
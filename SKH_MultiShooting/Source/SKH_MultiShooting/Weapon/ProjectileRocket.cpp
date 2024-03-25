#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
//#include "RocketMovementComponent.h"


AProjectileRocket::AProjectileRocket()
{
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	//RocketMovementComponent->bRotationFollowsVelocity = true;
	//RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::Destroyed()
{
	/*
	Destroy ��ȣ��ɶ� �ش��Լ��� ȣ��Ǵµ� �̶� �θ� Ŭ������ �����Լ��� ������+Super �������� �������� �θ�Ŭ���� �Լ��� �����ڵ带 ������ �� �ִ�. ������ ������ ��ƼŬ�� ���带 ��� �����Ѵ�.
	*/
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		// �浹�ڽ��� �ǰ� �̺�Ʈ�� ���ε� �θ�Ŭ���������� ���������� ���ε������� ������ ����� �������� ���Ͽ� ����Ʈ�� ���带 ���ؼ� Ŭ���̾�Ʈ�϶��� ���ε��� ���ش�.
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

	SpanwTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		// ����� ������Ʈ�� ����ť�� �����ϰ� �ش���忡 ���� ����ȿ������
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}

}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	// ���������� ����
	ExplodeDamage();

	// �ı� Ÿ�̸� ����
	StartDestroyTimer();
	
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

	// �浹���� ������ �޽��� �����.
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetVisibility(false);
	}

	// �浹�ڽ� OFF
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// �ǰݽ� ���̾ư��� ��ƼŬ OFF
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}

	// �ǰݽ� ����ť ������Ʈ�� ���Ͽ� ���� OFF
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

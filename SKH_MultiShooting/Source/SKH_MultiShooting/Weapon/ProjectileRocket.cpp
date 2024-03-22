#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"


AProjectileRocket::AProjectileRocket()
{
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
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

	if (TrailSystem)
	{
		// ������Ʈ�� ����
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
	APawn* FiringPawn = GetInstigator();

	if (FiringPawn && HasAuthority())
	{
		// ���ذ���� ���������� �����Ѵ�.

		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // ����� ��ġ�� ������Ʈ
				Damage, // �⺻ ������
				10.f, // �ּ� ������
				GetActorLocation(), // ������ġ
				200.f, // �ּ� ����
				500.f, // �ִ� ����
				1.f, // ������ Falloff
				UDamageType::StaticClass(), // ������Ÿ��Ŭ����
				TArray<AActor*>(), // ����ó��(Empty/����)
				this, // ������ Ŀ��(����)
				FiringController // �������� ��Ʈ�ѷ�
			);
		}
	}

	// �θ� Ŭ����(Projectile)���� Destroy�� ����� �ǰ���ƼŬ�� ���������� Ʈ��������Ʈ�� ���� ���ܵα����ؼ� Ÿ�̸ָ� ����Ͽ� ������ �ı��Ѵ�.
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ThisClass::DestroyTimerFinished,
		DestroyTime
	);
	
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

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

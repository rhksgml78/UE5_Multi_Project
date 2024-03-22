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
	Destroy 가호출될때 해당함수도 호출되는데 이때 부모 클래스의 가상함수를 재정의+Super 콜을하지 않음으로 부모클래스 함수의 실행코드를 무시할 수 있다. 때문에 별도로 파티클과 사운드를 재생 설정한다.
	*/
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		// 충돌박스의 피격 이벤트에 바인딩 부모클래스에서는 서버에서만 바인딩했으나 로켓의 변경된 로직으로 인하여 이펙트와 사운드를 위해서 클라이언트일때또 바인딩을 해준다.
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

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

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		// 오디오 컴포넌트에 사운드큐를 지정하고 해당사운드에 사운드 감쇠효과적용
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
		// 피해계산은 서버에서만 진행한다.

		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // 월드상에 위치한 오브젝트
				Damage, // 기본 데미지
				10.f, // 최소 데미지
				GetActorLocation(), // 생성위치
				200.f, // 최소 범위
				500.f, // 최대 범위
				1.f, // 데미지 Falloff
				UDamageType::StaticClass(), // 데미지타입클래스
				TArray<AActor*>(), // 예외처리(Empty/없음)
				this, // 데미지 커서(원인)
				FiringController // 소유주의 컨트롤러
			);
		}
	}

	// 부모 클래스(Projectile)에서 Destroy때 사운드와 피격파티클을 생성하지만 트레일이펙트를 좀더 남겨두기위해서 타이멀르 사용하여 별도로 파괴한다.
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ThisClass::DestroyTimerFinished,
		DestroyTime
	);
	
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

	// 충돌직후 액터의 메쉬를 숨긴다.
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetVisibility(false);
	}

	// 충돌박스 OFF
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 피격시 나이아가라 파티클 OFF
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}

	// 피격시 사운드큐 컴포넌트를 통하여 사운드 OFF
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

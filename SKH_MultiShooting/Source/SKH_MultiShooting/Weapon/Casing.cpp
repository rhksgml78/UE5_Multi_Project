#include "Casing.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);

	// 탄피가 배출되는 힘
	EjectionImpulse = 5.f;

	// 탄피가 소멸디는 시간
	DestroyTimer = 3.f;

	// 사운드는 한번만 재생 하기 위한 변수
	SoundPlayOnce = false;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	// 스태틱메시의 Hit와 바인딩
	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * EjectionImpulse);
	


	// 생성되자마자 5초의 타이머를 실행
	if (this)
	{
		// 타이머 설정
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;

		// 델리게이트에 Destroy 함수 바인딩
		TimerDel.BindLambda([this]()
			{
				this->Destroy();
			});

		// 5초 후에 Destroy 함수 실행
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, DestroyTimer, false);
		
		// 랜덤 회전 값 생성
		FRotator RandomRotation;
		RandomRotation.Yaw = FMath::RandRange(0.f, 360.f);
		RandomRotation.Pitch = FMath::RandRange(0.f, 360.f);
		RandomRotation.Roll = FMath::RandRange(0.f, 360.f);
		// 액터의 회전 값 설정
		SetActorRotation(RandomRotation);
	}

}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 충돌시 파괴 후 충돌 사운드 재생

	if (DropSound && !SoundPlayOnce)
	{
		SoundPlayOnce = true;
		UGameplayStatics::PlaySoundAtLocation(this, DropSound, GetActorLocation());
	}

	//Destroy();
	
}


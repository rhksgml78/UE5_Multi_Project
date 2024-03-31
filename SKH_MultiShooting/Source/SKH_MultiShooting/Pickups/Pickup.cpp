#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "SKH_MultiShooting/Weapon/WeaponTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	// 아이템은 서버 클라이언트 모두에게서 상호작용하기 때문에 복제되어야한다
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(100.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRelativeScale3D(FVector(3.f, 3.f, 3.f));

	// 커스텀 뎁스 스텐실 값 설정
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	PickupMesh->MarkRenderStateDirty();
	PickupMesh->SetRenderCustomDepth(true);

	EffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectComponent"));
	EffectComponent->SetupAttachment(PickupMesh);
	EffectComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	// 구체 충돌박스의 오버랩이벤트를 타이머로 바인딩. 단, 서버에서만 계산진행, 즉시 바인딩하면 아이템을 먹고 플레이어가 그위치에 그대로있을경우 플레이어때문에 생성되자마자 겹쳐지고 추가생성이 안되기때문에 아이템 생성후 일시적인 시간차를 두고나서 바인딩한다.
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&ThisClass::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}


void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자식클래스에서 재정의하여 사용중
}

void APickup::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}

void APickup::Destroyed()
{
	Super::Destroyed();
	// 플레이어가 아이템을 획득(파괴)하였을때 효과음 재생및 해당 액터 파괴 실행
	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickUpSound,
			GetActorLocation()
		);
	}
	// 아이템을 플레이어가 획득하여 소멸되었을때 이펙트 출현
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}


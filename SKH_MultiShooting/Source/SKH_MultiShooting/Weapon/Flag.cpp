#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"

AFlag::AFlag()
{
	// 깃발클래스는 스태틱매시만을 사용할 것이기때문에 해당메시를 루트로지정한다.
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 충돌에 사용할 구체와 픽업 위젯은 스켈레탈메시에 붙어있기 때문에 다시 스태틱메시(루트)에 연결
	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();

	// 깃발의 초기 위치 저장
	InitialTransform = GetActorTransform();
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	// 어태치 되어있는 무기를 월드상에 떨어뜨려두기
	FDetachmentTransformRules DetechRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetechRules);

	// 소유자 비우기
	SetOwner(nullptr);

	// 플레이어와 컨트롤러도 비우기
	PlayerOwnerCharacter = nullptr;
	PlayerOwnerController = nullptr;
}

void AFlag::ReSetFlag()
{
	// 가지고있던 플레이어 초기화
	APlayerCharacter* FlagBearer = Cast<APlayerCharacter>(GetOwner());
	if (FlagBearer)
	{
		// 플레이어의 깃발을 잡고있는 상태를 초기화한다.(서버와 클라 모두 실행)
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
	}

	// 이후의 작업은 클라이언트에서 할 필요가 없음. 서버에서만 세팅해주도록함.
	if (!HasAuthority()) return;

	// 플레이어에게서 떨어뜨리기
	FDetachmentTransformRules DetechRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetechRules);
	SetWeaponState(EWeaponState::EWS_Initial);

	// 초기 위치,회전값으로 되돌리기
	SetActorTransform(InitialTransform);

	// 위치를 이동시킨뒤에 충돌타입을 바꾸어줘야 2회 카운팅오류가 없어진다.
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// 소유자 비우기
	SetOwner(nullptr);

	// 플레이어와 컨트롤러도 비우기
	PlayerOwnerCharacter = nullptr;
	PlayerOwnerController = nullptr;
}

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	// 윤곽선 설정 OFF
	//EnableCustomDepth(false);
}

void AFlag::OnDroped()
{
	if (HasAuthority())
	{
		// 서버에서 작업할 것
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// 윤곽선 설정 ON
	//FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	//FlagMesh->MarkRenderStateDirty();
	//EnableCustomDepth(true);
}

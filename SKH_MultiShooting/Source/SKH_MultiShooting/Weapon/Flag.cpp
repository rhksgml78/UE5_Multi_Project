#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

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

void AFlag::OnEquipped()
{
	ShowPickupWidget(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 윤곽선 설정 OFF
	EnableCustomDepth(false);
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
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	FlagMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SKH_MultiShooting/PlayerComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 커스텀 뎁스 스텐실 값 설정
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	// 오버랩 바인딩
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//#include "Net/UnrealNetwork.h" 포함 필요
	DOREPLIFETIME(AWeapon, WeaponState);

	// SSR 사용 여부를 판단하는 변수는 플레이어인 각 오너에게만 전달.
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 오버랩된 Actor가 플레이어 형태라면
	APlayerCharacter* PlayerCharcter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharcter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && PlayerCharcter->GetTeam() != Team)
		{
			// 무기의 팀타입과 플레이어의 팀타입이 불일치시 함수 종료
			return;
		}
		if (PlayerCharcter->IsHoldingTheFlag())
		{
			// 플레이어가 깃발을 소지하고있을때 무기에 오버랩하지 않도록
			return;
		}
		PlayerCharcter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 오버랩이 끝난 Actor가 플레이어 형태라면
	APlayerCharacter* PlayerCharcter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharcter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && PlayerCharcter->GetTeam() != Team)
		{
			// 무기의 팀타입과 플레이어의 팀타입이 불일치시 함수 종료
			return;
		}
		if (PlayerCharcter->IsHoldingTheFlag())
		{
			// 플레이어가 깃발을 소지하고있을때 무기에 오버랩하지 않도록
			return;
		}
		//플레이어의 오버랩웨폰 객체를 nullptr 로 변경
		PlayerCharcter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	// 핑이 너무높으면 SSR을 사용하는 의미가 없기떄문에 매개변수로 들어온 값의 반대로 적용시켜준다.
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDroped();
		break;
	default:
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachinGun)
	{
		// 서브머신건의 피직스 사용을 위해서
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	// 윤곽선 설정 OFF
	EnableCustomDepth(false);

	// 플레이어의 컨트롤러에 접근하여 델리게이트 바인딩
	PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(GetOwner()) : PlayerOwnerCharacter;
	if (PlayerOwnerCharacter && bUseServerSideRewind)
	{
		PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(PlayerOwnerCharacter->Controller) : PlayerOwnerController;

		// 현재 델리게이트에 아무런 함수도 바인딩 되어잇지 않기때문에 not isbound 조건일때 함수를 바인딩 하도록 한다.
		if (PlayerOwnerController && 
			HasAuthority() && 
			!PlayerOwnerController->HighPingDelegate.IsBound())
		{
			PlayerOwnerController->HighPingDelegate.AddDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachinGun)
	{
		// 서브머신건의 피직스 사용을 위해서
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	// 윤곽선 설정 OFF
	EnableCustomDepth(false);

	// 플레이어의 컨트롤러에 접근하여 델리게이트 바인딩 해제
	PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(GetOwner()) : PlayerOwnerCharacter;
	if (PlayerOwnerCharacter && bUseServerSideRewind)
	{
		PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(PlayerOwnerCharacter->Controller) : PlayerOwnerController;

		if (PlayerOwnerController &&
			HasAuthority() &&
			PlayerOwnerController->HighPingDelegate.IsBound())
		{
			PlayerOwnerController->HighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDroped()
{
	if (HasAuthority())
	{
		// 서버에서 작업할 것
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// 윤곽선 설정 ON
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_WHITE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	// 플레이어의 컨트롤러에 접근하여 델리게이트 바인딩 해제
	PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(GetOwner()) : PlayerOwnerCharacter;
	if (PlayerOwnerCharacter && bUseServerSideRewind)
	{
		PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(PlayerOwnerCharacter->Controller) : PlayerOwnerController;

		if (PlayerOwnerController &&
			HasAuthority() &&
			PlayerOwnerController->HighPingDelegate.IsBound())
		{
			PlayerOwnerController->HighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void AWeapon::SpendRound()
{
	// 한번의 발사시 한개의  탄약을 소모시키고 플레이어의 HUD 업데이트 필요 (계산은 서버에서하고 서버도 업데이트) 이때 탄창은 최소 소지갯수 0개 ~ 최대소지갯수 를 벗어나지 않는다.
	Ammo = FMath::Clamp(Ammo -1, 0, MagCapacity);
	
	// 각플레이어는 본인의 HUDAmmo를 세팅하고
	SetHUDAmmo();

	if (HasAuthority())
	{
		// 서버에서는 각클라이언트에 함수를 호출시킨다.
		ClientUpdateAmmo(Ammo);
	}
	else if (PlayerOwnerCharacter && 
		PlayerOwnerCharacter->IsLocallyControlled())
	{
		// 서버에서는 해당작업 시퀀스의 횟수를 증가시킨다.
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	// 서버의 경우에는 리턴 시키고
	if (HasAuthority()) return;

	//각 클라이언트에서는 만일 처리되지않은 값이있다면 해당 값의 편차를 계산하여 직접 적용시킨다.
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	ClientAddAmmo(AmmoToAdd);
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;

	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);

	PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(GetOwner()) : PlayerOwnerCharacter;

	if (PlayerOwnerCharacter && 
		PlayerOwnerCharacter->GetCombat() && 
		IsFull())
	{
		PlayerOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	// 삼항연산으로 한번만 캐스팅하도록
	PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(GetOwner()) : PlayerOwnerCharacter;

	if (PlayerOwnerCharacter)
	{
		// 삼항연산으로 한번만 캐스팅하도록
		PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(PlayerOwnerCharacter->Controller) : PlayerOwnerController;

		if (PlayerOwnerController)
		{
			PlayerOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::OnRep_Owner()
{
	// 오너가 변경되었을때 실행되는 복제함수
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		// 플레이어와 컨트롤러도 비우기
		PlayerOwnerCharacter = nullptr;
		PlayerOwnerController = nullptr;
	}
	else
	{
		PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(Owner) : PlayerOwnerCharacter;

		if (PlayerOwnerCharacter && 
			PlayerOwnerCharacter->GetEquippedWeapon() &&
			PlayerOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		// AmmoEject 소켓위치로부터 탄피를 생성한다.
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}

	// 총알 소모
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	// 어태치 되어있는 무기를 월드상에 떨어뜨려두기
	FDetachmentTransformRules DetechRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetechRules);

	// 소유자 비우기
	SetOwner(nullptr);

	// 플레이어와 컨트롤러도 비우기
	PlayerOwnerCharacter = nullptr;
	PlayerOwnerController = nullptr;
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// 일정범위내에서 탄퍼짐 구현
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	// 생성된 원형 구역에 랜덤한 지점들을 선택한다
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}
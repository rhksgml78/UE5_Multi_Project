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

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		// ������ ���ε�
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,	UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// �������� Actor�� �÷��̾� ���¶��
	APlayerCharacter* PlayerCharcter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharcter)
	{
		PlayerCharcter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// �������� ���� Actor�� �÷��̾� ���¶��
	APlayerCharacter* PlayerCharcter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharcter)
	{
		//�÷��̾��� ���������� ��ü�� nullptr �� ����
		PlayerCharcter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_WeaponState()
{
	// ������ ���°� �ٲ�� �ش��Լ��� �ѹ� ����ȴ�.
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	default:
		break;
	}
}

void AWeapon::SetHUDAmmo()
{
	// ���׿������� �ѹ��� ĳ�����ϵ���
	PlayerOwnerCharacter = PlayerOwnerCharacter == nullptr ? Cast<APlayerCharacter>(GetOwner()) : PlayerOwnerCharacter;

	if (PlayerOwnerCharacter)
	{
		// ���׿������� �ѹ��� ĳ�����ϵ���
		PlayerOwnerController = PlayerOwnerController == nullptr ? Cast<AFirstPlayerController>(PlayerOwnerCharacter->Controller) : PlayerOwnerController;

		if (PlayerOwnerController)
		{
			PlayerOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{
	// �ѹ��� �߻�� �Ѱ���  ź���� �Ҹ��Ű�� �÷��̾��� HUD ������Ʈ �ʿ� (����� ���������ϰ� ������ ������Ʈ) �̶� źâ�� �ּ� �������� 0�� ~ �ִ�������� �� ����� �ʴ´�.
	Ammo = FMath::Clamp(Ammo -1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	// ź���� �Ҹ� �ɰ�� ������ ��� (Ŭ���̾�Ʈ�� �������� ����Ǿ��� ������ �Ի���� �ٷ� ������Ʈ)
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	// ���ʰ� ����Ǿ����� ����Ǵ� �����Լ�
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		// �÷��̾�� ��Ʈ�ѷ��� ����
		PlayerOwnerCharacter = nullptr;
		PlayerOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			// �������� �۾��� ��
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	default:
		break;
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//#include "Net/UnrealNetwork.h" ���� �ʿ�
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);

}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		// AmmoEject ������ġ�κ��� ź�Ǹ� �����Ѵ�.
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

	// �Ѿ� �Ҹ�
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	// ����ġ �Ǿ��ִ� ���⸦ ����� ����߷��α�
	FDetachmentTransformRules DetechRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetechRules);

	// ������ ����
	SetOwner(nullptr);

	// �÷��̾�� ��Ʈ�ѷ��� ����
	PlayerOwnerCharacter = nullptr;
	PlayerOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}


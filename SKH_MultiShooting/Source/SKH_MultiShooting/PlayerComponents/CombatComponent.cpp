#include "CombatComponent.h"
#include "SKH_MultiShooting/Weapon/Weapon.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "SKH_MultiShooting/HUD/PlayerHUD.h"
#include "SKH_MultiShooting/HUD/PlayerOverlay.h"
#include "SKH_MultiShooting/Character/PlayerAnimInstance.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 250.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ó���� �����Ӽӵ� ����
	SetMaxWalkSpeed(BaseWalkSpeed);

	if (Character)
	{
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())
		{
			// źâ���� �ʱ�ȭ (����������)
			InitializeCarriedAmmo();
		}
	}


}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		// �ǰ���ġ Ȯ��
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		// ũ�ν� ��� �׸���
		SetHUDCrosshairs(DeltaTime);

		// ���� ����
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		// �۵����ڸ��� �ѹ��� �߻�ǵ��� ��ȭ�� ������ �ٲپ� �ֵ����Ѵ�. ���� �߻簡���������� �ٽ� true�������ֱ�.
		bCanFire = false;
		ServerFire(HitTarget);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
	}

}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	// Ÿ�̸� ��ȭ - Ÿ�̸� �ڵ��� ���Ͽ� Ÿ�̸Ӹ� �����Ѵ�. �̰�ü, �ݹ��� �Լ�, ������Ÿ��
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;

	// Ÿ�̸ӷ� ���õǾ� FireDelay �ð��� �Ǿ����� �ش� �Լ��� �ݹ� �ȴ�.
	bCanFire = true; // ��ȭ�� On
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	// Ÿ�̸� ����� �Ѿ��� ���ٸ� ������ ȣ��
	ReloadEmptyWeapon();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Reloading &&
		(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_GrenadeLauncher))
	{
		// ���ǰ� ��ź �߻���� ����ó���� �Ʒ��� ���ǹ��� ���������ʵ��� ������ �ٷ� return
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::SetMaxWalkSpeed(float Value)
{
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Value;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	// ���� ����ź�� ��ô���϶�(ECS_Unoccupied ���ƴҶ�)�� ���� �ؾ��Ѵ�
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	// ���� ���� �������� ���Ⱑ�ִµ� �ٸ� ���⸦ �����Ϸ� �� ��� ���� ����ִ� ���渣 �����Ű�� �����⸦ �����Ѵ�
	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	// ���⸦ �����տ� ����
	AttachActorToRightHand(EquippedWeapon);

	// ������ ������ ���ʸ� ����
	EquippedWeapon->SetOwner(Character);

	// ���ʺ�������(�÷��̾ ���⸦ ����) HUDAmmo ����
	EquippedWeapon->SetHUDAmmo();

	// CarriedAmmo �����ϱ����� ������ Ÿ�� ����
	UpdateCarriedAmmo();

	// �������� ���� ���
	PlayEquipWeaponSound();

	// ���⸦ ���ڸ��� źâ�� ����ִٸ�
	ReloadEmptyWeapon();

	// ������ ������ �������� ���ϵ���
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachActorToLefttHand(AActor* ActorToAttach)
{
	if (Character == nullptr ||
		Character->GetMesh() == nullptr ||
		ActorToAttach == nullptr)
	{
		return;
	}

	// ���Ѱ� ����ӽŰ��� �����̱⶧���� �߰����� ������ �����Ͽ� �ش� �������� ���δ�.
	bool bUseLeftPistolSocket =
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachinGun;

	// ���� ������ ����Ͽ� ������ Ÿ���� �Ǵ��ϰ� Ÿ�Ժ��� ������ �������� ���δ�.
	FName SocketName = bUseLeftPistolSocket ? FName("LeftPistolSocket") : FName("LeftHandSocket");
		
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || 
		Character->GetMesh() == nullptr || 
		ActorToAttach == nullptr)
	{
		return;
	}
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound()
{
	if (Character && EquippedWeapon && EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty()) Reload();
}

void UCombatComponent::Reload()
{
	// ������ ź���� ���� 0���� ũ�� �÷��̾ ���ε� ���°� �ƴҰ��
	if (CarriedAmmo > 0 && 
		CombatState == ECombatState::ECS_Unoccupied && 
		EquippedWeapon &&
		!EquippedWeapon->IsFull())
	{
		// ���� ���� �Լ��� ȣ���Ѵ�.
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	// �������� ����Ǵ� ����
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::FinishReloading()
{
	// BP���� ȣ��Ǵ� ��Ƽ���� ���� �Լ�.
	if (Character == nullptr)
	{
		return;
	}
	if (Character->HasAuthority())
	{
		// �������� ����
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	// ��� �÷��̾��� ź�హ���� �������� ���
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		// ����ź���� �������� �������� ��ŭ�� ź���� ���ҽ�Ű�� �����Ѵ�.
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::ShotgunShellReload()
{
	// �������Ʈ ��Ƽ���̿��� �Լ��� ȣ��Ǹ� ������ ź�� ������Ʈ(��������)
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		// ���ǰ� ��ź�߻��� 1���� �����ȴ�.
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-1);
	// ���ǰ� ��ź�߻��� �ѹ߾� �����Ҷ����� �ٷ� �� �� �־�� �Ѵ�
	bCanFire = true;


	// ���۾������Ͽ� 1���� źâ�� ���ϰ� źâ�� ��á����� ShotgunEnd ������ ����ؾ��Ѵ�. (���۾��� ���������� ����ȴ�. ������ Ŭ���̾�Ʈ������ ���� ��Ű�����ؼ��� Weapon Ŭ������ ������ �������� OnRep �Լ������� ���� �۾��� ������ ��� �Ѵ�.
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	// ����ź ��ô �Ϸ�� �������·� �ǵ�����.
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	// �켱 �տ� ������ �ð��� ����ź�� �����.
	ShowAttachedGrenade(false);

	// �÷��̾�BP���� ������ ����ź�� �����Ͽ� �߻��Ѵ�.
	if (Character && Character->HasAuthority() && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = HitTarget - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			// ������Ʈ��(���÷��̾�)�� ����ź��ô�� ��Ÿ�ָ� �̹� ����Ͽ��⶧���� �����̾ƴҶ� �����Ű�����Ѵ�.
			Character->PlayThrowGrenadeMontage();
			AttachActorToLefttHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	default:
		break;
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReLoadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	// ź���� ������ ������ü����� �ֱ� ������ ���� Ŭ�������� ���� ���� �;� ��.
	if (EquippedWeapon == nullptr) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	// ������ Ÿ���� �����Ǿ� �־�� ��Ȯ�� ź���� ������ ���
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		// �����ϰ��ִ� ź���� ���� ����
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		// �ּ��� ������ ����������ִ� źâ���� ����� �Ҹ�Ⱦ� Ȥ�� ��������(���ε��Ҽ��ִ�)���� ź���� ���� ���� ���ڷ� ����
		int32 Least = FMath::Min(RoomInMag, AmountCarried);

		return FMath::Clamp(RoomInMag, 0, Least);

	}

	// ������ Ÿ���� �����Ǿ� ���� �ʴٸ� 0�� ��ȯ
	return 0;
}

void UCombatComponent::ThrowGrenade()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		// ����ź�� �ѹ������� ���� ���� �������� �ʵ��� ����
		return;
	}
	// ����ź ��ô�� ������ �÷��̾� ��ΰ� �˾ƾ� �Ѵ�. �÷��̾��� ���� ���´� �����Ǳ⶧���� ����� Ŭ���̾�Ʈ�� �ٷ� �� �� ������ ������ ���� �� �� ���� ������ �������� ȣ��� RPC�� �ʿ�
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLefttHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	// Ŭ���̾�Ʈ���� ���� �Լ��� ȣ���Ѵ�. ������ Ŭ���̾�Ʈ���ƴ� ������ ����ź�� ��ô�� ���� �߰������� ȣ���ؼ��� �ȵȴ�.
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	// ���� ȣ��
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLefttHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character &&
		Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		// ����ġ ���� ������ ���¸� Ȯ���� �������Ŀ� ����ġ ��ų �� �ֵ����Ѵ�.
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		// Ŭ���̾�Ʈ���� ���� ���
		PlayEquipWeaponSound();
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// ȭ���߾�(ũ�ν����)���� ���� �������� ���� �ϵ���
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ��ũ����ǥ�� ���μ��� ��(�߾�) ��ġ ����
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// ��ũ����ǥ���� ������ǥ�� �����ϱ����� �ʿ��� ����
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// ũ�ν������ ���� ��ǥ�� ������ ����
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	// ��ǥ�� ���忡 �����ߴٸ�
	if (bScreenToWorld)
	{
		// ����Ʈ���̽��� ���� ���������� ������ ����
		FVector Start = CrosshairWorldPosition;

		// ī�޶��� ��ġ���� ���� ���۵ǰ� ������ Self�� �޽��� �ٸ��޽��� ĳ���Ϳ� ī�޶���̿��� ��� ������ �ڷ� �ϴ� ������ �ִ�. ������ Ʈ���̽��� ���������� ī�޶��� ��ġ���� �տ��� ����

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		// �ǰ������� ��ȿ�� ���
		if (!TraceHitResult.IsValidBlockingHit())
		{
			// TRACE_LENGTH �Ÿ��� ImpactPoint�� ����
			TraceHitResult.ImpactPoint = Start + CrosshairWorldDirection * TRACE_LENGTH;
		}

		// �÷��̾�� �浹������ ũ�ν���� ���� ���� Ʈ���̽� ����� ���Ͱ� �ְ� ������ �������̽��� �����Ǿ� �ִٸ�
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	// ��Ʈ�ѷ��� nullptr �ϰ�� ĳ�����ؼ� �־��ְ� �ƴϸ� ��Ʈ�ѷ��� �״��
	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<APlayerHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				// ���⸦ ���������� �׹��⿡ ������ �ؽ�ó�� �־��ش�.
				HUDPackage.CrossHairCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrossHairLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrossHairRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrossHairTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrossHairBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				// ���⸦ �������� �ʾ����� ����α�
				HUDPackage.CrossHairCenter = nullptr;
				HUDPackage.CrossHairLeft = nullptr;
				HUDPackage.CrossHairRight = nullptr;
				HUDPackage.CrossHairTop = nullptr;
				HUDPackage.CrossHairBottom = nullptr;
			}

			// ũ�ν���� ���� ������Ʈ (ĳ������ �����ӿ� ���� �������ų� �����ҋ� ������)
			// ĳ������ �ּҼӵ�~�ִ�ӵ��� 0~1�� ������ ��ֶ�����
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// ���߿��ְų� ���������� �������Լ���
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.5f, DeltaTime, 2.5f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// ���� ������
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
		
			// ��� ���� �� �߰��Ȱ� 0���� ������Ű��
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 10.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor +
				CrosshairShootingFactor -
				CrosshairAimFactor;
		
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;


	// ���ؽ� ���⿡ ������ FOV���� �ӵ����� ���Ͽ� ���� �ܾƿ��� ����� �����Ѵ�.
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	
	// FOV ���� ������ �Ϸ���� Set
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) 		return;

	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	// ���� ������ �ӵ��� ����
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	// �÷��̾ ���û����̰� �������۶����� ���⸦ �����ϰ�������
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	// ���� ������ �ӵ��� ����
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

}

bool UCombatComponent::CanFire()
{
	// ������ ���Ⱑ ���ٸ� false ��ȯ
	if (EquippedWeapon == nullptr) return false;

	// ���ǰ� ��ź�߻�⿡ ���Ͽ� ����ó��
	if (!EquippedWeapon->IsEmpty() && bCanFire &&
		CombatState == ECombatState::ECS_Reloading &&
		(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_GrenadeLauncher))
	{
		return true;
	}

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;

}

void UCombatComponent::OnRep_CarriedAmmo()
{
	// Ŭ���̾�Ʈ ������Ʈ
	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	// ����or��ź�߻�� �϶� ���̻� �������� źâ�� ������� ��Ÿ�� ������ �ǳ� �پ�� �Ѵ�.
	bool bJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_GrenadeLauncher) &&
		CarriedAmmo == 0;

	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachinGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}


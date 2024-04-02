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
#include "SKH_MultiShooting/Weapon/Shotgun.h"

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
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 처음에 움직임속도 지정
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
			// 탄창관련 초기화 (서버에서만)
			InitializeCarriedAmmo();
		}
	}


}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		// 피격위치 확인
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		// 크로스 헤어 그리기
		SetHUDCrosshairs(DeltaTime);

		// 시점 변경
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
		// 작동되자마자 한번만 발사되도록 점화식 변수를 바꾸어 주도록한다. 이후 발사가끝난시점에 다시 true변경해주기.
		bCanFire = false;

		/*
		클라이언트(플레이어) 실행 흐름:
		플레이어 발사 요청: 플레이어가 발사 버튼을 누르면, 클라이언트에서 Fire() 함수가 호출됩니다.
		로컬 발사 처리: Fire() 함수 내에서 CanFire()를 통해 발사 가능 여부를 확인한 후, LocalFire(HitTarget)를 호출하여 로컬 플레이어에게 즉시 발사 효과를 표시합니다.
		서버에 발사 요청: 동시에 ServerFire(HitTarget)를 호출하여 서버에 발사 동작을 요청합니다.

		서버 실행 흐름:
		발사 요청 수신 및 전파: 서버에서 ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget) 함수가 호출되면, MulticastFire(TraceHitTarget)를 호출하여 모든 클라이언트에게 발사 동작을 전파합니다.
		*/


		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;

			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_MultiHitScan:
				FireMultiHitScanWeapon();
				break;
			default:
				break;
			}
		}
		StartFireTimer();
	}

}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 플레이어본인은 바로 발사를 실행
		if (!Character->HasAuthority()) LocalFire(HitTarget);

		// 서버에 실행요청하고 다른플레이어의 발사 동기화
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;

		// 플레이어본인은 바로 발사를 실행
		if (!Character->HasAuthority()) LocalFire(HitTarget);

		// 서버에 실행요청하고 다른플레이어의 발사 동기화
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireMultiHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
		if (EquippedWeapon)
		{
			// 임시 배열은 샷건클래스의 해당함수내에서 샷건의 탄퍼짐 갯수만큼 추가되기 때문에 여기서 따로 추가할것은없음
			TArray<FVector_NetQuantize> HitTargets;
			Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
			if (!Character->HasAuthority())
			{
				ShotgunLocalFire(HitTargets);
			}
			ServerShotgunFire(HitTargets);
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// 서버에서 로컬로 모든클라이언트에 복제 실행
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// 각 플레이어라면 리턴
	if (Character && 
		Character->IsLocallyControlled() &&
		!Character->HasAuthority()) return;

	// 본인이아닌 다른 플레이어의 발사를 복제실행
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	// 서버에서 로컬로 모든클라이언트에 복제 실행
	MulticastShotgunFire(TraceHitTarget);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	// 각 플레이어라면 리턴
	if (Character &&
		Character->IsLocallyControlled() &&
		!Character->HasAuthority()) return;

	// 본인이아닌 다른 플레이어의 발사를 복제실행
	ShotgunLocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	// 해당함수는 서버는 직접호출하고 클라이언트는 서버를 한번 거쳐서 실행되기때문에 약간의 타임렉이 있다.
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	if (CombatState == ECombatState::ECS_Reloading && 
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_GrenadeLauncher)
	{
		// 유탄 발사기의 예외처리는 아래의 조건문을 실행하지않도록 실행후 바로 return
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}

	if (CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	// 샷건은 따로 발사 함수를 마련하였기때문에 캐스팅먼저
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);

	if (Shotgun == nullptr || Character == nullptr) return;

	if (CombatState == ECombatState::ECS_Reloading ||
		CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
	}

}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	// 타이머 점화 - 타이머 핸들을 통하여 타이머를 세팅한다. 이객체, 콜백할 함수, 딜레이타임
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

	// 타이머로 세팅되어 FireDelay 시간이 되었을때 해당 함수는 콜백 된다.
	bCanFire = true; // 점화식 On
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	// 타이머 종료시 총알이 없다면 재장전 호출
	ReloadEmptyWeapon();
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

	// 만일 수류탄을 투척중일때(ECS_Unoccupied 가아닐때)는 리턴 해야한다
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		// 기본무기가 장착된 상태에서 보조무기가 비어있다면 보조무기를 장착
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		// 위의 조건문이아닐경우. 
		// 주무기 보조무기 모두 비어있거나 
		// 주무기만 비어있거나
		// 주무기 보조무기가 모두 장착되어있거나
		EquipPrimaryWeapon(WeaponToEquip);
	}

	// 아이템 장착시 정면으로 향하도록
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 어태치 전에 무기의 상태를 확실히 변경한후에 어태치 시킬 수 있도록한다.
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		// 클라이언트에서 사운드 재생
		PlayEquipWeaponSound(EquippedWeapon);

		// 무기교체시 상태가 바뀔때 클라이언트의 HUD 업데이트 되도록
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	// 클라이언트에서 적용
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttackActorToBackpack(SecondaryWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UCombatComponent::SwapWeapon()
{
	if (EquippedWeapon == nullptr || 
		SecondaryWeapon == nullptr ||
		CombatState != ECombatState::ECS_Unoccupied) return;

	// 스왑 방식을 사용하여 주무기와 보조무기의 값을 변경한다.
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	TempWeapon = nullptr;

	// 이후 주무기와 보조무기의 상세 설정을 진행한다.
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	ReloadEmptyWeapon();

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttackActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;

	// 만일 현재 장착중인 무기가있는데 다른 무기를 장착하려 할 경우 현재 들고있는 무길르 드랍시키고 새무기를 장착한다
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	// 무기를 오른손에 장착
	AttachActorToRightHand(EquippedWeapon);
	// 장착된 무기의 오너를 지정
	EquippedWeapon->SetOwner(Character);
	// 오너변경직후(플레이어가 무기를 장착) HUDAmmo 세팅
	EquippedWeapon->SetHUDAmmo();
	// CarriedAmmo 설정하기전에 무기의 타입 지정
	UpdateCarriedAmmo();
	// 서버에서 사운드 재생
	PlayEquipWeaponSound(WeaponToEquip);
	// 무기를 집자마자 탄창이 비어있다면
	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;

	// 보조무기 지정
	SecondaryWeapon = WeaponToEquip;
	// 보조무기의 상태를 장착으로 변경 및 상세 설정 변경
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	// 오너 지정
	SecondaryWeapon->SetOwner(Character);
	// 무기타입에따라 적절한 위치로 부착
	AttackActorToBackpack(WeaponToEquip);
	// 해당 무기의 장착 사운드 실행
	PlayEquipWeaponSound(WeaponToEquip);
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	// 실제 들고있는 탄약의 갯수를 늘려주고 HUD를 업데이트 해야함. 단 업데이트해야할것은 CarriedAmmo 가아닌 CarriedAmmoMap 이다.
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		// 최대소지량을 넘지 않도록
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);

		// 현재들고있는 무기 타입의 정보를 얻어 HUD 업데이트를 실행
		UpdateCarriedAmmo();
	}

	// 만일 현재들고있는 무기의 탄창이 0일경우 보충된다면 바로 자동 재장전을 실행
	if (EquippedWeapon && 
		EquippedWeapon->IsEmpty() && 
		EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
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

	// 권총과 서브머신건은 소형이기때문에 추가적인 소켓을 생성하여 해당 소켓으로 붙인다.
	bool bUseLeftPistolSocket =
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachinGun;

	// 삼항 연산을 사용하여 무기의 타입을 판단하고 타입별로 적절한 소켓으로 붙인다.
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
		ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttackActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr ||
		Character->GetMesh() == nullptr ||
		ActorToAttach == nullptr) return;

	// 권총과 서브머신건은 허리소켓에 부착해야한다.
	bool bUsePistolSocket =
		SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		SecondaryWeapon->GetWeaponType() == EWeaponType::EWT_SubmachinGun;
	FName SocketName = bUsePistolSocket ? FName("SpineSocket") : FName("NeckSocket");

	const USkeletalMeshSocket* SecondarySocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (SecondarySocket)
	{
		SecondarySocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

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

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
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
	// 소지한 탄약의 수가 0보다 크고 플레이어가 리로드 상태가 아닐경우
	if (CarriedAmmo > 0 && 
		CombatState == ECombatState::ECS_Unoccupied && 
		EquippedWeapon &&
		!EquippedWeapon->IsFull())
	{
		// 서버 실행 함수를 호출한다.
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	// 서버에서 실행되는 로직
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::FinishReloading()
{
	// BP에서 호출되는 노티파이 연결 함수.
	if (Character == nullptr)
	{
		return;
	}
	if (Character->HasAuthority())
	{
		// 서버에서 실행
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

	// 모든 플레이어의 탄약갯수는 서버에서 계산
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		// 여분탄약의 갯수에서 재장전할 만큼의 탄약을 감소시키고 대입한다.
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
	// 블루프린트 노티파이에서 함수가 호출되면 샷건의 탄을 업데이트(서버에서)
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
		// 샷건과 유탄발사기는 1개씩 증감된다.
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-1);
	// 샷건과 유탄발사기는 한발씩 장전할때마다 바로 쏠 수 있어야 한다
	bCanFire = true;


	// 위작업으통하여 1번씩 탄창을 더하고 탄창이 꽉찼을경우 ShotgunEnd 섹션을 재생해야한다. (이작업은 서버에서만 실행된다. 때문에 클라이언트에서도 실행 시키기위해서는 Weapon 클래스의 복제된 변수실행 OnRep 함수에서도 같은 작업을 실행해 줘야 한다.
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
	// 수류탄 투척 완료시 원래상태로 되돌리기.
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	// 우선 손에 부착된 시각용 수류탄을 숨긴다.
	ShowAttachedGrenade(false);

	// 각클라이언트에서 매프레임 업데이트되는 HitTarget의 값을 매개변수로 전달하여 수류탄을 서버에서 생성 한다.
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	// 플레이어BP에서 지정된 수류탄을 생성하여 발사한다. 단 생성은 서버에서만 하는데 각 클라이언트플레이어의 발사 각도는 따로 계산되어야한다.
	if (Character && Character->GetAttachedGrenade() &&	GrenadeClass)
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
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
			// 로컬컨트롤(각플레이어)는 수류탄투척시 몽타주를 이미 재생하였기때문에 로컬이아닐때 재생시키도록한다.
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
	// 탄약의 갯수는 무기자체가들고 있기 때문에 웨폰 클래스에서 값을 가져 와야 함.
	if (EquippedWeapon == nullptr) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	// 무기의 타입이 지정되어 있어야 정확한 탄약의 갯수를 계산
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		// 소지하고있는 탄약의 갯수 설정
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		// 최소의 갯수는 들고있을수있는 탄창에서 사용후 소모된양 혹은 소지중인(리로드할수있는)남은 탄약중 적은 쪽의 숫자로 지정
		int32 Least = FMath::Min(RoomInMag, AmountCarried);

		return FMath::Clamp(RoomInMag, 0, Least);

	}

	// 무기의 타입이 지정되어 있지 않다면 0을 반환
	return 0;
}

void UCombatComponent::ThrowGrenade()
{
	// 우선 던질 수 있는 수류탄이 있는지 확인(클라이언트)
	if (Grenades == 0) return;

	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr)
	{
		// 수류탄을 한번던지고 연속 으로 던져지지 않도록 방지
		return;
	}
	// 수류탄 투척시 서버와 플레이어 모두가 알아야 한다. 플레이어의 전투 상태는 복제되기때문에 변경시 클라이언트는 바로 알 수 있으나 서버는 따로 알 수 없기 떄문에 서버에서 호출될 RPC가 필요
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLefttHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	// 클라이언트에서 서버 함수를 호출한다. 하지만 클라이언트가아닌 서버가 수류탄을 투척할 경우는 추가적으로 호출해서는 안된다.
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}

	// 클라이언트가아닌 서버에서 호출되었다면
	if (Character && Character->HasAuthority())
	{
		// 수류탄이 던저졌다면 서버에서는 각플레이어의 수류탄 갯수를 감소시킨다.
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);

		// 이후 컨트롤러의 HUD 오버레이를 업데이트한다.
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	// 우선 던질 수 있는 수류탄이 있는지 확인(서버)
	if (Grenades == 0) return;

	// 서버 호출
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLefttHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	// 수류탄이 던저졌다면 서버에서는 각플레이어의 수류탄 갯수를 감소시킨다.
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);

	// 이후 컨트롤러의 HUD 오버레이를 업데이트한다.
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::OnRep_Grenades()
{
	// 각 클라이언트는 수류탄의 갯수가 변경될때 업데이트 시킨다.
	UpdateHUDGrenades();
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character &&
		Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// 화면중앙(크로스헤어)에서 월드 방향으로 추적 하도록
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 스크린좌표의 가로세로 반(중앙) 위치 저장
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// 스크린좌표에서 월드좌표로 변경하기위해 필요한 변수
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// 크로스헤어의 월드 좌표와 방향을 저장
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	// 좌표값 저장에 성공했다면
	if (bScreenToWorld)
	{
		// 라인트레이스를 위한 시작지점과 끝지점 설정
		FVector Start = CrosshairWorldPosition;

		// 카메라의 위치에서 부터 시작되고 있으나 Self의 메쉬나 다른메쉬가 캐릭터와 카메라사이에올 경우 조준을 뒤로 하는 문제가 있다. 때문에 트레이스의 시작지점을 카메라의 위치보다 앞에서 시작

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

		// 피격지점이 무효할 경우
		if (!TraceHitResult.IsValidBlockingHit())
		{
			// TRACE_LENGTH 거리에 ImpactPoint를 설정
			TraceHitResult.ImpactPoint = Start + CrosshairWorldDirection * TRACE_LENGTH;
		}

		// 플레이어와 충돌했을때 크로스헤어 색상 변경 트레이스 결과에 액터가 있고 액터의 인터페이스가 구현되어 있다면
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

	// 컨트롤러가 nullptr 일경우 캐스팅해서 넣어주고 아니면 컨트롤러를 그대로
	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<APlayerHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				// 무기를 장착했을때 그무기에 설정된 텍스처를 넣어준다.
				HUDPackage.CrossHairCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrossHairLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrossHairRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrossHairTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrossHairBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				// 무기를 장착하지 않았을때 비워두기
				HUDPackage.CrossHairCenter = nullptr;
				HUDPackage.CrossHairLeft = nullptr;
				HUDPackage.CrossHairRight = nullptr;
				HUDPackage.CrossHairTop = nullptr;
				HUDPackage.CrossHairBottom = nullptr;
			}

			// 크로스헤어 퍼짐 업데이트 (캐릭터의 움직임에 따라 벌어지거나 공격할떄 벌어짐)
			// 캐릭터의 최소속도~최대속도를 0~1의 값으로 노멀라이즈
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// 공중에있거나 점프했을때 벌어지게설정
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.5f, DeltaTime, 2.5f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// 조준 했을때
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
		
			// 사격 했을 때 추가된값 0으로 보간시키기
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


	// 조준시 무기에 설정된 FOV변경 속도값에 의하여 줌인 줌아웃의 기능을 구현한다.
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	
	// FOV 값의 보간이 완료된후 Set
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
	// 조준 상태의 속도로 조절
	if (Character)
	{
		if (!Character->GetIsSpeedUpBuff())
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		}

	}

	// 플레이어가 로컬상태이고 스나이퍼라이플 무기를 장착하고있을때
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	// 조준 상태의 속도로 조절
	if (Character)
	{
		if (!Character->GetIsSpeedUpBuff())
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		}
	}

}

bool UCombatComponent::CanFire()
{
	// 장착한 무기가 없다면 false 반환
	if (EquippedWeapon == nullptr) return false;

	// 샷건과 유탄발사기에 대하여 예외처리
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
	// 클라이언트 업데이트
	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	// 샷건or유탄발사기 일때 더이상 재장전할 탄창이 없을경우 몽타주 섹션을 건너 뛰어야 한다.
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

bool UCombatComponent::ShouldSwapWeapon()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}
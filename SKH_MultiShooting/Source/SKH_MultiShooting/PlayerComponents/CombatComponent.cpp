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
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
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

	// 만일 현재 장착중인 무기가있는데 다른 무기를 장착하려 할 경우 현재 들고있는 무길르 드랍시키고 새무기를 장착한다
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	// 오너변경직후(플레이어가 무기를 장착) HUDAmmo 세팅
	EquippedWeapon->SetHUDAmmo();

	// CarriedAmmo 설정하기전에 무기의 타입 지정
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AFirstPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	// 서버에서 사운드 재생
	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			EquippedWeapon->EquipSound, 
			Character->GetActorLocation()
		);
	}

	// 무기를 집자마자 탄창이 비어있다면
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}

	// 아이템 장착시 정면으로 향하도록
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	// 소지한 탄약의 수가 0보다 크고 플레이어가 리로드 상태가 아닐경우
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
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

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 어태치 전에 무기의 상태를 확실히 변경한후에 어태치 시킬 수 있도록한다.
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		// 클라이언트에서 사운드 재생
		if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedWeapon->EquipSound,
				Character->GetActorLocation()
			);
		}
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
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	// 조준 상태의 속도로 조절
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	// 조준 상태의 속도로 조절
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

}

bool UCombatComponent::CanFire()
{
	// 장착한 무기가 없다면 false 반환
	if (EquippedWeapon == nullptr) return false;

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
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
}


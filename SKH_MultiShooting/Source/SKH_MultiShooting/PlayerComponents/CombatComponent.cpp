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
//#include "SKH_MultiShooting/HUD/PlayerHUD.h"

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

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	// ���� ������ �ӵ��� ����
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
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

void UCombatComponent::OnRep_EquippedWeapon()
{
	// Ŭ���̾�Ʈ���� ������ ������ �������� ���ϵ���
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
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

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);

	// ������ ������ �������� ���ϵ���
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

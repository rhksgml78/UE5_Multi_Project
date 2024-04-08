#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "SKH_MultiShooting/Weapon/Weapon.h"
#include "SKH_MultiShooting/PlayerComponents/CombatComponent.h"
#include "SKH_MultiShooting/PlayerComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "SKH_MultiShooting/SKH_MultiShooting.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "SKH_MultiShooting/PlayerController/FirstPlayerController.h"
#include "SKH_MultiShooting/GameMode/PlayerGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "SKH_MultiShooting/PlayerState/FirstPlayerState.h"
#include "SKH_MultiShooting/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "SKH_MultiShooting/PlayerComponents/LagCompensationComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ĳ���ͽ����� �浹������ ��ĥ��� �������Ͽ� ������ ���� �ǵ���
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 450.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// �÷��̾ �̵��ϴ¹��⿡���� ȸ����Ű��
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// ������Ʈ�� ������ �����ؾ��Ѵ�.
	Combat = CreateAbstractDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateAbstractDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	// �������� ����� ������Ʈ (������ �ʿ�X)
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// ĳ���Ͱ� �����̴� �������� ȸ���Ҷ��� �ӵ� �⺻360.f
	GetCharacterMovement()->RotationRate.Yaw = 720.f;

	TurningInplace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	// Ÿ�Ӷ��� ������Ʈ
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// ����ź�� ��� ���� ��
	AttachedGrenade = CreateAbstractDefaultSubobject< UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetVisibility(false);

	/*
	���� �ǰ��� ������ �ڽ� ������Ʈ ����
	�� 16�� ���� ����
	���� ũ����� BP���� �����Ұ�
	*/
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	/*
	"Net/UnrealNetwork.h" ����ʿ��� ��ũ��
	OverlappingWeapon�� ���� ����ɋ�(���Ⱑ�ٲ�) ����(����)�ȴ�.
	��, �������� �ž�����Ʈ���� ���氪�� ��� ������Ʈ�����ʴ°��̴�.
	*/

	// Ư�� Ŭ���̾�Ʈ(����)�� �����ȴ�.
	DOREPLIFETIME_CONDITION(APlayerCharacter, OverlappingWeapon, COND_OwnerOnly);

	// ��� Ŭ�� ����
	DOREPLIFETIME(APlayerCharacter, Health);
	DOREPLIFETIME(APlayerCharacter, Shield);
	DOREPLIFETIME(APlayerCharacter, bDisableGameplay);
	DOREPLIFETIME(APlayerCharacter, SpeedUpBuff);
}

void APlayerCharacter::Destroyed()
{
	// �ش��Լ��� AActor�� �Լ� ������ �̱⶧���� ��� Ŭ���̾�Ʈ���� ȣ�� �Ǵ� �Լ��̴�.
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	APlayerGameMode* PlayerGameMode = Cast<APlayerGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = PlayerGameMode && PlayerGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		// ������ �������̾ƴҋ� ��, ����Ǿ��������̶�� ����ִ� ���⸦ ��������ʰ� �ı��Ѵ�.
		Combat->EquippedWeapon->Destroy();
	}

}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// ������Ʈ���� �����ʱ�ȭ�� ���Ͽ� �ʱ�ȭ ����. ������ ������Ʈ��  ������ Ŭ������������ ���� ������ �����Ѵ�.
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed, 
			GetCharacterMovement()->MaxWalkSpeedCrouched,
			GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<AFirstPlayerController>(Controller);
		}
	}
}

void APlayerCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		// ���������� �������������� �ٸ����� ����
		AnimInstance->Montage_JumpToSection(SectionName);
	}

}

void APlayerCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void APlayerCharacter::PlayReLoadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReLoadMontage)
	{
		AnimInstance->Montage_Play(ReLoadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachinGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_ShotGun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Sniper");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void APlayerCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// �⺻���� ������ ���� �� HUD źâ ������Ʈ
	SpawnDefaultWeapon();
	UpdateHUDAmmo();

	// HUD ü��&���� ������Ʈ
	UpdateHudHealth();
	UpdateHudShield();

	// ������ �ݹ��Լ��� ���������� ���ε� �Ұ�
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);

	// ī�޶� ������ ����
	HideCameraIfCharacterClose();
	
	// �÷��̾� ������Ʈ�� �ѹ� �ʱ�ȭ�Ѵ�.
	PollInit();
}

void APlayerCharacter::RotateInPlace(float DeltaTime)
{
	// �Է��� ���ѵ� ��� ���� ����
	if (bDisableGameplay) 
	{
		bUseControllerRotationYaw = false;
		TurningInplace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	// ���÷��� ���ؿ����� ĳ���Ͱ� ȸ���ϴ� ���� �ٸ���
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		//���ӿ����¿� ������Ʈ
		AimOffset(DeltaTime);
	}
	else
	{
		// �ð����� ������ �ϳ��ΰ� �����ð��� ������ ĳ������ �������̾����� ������Ʈ ���� �ʱ⶧���� ������ �ð��� ������� �����Ŀ�
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25)
		{
			// ������ �������� ������Ʈ ��Ų��.
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void APlayerCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void APlayerCharacter::Elim(bool bPlayerLeftGame)
{
	// ���� ����߸���
	DropOrDestroyWeapons();

	// �������� �� Ŭ���̾�Ʈ���� ��Ƽĳ��Ʈ �Լ� ȣ��
	MulticastElim(bPlayerLeftGame);
}

void APlayerCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	// ���޹��� ������ �÷��̾��� �������� ���� �Ѵ�.
	bLeftGame = bPlayerLeftGame;

	if (FirstPlayerController)
	{
		// �÷��̾� ����� ���Ⱑ ����Ǳ� ������ ���� AmmoHUD�� 0���� ǥ�� �ǵ��� �Ѵ�.
		FirstPlayerController->SetHUDWeaponAmmo(0);
	}

	// Ż��ó���� ĳ���͸� ���ó���ϰ� ������ ��ų �� �ֵ���
	bElimmed = true;
	PlayElimMontage();

	// DissolveMaterialInstances �迭�� ��Ұ� �ִٸ�
	if (DissolveMaterialInstances.Num() > 0)
	{
		// DissolveMaterialInstances�� ũ�⸸ŭ �ݺ�
		for (int32 Index = 0; Index < DissolveMaterialInstances.Num(); ++Index)
		{
			// ���� �ε����� �ش��ϴ� MaterialInstance ��������
			UMaterialInstance* MaterialInstance = DissolveMaterialInstances[Index];

			// ���� ��Ƽ���� �ν��Ͻ� ����
			UMaterialInstanceDynamic* DynamicInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);

			if (DynamicInstance)
			{
				// �޽��� �ش� �ε��� ��ġ�� ���� ��Ƽ���� �ν��Ͻ� ����
				GetMesh()->SetMaterial(Index, DynamicInstance);

				// �ʱ� �� ����
				DynamicInstance->SetScalarParameterValue(TEXT("Dissolve"), -1.f);
				DynamicInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);

				// DynamicDissolveMaterialInstances�� ���� �ν��Ͻ� �߰�
				DynamicDissolveMaterialInstances.Add(DynamicInstance);
			}
		}
	}
	// ���۾��� ������ ������ ����
	StartDissolve();

	// �÷��̾��� �Է� ����
	bDisableGameplay = true;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// ����� �ݸ����� ������� �ʾƷ��� �߶��ϴ°� ����
	GetCharacterMovement()->DisableMovement();

	// �ݸ��� ��ȿȭ
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Elim����Ʈ ����
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 150.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	// Wasted �ִϸ��̼� ��� ���⼭?
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

	if (FirstPlayerController)
	{
		FirstPlayerController->PlayDefeatsAnimation();
	}

	// ���� ������������ �������·� ���������� ������ ������ ���־��Ѵ�.
	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	// �����ð��� ������ Ÿ�̸Ӹ� Ȱ��ȭ �Ѵ�.
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinishied,
		ElimDelay
	);
}

void APlayerCharacter::ElimTimerFinishied()
{
	APlayerGameMode* PlayerGameMode = GetWorld()->GetAuthGameMode<APlayerGameMode>();

	// ��Ƽĳ��ƮElim �Լ����� ���ŵ� ������ ���� �÷��̾ ������ �������� �׷��� ������� �ٸ� ������ �����Ѵ�.
	if (PlayerGameMode && !bLeftGame)
	{
		// �÷��̾ ������ �������̾ƴ϶�� �������� ��û�ϰ�
		PlayerGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		// �÷��̾ �����ٸ� ��������Ʈ�� ��ε�ĳ��Ʈ
		OnLeftGame.Broadcast();
	}
}

void APlayerCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void APlayerCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void APlayerCharacter::ServerLeavGame_Implementation()
{
	// ���Ӹ��� ���¸� ��� �����Ѵ�. �̶� ���Ӹ��� Server �������ϰ������Ƿ� ������ ���Ӹ���� AuthGameMode �� ���Ͽ� ��´�.
	APlayerGameMode* PlayerGameMode = GetWorld()->GetAuthGameMode<APlayerGameMode>();

	FirstPlayerState = FirstPlayerState == nullptr ? GetPlayerState<AFirstPlayerState>() : FirstPlayerState;

	if (PlayerGameMode && FirstPlayerState)
	{
		// �����������ϴ� ���Ӹ�尡 ������ ���� �÷��̾��� ���¸� �����Ѵ�.
		PlayerGameMode->PlayerLeftGame(FirstPlayerState);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ThisClass::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ThisClass::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ThisClass::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ThisClass::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ThisClass::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ThisClass::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ThisClass::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ThisClass::GrenadeButtonPressed);


}

void APlayerCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;

	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));

		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;

	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void APlayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

// ȣ��
void APlayerCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		// �����Լ� ȣ��
		if (Combat->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerEquipButtonPressed();
		}

		bool bSwap =
			Combat->ShouldSwapWeapon() &&
			!HasAuthority() &&
			Combat->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;

		if (bSwap)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

// �������� �����Ͽ� ����
void APlayerCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapon())
		{
			Combat->SwapWeapon();
		}
	}
}

void APlayerCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void APlayerCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APlayerCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void APlayerCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}

}

void APlayerCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

}

void APlayerCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->Reload();
	}
}

void APlayerCharacter::GrenadeButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void APlayerCharacter::Jump()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

float APlayerCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void APlayerCharacter::OnRep_SpeedUp()
{
	// ������ ���� ����Ǹ� �Լ��� �ڵ������� ����ȴ�.
	SetSpeedUi(SpeedUpBuff);
}

void APlayerCharacter::AimOffset(float DeltaTime)
{
	// ���Ⱑ ������ ����
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// �ӵ��� 0�̰� �������°��ƴҶ�
	if (Speed == 0.f && !bIsInAir)
	{
		// ��Ʈ�� ȸ�� ����
		bRotateRootBone = true;

		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);

		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInplace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	// ĳ���Ͱ� �����̰��ְų� ���������϶�
	if (Speed > 0.f || bIsInAir)
	{
		// ��Ʈ�� ȸ�� ����
		bRotateRootBone = false;

		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInplace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void APlayerCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	// Ŭ���̾�Ʈ�� ���۵Ǵ� �����ǰ��� �������� 0~360�� ����� ����Ǿ� ���۵ǹǷ� ����ó���� �ʿ��ϴ�.
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRage(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRage, AO_Pitch);
	}
}

void APlayerCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;

	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInplace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	// ĳ���Ͱ� ȸ�������� ȸ�� �ִϸ��̼� ���
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	// ���밪 ������
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInplace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInplace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInplace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInplace = ETurningInPlace::ETIP_NotTurning;
}

void APlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	// �÷��̾ Elimeed ó���� ���� �������¿��� �ߺ��� �������� �Ա� �빮�� �ش纯���� �������ӿ� ����Ǿ������ �Ǵٽ� �������� ���� �ʵ��� ������ �д�.
	if (bElimmed) return;

	// �켱 ÿ���� ���̱����� ���尡 ���� �￩�� �Ѵ�.
	float DamageToHealth = Damage;
	if (Shield > 0)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield); 
			
			// ��簪�� ����� ���ȴٸ� ü�¿� ��ġ�� �������� 0�̴�.
			DamageToHealth = 0;
		}
		else
		{
			// ���尡 0�� �Ǿ���������� ������ �������� ������ ����(0)�� ���� ��, ������ �״���� ���� ü�¿� ��ġ�� �������� ���� �Ѵ�.
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}

	// ü�¹��� �����Ͽ� ���
	Health = FMath::Clamp(Health- DamageToHealth,0.f,MaxHealth);
	
	// HUD ������Ʈ
	UpdateHudHealth();
	UpdateHudShield();

	// �ǰ� ��� ����
	PlayHitReactMontage();

	// ü���� 0�� �Ǿ�����
	if (Health == 0.f)
	{
		APlayerGameMode* PlayerGameMode = GetWorld()->GetAuthGameMode<APlayerGameMode>();
		if (PlayerGameMode)
		{
			FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

			// ü���� 0���� ����(������) �÷��̾��� ��Ʈ�ѷ�
			AFirstPlayerController* AttackerController = Cast<AFirstPlayerController>(InstigatorController);

			PlayerGameMode->PlayerEliminated(this, FirstPlayerController, AttackerController);
		}
	}

}

void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// �ش��Լ��� UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) ��ũ�θ� ���Ͽ� �������� ���Ⱑ�ٲ� �ѹ� ȣ��ȴ�.
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void APlayerCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInplace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInplace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInplace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInplace = ETurningInPlace::ETIP_NotTurning;

			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void APlayerCharacter::HideCameraIfCharacterClose()
{
	// �÷����ϰ��ִ� ������ �ƴ϶�� ��ȯ
	if (!IsLocallyControlled()) return;

	// �÷����ϰ��ִ�(������Ʈ��) ������ ĳ���Ϳ��� ����
	if (CameraThreshold > (FollowCamera->GetComponentLocation() - GetActorLocation()).Size())
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void APlayerCharacter::OnRep_Health(float LastHealth)
{
	// ��� Ŭ���̾�Ʈ ���� �Լ�
	
	// ü�¾�����Ʈ
	UpdateHudHealth();

	// �ǰݸ���� ü����ȸ���ϴµ��� ����Ǹ� �ȵȴ�.
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void APlayerCharacter::OnRep_Shield(float LastShield)
{
	// ��� Ŭ���̾�Ʈ ���� �Լ�

	// ���������Ʈ
	UpdateHudShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void APlayerCharacter::UpdateHudHealth()
{
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

	if (FirstPlayerController)
	{
		FirstPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void APlayerCharacter::UpdateHudShield()
{
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

	if (FirstPlayerController)
	{
		FirstPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void APlayerCharacter::UpdateHUDAmmo()
{
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

	if (FirstPlayerController && Combat && Combat->EquippedWeapon)
	{
		FirstPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		FirstPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void APlayerCharacter::PollInit()
{
	if (FirstPlayerState == nullptr)
	{
		FirstPlayerState = GetPlayerState<AFirstPlayerState>();
		if (FirstPlayerState)
		{
			FirstPlayerState->AddToScore(0.f);
			FirstPlayerState->AddToDefeats(0);
		}
	}
}

void APlayerCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	for (auto* MaterialInstance : DynamicDissolveMaterialInstances)
	{
		if (MaterialInstance != nullptr) 
		{
			MaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		}
	}
}

void APlayerCharacter::StartDissolve()
{
	// Ÿ�Ӷ��ο� ���� ���ε�
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void APlayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// ���� �÷��̾�� ������ �ѹ� üũ�ؾ���
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	/*
	GetLifetimeReplicatedProps �Լ����� ���ʿ���(COND_OwnerOnly) �����ϴٷ� ������ �Ǿ��ֱ⶧���� �Ʒ��� ���ǹ��� ����� �� �ִ�.
	*/
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool APlayerCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool APlayerCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* APlayerCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)
	{
		return nullptr;
	}

	return Combat->EquippedWeapon;
}

FVector APlayerCharacter::GetHitTarget() const
{
	if (Combat == nullptr)
	{
		return FVector();
	}
	return Combat->HitTarget;
}

ECombatState APlayerCharacter::GetCombatState() const
{
	if (Combat == nullptr)
	{
		return ECombatState::ECS_MAX;
	}
	return Combat->CombatState;
}

void APlayerCharacter::SetSpeedUpBuff(bool isSpeedUpActive)
{
	// �Ű������� ���Ͽ� ���� ������ ���� �����ϰ�
	SpeedUpBuff = isSpeedUpActive;

	// ������ �ٲ�� Ŭ���̾�Ʈ�� OnRep_SpeedUp�Լ��� �ڵ����������� ������ �׷��� �ʱ� ������ ������ ���ǿ����� ���� �Լ��� ȣ���Ѵ�.

	if (HasAuthority()) // �������� ����
	{
		OnRep_SpeedUp();
	}
}

void APlayerCharacter::SetSpeedUi(bool isVisible)
{
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

	if (FirstPlayerController)
	{
		FirstPlayerController->SetSpeedUi(isVisible);
	}
}

void APlayerCharacter::SpawnDefaultWeapon()
{
	APlayerGameMode* PlayerGameMode = Cast<APlayerGameMode>(UGameplayStatics::GetGameMode(this));

	UWorld* World = GetWorld();
	if (PlayerGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

bool APlayerCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;

	return Combat->bLocallyReloading;
}
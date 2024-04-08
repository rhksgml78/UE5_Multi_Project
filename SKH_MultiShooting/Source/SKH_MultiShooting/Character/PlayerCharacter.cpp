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

	// 캐릭터스폰시 충돌지점이 겹칠경우 재조정하여 무조건 스폰 되도록
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 450.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 플레이어가 이동하는방향에따라 회전시키기
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// 컴포넌트는 복제가 가능해야한다.
	Combat = CreateAbstractDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateAbstractDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	// 서버에서 사용할 컴포넌트 (복제할 필요X)
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// 캐릭터가 움직이는 방향으로 회전할때의 속도 기본360.f
	GetCharacterMovement()->RotationRate.Yaw = 720.f;

	TurningInplace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	// 타임라인 컴포넌트
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// 수류탄을 들고 있을 것
	AttachedGrenade = CreateAbstractDefaultSubobject< UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetVisibility(false);

	/*
	서버 되감기 정보용 박스 컴포넌트 생성
	총 16개 관절 부위
	상세한 크기등은 BP에서 설정할것
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
	"Net/UnrealNetwork.h" 헤더필요한 메크로
	OverlappingWeapon의 값이 변경될떄(무기가바뀔때) 갱신(복제)된다.
	즉, 매프레임 매업데이트마다 변경값을 계속 업데이트하지않는것이다.
	*/

	// 특정 클라이언트(오너)만 복제된다.
	DOREPLIFETIME_CONDITION(APlayerCharacter, OverlappingWeapon, COND_OwnerOnly);

	// 모든 클라에 복제
	DOREPLIFETIME(APlayerCharacter, Health);
	DOREPLIFETIME(APlayerCharacter, Shield);
	DOREPLIFETIME(APlayerCharacter, bDisableGameplay);
	DOREPLIFETIME(APlayerCharacter, SpeedUpBuff);
}

void APlayerCharacter::Destroyed()
{
	// 해당함수는 AActor의 함수 재정의 이기때문에 모든 클라이언트에서 호출 되는 함수이다.
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	APlayerGameMode* PlayerGameMode = Cast<APlayerGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = PlayerGameMode && PlayerGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		// 게임이 진행중이아닐떄 즉, 종료되었을시점이라면 들고있던 무기를 드랍하지않고 파괴한다.
		Combat->EquippedWeapon->Destroy();
	}

}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 컴포넌트들을 사전초기화를 통하여 초기화 진행. 각각의 컴포넌트는  프랜드 클래스선언으로 직접 적으로 접근한다.
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
		// 조준했을때 안했을때에따라 다른섹션 지정
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

	// 기본무기 생성및 장착 후 HUD 탄창 업데이트
	SpawnDefaultWeapon();
	UpdateHUDAmmo();

	// HUD 체력&쉴드 업데이트
	UpdateHudHealth();
	UpdateHudShield();

	// 데미지 콜백함수는 서버에서만 바인딩 할것
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

	// 카메라 가려짐 보안
	HideCameraIfCharacterClose();
	
	// 플레이어 스테이트를 한번 초기화한다.
	PollInit();
}

void APlayerCharacter::RotateInPlace(float DeltaTime)
{
	// 입력이 제한될 경우 변수 설정
	if (bDisableGameplay) 
	{
		bUseControllerRotationYaw = false;
		TurningInplace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	// 로컬룰의 기준에따라 캐릭터가 회전하는 것을 다르게
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		//에임오프셋용 업데이트
		AimOffset(DeltaTime);
	}
	else
	{
		// 시간측정 변수를 하나두고 일정시간이 지나도 캐릭터의 움직임이없으면 업데이트 되지 않기때문에 변수의 시간이 어느정도 지난후에
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25)
		{
			// 복제된 움직임을 업데이트 시킨다.
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
	// 무기 떨어뜨리기
	DropOrDestroyWeapons();

	// 서버에서 각 클라이언트에게 멀티캐스트 함수 호출
	MulticastElim(bPlayerLeftGame);
}

void APlayerCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	// 전달받은 값으로 플레이어의 변수값을 갱신 한다.
	bLeftGame = bPlayerLeftGame;

	if (FirstPlayerController)
	{
		// 플레이어 사망시 무기가 드랍되기 떄문에 현재 AmmoHUD를 0으로 표기 되도록 한다.
		FirstPlayerController->SetHUDWeaponAmmo(0);
	}

	// 탈락처리된 캐릭터를 사망처리하고 리스폰 시킬 수 있도록
	bElimmed = true;
	PlayElimMontage();

	// DissolveMaterialInstances 배열에 요소가 있다면
	if (DissolveMaterialInstances.Num() > 0)
	{
		// DissolveMaterialInstances의 크기만큼 반복
		for (int32 Index = 0; Index < DissolveMaterialInstances.Num(); ++Index)
		{
			// 현재 인덱스에 해당하는 MaterialInstance 가져오기
			UMaterialInstance* MaterialInstance = DissolveMaterialInstances[Index];

			// 동적 머티리얼 인스턴스 생성
			UMaterialInstanceDynamic* DynamicInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);

			if (DynamicInstance)
			{
				// 메쉬의 해당 인덱스 위치에 동적 머티리얼 인스턴스 설정
				GetMesh()->SetMaterial(Index, DynamicInstance);

				// 초기 값 세팅
				DynamicInstance->SetScalarParameterValue(TEXT("Dissolve"), -1.f);
				DynamicInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);

				// DynamicDissolveMaterialInstances에 동적 인스턴스 추가
				DynamicDissolveMaterialInstances.Add(DynamicInstance);
			}
		}
	}
	// 위작업이 끝난뒤 디졸브 실행
	StartDissolve();

	// 플레이어의 입력 방지
	bDisableGameplay = true;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// 사망시 콜리전이 사라지고 맵아래로 추락하는것 방지
	GetCharacterMovement()->DisableMovement();

	// 콜리전 무효화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Elim이펙트 실행
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
	// Wasted 애니메이션 재생 여기서?
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

	if (FirstPlayerController)
	{
		FirstPlayerController->PlayDefeatsAnimation();
	}

	// 만일 스나이퍼줌이 켜진상태로 사망했을경우 스코프 위젯을 없애야한다.
	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}

	// 일정시간뒤 실행할 타이머를 활성화 한다.
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

	// 멀티캐스트Elim 함수에서 갱신된 변수에 따라 플레이어가 게임을 떠난경우와 그렇지 않은경우 다른 로직을 실행한다.
	if (PlayerGameMode && !bLeftGame)
	{
		// 플레이어가 게임을 떠난것이아니라면 리스폰을 요청하고
		PlayerGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		// 플레이어가 떠낫다면 델리게이트에 브로드캐스트
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
	// 게임모드와 상태를 얻어 실행한다. 이때 게임모드는 Server 가관리하고있으므로 서버의 게임모드인 AuthGameMode 를 통하여 얻는다.
	APlayerGameMode* PlayerGameMode = GetWorld()->GetAuthGameMode<APlayerGameMode>();

	FirstPlayerState = FirstPlayerState == nullptr ? GetPlayerState<AFirstPlayerState>() : FirstPlayerState;

	if (PlayerGameMode && FirstPlayerState)
	{
		// 서버가관리하는 게임모드가 게임을 떠난 플레이어의 상태를 전달한다.
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

// 호출
void APlayerCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		// 복제함수 호출
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

// 서버에서 복제하여 실행
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
	// 지정한 값이 변경되면 함수가 자동적으로 실행된다.
	SetSpeedUi(SpeedUpBuff);
}

void APlayerCharacter::AimOffset(float DeltaTime)
{
	// 무기가 없으면 리턴
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// 속도가 0이고 점프상태가아닐때
	if (Speed == 0.f && !bIsInAir)
	{
		// 루트본 회전 관련
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

	// 캐릭터가 움직이고있거나 점프상태일때
	if (Speed > 0.f || bIsInAir)
	{
		// 루트본 회전 관련
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
	// 클라이언트에 전송되는 각도의값은 음수없이 0~360의 양수로 압축되어 전송되므로 예외처리가 필요하다.
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

	// 캐릭터가 회전했을때 회전 애니메이션 재생
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	// 절대값 먼저비교
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
	// 플레이어가 Elimeed 처리가 되지 않은상태에서 중복된 데미지를 입기 대문에 해당변수가 한프레임에 변경되었을경우 또다시 데미지를 받지 않도록 제한을 둔다.
	if (bElimmed) return;

	// 우선 첼력이 깍이기전에 쉴드가 먼저 깍여야 한다.
	float DamageToHealth = Damage;
	if (Shield > 0)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield); 
			
			// 모든값이 쉴드로 상쇄된다면 체력에 미치는 데미지는 0이다.
			DamageToHealth = 0;
		}
		else
		{
			// 쉴드가 0이 되어버리고나서는 들어오는 데미지의 값에서 쉴드(0)을 뺀값 즉, 데미지 그대로의 값을 체력에 미치는 데미지로 지정 한다.
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}

	// 체력범위 제한하여 계산
	Health = FMath::Clamp(Health- DamageToHealth,0.f,MaxHealth);
	
	// HUD 업데이트
	UpdateHudHealth();
	UpdateHudShield();

	// 피격 모션 실행
	PlayHitReactMontage();

	// 체력이 0이 되었을때
	if (Health == 0.f)
	{
		APlayerGameMode* PlayerGameMode = GetWorld()->GetAuthGameMode<APlayerGameMode>();
		if (PlayerGameMode)
		{
			FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

			// 체력을 0으로 만든(공격한) 플레이어의 컨트롤러
			AFirstPlayerController* AttackerController = Cast<AFirstPlayerController>(InstigatorController);

			PlayerGameMode->PlayerEliminated(this, FirstPlayerController, AttackerController);
		}
	}

}

void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// 해당함수는 UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) 매크로를 통하여 오버랩된 무기가바뀔때 한번 호출된다.
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
	// 플레이하고있는 본인이 아니라면 반환
	if (!IsLocallyControlled()) return;

	// 플레이하고있는(로컬컨트롤) 본인의 캐릭터에만 적용
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
	// 모든 클라이언트 실행 함수
	
	// 체력업데이트
	UpdateHudHealth();

	// 피격모션은 체력이회복하는동안 재생되면 안된다.
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void APlayerCharacter::OnRep_Shield(float LastShield)
{
	// 모든 클라이언트 실행 함수

	// 쉴드업데이트
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
	// 타임라인에 동적 바인딩
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void APlayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// 서버 플레이어는 별도로 한번 체크해야함
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	/*
	GetLifetimeReplicatedProps 함수에서 오너에게(COND_OwnerOnly) 복사하다록 구조가 되어있기때문에 아래의 조건문을 사용할 수 있다.
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
	// 매개변수를 통하여 복제 변수의 값을 변경하고
	SpeedUpBuff = isSpeedUpActive;

	// 변수가 바뀌면 클라이언트는 OnRep_SpeedUp함수를 자동실행하지만 서버는 그렇지 않기 때문에 서버의 조건에서는 직접 함수를 호출한다.

	if (HasAuthority()) // 서버에서 실행
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
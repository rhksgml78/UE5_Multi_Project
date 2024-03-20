#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "SKH_MultiShooting/Weapon/Weapon.h"
#include "SKH_MultiShooting/PlayerComponents/CombatComponent.h"
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

	Combat = CreateAbstractDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

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
	DOREPLIFETIME(APlayerCharacter, bDisableGameplay);
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

	// 컴뱃 컴포넌트의 값을 먼저 한번 갱신한다.
	if (Combat)
	{
		Combat->Character = this;
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
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// HUD 체력 업데이트
	UpdateHudHealth();

	// 데미지 콜백함수는 서버에서만 바인딩 할것
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
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

void APlayerCharacter::Elim()
{
	// 무기 떨어뜨리기
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}

	// 서버에서 각 클라이언트에게 멀티캐스트 함수 호출
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinishied,
		ElimDelay
	);
}

void APlayerCharacter::MulticastElim_Implementation()
{
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

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	
	// 플레이어의 입력 방지
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();

	// 캐릭터의 움직임 제한 (이동 및 입력)
	//GetCharacterMovement()->DisableMovement(); 
	//GetCharacterMovement()->StopMovementImmediately();

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
}

void APlayerCharacter::ElimTimerFinishied()
{
	APlayerGameMode* PlayerGameMode = GetWorld()->GetAuthGameMode<APlayerGameMode>();
	if (PlayerGameMode)
	{
		PlayerGameMode->RequestRespawn(this, Controller);
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

// 서버에서 호출
void APlayerCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

// 클라이언트에서 호출
void APlayerCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
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
	// 체력범위 제한하여 계산
	Health = FMath::Clamp(Health-Damage,0.f,MaxHealth);
	
	// HUD 업데이트
	UpdateHudHealth();

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

void APlayerCharacter::OnRep_Health()
{
	// 모든 클라이언트 실행 함수
	
	// 체력업데이트
	UpdateHudHealth();

	// 피격모션 실행
	PlayHitReactMontage();
}

void APlayerCharacter::UpdateHudHealth()
{
	FirstPlayerController = FirstPlayerController == nullptr ? Cast<AFirstPlayerController>(Controller) : FirstPlayerController;

	if (FirstPlayerController)
	{
		FirstPlayerController->SetHUDHealth(Health, MaxHealth);
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

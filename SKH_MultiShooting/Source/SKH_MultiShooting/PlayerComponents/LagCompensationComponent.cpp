#include "LagCompensationComponent.h"
#include "SKH_MultiShooting/SKH_MultiShooting.h"
#include "Components/BoxComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "SKH_MultiShooting/Weapon/Weapon.h"
#include "DrawDebugHelpers.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 서버되감기 기능은 서버에서만 사용할 것이다.
	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;

	if (FrameHistory.Num() <= 1)
	{
		// 양방향 연결리스트가 비어있을경우 0,1 배열 즉 헤드와 테일이 없을경우 2번 리스트에 추가하여 헤드와 테일을 추가한다.
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		// 헤드와 테일이 있을경우 처음(헤드)에 저장된 시간과 끝(테일)에 저장된 시간의 차이를 확인하고 지정한 최대 시간을 넘어 설 경우
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			// 제일이전에 저장된(테일)노드를 제거하고
			FrameHistory.RemoveNode(FrameHistory.GetTail());

			// 시간의 차이를 다시 측정한다.
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}

		// 반복문의 조건에 맞지 않을때에는 계속해서 최신노드(헤드)를 갱신해가며 추가한다.
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<APlayerCharacter>(GetOwner()) : Character;
	if (Character)
	{
		// 현프로젝트에서 서버와 클라이언트는 RTT를 계산하고 시간을 동기화 하기 때문에 단순히 월드상의 타임을 얻어 사용한다.
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			// 플레이어의 TMap<FName, class UBoxComponent*> HitCollisionBoxes 정보의 키, 값 두개를 충돌박스 구조체에 그대로 저장한다.
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(
	const FFramePackage& OlderFrame, 
	const FFramePackage& YoungerFrame, 
	float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		// 복사가아닌 원본을 참조& 하는 형태이기때문에 그대로 계산

		// 이름의 값을 저장(키+값 으로 되어있기때문에 이름도필요!)
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		// 보간된 값을 저장할 임시 객체
		FBoxInformation InterpBoxInfo;

		// 보간해야할 것들은 위치와 회전값! 박스의 크기는 항상 동일하기때문에 단순하게 최근의 정보를 넣어주기만 하면 된다.
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		// 보간이 완료된 정보를 구조체의 맵에 키와 값을 설정하여 넣는다.
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	// 반복문을 끝내고 담겨진 값을 넘긴다.
	return InterpFramePackage;
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(
	APlayerCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation, 
	float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(
	APlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity,
	float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<APlayerCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (APlayerCharacter* HitCharcter : HitCharacters)
	{
		// 프레임체크및 보간까지 완료하여 배열에 저장
		FramesToCheck.Add(GetFrameToCheck(HitCharcter, HitTime));
	}
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(APlayerCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;

	if (bReturn) return FFramePackage();

	// 피격을 확인하기위한 프레임페키지
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	// 공격을 당한 플레이어
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	// 오래된 프레임의 시간이 피격시간보다 클경우(오래되었을경우)
	while (Older->GetValue().Time > HitTime)
	{
		// (OlderTime) < (HitTime) < (YoungerTime)
		if (Older->GetNextNode() == nullptr) break;

		Older = Older->GetNextNode();

		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if (bShouldInterpolate)
	{
		// 오랜정보와 최근정보사이에서 보간해야함.
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(
	APlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity,
	float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

	if (Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(
	APlayerCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation, 
	float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<APlayerCharacter*>& HitCharacters, 
	const FVector_NetQuantize& TraceStart, 
	const TArray<FVector_NetQuantize>& HitLocations, 
	float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		// 피격당한 플레이어가없거나 무기가 없을경우 다음반복문을 실행
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr) continue;

		float TotalDamage = 0.f;

		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * Character->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(
	const FFramePackage& Package, 
	APlayerCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// 플레이어의 되감기용 히트박스는 콜리전이 OFF상태이기때문에 켜야함
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	// 라인트레이스 진행
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (ConfirmHitResult.bBlockingHit)
		{
			//if (ConfirmHitResult.Component.IsValid())
			//{
			//	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
			//	if (Box)
			//	{
			//		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 3.f);
			//	}
			//}
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else // 머리가아닌 다른 박스
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (ConfirmHitResult.bBlockingHit)
			{
				//if (ConfirmHitResult.Component.IsValid())
				//{
				//	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				//	if (Box)
				//	{
				//		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 3.f);
				//	}
				//}
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}

	// 충돌박스를 리셋
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

	// 피격당하지 않았을경우 모두 false 반환
	return FServerSideRewindResult{ false, false };
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(
	const FFramePackage& Package,
	APlayerCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity,
	float HitTime)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// 플레이어의 되감기용 히트박스는 콜리전이 OFF상태이기때문에 켜야함
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	// 투사체는 라인트레이스가아닌 경로 추적이 필요하다.
	FPredictProjectilePathParams PathParams;
	FPredictProjectilePathResult PathResult;

	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = MaxRecordTime;
	PathParams.DrawDebugType = EDrawDebugTrace::None;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	
	if (PathResult.HitResult.bBlockingHit)
	{
		// 헤드샷
		//if (PathResult.HitResult.Component.IsValid())
		//{
		//	UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
		//	if (Box)
		//	{
		//		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 3.f);
		//	}
		//}
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{ true, true };
	}
	else
	{
		// 반복문을 통하여 피격한 캐릭터의 충돌박스를 반복문을통하여 충돌체의 설정을 커스텀 채널로 설정한다.
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}

		//다시 경로를 계산하고
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			//if (PathResult.HitResult.Component.IsValid())
			//{
			//	UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			//	if (Box)
			//	{
			//		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 3.f);
			//	}
			//}
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, false };
		}
	}

	// 충돌박스를 리셋
	ResetHitBoxes(HitCharacter, CurrentFrame);

	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

	// 피격당하지 않았을경우 모두 false 반환
	return FServerSideRewindResult{ false, false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(
	const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		// 반복문이 해당 함수에서 꾀 많이 이루어지기때문에 빠르게 종료할수도있는 조건을 만들어 조건에 해당하지않는다면 많은 반복문을 실행하지 않도록 방지
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}

	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		// 플레이어의 되감기용 히트박스는 콜리전이 OFF상태이기때문에 켜야함
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("Head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	UWorld* World = GetWorld();

	// 헤드샷 체크하기
	for (auto& HitLocation : HitLocations)
	{
		// 라인트레이스 진행
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ConfirmHitResult.GetActor());
			if (PlayerCharacter)
			{
				//if (ConfirmHitResult.Component.IsValid())
				//{
				//	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				//	if (Box)
				//	{
				//		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 3.f);
				//	}
				//}

				if (ShotgunResult.HeadShots.Contains(PlayerCharacter))
				{
					// 배열에 이미 피격 기록이있다면 피격횟수 증가
					ShotgunResult.HeadShots[PlayerCharacter]++;
				}
				else
				{
					// 처음 피격이라면 피격횟수 1로 배열에 추가
					ShotgunResult.HeadShots.Emplace(PlayerCharacter, 1);
				}
			}
		}
	}

	// 머리가아닌 다른 부위의 충돌박스를 켜주고
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		// 머리외의 부위는 충돌박스를 켜고 머리는 이미 피격확인을 하였기 때문에 꺼야함
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("Head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 바디샷 체크하기
	for (auto& HitLocation : HitLocations)
	{
		// 라인트레이스 진행
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(ConfirmHitResult.GetActor());
			if (PlayerCharacter)
			{
				//if (ConfirmHitResult.Component.IsValid())
				//{
				//	UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				//	if (Box)
				//	{
				//		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 3.f);
				//	}
				//}

				if (ShotgunResult.BodyShots.Contains(PlayerCharacter))
				{
					// 배열에 이미 피격 기록이있다면 피격횟수 증가
					ShotgunResult.BodyShots[PlayerCharacter]++;
				}
				else
				{
					// 처음 피격이라면 피격횟수 1로 배열에 추가
					ShotgunResult.BodyShots.Emplace(PlayerCharacter, 1);
				}
			}
		}
	}

	// 충돌박스를 리셋
	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;
}

void ULagCompensationComponent::CacheBoxPositions(APlayerCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package)
{

	if (HitCharacter == nullptr) return;

	// 수정전
	//for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	//{
	//	if (HitBoxPair.Value != nullptr)
	//	{
	//		HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
	//		HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
	//		HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
	//	}
	//}

	// 수정후
	for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);

			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
			}

		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	// 수정전
	//for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	//{
	//	if (HitBoxPair.Value != nullptr)
	//	{
	//		HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
	//		HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
	//		HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
	//		HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	}
	//}

	// 수정후
	for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);

			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(APlayerCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}


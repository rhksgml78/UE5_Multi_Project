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

	// �����ǰ��� ����� ���������� ����� ���̴�.
	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;

	if (FrameHistory.Num() <= 1)
	{
		// ����� ���Ḯ��Ʈ�� ���������� 0,1 �迭 �� ���� ������ ������� 2�� ����Ʈ�� �߰��Ͽ� ���� ������ �߰��Ѵ�.
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		// ���� ������ ������� ó��(���)�� ����� �ð��� ��(����)�� ����� �ð��� ���̸� Ȯ���ϰ� ������ �ִ� �ð��� �Ѿ� �� ���
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			// ���������� �����(����)��带 �����ϰ�
			FrameHistory.RemoveNode(FrameHistory.GetTail());

			// �ð��� ���̸� �ٽ� �����Ѵ�.
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}

		// �ݺ����� ���ǿ� ���� ���������� ����ؼ� �ֽų��(���)�� �����ذ��� �߰��Ѵ�.
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
		// ��������Ʈ���� ������ Ŭ���̾�Ʈ�� RTT�� ����ϰ� �ð��� ����ȭ �ϱ� ������ �ܼ��� ������� Ÿ���� ��� ����Ѵ�.
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			// �÷��̾��� TMap<FName, class UBoxComponent*> HitCollisionBoxes ������ Ű, �� �ΰ��� �浹�ڽ� ����ü�� �״�� �����Ѵ�.
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
		// ���簡�ƴ� ������ ����& �ϴ� �����̱⶧���� �״�� ���

		// �̸��� ���� ����(Ű+�� ���� �Ǿ��ֱ⶧���� �̸����ʿ�!)
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		// ������ ���� ������ �ӽ� ��ü
		FBoxInformation InterpBoxInfo;

		// �����ؾ��� �͵��� ��ġ�� ȸ����! �ڽ��� ũ��� �׻� �����ϱ⶧���� �ܼ��ϰ� �ֱ��� ������ �־��ֱ⸸ �ϸ� �ȴ�.
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		// ������ �Ϸ�� ������ ����ü�� �ʿ� Ű�� ���� �����Ͽ� �ִ´�.
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	// �ݺ����� ������ ����� ���� �ѱ��.
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
		// ������üũ�� �������� �Ϸ��Ͽ� �迭�� ����
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

	// �ǰ��� Ȯ���ϱ����� ��������Ű��
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	// ������ ���� �÷��̾�
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

	// ������ �������� �ð��� �ǰݽð����� Ŭ���(�����Ǿ������)
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
		// ���������� �ֱ��������̿��� �����ؾ���.
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
		// �ǰݴ��� �÷��̾���ų� ���Ⱑ ������� �����ݺ����� ����
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

	// �÷��̾��� �ǰ���� ��Ʈ�ڽ��� �ݸ����� OFF�����̱⶧���� �Ѿ���
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	// ����Ʈ���̽� ����
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
		else // �Ӹ����ƴ� �ٸ� �ڽ�
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

	// �浹�ڽ��� ����
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

	// �ǰݴ����� �ʾ������ ��� false ��ȯ
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

	// �÷��̾��� �ǰ���� ��Ʈ�ڽ��� �ݸ����� OFF�����̱⶧���� �Ѿ���
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	// ����ü�� ����Ʈ���̽����ƴ� ��� ������ �ʿ��ϴ�.
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
		// ��弦
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
		// �ݺ����� ���Ͽ� �ǰ��� ĳ������ �浹�ڽ��� �ݺ��������Ͽ� �浹ü�� ������ Ŀ���� ä�η� �����Ѵ�.
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}

		//�ٽ� ��θ� ����ϰ�
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

	// �浹�ڽ��� ����
	ResetHitBoxes(HitCharacter, CurrentFrame);

	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);

	// �ǰݴ����� �ʾ������ ��� false ��ȯ
	return FServerSideRewindResult{ false, false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(
	const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		// �ݺ����� �ش� �Լ����� �� ���� �̷�����⶧���� ������ �����Ҽ����ִ� ������ ����� ���ǿ� �ش������ʴ´ٸ� ���� �ݺ����� �������� �ʵ��� ����
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
		// �÷��̾��� �ǰ���� ��Ʈ�ڽ��� �ݸ����� OFF�����̱⶧���� �Ѿ���
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("Head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	UWorld* World = GetWorld();

	// ��弦 üũ�ϱ�
	for (auto& HitLocation : HitLocations)
	{
		// ����Ʈ���̽� ����
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
					// �迭�� �̹� �ǰ� ������ִٸ� �ǰ�Ƚ�� ����
					ShotgunResult.HeadShots[PlayerCharacter]++;
				}
				else
				{
					// ó�� �ǰ��̶�� �ǰ�Ƚ�� 1�� �迭�� �߰�
					ShotgunResult.HeadShots.Emplace(PlayerCharacter, 1);
				}
			}
		}
	}

	// �Ӹ����ƴ� �ٸ� ������ �浹�ڽ��� ���ְ�
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
		// �Ӹ����� ������ �浹�ڽ��� �Ѱ� �Ӹ��� �̹� �ǰ�Ȯ���� �Ͽ��� ������ ������
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("Head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// �ٵ� üũ�ϱ�
	for (auto& HitLocation : HitLocations)
	{
		// ����Ʈ���̽� ����
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
					// �迭�� �̹� �ǰ� ������ִٸ� �ǰ�Ƚ�� ����
					ShotgunResult.BodyShots[PlayerCharacter]++;
				}
				else
				{
					// ó�� �ǰ��̶�� �ǰ�Ƚ�� 1�� �迭�� �߰�
					ShotgunResult.BodyShots.Emplace(PlayerCharacter, 1);
				}
			}
		}
	}

	// �浹�ڽ��� ����
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

	// ������
	//for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	//{
	//	if (HitBoxPair.Value != nullptr)
	//	{
	//		HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
	//		HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
	//		HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
	//	}
	//}

	// ������
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

	// ������
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

	// ������
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


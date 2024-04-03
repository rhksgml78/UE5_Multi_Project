#include "LagCompensationComponent.h"
#include "SKH_MultiShooting/Character/PlayerCharacter.h"
#include "Components/BoxComponent.h"
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

		ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<APlayerCharacter>(GetOwner()) : Character;
	if (Character)
	{
		// ��������Ʈ���� ������ Ŭ���̾�Ʈ�� RTT�� ����ϰ� �ð��� ����ȭ �ϱ� ������ �ܼ��� ������� Ÿ���� ��� ����Ѵ�.
		Package.Time = GetWorld()->GetTimeSeconds();

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

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
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

void ULagCompensationComponent::ServerSideRewind(APlayerCharacter* HitCharcter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharcter == nullptr ||
		HitCharcter->GetLagCompensation() == nullptr ||
		HitCharcter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharcter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;

	if (bReturn) return;

	// �ǰ��� Ȯ���ϱ����� ��������Ű��
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	// ������ ���� �÷��̾�
	const TDoubleLinkedList<FFramePackage>& History = HitCharcter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime)
	{
		return;
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
	}


}


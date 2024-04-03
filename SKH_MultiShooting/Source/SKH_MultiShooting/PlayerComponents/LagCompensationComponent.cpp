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
		FFramepackage ThisFrame;
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
		FFramepackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramepackage& Package)
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

void ULagCompensationComponent::ShowFramePackage(const FFramepackage& Package, const FColor& Color)
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


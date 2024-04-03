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

		ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<APlayerCharacter>(GetOwner()) : Character;
	if (Character)
	{
		// 현프로젝트에서 서버와 클라이언트는 RTT를 계산하고 시간을 동기화 하기 때문에 단순히 월드상의 타임을 얻어 사용한다.
		Package.Time = GetWorld()->GetTimeSeconds();

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

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
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

void ULagCompensationComponent::ServerSideRewind(APlayerCharacter* HitCharcter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn =
		HitCharcter == nullptr ||
		HitCharcter->GetLagCompensation() == nullptr ||
		HitCharcter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharcter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;

	if (bReturn) return;

	// 피격을 확인하기위한 프레임페키지
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	// 공격을 당한 플레이어
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
	}


}


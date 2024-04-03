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
		FFramepackage ThisFrame;
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


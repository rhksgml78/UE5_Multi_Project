#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "LagCompensationComponent.generated.h"

/*
�ش� ������Ʈ�� ����ĳ���Ͱ� Ŭ���̾�Ʈ ĳ������ ������ �����Ͽ� �����ϰ�
������ ���Ͽ� �ǰ��⸦ ����� �� �ֵ��� �ϱ����� ������Ʈ

���������� ������ ������ �� �־���Ѵ� Frame History
�����Ҷ� �ܼ��� �÷��̾��� ��ġ�� �����ϱ⺸�ٴ� �÷��̾��� �浹�����õ�
������ �����ؾ��ϸ� �ش� ������ �������� ������ ���� �ٸ���.
�������� ���� ���ɿ� �δ��� �ǰ� ����� ����.

���� �����Ѱ��� ĸ�� = ��Ȯ�� ������
�߰��ܰ�� �ڽ����� �浹ü�� �÷��̾� �������� ��ġ = ������� ��Ȯ��
���� ����̳������� �޽����� = ��Ȯ�� ĳ���������浹�� �ſ��δ�!
*/

USTRUCT(BlueprintType)
struct FBoxInformation
{
	// �÷��̾ ��ġ�� �ڽ��� ������ ��� ����ü

	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	// ������ ��Ű���� �÷��̾��� ��� �ڽ�������Ʈ�� ������ ���� �̶� GC ������ �÷����� ������ �ޱ� ���ؼ� UPROPERTY �� ��ũ�θ� �����Ѵ�.
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKH_MULTISHOOTING_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class APlayerCharacter;

	ULagCompensationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	void ServerSideRewind(class APlayerCharacter* HitCharcter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

protected:
	virtual void BeginPlay() override;

	// �����͸� �����ϴ� �Լ�
	void SaveFramePackage(FFramePackage& Package);

	// ������ �Լ�
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

private:
	UPROPERTY()
	APlayerCharacter* Character;

	UPROPERTY()
	class AFirstPlayerController* Controller;

	// ����⿬�Ḯ��Ʈ�� BP������������ʴ´� ������ C++ �ڵ���
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 2.f;

public:	

		
};

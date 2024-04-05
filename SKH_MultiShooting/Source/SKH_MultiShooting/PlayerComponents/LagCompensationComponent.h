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

	UPROPERTY()
	APlayerCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	// �÷��̾ �ǰ����ߴ���, �ش��ǰ��� ��弦����
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	// �÷��̾ �ǰ����ߴ���, �ش��ǰ��� ��弦���� (���ǿ�)
	GENERATED_BODY()

	UPROPERTY()
	TMap<APlayerCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<APlayerCharacter*, uint32> BodyShots;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKH_MULTISHOOTING_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();

	friend class APlayerCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	// �Ϲ� ��Ʈ��ĵ�� �ǰ���
	FServerSideRewindResult ServerSideRewind(
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
		class AWeapon* DamageCauser
	);

	// ���ǿ� �ǰ���
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<APlayerCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<APlayerCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

protected:
	virtual void BeginPlay() override;

	// �����͸� �����ϴ� �Լ�
	void SaveFramePackage(FFramePackage& Package);

	// ������ �Լ�
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	FServerSideRewindResult ConfirmHit(const FFramePackage& Package,
		APlayerCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	void CacheBoxPositions(APlayerCharacter* HitCharacter, FFramePackage& OutFramePackage);

	void MoveBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package);

	void ResetHitBoxes(APlayerCharacter* HitCharacter, const FFramePackage& Package);

	void EnableCharacterMeshCollision(APlayerCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	void SaveFramePackage();

	FFramePackage GetFrameToCheck(APlayerCharacter* HitCharacter, float HitTime);

	// ���ǿ� �ǰ����� Ȯ��
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages, 
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations);

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

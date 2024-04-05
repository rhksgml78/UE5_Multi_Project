#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "LagCompensationComponent.generated.h"

/*
해당 컴포넌트는 서버캐릭터가 클라이언트 캐릭터의 정보를 저장하여 대조하고
지연에 관하여 되감기를 사용할 수 있도록 하기위한 컴포넌트

매프레임의 정보를 저장할 수 있어야한다 Frame History
저장할때 단순히 플레이어의 위치만 저장하기보다는 플레이어의 충돌과관련된
정보를 저장해야하며 해당 정보의 디테일은 설정에 따라 다르다.
디테일할 수록 성능에 부담이 되고 비용이 높다.

가장 저렴한것은 캡슐 = 정확도 떨어짐
중간단계는 박스단위 충돌체를 플레이어 관절마다 배치 = 어느정도 정확함
가방 비용이높은것은 메쉬단위 = 정확한 캐릭터형태충돌은 매우비싸다!
*/

USTRUCT(BlueprintType)
struct FBoxInformation
{
	// 플레이어에 배치된 박스의 정보를 담는 구조체
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
	// 프레임 패키지는 플레이어의 모든 박스컴포넌트의 정보를 저장 이때 GC 가비지 컬렉션의 보조를 받기 위해서 UPROPERTY 의 매크로를 선언한다.
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
	// 플레이어가 피격을했는지, 해당피격이 헤드샷인지
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	// 플레이어가 피격을했는지, 해당피격이 헤드샷인지 (샷건용)
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

	// 일반 히트스캔용 되감기
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

	// 샷건용 되감기
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

	// 데이터를 저장하는 함수
	void SaveFramePackage(FFramePackage& Package);

	// 보간용 함수
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

	// 샷건용 피격판정 확인
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages, 
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations);

private:
	UPROPERTY()
	APlayerCharacter* Character;

	UPROPERTY()
	class AFirstPlayerController* Controller;

	// 양방향연결리스트는 BP에서노출되지않는다 오로지 C++ 코드사용
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 2.f;

public:	

		
};

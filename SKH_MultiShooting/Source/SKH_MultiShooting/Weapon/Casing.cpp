#include "Casing.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);

	// ź�ǰ� ����Ǵ� ��
	EjectionImpulse = 5.f;

	// ź�ǰ� �Ҹ��� �ð�
	DestroyTimer = 3.f;

	// ����� �ѹ��� ��� �ϱ� ���� ����
	SoundPlayOnce = false;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	// ����ƽ�޽��� Hit�� ���ε�
	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * EjectionImpulse);
	


	// �������ڸ��� 5���� Ÿ�̸Ӹ� ����
	if (this)
	{
		// Ÿ�̸� ����
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;

		// ��������Ʈ�� Destroy �Լ� ���ε�
		TimerDel.BindLambda([this]()
			{
				this->Destroy();
			});

		// 5�� �Ŀ� Destroy �Լ� ����
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, DestroyTimer, false);
		
		// ���� ȸ�� �� ����
		FRotator RandomRotation;
		RandomRotation.Yaw = FMath::RandRange(0.f, 360.f);
		RandomRotation.Pitch = FMath::RandRange(0.f, 360.f);
		RandomRotation.Roll = FMath::RandRange(0.f, 360.f);
		// ������ ȸ�� �� ����
		SetActorRotation(RandomRotation);
	}

}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// �浹�� �ı� �� �浹 ���� ���

	if (DropSound && !SoundPlayOnce)
	{
		SoundPlayOnce = true;
		UGameplayStatics::PlaySoundAtLocation(this, DropSound, GetActorLocation());
	}

	//Destroy();
	
}


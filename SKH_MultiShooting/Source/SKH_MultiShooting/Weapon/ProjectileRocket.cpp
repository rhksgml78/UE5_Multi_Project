#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // ����� ��ġ�� ������Ʈ
				Damage, // �⺻ ������
				10.f, // �ּ� ������
				GetActorLocation(), // ������ġ
				200.f, // �ּ� ����
				500.f, // �ִ� ����
				1.f, // ������ Falloff
				UDamageType::StaticClass(), // ������Ÿ��Ŭ����
				TArray<AActor*>(), // ����ó��(Empty/����)
				this, // ������ Ŀ��(����)
				FiringController // �������� ��Ʈ�ѷ�
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

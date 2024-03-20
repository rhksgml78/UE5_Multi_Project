#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// ������ �ı��ɶ� �ֺ��� ���÷��� ���·� �������� �ش�

	// �߻�� ����ü�� �߻��� ������ ������(Instigator)�� �˼��ִ�.
	APawn* FiringPawn =	GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			// ��缱���� �������� �ش�
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // ������� ������Ʈ ����
				Damage, // �⺻ ������
				10.f, // �ּ� ������
				GetActorLocation(), //�߻���ġ
				200.f, // ���ιݰ�
				500.f, // �ܺιݰ�
				1.f, // ������������
				UDamageType::StaticClass(), // ������Ÿ��
				TArray<AActor*>(), // ����ó���� Ŭ����(����) ���ε� �ջ��� �Դ� �ɼ����� ����ó���� �����
				this, // �������� ����
				FiringController // ������ ��Ʈ�ѷ�
			);
		}
	}
	
	// �θ��� Onhit ������ �ı��ϸ鼭 ��ƼŬ�� ���带 �����Ų��.
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

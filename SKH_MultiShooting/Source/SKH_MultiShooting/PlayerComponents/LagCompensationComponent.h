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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SKH_MULTISHOOTING_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:	

		
};

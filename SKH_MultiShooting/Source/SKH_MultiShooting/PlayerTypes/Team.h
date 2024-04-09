#pragma once

// 플레이어의 팀을 설정하는 헤더
UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_RedTeam UMETA(DisplayName = "RedTeam"),
	ET_BlueTeam UMETA(DisplayName = "BlueTeam"),
	ET_NoTeam UMETA(DisplayName = "NoTeam"),
	ET_MAX UMETA(DisplayName = "DefaultMAX")
};
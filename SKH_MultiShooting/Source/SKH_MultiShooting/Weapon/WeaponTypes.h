#pragma once

#define TRACE_LENGTH 8000.f

//Custom Stencil value. 250 = Red. 251 = Green, 252 = White
#define CUSTOM_DEPTH_RED 250
#define CUSTOM_DEPTH_GREEN 251
#define CUSTOM_DEPTH_WHITE 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubmachinGun UMETA(DisplayName = "Submachin Gun"),
	EWT_ShotGun UMETA(DisplayName = "Shot Gun"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
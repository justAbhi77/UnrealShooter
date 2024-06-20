﻿#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubMachineGun UMETA(DisplayName = "SubMachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),

	EWT_Max UMETA(DisplayName = "DefaultMax")
};
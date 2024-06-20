// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Engine/DataTable.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* PickupSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsMiddle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsLeft;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsRight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsBottom;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* MuzzleFlash;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotDamage;
};

UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();
	
	virtual void Tick(float DeltaTime) override;
protected:
	void StopFalling();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	void FinishMovingSlide();

	/** move things that should move when firing gun */
	void UpdateSlideDisplacement();
	
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	int32 Ammo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	EWeaponType WeaponType; // Type of weapon

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	EAmmoType AmmoType; // ammo for weapon

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

	/** Datable for properties */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	int32 PreviousMaterialIndex;

	/** Texture for Crosshair */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsMiddle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsRight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsBottom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsTop;

	/** Interval between two consecutive bullet fire */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	float AutoFireRate;

	/** Particle system at muzzle of gun */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;

	/** Sound played when weapon fired */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	USoundCue* FireSound;

	/** Bone to hide for this weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataTable, meta=(AllowPrivateAccess = "true"))
	FName BoneToHide;

	/** Sliding value for displacement when pistol fires */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta=(AllowPrivateAccess = "true"))
	float SlideDisplacement;

	/** Sliding Curve when pistol fires */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta=(AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;

	/** Timer for updating Sliding of handle */
	FTimerHandle SlideTimer;

	/** Time for displacement */
	float SlideDisplacementTime;

	/** True when moving pistol slider */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta=(AllowPrivateAccess = "true"))
	bool bMovingSlide;

	/** value to move pistol slider by */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta=(AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;

	/** value to rotate pistol in hand*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pistol, meta=(AllowPrivateAccess = "true"))
	float MaxRecoilRotation;

	/** Rotation value when pistol fires */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pistol, meta=(AllowPrivateAccess = "true"))
	float RecoilRotation;

	/** is automatic weapon?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))	
	bool bAutomatic;

	/** Amount of damage caused by bullet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	float Damage;

	/** Damage done when head was hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta=(AllowPrivateAccess = "true"))
	float HeadShotDamage;
	
public:	
	void ThrowWeapon(); // impulse for falling

	FORCEINLINE int32 GetAmmo() const { return Ammo; }

	void DecrementAmmo(); // Fired a shot

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }

	void ReloadAmmo(int32 Amount);

	bool ClipIsFull() const;

	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }

	void StartSlideTimer();

	FORCEINLINE bool GetAutomatic() const { return bAutomatic; }

	FORCEINLINE float GetDamage() const { return Damage; }
	
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
};

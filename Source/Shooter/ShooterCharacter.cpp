// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Ammo.h"
#include "BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Weapon.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	MouseHipTurnRate(1.f),
	MouseAimingTurnRate(0.7f),
	MouseHipLookUpRate(1.f),
	MouseAimingLookupRate(0.7f),
	bAiming(false),
	CameraDefaultFOV(0.f),
	CameraZoomFov(25.f),
	CurrentCameraFOV(0.f),
	FOVZoomInterpSpeed(20.f),
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	bFireButtonPressed(false),
	bShouldFire(true),
	bShouldTraceForItems(false),
	// Camera Interp Location
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(450.f),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(45.f),
	BaseGroundFriction(2.f),
	CrouchingGroundFriction(100.f),
	bCanStopAimingWhileReloading(false),
	bAimingButtonPressed(false),
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.1f),
	EquipSoundResetTime(0.1f),
	HighlightedSlot(-1),
	bCanStopAimingWhileExchanging(false),
	Health(100.f),
	MaxHealth(100.f),
	StunChance(.25f),
	bDead(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// create component camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; //Follow distance
	CameraBoom->bUsePawnControlRotation = true; // Use the controllers rotation
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	//Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // no Local axes rotation

	// Controller only affects camera not mesh
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true; // Use Controller Yaw
	bUseControllerRotationRoll = false;

	//Mesh rotation with camera 
	UCharacterMovementComponent* CharMovComp = GetCharacterMovement();
	CharMovComp->bOrientRotationToMovement = false; 
	// the bool and rotation value seems to work in cpp but doesnt update the editor in BP side
	// Manually updating the value of RotationRate seems to work but have to update both pitch 
	// and yaw for it to work
	CharMovComp->RotationRate = FRotator(540.f,540.f,0.f); // Rate of rotation

	CharMovComp->JumpZVelocity = 600.f;
	CharMovComp->AirControl = 0.2f;

	// Interpolation components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpolationComp"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());
	
	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp2"));
	InterpComp2->SetupAttachment(GetFollowCamera());
	
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp3"));
	InterpComp3->SetupAttachment(GetFollowCamera());
	
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp4"));
	InterpComp4->SetupAttachment(GetFollowCamera());
	
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp5"));
	InterpComp5->SetupAttachment(GetFollowCamera());
	
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(Health - DamageAmount <= 0.f)
	{
		Health = 0.f;

		if(auto EnemyController = Cast<AEnemyController>(EventInstigator))
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CharacterDead"), true);
		}
		
		Die();
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera) {
		CameraDefaultFOV = FollowCamera->FieldOfView;
		CurrentCameraFOV = CameraDefaultFOV;
	}

	EquipWeapon(SpawnDefaultWeapon()); // Spawn and equip default weapon

	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);

	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	//create fInterpLocation for interp locations
	InitializeInterpLocations();
}

void AShooterCharacter::MoveForward(float Value)
{
	if(bDead) return;
	
	if ((Controller != nullptr) && (Value != 0.0f)) {
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0,Rotation.Yaw,0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) }; //Forward Axis

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if(bDead) return;
	
	if ((Controller != nullptr) && (Value != 0.0f)) {
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0,Rotation.Yaw,0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) }; //Right-Left Side Axis

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor;

	if (bAiming)
		TurnScaleFactor = MouseAimingTurnRate;
	else
		TurnScaleFactor = MouseHipTurnRate;

	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor;

	if (bAiming)
		LookUpScaleFactor = MouseAimingLookupRate;
	else
		LookUpScaleFactor = MouseHipLookUpRate;

	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;

	if (CombatState != ECombatState::ECS_Unoccupied)	return;

	if (WeaponHasAmmo())
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, "Fire from cpp"); //Print to screen in Editor

		PlayFireSound();

		SendBullet();

		PlayGunFireMontage();
		
		StartCrosshairBulletFire();

		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

		if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			// Move slider of weapon -> Generic
			EquippedWeapon->StartSlideTimer();
		}
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult) const
{
	FVector OutBeamLocation;
	FHitResult CrosshairHitResult;

	// bool bCrosshairHit = 
	if (TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation)) {
		OutBeamLocation = CrosshairHitResult.Location; 
	}
	else {
		// OutBeamLocation is the hit point in world
	}

	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ OutBeamLocation + StartToEnd * 1.25f };

	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd,
		ECC_Visibility);
	if (!OutHitResult.bBlockingHit) {
		OutHitResult.Location = OutBeamLocation;
		return false;
	}
	return true;
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::AimingButtonPressed()
{
	if(bDead) return;
	
	bAimingButtonPressed = true;
	if((CombatState != ECombatState::ECS_Reloading) && (CombatState != ECombatState::ECS_Equipping) &&
		(CombatState != ECombatState::ECS_Stunned))
		Aim();
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if(!bCrouching)
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	// check to see if Ammo Maps has AmmoType
	if(AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] }; // Get amount of Ammo in map for type

		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount; // set count to appropriate type
	}

	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// if empty barrel
		if(EquippedWeapon->GetAmmo() == 0)
			ReloadWeapon();
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	const FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
	InterpLocations.Add(WeaponLocation);
	
	const FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);
	const FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);
	const FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);
	const FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);
	const FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);
	const FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);
}

void AShooterCharacter::FKeyPressed()
{
	if(bDead) return;
	
	auto const SlotIndex = EquippedWeapon->GetSlotIndex(); 
	if(SlotIndex == 0) return;

	ExchangeInventoryItems(SlotIndex, 0);
}

void AShooterCharacter::OneKeyPressed()
{	
	if(bDead) return;
	
	auto const SlotIndex = EquippedWeapon->GetSlotIndex(); 
	if(SlotIndex == 1) return;

	ExchangeInventoryItems(SlotIndex, 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if(bDead) return;
	
	auto const SlotIndex = EquippedWeapon->GetSlotIndex(); 
	if(SlotIndex == 2) return;

	ExchangeInventoryItems(SlotIndex, 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if(bDead) return;
	
	auto const SlotIndex = EquippedWeapon->GetSlotIndex(); 
	if(SlotIndex == 3) return;

	ExchangeInventoryItems(SlotIndex, 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if(bDead) return;
	
	auto const SlotIndex = EquippedWeapon->GetSlotIndex(); 
	if(SlotIndex == 4) return;

	ExchangeInventoryItems(SlotIndex, 4);
}

void AShooterCharacter::FiverKeyPressed()
{
	if(bDead) return;
	
	auto const SlotIndex = EquippedWeapon->GetSlotIndex(); 
	if(SlotIndex == 5) return;

	ExchangeInventoryItems(SlotIndex, 5);
}

void AShooterCharacter::ExchangeInventoryItems(const int32 CurrentItemIndex, const int32 NewItemIndex)
{
	const bool bCanExchangeItems = (CurrentItemIndex != NewItemIndex) && (NewItemIndex < Inventory.Num()) &&
		((CombatState == ECombatState::ECS_Unoccupied) || (CombatState == ECombatState::ECS_Equipping));
	
	if(!bCanExchangeItems) return;

	if(bCanStopAimingWhileExchanging && bAiming)
		StopAiming();

	const auto OldEquippedWeapon = EquippedWeapon;
	const auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);

	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);

	CombatState = ECombatState::ECS_Equipping;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Equip"));
	}

	NewWeapon->PlayEquipSound(true);
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for(int32 i = 0; i<Inventory.Num();i++)
	{
		if(Inventory[i] == nullptr)
		{
			return i;
		} 
	}

	if(Inventory.Num() < Inventory_Capacity)
	{
		return Inventory.Num();
	}

	return -1; // full inventory
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot{ GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;

	const FVector Start{ GetActorLocation() }, End{ Start + FVector(0.f,0.f,-400.f)};

	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End,ECC_Visibility, QueryParams);

	// auto HitSurface = HitResult.PhysMaterial->SurfaceType;
	// if(HitSurface == EPS_Grass)
		// UE_LOG(LogTemp, Warning, TEXT("Hit grass"));
	// UE log has errors in rider (known bug)
	// if(HitResult.GetActor())
		// UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitResult.GetActor()->GetName());

	EPhysicalSurface DetermineSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

	return DetermineSurfaceType;
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if(bAimingButtonPressed)
		Aim();
}

void AShooterCharacter::Die()
{
	bDead = true;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

void AShooterCharacter::Stun()
{
	if(Health <= 0.f) return;
	
	CombatState = ECombatState::ECS_Stunned;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for(int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if(InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}
	
	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if(Amount < -1 || Amount > 1) return;

	if(InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{	
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

void AShooterCharacter::AimingButtonReleased()
{
	if(bDead) return;
	
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	const FVector2d WalkSpeedRange{ 0.f,600.f };
	const FVector2D VelocityMultiplierRange{ 0.f,1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,
		VelocityMultiplierRange, Velocity.Size());

	if (GetCharacterMovement()->IsFalling())
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	else
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

	if (bAiming)
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	else
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);

	if (bFiringBullet)
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	else
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + 
		CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, 
		&AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	if(bDead) return;
	
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	if(bDead) return;
	
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if(EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this,
		&AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
	
}

void AShooterCharacter::AutoFireReset()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	
	if(EquippedWeapon == nullptr) return;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
			FireWeapon();
	}
	else
	{
		// Reload Weapon
		ReloadWeapon();
	}		
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) const
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) { // Screen Size
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Set Location for crosshair on screen
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	FVector CrosshairWorldPosition, CrosshairWorldDirection;

	// Screen to world Matrix
	// bool bScreenToWorld = 
	if (UGameplayStatics::DeprojectScreenToWorld(PlayerController,
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection)) {
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50000.f };
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

		if (OutHitResult.bBlockingHit) {
			OutHitLocation = OutHitResult.Location;
			FString MyString{"Hello Debug"}; 
			if(OutHitResult.GetActor())
				MyString = OutHitResult.GetActor()->GetName();
			if(OutHitResult.GetComponent())
				MyString += " " + OutHitResult.GetComponent()->GetName();
			if(GEngine)
				GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Blue, MyString);
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems) {
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit) {
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());

			if(Cast<AWeapon>(TraceHitItem))
			{
				if(HighlightedSlot == -1)
				{
					HighlightInventorySlot(); // highlight a slot for the first time
				}
			}
			else
			{
				if(HighlightedSlot != -1)
				{
					UnHighlightInventorySlot();
				}
			}

			if(TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}
			
			if (TraceHitItem && TraceHitItem->GetPickupWidget()) {
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if(Inventory.Num() >= Inventory_Capacity)
				{
					// full inventory
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					// inventory has space
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}
			// already hit an item
			if(TraceHitItemLastFrame)
			{
				if(TraceHitItem != TraceHitItemLastFrame) // either null or different AItem hit this frame
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if(TraceHitItemLastFrame)
	{
		// no longer overlapping items
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon() const
{
	if(DefaultWeaponClass) // blueprint reference
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass); // instantiate
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if (WeaponToEquip)
	{		
		if(const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket")))
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh()); //attach spawned weapon to socket
		}

		if(EquippedWeapon == nullptr)
		{
			// -1 when no equipped weapon
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if(!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(),WeaponToEquip->GetSlotIndex());
		}
		
		EquippedWeapon = WeaponToEquip; // currently equipped weapon is the spawned weapon
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon() const
{
	if(EquippedWeapon)
	{
		const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld,true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if(bDead) return;
	
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	
	if(TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AShooterCharacter::SelectButtonReleased()
{
	if(bDead) return;
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if(const int32 EquippedWeaponSlot = EquippedWeapon->GetSlotIndex(); (Inventory.Num() - 1) >= EquippedWeaponSlot)
	{
		Inventory[EquippedWeaponSlot] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeaponSlot); 
	}
	
	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo() const
{
	if(EquippedWeapon == nullptr)	return false;
	
	return (EquippedWeapon->GetAmmo() > 0);
}

void AShooterCharacter::PlayFireSound() const
{
	if (USoundCue* SoundCue = EquippedWeapon->GetFireSound()) {
		UGameplayStatics::PlaySound2D(this,SoundCue);
	}
}

void AShooterCharacter::SendBullet()
{
	// this function cant be const because of usage of this keyword
	if (const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket")) {
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (UParticleSystem* EmitterTemplate = EquippedWeapon->GetMuzzleFlash()) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterTemplate, SocketTransform);
		}

		FHitResult BeamHitResult;

		// bool bBeamEnd = 
		if (GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult)) {
			
			// does the currently hit actor implement the bullet hit interface (is an enemy)
			if(AActor* Actor = BeamHitResult.GetActor())
			{
				if(IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(Actor))
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}
				if(AEnemy* HitEnemy = Cast<AEnemy>(Actor))
				{
					int32 Damage{};
					bool bHeadShot{};
					
					if(BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						// Headshot
						Damage = EquippedWeapon->GetHeadShotDamage();
						bHeadShot = true;
					}
					else
					{
						// Bodyshot
						Damage = EquippedWeapon->GetDamage();
						bHeadShot = false;
					}
						
					UGameplayStatics::ApplyDamage(Actor, Damage,
						GetController(), this, UDamageType::StaticClass());
					
					HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location,bHeadShot);
				}
				else
				{
					if (ImpactParticles) {
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles,
							BeamHitResult.Location);
					}

					if (BeamParticles) {
						UParticleSystemComponent* Beam =
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
						Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
					}
				}
			}
		}
	}
	
}

void AShooterCharacter::PlayGunFireMontage() const
{	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage) {
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	if(bDead) return;
	
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState!= ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon == nullptr) return;

	// do we have ammo of the correct type

	if(CarryingAmmo() && !EquippedWeapon->ClipIsFull()) // carryingAmmo()
	{
		// I like that if we are aiming and reloading we dont want to stop aiming to reload
		// we just reload without stopping the aiming animation
		if(bCanStopAimingWhileReloading && bAiming)
			StopAiming();
		
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}	
}

void AShooterCharacter::FinishReloading()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied; // update combat state

	if(bAimingButtonPressed)
		Aim();

	if(EquippedWeapon == nullptr) return;

	// Update Ammo Map/Dictionary
	if(const auto AmmoType{ EquippedWeapon->GetAmmoType() }; AmmoMap.Contains(AmmoType))
	{
		// amount of ammo on player
		int32 CarriedAmmo = AmmoMap[AmmoType];

		// space left in magazine of weapon

		if(const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo(); MagEmptySpace > CarriedAmmo)
		{
			// reload mag with all ammo we have
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
		}
		else
		{
			// reload mag (fill)
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
		}		
		AmmoMap.Add(AmmoType, CarriedAmmo);
	}
}

void AShooterCharacter::FinishEquipping()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	
	if(bCanStopAimingWhileExchanging && bAimingButtonPressed)
		Aim();
}

bool AShooterCharacter::CarryingAmmo()
{
	if(EquippedWeapon == nullptr) return false;

	const auto AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		return (AmmoMap[AmmoType] > 0);
	}
	return false;
}

void AShooterCharacter::CrouchButtonPressed()
{
	if(bDead) return;
	if(HasAuthority())
	{
		CrouchButtonPressed_MultiCast();
	}
	else
	{
		CrouchButtonPressed_Server();
	}
}

void AShooterCharacter::CrouchButtonPressed_Server_Implementation()
{
	CrouchButtonPressed_MultiCast();
}

void AShooterCharacter::CrouchButtonPressed_MultiCast_Implementation()
{
	EventCrouchButtonPressed();
}

void AShooterCharacter::EventCrouchButtonPressed()
{	
	if(!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	
	if(bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AShooterCharacter::Jump()
{
	if(bDead) return;
	
	if(bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
		Super::Jump();
}

void AShooterCharacter::StopJumping()
{
	if(bDead) return;
	
	Super::StopJumping();
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight;
	if(bCrouching)
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	else
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;

	const float InterpHalfHeight{ FMath::FInterpTo(
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHalfHeight,
		DeltaTime,
		20.f) };

	// -ve value if crouching , +ve if standing
	const float DeltaCapsuleHalfHeight{ InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };

	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };

	GetMesh()->AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAiming) {
		CurrentCameraFOV = FMath::FInterpTo(CurrentCameraFOV, 
			CameraZoomFov,DeltaTime, FOVZoomInterpSpeed);

		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else {
		CurrentCameraFOV = FMath::FInterpTo(CurrentCameraFOV,
			CameraDefaultFOV, DeltaTime, FOVZoomInterpSpeed);

		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
	FollowCamera->SetFieldOfView(CurrentCameraFOV);

	CalculateCrosshairSpread(DeltaTime);
	
	TraceForItems();

	// Change Capsule collision when crouching
	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed,this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	
	PlayerInputComponent->BindAction("DropWeapon/Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("DropWeapon/Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton",IE_Pressed,this, &AShooterCharacter::ReloadButtonPressed);
	
	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this, &AShooterCharacter::CrouchButtonPressed);
	
	PlayerInputComponent->BindAction("FKey",IE_Pressed,this, &AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key",IE_Pressed,this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key",IE_Pressed,this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key",IE_Pressed,this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key",IE_Pressed,this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key",IE_Pressed,this, &AShooterCharacter::FiverKeyPressed);
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemsCount + Amount <= 0) {
		OverlappedItemsCount = 0;
		bShouldTraceForItems = false;
	}
	else {
		OverlappedItemsCount += Amount;
		bShouldTraceForItems = true;
	}
}


/* No longer needed, updated in AItem
FVector AShooterCharacter::GetCameraInterpLocation() const
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };

	// Desired = Camera Location + <some distance>
	return CameraWorldLocation + CameraForward * CameraInterpDistance +
		FVector(0.f,0.f,CameraInterpElevation);
}
*/

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();
	
	if(const auto Weapon = Cast<AWeapon>(Item))
	{
		if(Inventory.Num() < Inventory_Capacity)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else
		{
			// Inventory full! swap weapon
			SwapWeapon(Weapon);
		}
	}

	if(const auto Ammo = Cast<AAmmo>(Item))
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if(Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}


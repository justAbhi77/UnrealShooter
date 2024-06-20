// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppMemberFunctionMayBeStatic
#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"

// Sets default values
AItem::AItem():
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	ItemInterpStartLocation(FVector(0.f)),
	CameraTargetLocation(FVector(0.f)),
	bInterping(false),
	ZCurveTime(0.7f),
	ItemInterpX(0.f),
	ItemInterpY(0.f),
	InterpInitialYawOffset(0.f),
	ItemType(EItemType::EIT_Max),
	InterpLocIndex(0),
	MaterialIndex(0),
	bCanChangeCustomDepth(true),
	PulseCurveTime(5.f),
	GlowAmount(150.f),
	FresnelExponent(3.f),
	FresnelReflectFraction(4.f),
	SlotIndex(0),
	bCharacterInventoryFull(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, "Hello World! Item created from cpp");

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility,
		ECR_Block);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickUpWidget->SetupAttachment(ItemMesh);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(ItemMesh);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	if(PickUpWidget)
		PickUpWidget->SetVisibility(false);

	// Setup Overlap Events
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	SetActiveStars(); // Set stars based on rarity

	SetItemProperties(ItemState);

	InitializeCustomDepth(); // dont start with highlight

	StartPulseTimer();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) {
		if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor)) {
			ShooterCharacter->IncrementOverlappedItemCount(1);
			CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Block);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) {
		if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor)) {
			CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECollisionResponse::ECR_Ignore);
			ShooterCharacter->IncrementOverlappedItemCount(-1);
			ShooterCharacter->UnHighlightInventorySlot();
		}
	}
}

void AItem::SetActiveStars()
{
	// the zero element is not used so that index corresponds to stars
	for (int32 i =0;i<=5;i++)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
		
	case EItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
		
	case EItemRarity::EIR_UnCommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		break;
		
	case EItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
		
	case EItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;

	default:
		break;
	}
}

void AItem::SetItemProperties(EItemState State) const
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		// Mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Area sphere properties 
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Collision box
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_EquipInterping:
		PickUpWidget->SetVisibility(false);

		// Mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Area sphere properties 
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Collision box
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp:
		PickUpWidget->SetVisibility(false);

		// Mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Area sphere properties 
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Collision box
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Equipped:
		// Widget
		PickUpWidget->SetVisibility(false);
		
		// Mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Area sphere properties 
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Collision box
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic,ECollisionResponse::ECR_Block);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic,ECollisionResponse::ECR_Block);

		// Area sphere properties 
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Collision box
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Max:
		break;
	default: ;
	}
}

void AItem::FinishInterping()
{
	bInterping = false;
	if(Character)
	{
		// Subtract 1 to item count for this location
		Character->IncrementInterpLocItemCount(InterpLocIndex, -1);
		Character->GetPickupItem(this);
		Character->UnHighlightInventorySlot();
	}
	SetActorScale3D(FVector(1.f)); // After animation reset scale to default

	DisableGlowMaterial();
	bCanChangeCustomDepth = true;
	DisableCustomDepth();
}

void AItem::ItemInterp(const float DeltaTime)
{
	if(!bInterping)
		return;
	if(Character && ItemZCurve)
	{
		// elapsed time sine animation started
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);

		// value from curve at time
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		FVector ItemLocation = ItemInterpStartLocation;
		// final location in front of camera
		const FVector CameraInterpLocation{ GetInterpLocation() };
		// Vector from item to cameraInterp Location
		const FVector ItemToCamera { FVector(0.f,0.f,(CameraInterpLocation-ItemLocation).Z) };
		const float DeltaZ = ItemToCamera.Size(); // Scale factor to * with Curve Value

		const FVector CurrentLocation{ GetActorLocation() };
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y,DeltaTime , 30.f);

		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;
		
		ItemLocation.Z += CurveValue * DeltaZ; // adding curve value to z
		SetActorLocation(ItemLocation,true,nullptr,ETeleportType::TeleportPhysics);

		// Make Weapon a static rotation not follow character when animating
		const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation() };
		const FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f };

		SetActorRotation(ItemRotation,ETeleportType::TeleportPhysics);

		if(ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
	}
}

FVector AItem::GetInterpLocation() const
{
	if(Character == nullptr) return FVector(0.f);

	switch (ItemType)
	{
		case EItemType::EIT_Ammo:
			return Character->GetInterpLocation(InterpLocIndex).SceneComponent->GetComponentLocation();
		case EItemType::EIT_Weapon:
			return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
		case EItemType::EIT_Max:
			return FVector(0.f);
		default: 
			return FVector(0.f);
	}
}

void AItem::PlayPickupSound(bool bForcePlaySound) const
{
	if(Character)
	{
		if(bForcePlaySound)
		{
			if(PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
		else if(Character->ShouldPlayPickupSound())
		{
			Character->StartPickupSoundTimer();
			if(PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
	}
}

void AItem::EnableCustomDepth()
{
	if(bCanChangeCustomDepth)
		ItemMesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	if(bCanChangeCustomDepth)
		ItemMesh->SetRenderCustomDepth(false);
}

void AItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	// read datatable

	// path of datatable in project
	const FString RarityTablePath(TEXT("/Script/Engine.DataTable'/Game/_Game/DataTables/DT_ItemRarity.DT_ItemRarity'"));

	if(const UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath)))
	{
		const FItemRarityTable* RarityRow = nullptr;
		switch (ItemRarity)
		{
			case EItemRarity::EIR_Damaged:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
				break;
			case EItemRarity::EIR_Common:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
				break;
			case EItemRarity::EIR_UnCommon:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("UnCommon"), TEXT(""));
				break;
			case EItemRarity::EIR_Rare:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
				break;
			case EItemRarity::EIR_Legendary:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
				break;
			case EItemRarity::EIR_Max:
				break;
			
			default: ;
		}
		if(RarityRow)
		{
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStars = RarityRow->NumberOfStars;
			IconBackground = RarityRow->IconBackground;
			if(const auto Mesh = GetItemMesh())
			{
				Mesh->SetCustomDepthStencilValue(RarityRow->CustomDepthStencilVal);
			}
		}
	}
	
	if(MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelBaseColor"),GlowColor);
		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
		EnableGlowMaterial();
	}	
}

void AItem::EnableGlowMaterial() const
{
	if(DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendValue"), 0);
	}
}

void AItem::UpdatePulse() const
{
	float ElapsedTime;
	FVector CurveValue{};
	switch(ItemState)
	{
		case EItemState::EIS_Pickup:
			if(PulseCurve)
			{
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
				CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
			}
			break;
		case EItemState::EIS_EquipInterping:
			if(InterpPulseCurve)
			{				
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
				CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
			}
			break;
		default: ;
	}
	if(DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelBaseReflection"), CurveValue.Z * FresnelReflectFraction);
	}
}

void AItem::DisableGlowMaterial() const
{
	if(DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendValue"), 1);
	}
}

void AItem::PlayEquipSound(bool bForcePlaySound) const
{
	if(Character)
	{
		if(bForcePlaySound)
		{
			if(EquippedSound)
			{
				UGameplayStatics::PlaySound2D(this, EquippedSound);
			}
		}
		else if(Character->ShouldPlayEquipSound())
		{
			Character->StartEquipSoundTimer();
			if(EquippedSound)
			{
				UGameplayStatics::PlaySound2D(this, EquippedSound);
			}
		}
	}	
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime); // Item Equip Animation

	UpdatePulse(); // set pulsing values in dynamic material 
}

void AItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if(ItemState== EItemState::EIS_Pickup)
	{
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}	
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound)
{
	Character = Char;

	// Get array index with the least number item count
	InterpLocIndex = Character->GetInterpLocationIndex();
	
	// Add 1 to item count for this location
	Character->IncrementInterpLocItemCount(InterpLocIndex, 1);

	PlayPickupSound(bForcePlaySound);

	ItemInterpStartLocation = GetActorLocation(); // initial location of item

	bInterping = true;

	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().ClearTimer(PulseTimer);

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);

	const float CameraRotationYaw { static_cast<float>(Character->GetFollowCamera()->GetComponentRotation().Yaw) };
	const float ItemRotationYaw { static_cast<float>(GetActorRotation().Yaw) };

	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;

	bCanChangeCustomDepth = false;
}


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_UnCommon UMETA(DisplayName = "UnCommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),
	EIR_Max UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickepUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),	
	EIS_Max UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemType: uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),

	EIT_Max UMETA(DisplayName = "DefaultMax")
};

USTRUCT(BlueprintType)	
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencilVal;
};

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,
		const FHitResult& SweepResult); // Begin Overlap with Any Actor 

	// End overlap with any Actor 
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SetActiveStars(); //Set active stars array of bool based on rarity

	virtual void SetItemProperties(EItemState State) const; // Set various variables based on state in world

	void FinishInterping(); // called after animation

	void ItemInterp(float DeltaTime); // Handling animation for equipping item

	/** Get interp location based on item type */
	FVector GetInterpLocation() const;

	void PlayPickupSound(bool bForcePlaySound = false) const;
	
	virtual void InitializeCustomDepth();

	virtual void OnConstruction(const FTransform& Transform) override;

	void EnableGlowMaterial() const;

	void UpdatePulse() const;
	
	void ResetPulseTimer();

	void StartPulseTimer();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh; // 3d Mesh for item

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox; // Line Trace For Hub Widgets

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickUpWidget; //Popup widget for line Trace

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere; // Area for Item Activation

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount; // Ammo Count

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity; // Rarity Stars in gui Widget

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))	
	TArray<bool> ActiveStars;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState; // State of item in World ( on ground, picking, picked, etc. )

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve; // curve for z velocity

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation; // Start Location of weapon for animation

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation; // Final Destination for animation

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bInterping; // Is the item Interping?

	FTimerHandle ItemInterpTimer; // Animation Handler

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* Character; // Character

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime; // Duration of animation timer

	float ItemInterpX;
	float ItemInterpY;

	float InterpInitialYawOffset; // initial rotation when picking up item

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve; // Curve for scaling item

	/** Sound Played when item is picked up*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;
	
	/** Sound Played when item is Equipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* EquippedSound;

	/** Enum for type of item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	/** index of the location this item is interping to */ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocIndex;
	
	/** Index for Material */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	/** Dynamic Instance changed at runtime */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	/** Material to make instance of */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;

	/** Curve to change Material Parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveVector* PulseCurve;

	/** Curve to change Material Parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpPulseCurve;
	
	FTimerHandle PulseTimer;

	/** Time for pulsing to change Material Parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;
	
	UPROPERTY(VisibleAnywhere, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;
	
	UPROPERTY(VisibleAnywhere, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;

	/** Item image for inventory widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;
	
	/** Ammo image for inventory widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIcon;

	/** index for inventory */ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/** whether the player inventory is full */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

	/** Item Rarity Datatable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= DataTable, meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDataTable;

	/** Color for glow material */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta=(AllowPrivateAccess = "true"))
	FLinearColor GlowColor;
	
	/** First Color for pickup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta=(AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	/** Second Color for pickup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta=(AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	/** Stars for pickup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta=(AllowPrivateAccess = "true"))
	int32 NumberOfStars;
	
	/** Background for pickup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta=(AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;
	
public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickUpWidget; }

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }

	FORCEINLINE EItemState GetItemState() const { return ItemState; }

	void SetItemState(EItemState State);

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }

	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE void SetPickupSound(USoundCue* Sound) { PickupSound = Sound; }
	
	FORCEINLINE USoundCue* GetEquippedSound() const { return EquippedSound; }
	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquippedSound = Sound; }
	
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }

	void StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound = false); // Called from the shooter class
		
	// called by shooter character get pickup item
	void PlayEquipSound(bool bForcePlaySound = false) const;
	
	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	
	void DisableGlowMaterial() const;

	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(const int32 index) { SlotIndex = index; }
	FORCEINLINE void SetCharacter(AShooterCharacter* Char) { Character = Char; }
	FORCEINLINE void SetCharacterInventoryFull(const bool bFull) { bCharacterInventoryFull = bFull; }
	FORCEINLINE void SetItemName(FString Name) { ItemName = Name;}

	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }
	FORCEINLINE void SetAmmoIcon(UTexture2D* Icon) { AmmoIcon = Icon; }

	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) { MaterialInstance = Instance; }
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }
	
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance) { DynamicMaterialInstance = Instance; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }

	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }
	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }	
	FORCEINLINE void SetMaterialIndex(int32 Index) { MaterialIndex = Index; }
};

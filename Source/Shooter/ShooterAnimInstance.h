// ReSharper disable CppUEBlueprintCallableFunctionUnused

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_Max UMETA(DislayName = "DefaultMax")
};

UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	UShooterAnimInstance();
	
	UFUNCTION(BlueprintCallable)
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;
	
protected:

	/** Handle Turning is place variables */
	void TurnInPlace();

	/** Lean calculation*/
	void Lean(float DeltaTime);
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement , meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw; // Strafing Yaw Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw; // previous OffsetYaw before stopping

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/** Turn in place Yaw this frame only used when standing still*/
	float TIPCharacterYaw;

	/** Turn in place  Yaw Last frame only used when standing still*/
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	// Rotation curve value this frame
	float RotationCurve;

	// rotation curve value last frame
	float RotationCurveLastFrame;

	/** Pitch for aim Rotation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	/** is Reloading? to prevent aim offset while reloading*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	/** aim offset to use*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	/** Rotation Last frame */
	FRotator CharacterRotation;
	
	/** Rotation Last frame */
	FRotator CharacterRotationLastFrame;

	/** Yaw delta used for leaning in the running blend space*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category=Lean, meta = (AllowPrivateAccess = true))
	float YawDelta;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category=Crouching, meta = (AllowPrivateAccess = true))
	bool bCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category=Crouching, meta = (AllowPrivateAccess = true))
	bool bEquipping;

	/** Changed at runtime for recoil based on crouching */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category=Combat, meta = (AllowPrivateAccess = true))
	float RecoilWeight;

	/** Whether turning in place */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category=Combat, meta = (AllowPrivateAccess = true))
	bool bTurningInPlace;

	/** Currently equipped weapon type */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category=Combat, meta = (AllowPrivateAccess = true))
	EWeaponType EquippedWeaponType;

	/** Dont Ik When Reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category=Combat, meta = (AllowPrivateAccess = true))
	bool bShouldUseFABRIK;
};

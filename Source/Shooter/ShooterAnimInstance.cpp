// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	ShooterCharacter(nullptr),
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	TIPCharacterYaw(0.f),
	TIPCharacterYawLastFrame(0.f),
	RootYawOffset(0.f),
	RotationCurve(0),
	RotationCurveLastFrame(0),
	Pitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	YawDelta(0.f),
	bCrouching(false),
	bEquipping(false),
	RecoilWeight(1.f),
	bTurningInPlace(false),
	EquippedWeaponType(EWeaponType::EWT_Max),
	bShouldUseFABRIK(false)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr) {
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter) {

		bCrouching = ShooterCharacter->GetCrouching();

		bReloading = (ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading);
		
		bEquipping = (ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping);

		bShouldUseFABRIK = (ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied)
			|| (ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress);
		
		//get speed from Velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		const UCharacterMovementComponent* MovementComp = ShooterCharacter->GetCharacterMovement();

		//Is in Air
		bIsInAir = MovementComp->IsFalling();

		//Is Accelerating
		if (MovementComp->GetCurrentAcceleration().Size() > 0.f)
			bIsAccelerating = true;
		else
			bIsAccelerating = false;

		const FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();

		const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if (bIsAccelerating)
			LastMovementOffsetYaw = MovementOffsetYaw;

		/* FString RotationMsg = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, RotationMsg); //Print to screen in Editor
		*/

		bAiming = ShooterCharacter->GetAiming();

		if(bReloading)
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else if(bIsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if(ShooterCharacter->GetAiming())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;			
		}

		// Get equipped weapon
		if(ShooterCharacter->GetEquippedWeapon())
		{
			EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
		}
	}
	TurnInPlace();

	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if(ShooterCharacter == nullptr) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
	
	if(Speed >0 || bIsInAir)
	{
		// dont turn in place character moving
		RootYawOffset = 0.f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

		// root yaw offset updated and clamped to [-/+ 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		// 1 if turning 0 otherwise
		if(const float Turning{ GetCurveValue(TEXT("Turning")) }; Turning > 0)
		{
			bTurningInPlace = true;
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("RotationCurve"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			// RootYawOffset > 0 => turning left, right otherwise
			(RootYawOffset > 0) ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if(ABSRootYawOffset > 90)
			{
				const float YawExcess{ ABSRootYawOffset - 90 };
				(RootYawOffset > 0) ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
		{
			bTurningInPlace = false;
		}
	}
	
	if(bTurningInPlace)
	{
		if(bReloading || bEquipping)
			RecoilWeight = 1.f;
		else
			RecoilWeight = 0.f;
	}
	else
	{
		// Idling -> not turning in place
		if(bCrouching)
		{
			if(bReloading || bEquipping)
				RecoilWeight = 1.f;
			else
				RecoilWeight = 0.1f;
		}
		else
		{
			if(bAiming || bReloading || bEquipping)
				RecoilWeight = 1.f;
			else
				RecoilWeight = 0.5f;
		}
	}
}

void UShooterAnimInstance::Lean(const float DeltaTime)
{
	if(ShooterCharacter == nullptr) return;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };
	
	const float Target = Delta.Yaw / DeltaTime; // Braced initialization not working used = 

	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
}

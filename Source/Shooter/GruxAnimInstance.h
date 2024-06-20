// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GruxAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	// ReSharper disable once CppUEBlueprintCallableFunctionUnused
	void UpdateAnimationProperties(float DeltaTime);
	
private:
	/** Movement speed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class AEnemy* Enemy;
};

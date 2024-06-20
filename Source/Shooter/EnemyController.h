// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()
public:
	AEnemyController();

	virtual void OnPossess(APawn* InPawn) override;

private:
	/** Blackboard for the AI */
	UPROPERTY(BlueprintReadWrite, Category = "Ai Behavior", meta=(AllowPrivateAccess = "true"))
	UBlackboardComponent* BlackboardComponent;

	/** behavior for the AI */
	UPROPERTY(BlueprintReadWrite, Category = "Ai Behavior", meta=(AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:
	// ReSharper disable once CppHidingFunction
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
};

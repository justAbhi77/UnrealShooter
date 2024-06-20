// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletHitInterface.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();

	void ShowHealthBar_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	void Die();

	void PlayHitMontage(FName Section, float PlayRate = 1.f);

	void ResetHitReactTimer();

	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumber);

	void UpdateHitNumbers();

	UFUNCTION()
	void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);
	
	void SetInAttackRange(bool Value);

	UFUNCTION()
	void CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate = 1.f);
	
	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName() const;
	void DoDamage(class AShooterCharacter* Victim);
	void SpawnBlood(const AShooterCharacter* Victim, FName SocketName) const;

	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,
		const FHitResult& SweepResult);

	// UFUNCTION()
	// void OnLeftWeaponEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();

	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();

	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();

	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();

	/** Attempt to Stun character */
	static void StunCharacter(AShooterCharacter* Victim);

	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	UFUNCTION()
	void DestroyEnemy();

private:

	/** Particles to spawn when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/** Sound to Play when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	/** Current health of enemy*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float MaxHealth;

	/** Name of the Head Bone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	FString HeadBone;

	/** Time to display HealthBar for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;

	FTimerHandle HealthBarTimer;

	/** Montage for hit and death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	FTimerHandle HitReactTimer;

	bool bCanHitReact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float HitReactTimeMin;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float HitReactTimeMax;

	/** Map to store widgets */
	UPROPERTY(VisibleAnywhere, Category = Combat, meta=(AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitNumbers;

	/** Time before we remove the widget from screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float HitNumberDestroyTime;

	/** AI for enemy */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta=(AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	/** Point for the enemy to move to with a widget to move this in editor */
	UPROPERTY(EditAnywhere, Category= " behavior Tree", meta=(AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;
	
	/** Point 2 for the enemy to move to with a widget to move this in editor */
	UPROPERTY(EditAnywhere, Category= " behavior Tree", meta=(AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObject
	class AEnemyController* EnemyController;

	/** Agro sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Combat, meta=(AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;

	/** did the enemy get hit by something to stop it */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= Combat, meta=(AllowPrivateAccess = "true"))
	bool bStunned;

	/** Chance of being Stunned. Range 0 to 1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat,meta=(AllowPrivateAccess = "true"))
	float StunChance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= Combat, meta=(AllowPrivateAccess = "true"))
	bool bInAttackRange;
	
	/** Agro sphere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Combat, meta=(AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;
	
	/** Montage for Attacking Player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	/** Four of the montages for attacking Player */
	FName AttackLFast;
	/** Four of the montages for attacking Player */
	FName AttackRFast;
	/** Four of the montages for attacking Player */
	FName AttackL;
	/** Four of the montages for attacking Player */
	FName AttackR;

	/** Collision for left Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWeaponCollision;
	
	/** Collision for right Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess = "true"))
	UBoxComponent* RightWeaponCollision;

	/** Damage done by enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess = "true"))
	float BaseDamage;

	/** Socket for Blood Particle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess = "true"))
	FName LeftWeaponSocket;
	
	/** Socket for Blood Particle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess = "true"))
	FName RightWeaponSocket;

	UPROPERTY(VisibleAnywhere, Category=Combat, meta=(AllowPrivateAccess = "true"))
	bool bCanAttack;

	FTimerHandle AttackWaitTimer;
	
	UPROPERTY(EditAnywhere, Category=Combat, meta=(AllowPrivateAccess = "true"))
	float AttackWaitTime;

	/** Death Animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	bool bDying;

	FTimerHandle DeathTimer;

	/** Time after Death Animation to destroy Actor*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat, meta=(AllowPrivateAccess = "true"))
	float DeathTime; 

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(int32 Damage, FVector HitLocation, bool bHeadShot);

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	
};
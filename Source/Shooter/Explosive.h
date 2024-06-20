// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletHitInterface.h"
#include "GameFramework/Actor.h"
#include "Explosive.generated.h"

UCLASS()
class SHOOTER_API AExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/** Particles for explosion when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UParticleSystem* ExpoldeParticles;

	/** Sound to Play when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	/** Range of Effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	class USphereComponent* OverlapSphere;

	/** Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UStaticMeshComponent* ExplosiveMesh;

	/** Damage by Explosive */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	float Damage;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CSTrackerBot.generated.h"

class UStaticMeshComponent;
class UCSHealthComponent;
class UParticleSystem;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ACSTrackerBot : public APawn
{
	GENERATED_BODY() 

public:
	// Sets default values for this pawn's properties
	ACSTrackerBot();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	UCSHealthComponent* HealthComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tracker Bot")
	float SphereCompRadius;

	bool bExploded;

	bool bHasStartedSelfDestruction;

	FTimerHandle TimerHandle_DamageSelf;
	FTimerHandle TimerHandle_BotProximity;
	FTimerHandle TimerHandle_RefreshPath;

	//Dynamic Material Instance
	UMaterialInstanceDynamic* MatInstance;


	#pragma region Movement

//Next point in path to navigate to
	FVector NextPathPoint;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Movement")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Movement")
	bool bUseVelocityChange;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Movement")
	float RequiredDistanceToTarget;

	/* The amount of distance the target can move, before the tracker bot will recalculate path to it */
	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Movement")
	float MaxTargetMoveDistance;

#pragma endregion Movement

	#pragma region Combat

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot")
	float DamageSelfInterval;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tracker Bot|Proximity Attributes")
	float BotProximityDamageMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tracker Bot|Proximity Attributes")
	int MaxBotProximityMultiplierCount;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tracker Bot|Proximity Attributes")
	int BotsInProximityCount;

#pragma endregion Combat

#pragma region Effects

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Effects")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Effects")
	FVector ExplosionEffectScale;



	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Effects")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "Tracker Bot|Effects")
	USoundCue* ExplosionSound;

#pragma endregion Effects




	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void RefreshPath();

	FVector GetNextPathPoint();

	void SelfDestruct();

	void DamageSelf();

	UFUNCTION()
	void HandleTakeDamage(UCSHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void CheckProximity();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};

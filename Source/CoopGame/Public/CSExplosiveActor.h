// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSExplosiveActor.generated.h"

class UStaticMeshComponent;
class UCSHealthComponent;
class URadialForceComponent;

UCLASS()
class COOPGAME_API ACSExplosiveActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSExplosiveActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCSHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URadialForceComponent* RadForceComp;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosive Actor")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosive Actor")
	FVector ExplosionScale;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Actor")
	bool bHasExploded;

	UFUNCTION()
	void OnDeath(UCSHealthComponent* InHealthComp, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


};

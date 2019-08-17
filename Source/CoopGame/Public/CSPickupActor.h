// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ACSPowerupActor;

UCLASS()
class COOPGAME_API ACSPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSPickupActor();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UDecalComponent* DecalComp;


	UPROPERTY(EditInstanceOnly, Category = "Pickup")
	TSubclassOf<ACSPowerupActor> PowerupClass;

	ACSPowerupActor* PowerupInstance;

	UPROPERTY(EditInstanceOnly, Category = "Pickup")
	float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;



	virtual void BeginPlay() override;

	void Respawn();

public:

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};

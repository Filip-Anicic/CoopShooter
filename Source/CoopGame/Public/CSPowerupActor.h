// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSPowerupActor.generated.h"

UCLASS()
class COOPGAME_API ACSPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSPowerupActor();

protected:

	UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
	bool bIsPowerupActive;

	/* Time between powerup ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerup")
	float PowerupInterval;

	UPROPERTY(BlueprintReadWrite, Category = "Powerup")
	AActor* InstigatorActor;

	/* Total number of times we apply the powerup effect */
	UPROPERTY(EditDefaultsOnly, Category = "Powerup")
	uint16 TotalNumOfTicks;

	/* Total number of ticks applied */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Powerup")
	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerupTick;

	//Replicates the state of Powerup
	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerup")
	void OnPowerupStateChanged(bool bNewIsActive);

	UFUNCTION()
	void OnTickPowerup();



public:	
	
	void ActivatePowerup(AActor* TriggeringActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerup")
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerup")
	void OnPowerupTicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerup")
	void OnExpired();

};

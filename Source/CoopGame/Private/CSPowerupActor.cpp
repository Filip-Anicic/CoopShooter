// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPowerupActor.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACSPowerupActor::ACSPowerupActor()
{
	SetReplicates(true);
	
	bIsPowerupActive = false;
	PowerupInterval = 0.0f;
	InstigatorActor = nullptr;

	TotalNumOfTicks = 0;
	TicksProcessed = 0;
}

void ACSPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(true);
}

void ACSPowerupActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNumOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		InstigatorActor = nullptr;
		OnRep_PowerupActive();

		//Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ACSPowerupActor::ActivatePowerup(AActor* TriggeringActor)
{
	InstigatorActor = TriggeringActor;
	
	OnActivated();
	bIsPowerupActive = true;
	//Call it on the server
	OnRep_PowerupActive();

	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ACSPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}


void ACSPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSPowerupActor, bIsPowerupActive);
}

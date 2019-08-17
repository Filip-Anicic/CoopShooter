// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CSGameState.generated.h"


UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,
	
	WaveInProgress,

	//No longer spawning bots. Waiting for players to kill remaining bots
	WaitingToComplete,

	WaveComplete,

	GameOver
};


UCLASS()
class COOPGAME_API ACSGameState : public AGameStateBase
{
	GENERATED_BODY()


protected:

	UPROPERTY(ReplicatedUsing = OnRep_WaveState, BlueprintReadOnly, Category = "Game State")
	EWaveState WaveState;



	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Game State")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);


public:

	UFUNCTION()
	void SetWaveState(EWaveState NewState);
};

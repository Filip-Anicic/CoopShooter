// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CSGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController); 

UCLASS()
class COOPGAME_API ACSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ACSGameMode();


protected:

	UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
	float WaveInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
	float BotSpawnInterval;
	

	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	int WaveCount;

	//Bots to spawn in current wave
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	int NumOfBotsToSpawn;
	
	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;

protected:


	
	// Start spawning bots
	void StartWave();

	void SpawnBotTimerElapsed();

	/* Hook for BP to spawn a single bot */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Mode")
	void SpawnNewBot();

	// Stop spawning bots
	void EndWave();

	// Set timer for next wave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();

public:

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	/* Returns when an actor dies. Killed actor, Killer actor, Killer controller */
	UPROPERTY(BlueprintAssignable, Category = "Game Mode")
	FOnActorKilled OnActorKilled;
	
};

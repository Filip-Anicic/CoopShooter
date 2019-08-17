// Fill out your copyright notice in the Description page of Project Settings.


#include "CSGameMode.h"
#include "CSGameState.h"
#include "CSPlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/CSHealthComponent.h"


ACSGameMode::ACSGameMode()
{
	GameStateClass = ACSGameState::StaticClass();
	PlayerStateClass = ACSPlayerState::StaticClass();
	
	WaveInterval = 2.0f;
	BotSpawnInterval = 0.2f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}



void ACSGameMode::StartWave()
{
	WaveCount++;
	NumOfBotsToSpawn = 2 * WaveCount;

	UE_LOG(LogTemp, Log, TEXT("Game: Wave %d started!"), WaveCount);
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ACSGameMode::SpawnBotTimerElapsed, BotSpawnInterval, true);

	SetWaveState(EWaveState::WaveInProgress);
}


void ACSGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NumOfBotsToSpawn--;

	if (NumOfBotsToSpawn <= 0)
	{
		EndWave();
	}
}


void ACSGameMode::EndWave()
{
	UE_LOG(LogTemp, Log, TEXT("Game: Wave %d has ended!"), WaveCount);
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	SetWaveState(EWaveState::WaitingToComplete);
}


void ACSGameMode::PrepareForNextWave()
{
	RestartDeadPlayers();

	UE_LOG(LogTemp, Log, TEXT("Game: Preparing for next wave!"));
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ACSGameMode::StartWave, WaveInterval, false);

	SetWaveState(EWaveState::WaitingToStart);
}



void ACSGameMode::CheckWaveState()
{
	bool bIsPreparingForNextWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	
	if (NumOfBotsToSpawn > 0 || bIsPreparingForNextWave)
		return;


	bool bIsAnyBotAlive = false;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();

		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
			continue;

		UCSHealthComponent* HealthComp = Cast<UCSHealthComponent>(TestPawn->GetComponentByClass(UCSHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);
		PrepareForNextWave();
	}
}

void ACSGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			UCSHealthComponent* HealthComp = Cast<UCSHealthComponent>(MyPawn->GetComponentByClass(UCSHealthComponent::StaticClass()));
			
			//Ensure that the player pawn has a Health Component or cause a break in the code
			if (ensure(HealthComp))
			{
				return;
			}
		}
	}

	//No player alive
	GameOver();
}




void ACSGameMode::GameOver()
{
	EndWave();

	// @TODO: Finish up the match, present 'Game Over' to the players.

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died!"));
	SetWaveState(EWaveState::GameOver);
}


void ACSGameMode::SetWaveState(EWaveState NewState)
{
	ACSGameState* GS = GetGameState<ACSGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}




void ACSGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}




void ACSGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}

void ACSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
	CheckAnyPlayerAlive();
}


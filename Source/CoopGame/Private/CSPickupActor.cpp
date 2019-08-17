// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPickupActor.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"
#include "CSPowerupActor.h"
#include "CSCharacter.h"

// Sets default values
ACSPickupActor::ACSPickupActor()
{
	SetReplicates(true);
	
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateAbstractDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(0, 90, 0));
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetupAttachment(RootComponent);


	CooldownDuration = 10.0f; 
}


// Called when the game starts or when spawned
void ACSPickupActor::BeginPlay()
{
	Super::BeginPlay();

	if(Role == ROLE_Authority)
		Respawn();	
}

void ACSPickupActor::Respawn()
{
	if (PowerupClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerupClass is nullptr in %s. Please update your blueprint"), *GetName());
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerupInstance = GetWorld()->SpawnActor<ACSPowerupActor>(PowerupClass, GetTransform(), SpawnParameters);
}



void ACSPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (Role == ROLE_Authority && PowerupInstance && Cast<ACSCharacter>(OtherActor))
	{
		PowerupInstance->ActivatePowerup(OtherActor);

		PowerupInstance = nullptr;

		//Set timer to respawn
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ACSPickupActor::Respawn, CooldownDuration);
	}
}


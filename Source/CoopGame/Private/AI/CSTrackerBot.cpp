// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CSTrackerBot.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CSHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "CSCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetSystemLibrary.h"


static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(
	TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
	TEXT("Draw debug lines for tracker bots"),
	ECVF_Cheat);


// Sets default values
ACSTrackerBot::ACSTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Components
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<UCSHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ACSTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetSphereRadius(200.0f);
	SphereComp->SetupAttachment(RootComponent);

	SphereCompRadius = 200.0f;

	//Movement
	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;
	MaxTargetMoveDistance = 25.0f;

	//Combat
	ExplosionDamage = 60;
	ExplosionRadius = 350;
	DamageSelfInterval = 0.25f;

	BotProximityDamageMultiplier = 0.1f;
	MaxBotProximityMultiplierCount = 3;
	BotsInProximityCount = 0;
	
	bExploded = false;
	bHasStartedSelfDestruction = false;

	//Effects
	ExplosionEffectScale = FVector::OneVector;

}

// Called when the game starts or when spawned
void ACSTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if(Role == ROLE_Authority)
	{
		//Find initial move to
		NextPathPoint = GetNextPathPoint();

		//Set proximity check timer
		GetWorldTimerManager().SetTimer(TimerHandle_BotProximity, this, &ACSTrackerBot::CheckProximity, 1.0f, true, 0.0f);
	}

	if (MatInstance == nullptr)
		MatInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

	if (MatInstance)
		MatInstance->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
}



void ACSTrackerBot::RefreshPath()
{
	//Find new path
	NextPathPoint = GetNextPathPoint();
}


FVector ACSTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();

		if (TestPawn == nullptr || UCSHealthComponent::IsFriendly(this, TestPawn))
			continue;

		UCSHealthComponent* TestPawnHealthComp = Cast<UCSHealthComponent>(TestPawn->GetComponentByClass(UCSHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
			if (NearestTargetDistance > Distance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ACSTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			//Return next point in path
			return NavPath->PathPoints[1];
		}
	}
	

	// Failed to find path
	return GetActorLocation();
}

void ACSTrackerBot::SelfDestruct()
{
	if (bExploded) return;


	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FRotator::ZeroRotator, ExplosionEffectScale);
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Server logic
	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		//Apply radial damage
		float TotalDamage = ExplosionDamage;
		if (BotsInProximityCount > 0) 
			TotalDamage *= 1 + (BotsInProximityCount * BotProximityDamageMultiplier);

		UGameplayStatics::ApplyRadialDamage(this, TotalDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		//Destroy actor immidiatly
		SetLifeSpan(2.0f);
	}
}


void ACSTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ACSTrackerBot::HandleTakeDamage(UCSHealthComponent* HealthComponent, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// @TODO Pulse material on hit
	if(MatInstance == nullptr)
		MatInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

	if (MatInstance)
		MatInstance->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);

	//Explode if health is 0
	if (Health == 0)
	{
		SelfDestruct();
	}
}



void ACSTrackerBot::CheckProximity()
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<AActor*> OutActors;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), SphereComp->GetScaledSphereRadius(), 
		ObjectTypes, ACSTrackerBot::StaticClass(), ActorsToIgnore, OutActors);

	BotsInProximityCount = FMath::Clamp(OutActors.Num(), 0, MaxBotProximityMultiplierCount);
	float GlowAmount = 0;

	if (OutActors.Num() > 0)
		GlowAmount = BotsInProximityCount / (float)MaxBotProximityMultiplierCount;


	//UE_LOG(LogTemp, Log, TEXT("Glow amount: %f"), GlowAmount);

	if (MatInstance == nullptr)
		MatInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

	if (MatInstance)
		MatInstance->SetScalarParameterValue("GlowAmount", GlowAmount);
}



// Called every frame
void ACSTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (Role == ROLE_Authority && !bExploded)
	{
		FVector TargetDelta = (NextPathPoint - GetActorLocation());

		if (TargetDelta.Size() <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();

			if (DebugTrackerBotDrawing)
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!", NULL, FColor::Green, 1);
		}
		else
		{
			// Keep moving to next target
			FVector ForceDirection = TargetDelta;
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing)
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32.0f, FColor::Red, false, 0.0f, 0, 3.0f);
		}

		if (DebugTrackerBotDrawing)
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 4.0f, 1.0f);
	}
	
}


void ACSTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!bHasStartedSelfDestruction && !bExploded)
	{
		if (HealthComp && !UCSHealthComponent::IsFriendly(this, OtherActor))
		{
			//We overlapped with player!
			if (Role == ROLE_Authority)
			{
				//Start self destructing
				GetWorldTimerManager().SetTimer(TimerHandle_DamageSelf, this, &ACSTrackerBot::DamageSelf, DamageSelfInterval, true, 0.0f);
			}

			bHasStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}
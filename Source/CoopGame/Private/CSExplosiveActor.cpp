// Fill out your copyright notice in the Description page of Project Settings.


#include "CSExplosiveActor.h"
#include "Components/CSHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

static int32 DebugExplosiveActorDrawing = 0;
FAutoConsoleVariableRef CVARDebugExplosiveActorDrawing(
	TEXT("COOP.DebugExplosiveActor"),
	DebugExplosiveActorDrawing,
	TEXT("Draw debug lines for explosive actors"),
	ECVF_Cheat);



// Sets default values
ACSExplosiveActor::ACSExplosiveActor()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->bApplyImpulseOnDamage = true;
	//MeshComp->SetSimulatingPhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<UCSHealthComponent>(TEXT("HealthComp"));

	HealthComp->OnDeath.AddDynamic(this, &ACSExplosiveActor::OnDeath);

	RadForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadForceComp"));
	RadForceComp->AddCollisionChannelToAffect(ECC_Visibility);
	RadForceComp->ForceStrength = 1000;
	RadForceComp->Radius = 100;
	RadForceComp->SetupAttachment(MeshComp);

	ExplosionScale = FVector::OneVector;
}

void ACSExplosiveActor::OnDeath(UCSHealthComponent* InHealthComp, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//Explode
	if (ExplosionEffect)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawned Explosion Effect!"));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FRotator::ZeroRotator, ExplosionScale);
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), RadForceComp->Radius, 16, FColor::Red, false, 3.0f);
	RadForceComp->FireImpulse();

	//Apply damage on server
	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(GetWorld(), 40.0f, GetActorLocation(), RadForceComp->Radius, nullptr, IgnoredActors, this);
	}

	bHasExploded = true;
}


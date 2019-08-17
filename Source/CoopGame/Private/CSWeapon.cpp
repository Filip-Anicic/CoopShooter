// Fill out your copyright notice in the Description page of Project Settings.


#include "CSWeapon.h"
#include "CoopGame.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Camera/CameraShake.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw debug lines for weapons"), 
	ECVF_Cheat);

// Sets default values
ACSWeapon::ACSWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TrailTargetName = "Target";


	BaseDamage = 20.0f;
	BaseDamageMultiplier = 1.0f;
	CriticalHitMultiplier = 2.5f;
	RateOfFire = 600;

	//Spread
	SpreadAngleMin = 0.0f;
	SpreadAngleMax = 0.3f;
	SpreadCurrent = SpreadAngleMin;
	SpreadIncreaseAmount = 0.1f;
	SpreadDecreaseSpeed = 0.1f;


	//Ammo
	MagMaxAmount = 30;
	MagCount = MagMaxAmount;
	AmmoMaxCapacity = 270;
	AmmoCount = AmmoMaxCapacity;
}


void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

void ACSWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SpreadCurrent >= SpreadAngleMin)
	{
		DecreaseSpread(SpreadDecreaseSpeed * DeltaSeconds);
	}
}




bool ACSWeapon::CanReload()
{
	if (MagCount != MagMaxAmount && AmmoCount != 0)
		return true;
	else
		return false;
}

void ACSWeapon::StartReload()
{
	bCanFire = false;
}

void ACSWeapon::EndReload()
{
	if (MagCount != MagMaxAmount || AmmoCount != 0)
	{
		int delta = MagMaxAmount - MagCount;
		
		//If there is less ammo left than required to refil the magazine
		if (delta > AmmoCount)
			delta = AmmoCount;

		AmmoCount -= delta;
		MagCount += delta;

		bCanFire = true;
	}
}


#pragma region Firing

void ACSWeapon::StartFire()
{
	bool bIsTimerActive = GetWorldTimerManager().IsTimerActive(TimerHandle_TimeBetweenShots);

	if (!bIsTimerActive && MagCount > 0)
	{
		//TODO Play pulled trigger sound
		float FirstDelay = FMath::Max(LastFiredTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACSWeapon::Fire, TimeBetweenShots, true, FirstDelay);
	}
}

void ACSWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}


void ACSWeapon::Fire()
{
	APawn* MyPawn = Cast<APawn>(GetOwner());
	
	if (MagCount <= 0 && MyPawn->IsLocallyControlled()) return;
	
	//Call Server to replicate this on other clients
	if (Role != ROLE_Authority)
	{
		ServerFire();
	}

	//Trace the world from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		//Update weapon magazine
		MagCount--;
		
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		//Shot direction with weapon spread
		FVector ShotDirection = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(EyeRotation.Vector(), SpreadCurrent);
		

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		//Trace hit location
		FVector TracerEndPoint = TraceEnd;
		FColor TraceHitStatusColor = FColor::Red;

		EPhysicalSurface SurfaceType = SurfaceType_Default;
		bool bHasHit = false;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			//Damage is only dealt on Server
			if (Role == ROLE_Authority)
			{
				float DamageDelt = BaseDamage * BaseDamageMultiplier;

				//Deal more damage if it is a critical hit
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
					DamageDelt *= CriticalHitMultiplier;

				//Apply damage to the hit actor
				UGameplayStatics::ApplyPointDamage(HitActor, DamageDelt, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
			}

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			bHasHit = true;
			TracerEndPoint = Hit.ImpactPoint;
			TraceHitStatusColor = FColor::Green;

			
		}

		PlayFireEffects(TracerEndPoint);

		IncreaseSpread(SpreadIncreaseAmount);


		if(DebugWeaponDrawing > 0)
			DrawDebugLine(GetWorld(), EyeLocation, TracerEndPoint, TraceHitStatusColor, false, 1.0f, 0, 1.0f);


		if (Role == ROLE_Authority)
		{
			HitScanTrace.bHasHit = bHasHit;
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFiredTime = GetWorld()->TimeSeconds;
	}
}

void ACSWeapon::ServerFire_Implementation()
{
	Fire();
}
bool ACSWeapon::ServerFire_Validate()
{ return true; }









void ACSWeapon::AddAmmo_Implementation(int amount)
{
	if (amount < 0) return;

	AmmoCount = FMath::Clamp(AmmoCount + amount, 0, AmmoMaxCapacity);
}

bool ACSWeapon::AddAmmo_Validate(int amount)
{ return true; }

void ACSWeapon::AddAmmoMag_Implementation(int amount)
{
	if (amount < 0) return;

	AmmoCount = FMath::Clamp(AmmoCount + (amount * MagMaxAmount), 0, AmmoMaxCapacity);
}

bool ACSWeapon::AddAmmoMag_Validate(int amount)
{ return true; }

//Replication Events
void ACSWeapon::OnRep_HitScanTrace()
{
	//Play cosmetic effects
	PlayFireEffects(HitScanTrace.TraceTo);
	if(HitScanTrace.bHasHit)
		PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

#pragma endregion Firing



void ACSWeapon::IncreaseSpread(float amount)
{
	SpreadCurrent += amount;

	if (SpreadCurrent > SpreadAngleMax)
		SpreadCurrent = SpreadAngleMax;
}

void ACSWeapon::DecreaseSpread(float amount)
{
	SpreadCurrent -= amount;

	if (SpreadCurrent < SpreadAngleMin)
		SpreadCurrent = SpreadAngleMin;
}








void ACSWeapon::PlayFireEffects(FVector TraceEndPoint)
{
	//Muzzle Effect
	if (MuzzleEffect)
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);

	//Bullet Trail Effect
	if (TrailEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TrailEffectComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailEffect, MuzzleLocation);
		if (TrailEffectComp)
		{
			TrailEffectComp->SetVectorParameter(TrailTargetName, TraceEndPoint);
		}
	}

	//Camera Shake
	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (MyPawn)
	{
		APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
		if (PC)
			PC->ClientPlayCameraShake(FireCamShake);
	}
}

void ACSWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	//Spawn Impact particle effect
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}




#pragma region Setter Methods

void ACSWeapon::AddBaseDamage_Implementation(float amount)
{
	BaseDamage += amount;
	if (BaseDamage < 0) BaseDamage = 0;
}

bool ACSWeapon::AddBaseDamage_Validate(float amount)
{ return true;}


void ACSWeapon::AddBaseDamagePercentage_Implementation(float percent)
{
	BaseDamageMultiplier += (percent / 100);
	if (BaseDamageMultiplier < 0) BaseDamageMultiplier = 0.0f;
}

bool ACSWeapon::AddBaseDamagePercentage_Validate(float percent)
{ return true; }

void ACSWeapon::AddCriticalHitPercentage_Implementation(float percent)
{
	CriticalHitMultiplier += (percent / 100);
	if (CriticalHitMultiplier < 0.0f) CriticalHitMultiplier = 0.0f;
}

bool ACSWeapon::AddCriticalHitPercentage_Validate(float percent)
{ return true; }

#pragma endregion Setter Methods




void ACSWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSWeapon, MagMaxAmount);
	DOREPLIFETIME(ACSWeapon, MagCount);
	DOREPLIFETIME(ACSWeapon, AmmoMaxCapacity);
	DOREPLIFETIME(ACSWeapon, AmmoCount);



	DOREPLIFETIME_CONDITION(ACSWeapon, HitScanTrace, COND_SkipOwner);
}




// Fill out your copyright notice in the Description page of Project Settings.


#include "CSProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void ACSProjectileWeapon::Fire()
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
		MagCount--;
		
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		const USkeletalMeshSocket* MeshSocket = MeshComp->GetSocketByName(MuzzleSocketName);
		
		//if a socket on the skeletal mesh is found
		if (MeshSocket)
		{
			FVector MuzzleLocation = MeshSocket->GetSocketLocation(MeshComp);

			FRotator AimDirection = (TraceEnd - MuzzleLocation).Rotation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Instigator = MyOwner->GetInstigatorController()->GetPawn();

			AActor* Projectile = GetWorld()->SpawnActor(ProjectileClass, &MuzzleLocation, &AimDirection, ActorSpawnParams);
		}

		//Since there is no Trail effect being used by this weapon class, the TracerEnd vector will not be used
		PlayFireEffects(FVector::ZeroVector);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSWeapon.h"
#include "CSProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ACSProjectileWeapon : public ACSWeapon
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AActor> ProjectileClass;

	void Fire() override;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;


// Contains information of a single hitscan weapon line trace
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	bool bHasHit;
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY()
	FVector_NetQuantize TraceTo;
};


UCLASS()
class COOPGAME_API ACSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSWeapon();

	


protected:

	//Variables

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;



	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bCanFire;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamageMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float CriticalHitMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ReloadSpeed;

#pragma region RateOfFire

	FTimerHandle TimerHandle_TimeBetweenShots;
	float LastFiredTime;

	/* BPM - Bullets per minute fired by weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float RateOfFire;

	/* Derived from BPM */
	float TimeBetweenShots;

#pragma endregion RateOfFire

#pragma region Ammo

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	int MagMaxAmount;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int MagCount;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	int AmmoMaxCapacity;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Ammo")
	int AmmoCount;

#pragma endregion Ammo
	
#pragma region Spread

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float SpreadAngleMin;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float SpreadAngleMax;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread")
	float SpreadIncreaseAmount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread")
	float SpreadDecreaseSpeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Spread", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float SpreadCurrent;

#pragma endregion Spread

#pragma region Effects

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	UParticleSystem* MuzzleEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	UParticleSystem* TrailEffect;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	FName TrailTargetName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
	TSubclassOf<UCameraShake> FireCamShake;

#pragma endregion Effects

	//TODO Add sound effects for Empty chamber
	//TODO Add sound effects for Firing
	//TODO Add sound effects for Impact




	
	//Methods

	virtual void BeginPlay() override;

	void PlayFireEffects(FVector TraceEndPoint);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	virtual void Fire();
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerFire();
	virtual void IncreaseSpread(float amount);
	virtual void DecreaseSpread(float amount);



	/* Add given amount to weapon base damage */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Weapon")
	void AddBaseDamage(float amount);

	/* Add given percent to weapon base damage multiplier
	Multiplier can not be lower than 0% */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Weapon")
	void AddBaseDamagePercentage(float percent);

	/* Add given percent to weapon critical hit multiplier 
	Multiplier can not be lower than 0% */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Weapon")
	void AddCriticalHitPercentage(float percent);


	/* Add given amount to weapon ammo count */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Weapon")
	void AddAmmo(int amount);

	/* Add given amount of magazines to ammo count */
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Weapon")
	void AddAmmoMag(int amount);

	//Replication Events

	UFUNCTION()
	void OnRep_HitScanTrace();

public:

	//Methods
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BLueprintCallable, Category = "Weapon")
	bool CanReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void EndReload();



	virtual void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StopFire();

};

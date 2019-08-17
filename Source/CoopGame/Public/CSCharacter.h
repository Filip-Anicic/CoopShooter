// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CSCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ACSWeapon;
class UCSHealthComponent;


UCLASS()
class COOPGAME_API ACSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACSCharacter();

	virtual FVector GetPawnViewLocation() const override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCSHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;



	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bIsReloading;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bIsAiming;
	   
	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.0f, ClampMax = 100))
	float AimInterpSpeed;


	//Movement Speed
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bIsSprinting;

	UPROPERTY(ReplicatedUsing = OnRep_MaxSpeed, BlueprintReadOnly, Category = "Movement")
	float MaxSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,  Category = "Player", meta = (ClampMin = 0.0f))
	float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,  Category = "Player", meta = (ClampMin = 0.0f))
	float SprintSpeed;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly,  Category = "Player", meta = (ClampMin = 0.0f))
	float SpeedMultiplier;


	// Default camera field of view
	float FOV_Default;

	// Camera field of view when aiming down weapon sights (ADS)
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float FOV_Aim;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Player")
	ACSWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ACSWeapon> DefaultWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//Movement methods
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();
	void BeginSprint();
	void EndSprint();
	void BeginJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetSpeedMultiplier(float speed);


	//Combat Methods	
	UFUNCTION(Server, Reliable, WithValidation)
	void BeginServerAim();
	UFUNCTION(Server, Reliable, WithValidation)
	void EndServerAim();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();


	//Events

	/* Blueprint event called when the Character has started Reloading. Used for updating the Character AnimBP */
	UFUNCTION(BlueprintImplementableEvent, Category = "Actions")
	void OnBeginReload();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	void OnReloadComplete();

	
	UFUNCTION()
	void OnDeathEvent(UCSHealthComponent* HealthComponent, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


	//Rep Events
	UFUNCTION()
	void OnRep_MaxSpeed();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	//Combat Methods
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void BeginAim();
	UFUNCTION(BlueprintCallable, Category = "Player")
	void EndAim();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void Reload();

};

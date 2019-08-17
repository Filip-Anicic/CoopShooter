// Fill out your copyright notice in the Description page of Project Settings.


#include "CSCharacter.h"
#include "CoopGame.h"
#include "CSWeapon.h"
#include "Components/InputComponent.h"
#include "Components/CSHealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACSCharacter::ACSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComp = CreateDefaultSubobject<UCSHealthComponent>(TEXT("InHealthComp"));

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	
	WeaponAttachSocketName = "WeaponSocket";
	bDied = false;
	bIsAiming = false;
	bIsReloading = false;
	bIsSprinting = false;
	
	FOV_Aim = 65.0f;
	AimInterpSpeed = 20.0f;

	WalkSpeed = 250.0f;
	SprintSpeed = 600.0f;
	SpeedMultiplier = 1.0f;

	MaxSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

FVector ACSCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
		return CameraComp->GetComponentLocation();

	return Super::GetPawnViewLocation();
}

// Called when the game starts or when spawned
void ACSCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Update character speed
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	//Events
	HealthComp->OnDeath.AddDynamic(this, &ACSCharacter::OnDeathEvent);

	FOV_Default = CameraComp->FieldOfView;


	if (Role == ROLE_Authority)
	{
		//Spawn default weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<ACSWeapon>(DefaultWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}


//Movement Methods
#pragma region Movement Methods

void ACSCharacter::MoveForward(float Value)
{ AddMovementInput(GetActorForwardVector() * Value); }

void ACSCharacter::MoveRight(float Value)
{ AddMovementInput(GetActorRightVector() * Value); }


void ACSCharacter::BeginCrouch()
{ Crouch(); }

void ACSCharacter::EndCrouch()
{ UnCrouch(); }


void ACSCharacter::BeginSprint()
{
	bIsSprinting = true;
	MaxSpeed = SprintSpeed * SpeedMultiplier;
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
}

void ACSCharacter::EndSprint()
{
	bIsSprinting = false;
	MaxSpeed = WalkSpeed * SpeedMultiplier;
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
}


void ACSCharacter::BeginJump()
{ Jump(); }

void ACSCharacter::SetSpeedMultiplier(float multiplier)
{
	if (multiplier < 0.0f)
		multiplier = 0.0f;

	//Will replicate to clients
	SpeedMultiplier = multiplier;
	
	MaxSpeed = bIsSprinting ? SprintSpeed * SpeedMultiplier : WalkSpeed * SpeedMultiplier;
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
}

#pragma endregion Movement Methods



//Combat Methods
#pragma region Combat Methods

//Aim weapon
void ACSCharacter::BeginAim()
{ BeginServerAim(); }

void ACSCharacter::BeginServerAim_Implementation()
{ bIsAiming = true; }
bool ACSCharacter::BeginServerAim_Validate()
{return true;}


void ACSCharacter::EndAim()
{ EndServerAim(); }

void ACSCharacter::EndServerAim_Implementation()
{ bIsAiming = false;}
bool ACSCharacter::EndServerAim_Validate()
{ return true; }



//Fire weapon
void ACSCharacter::StartFire()
{
	if (bDied)
		return;
	
	if (CurrentWeapon)
		CurrentWeapon->StartFire();
}


void ACSCharacter::StopFire()
{
	if (CurrentWeapon)
		CurrentWeapon->StopFire();
}



//Reload weapon
void ACSCharacter::Reload()
{
	if (bDied) 
		return;

	//Call the server to Reload
	if (Role != ROLE_Authority)
		ServerReload();

	if (!bIsReloading && CurrentWeapon && CurrentWeapon->CanReload())
	{
		bIsReloading = true;
		GetController()->SetIgnoreMoveInput(true);
		GetMovementComponent()->SetJumpAllowed(false);

		CurrentWeapon->StartReload();
		
		OnBeginReload();
	}
}

void ACSCharacter::ServerReload_Implementation()
{
	Reload();
}
bool ACSCharacter::ServerReload_Validate()
{ return true; }

#pragma endregion Combat Methods

//Events
#pragma region Events

void ACSCharacter::OnReloadComplete_Implementation()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->EndReload();

		GetMovementComponent()->SetJumpAllowed(true);
		GetController()->SetIgnoreMoveInput(false);
		bIsReloading = false;
	}
}




void ACSCharacter::OnDeathEvent(UCSHealthComponent* HealthComponent, float HealthDelta, const class UDamageType* DamageType, 
	class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!bDied)
	{
		//Die
		bDied = true;

		//Stop any actions
		if (CurrentWeapon) 
			StopFire();
		EndAim();

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
	}
}

void ACSCharacter::OnRep_MaxSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
}

#pragma endregion Events









// Called every frame
void ACSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bIsAiming ? FOV_Aim : FOV_Default;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ACSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ACSCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ACSCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACSCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACSCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ACSCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ACSCharacter::EndSprint);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACSCharacter::BeginJump);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ACSCharacter::BeginAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ACSCharacter::EndAim);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACSCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACSCharacter::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ACSCharacter::Reload);
}


void ACSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSCharacter, CurrentWeapon);
	DOREPLIFETIME(ACSCharacter, bDied);
	DOREPLIFETIME(ACSCharacter, bIsReloading);
	DOREPLIFETIME(ACSCharacter, bIsAiming);
	DOREPLIFETIME(ACSCharacter, MaxSpeed);
}

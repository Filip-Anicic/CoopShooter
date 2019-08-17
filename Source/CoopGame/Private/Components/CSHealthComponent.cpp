// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CSHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "CSGameMode.h"

UCSHealthComponent::UCSHealthComponent()
{
	SetIsReplicated(true);

	bIsDead = false;
	DefaultHealth = 100.0f;
	TeamNum = 255;
}


// Called when the game starts
void UCSHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	//Only done by server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UCSHealthComponent::HandleTakeAnyDamage);
	}
	
	Health = DefaultHealth;
}

void UCSHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead)
		return;

	if (IsFriendly(DamagedActor, DamageCauser) && DamagedActor != DamageCauser)
	{
		//@TODO: Add friendly fire option :)
		return;
	}


	FString Prefix = Damage < 0 ? "+" : "-";

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	bIsDead = Health <= 0.0f;

	UE_LOG(LogTemp, Log, TEXT("%s took damage: %s%s (%sHP)"), *GetOwner()->GetName(), *Prefix, *FString::SanitizeFloat(Damage), *FString::SanitizeFloat(Health));

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		ACSGameMode* GM = Cast<ACSGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}

		OnDeath.Broadcast(this, Damage, DamageType, InstigatedBy, DamageCauser);
	}
}

void UCSHealthComponent::OnRep_Health(float OldHealth)
{
	float Delta = Health - OldHealth;
	FString Prefix = Delta < 0 ? "" : "+";

	UE_LOG(LogTemp, Log, TEXT("Client: %s Health: %s%s"), *GetOwner()->GetName(), *Prefix, *FString::SanitizeFloat(Delta));


	OnHealthChanged.Broadcast(this, Health, Delta, nullptr, nullptr, nullptr);
}



float UCSHealthComponent::GetHealth() const
{
	return Health;
}

void UCSHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0 || Health <= 0.0f)
		return;

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("%s got healed: +%s (%sHP)"), *GetOwner()->GetName(), *FString::SanitizeFloat(HealAmount), *FString::SanitizeFloat(Health));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}







/* 
Returns if Actor A and B are from the same team 
If any of the actors is nullptr or if any actor doesn't have a CSHealthComponent
It will return false and assume that the other actor is not friendly
*/
bool UCSHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
		return false;

	UCSHealthComponent* HealthCompA = Cast<UCSHealthComponent>(ActorA->GetComponentByClass(UCSHealthComponent::StaticClass()));
	UCSHealthComponent* HealthCompB = Cast<UCSHealthComponent>(ActorB->GetComponentByClass(UCSHealthComponent::StaticClass()));


	if (HealthCompA == nullptr || HealthCompB == nullptr)
		return false;

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

void UCSHealthComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCSHealthComponent, Health);
}


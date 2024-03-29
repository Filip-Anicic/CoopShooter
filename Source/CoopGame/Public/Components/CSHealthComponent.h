// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CSHealthComponent.generated.h"

//OnHealthChanged event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UCSHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnDeathSignature, UCSHealthComponent*, HealthComp, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Coop), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API UCSHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCSHealthComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Component")
	uint8 TeamNum;

protected:

	bool bIsDead;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health Component")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Component")
	float DefaultHealth;

	// Called when the game starts
	virtual void BeginPlay() override;


	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnRep_Health(float OldHealth);
		
public:

	UFUNCTION(BlueprintCallable, Category = "Health Component")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health Component")
	void Heal(float HealAmount);

	UPROPERTY(BlueprintAssignable, Category = "Health Component|Event")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health Component|Event")
	FOnDeathSignature OnDeath;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health Component")
	static bool IsFriendly(AActor* ActorA, AActor* ActorB);


};

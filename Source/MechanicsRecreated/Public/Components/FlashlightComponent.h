// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SpotLightComponent.h"
#include "Particles/ParticleSystem.h"
#include "Interfaces/DamageInterface.h"
#include "FlashlightComponent.generated.h"

// DECLARATIONS
class AFlashlightCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHANICSRECREATED_API UFlashlightComponent : public UActorComponent, public IDamageInterface
{
	GENERATED_BODY()

	
public:
	// CONSTRUCTORS
	UFlashlightComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// SET DEFAULT VALUES
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1- STATS")
	float FlashRange = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "1- STATS")
	float Damage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="2 - PARTICLES")
	UParticleSystem* FlashlightDamageParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="2 - PARTICLES")
	FVector ParticleSize {1.0, 1.0, 1.0};

	UFUNCTION()
	void FlashlightLineTrace();

	AFlashlightCharacterBase* GetPlayerRef() const { return PlayerRef; }
	USpotLightComponent* GetPlayerSpotlight() const { return PlayerSpotlight; }
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AFlashlightCharacterBase* PlayerRef;	// Variable PlayerRef points to base character address
	
	UPROPERTY()
	USpotLightComponent* PlayerSpotlight;	// points to spot light component class
	
};

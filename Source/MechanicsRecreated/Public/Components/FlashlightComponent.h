// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SpotLightComponent.h"
#include "Interfaces/DamageInterface.h"
#include "FlashlightComponent.generated.h"

class AFlashlightCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHANICSRECREATED_API UFlashlightComponent : public UActorComponent, public IDamageInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFlashlightComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "FlashlightProperties")
	float FlashRange = 1000;

	UPROPERTY(EditAnywhere, Category = "FlashlightProperties")
	float Damage = 1;

private:

	AFlashlightCharacterBase* PlayerRef;
	USpotLightComponent* PlayerSpotlight;

	UFUNCTION()
	void FlashlightLineTrace();
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FlashlightComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "Components/FlashlightComponent.h"

#include "CollisionDebugDrawingPublic.h"
#include "Character/FlashlightCharacterBase.h"

// Sets default values for this component's properties
UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	this->Deactivate();
	this->SetComponentTickEnabled(false);
	
	PlayerRef = Cast<AFlashlightCharacterBase>(GetOwner());
	if(PlayerRef)
	{
		PlayerSpotlight = PlayerRef->FlashlightSpotLight;
	}
}


// Called every frame
void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (this->IsActive() && PlayerSpotlight)
	{
		FlashlightLineTrace();
	}
	
}


void UFlashlightComponent::FlashlightLineTrace()
{

	if (PlayerSpotlight && PlayerRef)
	{
		// Start and end vectors of line trace
		FVector StartLocation = PlayerSpotlight->GetComponentLocation();
		FVector ForwardVector = PlayerSpotlight->GetForwardVector();
		FVector EndLocation = StartLocation + (ForwardVector * FlashRange);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(PlayerRef);	// ignore self in trace

		// Perform line trace
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Camera, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && HitActor->ActorHasTag(FName("enemy")) && !HitActor->ActorHasTag(FName("shielddown")) && HitActor->GetClass()->ImplementsInterface(UDamageInterface::StaticClass()))
			{
				// Hit line
				DrawDebugLine(
					GetWorld(),
					StartLocation,
					EndLocation,
					FColor::Red, // Green color for the line
					false,
					0.2f,
					0, // Depth priority
					1.0f
				);

				// Calls Damage Interface event in enemy
				Execute_FlashlightDamage(HitActor, Damage);
			}

			else
			{
				// Non-Hit line
				DrawDebugLine(
					GetWorld(),
					StartLocation,
					EndLocation,
					FColor::Green, // Green color for the line
					false,
					0.2f,
					0, // Depth priority
					1.0f
				);
			}
		}
		
	}
}


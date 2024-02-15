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

// CONSTRUCTOR
UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	// INITIALIZE PlayerRef & PlayerSpotlight VIA CAST TO CURRENT OWNER
	PlayerRef = Cast<AFlashlightCharacterBase>(GetOwner());
	if(PlayerRef)
	{
		PlayerSpotlight = PlayerRef->FlashlightSpotLight;
	}

	// DEACTIVATE AIM LINE TRACE FUNCTIONALITY
	this->SetComponentTickEnabled(false);
	
}



void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	// AIM LINE TRACE ACTIVATED WHEN PLAYER CALLS UseFlashlight()
	if (this->IsActive() && PlayerSpotlight)
	{
		FlashlightLineTrace();
	}
}


// AIM LINE TRACE FUNCTIONALITY
void UFlashlightComponent::FlashlightLineTrace()
{
	
	if (PlayerSpotlight && PlayerRef)
	{
		// INITIALIZE LINE TRACE LENGTH AND ROTATION
		FVector StartLocation = PlayerSpotlight->GetComponentLocation();
		FVector ForwardVector = PlayerSpotlight->GetForwardVector();
		FVector EndLocation = StartLocation + (ForwardVector * FlashRange);

		// SET VARIABLES FOR LINE TRACE ARGUMENTS
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(PlayerRef);	// ignore self in trace

		// PERFORM LINE TRACE
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Camera, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && HitActor->ActorHasTag(FName("enemy")) && !HitActor->ActorHasTag(FName("shieldDown")) && HitActor->GetClass()->ImplementsInterface(UDamageInterface::StaticClass()))
			{
				// Hit Line
				/*DrawDebugLine(
					GetWorld(),
					StartLocation,
					EndLocation,
					FColor::Red, // Green color for the line
					false,
					0.2f,
					0, // Depth priority
					1.0f
				); */

				
				// SEND INTERFACE EVENT MESSAGE TO HIT ENEMY TO TAKE FLASHLIGHT DAMAGE
				Execute_FlashlightDamage(HitActor, DamageScale * GetWorld()->GetDeltaSeconds());
				
				
				// VFX SPAWNED AT HIT LOCATION
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					FlashlightDamageParticles,
					HitResult.ImpactPoint,
					HitResult.ImpactNormal.Rotation(),
					ParticleSize,
					true
					);
			}

			else
			{
				// Non-Hit line
				/*DrawDebugLine(
					GetWorld(),
					StartLocation,
					EndLocation,
					FColor::Green, // Green color for the line
					false,
					0.2f,
					0, // Depth priority
					1.0f
					);*/
			}
		}
		
	}
}
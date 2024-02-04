// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/FlashlightComponent.h"
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

	ThirdPersonCharacterRef = Cast<AFlashlightCharacterBase>(GetOwner());
	
}


// Called every frame
void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}


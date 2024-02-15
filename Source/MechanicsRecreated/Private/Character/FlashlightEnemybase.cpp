// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/FlashlightEnemybase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// CONSTRUCTOR
AFlashlightEnemybase::AFlashlightEnemybase()
{
	PrimaryActorTick.bCanEverTick = true;

	// NOW PLAYER'S FlashlightComponent CANNOT DETECT ENEMY'S CAPSULE COMPONENT
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// TAG RECOGNIZED BY FlashlightComponent - used by Player Character
	Tags.Add("enemy");
	
	// SETUP WIDGET FOR ENEMY SHIELD STATUS
	ShieldWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ShieldWidget"));
	ShieldWidget->SetupAttachment(this->GetMesh());
	ShieldWidget->SetWidgetSpace(EWidgetSpace::World);
}


void AFlashlightEnemybase::BeginPlay()
{
	Super::BeginPlay();

	// GETS THE WIDGET ASSIGNED TO THE WIDGET COMPONENT IN THE CHILD CLASS
	if (ShieldWidget->GetUserWidgetObject() != nullptr)
		UserWidget = ShieldWidget->GetUserWidgetObject();
}

// Called every frame
void AFlashlightEnemybase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


//////////// FUNCTIONS //////////////


// INTERFACE EVENT - when mesh is hit by flashlight component line trace (called in FlashlightComponent.cpp)
void AFlashlightEnemybase::FlashlightDamage_Implementation(float Damage)
{
	// UPDATE SHIELD HEALTH - clamped between 0-100
	currentShield = FMath::Clamp(currentShield - Damage, 0, maxShield);


	// IF SHIELD HEALTH IS DEPLETED - hide shield health widget and play montage
	if (UserWidget && currentShield == 0)
	{
		Tags.Add("shieldDown");								// Stops flashlight component line trace hit
		UserWidget->SetVisibility(ESlateVisibility::Hidden);	// Hides shield health widget

		PlayAnimMontage(shieldBreakMontage, 1.0, "None");
	}

	// IF SHIELD HEALTH NOT DEPLETED, 
	if (UserWidget && UserWidget->Implements<UDamageInterface>() && currentShield > -1)
	{
		Execute_FlashlightDamage(UserWidget, currentShield);	// Lower shield health bar widget
	}
	
}


// INTERFACE EVENT - called in FlashlightCharacterBase's WeaponCollision OnBeginOverlap
void AFlashlightEnemybase::MeleeDamage_Implementation()
{
	EnemyDead = true;
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlashlightEnemybase::BulletDamage_Implementation(FName ClosestBoneName, FVector ImpulseDirection)
{
	
	
	if(!EnemyDead)
	{
		EnemyDead = true;
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->AddImpulseToAllBodiesBelow(ImpulseDirection * 1000, ClosestBoneName, true, true);

		for(auto CurrentBoneName : BoneNames)
		{
			if (CurrentBoneName == ClosestBoneName)
			{
				if (ClosestBoneName == "HeadTop_End")
					GetMesh()->HideBoneByName("Head", PBO_None);
				else
					GetMesh()->HideBoneByName(CurrentBoneName, PBO_None);
			}

			// SPAWN BLOOD DECAL AND SKELETAL MESH WITH ONLY HIT BONE VISIBLE HERE //
		}
		
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
		
}



// PLAYS MONTAGE WHEN CALLED
float AFlashlightEnemybase::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	float MontageLength = 0.0f;		// Default to 0, indicating montage did not play
	
	if(AnimMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		
		if(AnimInstance)
		{
			MontageLength = AnimInstance->Montage_Play(AnimMontage, InPlayRate);
			
			if (MontageLength > 0.f && StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}
		}
	}

	return MontageLength;
}



/*void AFlashlightEnemybase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}*/



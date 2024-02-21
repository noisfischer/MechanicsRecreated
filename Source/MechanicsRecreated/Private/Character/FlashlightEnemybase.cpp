// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/FlashlightEnemybase.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"


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


void AFlashlightEnemybase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


//////////// FUNCTIONS //////////////


// INTERFACE EVENT - when mesh is hit by flashlight component line trace (called in FlashlightComponent.cpp)
void AFlashlightEnemybase::FlashlightDamage_Implementation(float Damage)
{
	// UPDATE SHIELD HEALTH - clamped between 0-100
	CurrentShield = FMath::Clamp(CurrentShield - Damage, 0, MaxShield);


	// IF SHIELD HEALTH IS DEPLETED - hide shield health widget and play montage
	if (UserWidget && CurrentShield == 0)
	{
		Tags.Add("shieldDown");								// Stops flashlight component line trace hit
		UserWidget->SetVisibility(ESlateVisibility::Hidden);	// Hides shield health widget

		PlayAnimMontage(ShieldBreakMontage, 1.0, "None");
	}

	// IF SHIELD HEALTH NOT DEPLETED, 
	if (UserWidget && UserWidget->Implements<UDamageInterface>() && CurrentShield > -1)
	{
		Execute_FlashlightDamage(UserWidget, CurrentShield);	// Lower shield health bar widget
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
		if (ClosestBoneName == "Head" || ClosestBoneName == "HeadTop_End")
			{
				CurrentHealth = 0;
				EnemyDead = true;
				GetMesh()->SetSimulatePhysics(true);
			}
		

		else
			{
				CurrentHealth -= MaxHealth/4;
				
			
				if (CurrentHealth <= 0)
				{
					EnemyDead = true;
					GetMesh()->SetSimulatePhysics(true);
				}
			}
	}
	

	if(EnemyDead)
	{
		GetMesh()->AddImpulseToAllBodiesBelow(ImpulseDirection * 750, ClosestBoneName, true, true);

		for(auto CurrentBoneName : BoneNames)
		{
			if (CurrentBoneName == ClosestBoneName)
			{
				if (ClosestBoneName == "HeadTop_End")
					GetMesh()->HideBoneByName("Head", PBO_None);
				else
					GetMesh()->HideBoneByName(CurrentBoneName, PBO_None);
			}
		}
	}

	if(BloodDecal)
		SpawnDecal(ClosestBoneName);
	else
		UE_LOG(LogTemp, Error, TEXT("Please assign a value to enemy's BloodDecal material"));
		
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

void AFlashlightEnemybase::SpawnDecal(FName BoneName)
{
	FVector StartLocation = GetMesh()->GetBoneLocation(BoneName);
	FVector EndLocation {StartLocation.X, StartLocation.Y, StartLocation.Z - 1000};
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		FVector DecalSize = FVector(15.0f, 100.0f, 100.0f); // Adjust the size to fit your needs
		FRotator DecalRotation = HitResult.ImpactNormal.Rotation();
		DecalRotation.Pitch += 180.0f;
		
		// Spawn the decal at the bone's location
		UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BloodDecal, DecalSize, HitResult.Location, DecalRotation, 0.0f); // 10 seconds lifespan, adjust as needed
	}
}

	


/*void AFlashlightEnemybase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}*/



// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/FlashlightEnemybase.h"

// Sets default values
AFlashlightEnemybase::AFlashlightEnemybase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add custom tag for FlashlightComponent to recognize
	Tags.Add("enemy");
	
	// Initialize shield health widget component and attach to mesh
	ShieldWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ShieldWidget"));
	ShieldWidget->SetupAttachment(this->GetMesh());
	ShieldWidget->SetWidgetSpace(EWidgetSpace::World);

}

// Called when the game starts or when spawned
void AFlashlightEnemybase::BeginPlay()
{
	Super::BeginPlay();

	// Assign specific User Widget to widget component
	UserWidget = Cast<UUserWidget>(ShieldWidget->GetUserWidgetObject());
	
}

// Called every frame
void AFlashlightEnemybase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AFlashlightEnemybase::FlashlightDamage_Implementation(float Damage)
{
	currentShield = FMath::Clamp(currentShield - Damage, 0, maxShield);	// Clamp currentShield value to min 0

	if (currentShield == 0)
	{
		Tags.Add("shieldDown");								// Stops flashlight component line trace hit
		UserWidget->SetVisibility(ESlateVisibility::Hidden);	// Hides shield health widget

		PlayAnimMontage(shieldBreakMontage, 1.0, "None");
	}

	if (UserWidget && UserWidget->Implements<UDamageInterface>() && currentShield > -1)
	{
		Execute_FlashlightDamage(UserWidget, currentShield);	// Lower shield health bar widget
	}
	
}

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


void AFlashlightEnemybase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}



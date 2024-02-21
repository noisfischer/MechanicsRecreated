// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/FlashlightCharacterBase.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/FlashlightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SimpleReticle.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);


// CONSTRUCTOR INITIALIZATION
AFlashlightCharacterBase::AFlashlightCharacterBase()
{

	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// CAMERA SETTINGS - player always faces forward
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false; 
	GetCharacterMovement()->bUseControllerDesiredRotation = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 


	// SET DEFAULT CHAR MOVEMENT VALUES
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	
	// SETUP CAMERA BOOM COMPONENT AND SET DEFAULT VALUES
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// SETUP FOLLOW CAMERA AND SET DEFAULT VALUES
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	

	// SETUP FLASHLIGHT MESH COMPONENT ATTACH TO MESH SKELETON SOCKET
	FlashlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightMesh"));
	FlashlightMesh->SetupAttachment(GetMesh(), "LeftHandSocket");

	
	// SETUP SPOTLIGHT COMPONENT AND SET DEFAULT VALUES
	FlashlightSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightSpotlight"));
	FlashlightSpotLight->SetupAttachment(FlashlightMesh, "Bulb");
	FlashlightSpotLight->Intensity = StartLightIntensity;
	FlashlightSpotLight->InnerConeAngle = StartInnerConeAngle;
	FlashlightSpotLight->OuterConeAngle = StartOuterConeAngle;
	FlashlightSpotLight->AttenuationRadius = 1000.0f;
	

	// SETUP WEAPON MESH COMPONENT
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), "RightHandSocket");

	// SETUP WEAPON COLLISION COMPONENT
	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

	// SETUP & DEACTIVATE FLASHLIGHT ACTOR COMPONENT - performs tick line trace when activated
	FlashlightComponent = CreateDefaultSubobject<UFlashlightComponent>(TEXT("Flashlight"));
	
}



void AFlashlightCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	// SETUP INPUT MAPPINGS
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
	

	// BINDING FOR WEAPON COLLISION OnBeginOverlap
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AFlashlightCharacterBase::OnWeaponCollisionOverlap);
	

	// RETRIEVE PLAYER CHARACTER'S ANIMATION INSTANCE & BIND OnMontageFinished FUNCTION TO MONTAGE COMPLETION
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr)
		AnimInstance->OnMontageEnded.AddDynamic(this, &AFlashlightCharacterBase::OnMontageFinished);


	// INIT TIMELINES
	if (TimelineCurve)
	{
		// USE FLASHLIGHT //
		FOnTimelineFloat FlashlightTimelineProgress;
		FlashlightTimelineProgress.BindUFunction(this, FName("FlashlightTimelineProgress"));
        
		FlashlightTimeline.AddInterpFloat(TimelineCurve, FlashlightTimelineProgress);
		
		FOnTimelineEvent OnFlashlightTimelineFinished;
		OnFlashlightTimelineFinished.BindUFunction(this, FName("FlashlightTimelineFinished"));
		FlashlightTimeline.SetTimelineFinishedFunc(OnFlashlightTimelineFinished);

		FlashlightTimeline.SetLooping(false);

		// AIM //
		FOnTimelineFloat AimTimelineProgress;
		AimTimelineProgress.BindUFunction(this, FName("AimTimelineProgress"));

		AimTimeline.AddInterpFloat(TimelineCurve,AimTimelineProgress);

		FOnTimelineEvent OnAimTimelineFinished;
		OnAimTimelineFinished.BindUFunction(this, FName("AimTimelineFinished"));
		AimTimeline.SetTimelineFinishedFunc(OnAimTimelineFinished);

		AimTimeline.SetLooping(false);
	}


	// CREATE AIM WIDGET
	if (AimWidgetClass != nullptr)
	{
		AimWidget = CreateWidget<UUserWidget>(GetWorld(), AimWidgetClass);
		if (AimWidget != nullptr)
		{
			AimWidget->AddToViewport();
			AimWidget->SetVisibility(ESlateVisibility::Hidden); // Initially hidden
		}
	}
	
	FlashlightSpotLight->SetIntensity(StartLightIntensity);
	FlashlightSpotLight->SetOuterConeAngle(StartOuterConeAngle);
	FlashlightSpotLight->SetInnerConeAngle(StartInnerConeAngle);
	
}


void AFlashlightCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFlashlightActive)
		FlashlightTimeline.TickTimeline(DeltaTime);
	
	if (bAimActive)
		AimTimeline.TickTimeline(DeltaTime);
}


//////// INPUT ////////

void AFlashlightCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFlashlightCharacterBase::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFlashlightCharacterBase::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! "), *GetNameSafe(this));
	}

	// BIND INPUT ACTIONS CREATE IN PROJECT SETTINGS TO FUNCTIONS
	PlayerInputComponent->BindAction("UseFlashlight", IE_Pressed, this, &AFlashlightCharacterBase::UseFlashlight);
	PlayerInputComponent->BindAction("UseFlashlight", IE_Released, this, &AFlashlightCharacterBase::StopUsingFlashlight);

	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &AFlashlightCharacterBase::Melee);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AFlashlightCharacterBase::Shoot);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AFlashlightCharacterBase::StartAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AFlashlightCharacterBase::StopAim);
	
}

void AFlashlightCharacterBase::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFlashlightCharacterBase::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}



// FOR PLAYING MONTAGES USING PLAYER CHARACTER'S ANIMINSTANCE //
float AFlashlightCharacterBase::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	float MontageLength = 0.0f;	// Default length
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



// MELEE //
void AFlashlightCharacterBase::Melee()
{
	if(!bAttacking && !bAiming)
	{
		bAttacking = true;
		PlayAnimMontage(MeleeMontage, MeleeMontageSpeed, FName("None"));
	}
}

// ACTIVATE WEAPON COLLISION COMPONENT - called in MeleeAnimNotify.cpp
void AFlashlightCharacterBase::ActivateWeapon()
{
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// DEACTIVATE WEAPON COLLISION COMPONENT - called in MeleeAnimNotify.cpp
void AFlashlightCharacterBase::DeactivateWeapon()
{
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// BOUND TO WEAPON COLLISION COMPONENT'S OnBeginOverlap EVENT
void AFlashlightCharacterBase::OnWeaponCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor->ActorHasTag("shieldDown") && OtherActor->Implements<UDamageInterface>())
	{
		Execute_MeleeDamage(OtherActor);	// INTERFACE EVENT CALLED IN OtherActor OBJECT - FlashlightEnemyBase.cpp
	}
}

// ALLOWS PLAYER TO MELEE AGAIN ONCE MELEE MONTAGE IS COMPLETE
void AFlashlightCharacterBase::OnMontageFinished(UAnimMontage* Montage, bool bMontageInterrupted)
{
	bAttacking = false;
}


// SHOOT & AIM //

void AFlashlightCharacterBase::Shoot()
{
	if(bAiming)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			MuzzleFlash,
			WeaponMesh->GetSocketLocation("Barrel"),
			WeaponMesh->GetSocketRotation("Barrel"),
			MuzzleFlashSize,
			true
			);

		if (PistolCameraShake)
		{
			UGameplayStatics::PlayWorldCameraShake(
				GetWorld(),
				PistolCameraShake,
				GetActorLocation(),
				0.f,
				500.f,
				1.f,
				false
				);
		}
	
	
		FVector StartLocation = FollowCamera->GetComponentLocation();
		FVector ForwardVector = FollowCamera->GetForwardVector();
		FVector EndLocation = StartLocation + (ForwardVector * 10000);

		/*DrawDebugLine(
					GetWorld(),
					StartLocation,
					EndLocation,
					FColor::Red, // Green color for the line
					false,
					0.2f,
					0, // Depth priority
					1.0f
				);*/

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Camera, QueryParams))
		{

			
			AActor* HitActor = HitResult.GetActor();
			if (HitActor && HitActor->ActorHasTag(FName("shieldDown")) && HitActor->GetClass()->ImplementsInterface(UDamageInterface::StaticClass()))
			{
				USkeletalMeshComponent* EnemyMesh = Cast<USkeletalMeshComponent>(HitResult.Component.Get());
				FName ClosestBoneName = EnemyMesh->FindClosestBone(HitResult.Location);
				// UE_LOG(LogTemp, Warning, TEXT("%s"), *ClosestBoneName.ToString());
				Execute_BulletDamage(HitActor, ClosestBoneName, GetActorForwardVector());

				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					BulletImpact,
					HitResult.ImpactPoint,
					HitResult.ImpactNormal.Rotation(),
					BulletImpactSize,
			true
					);
			}
		}
	}
}

void AFlashlightCharacterBase::StartAim()
{
	if (!bAttacking && !bUsingFlashlight && TimelineCurve)
	{
		bAiming = true;		// FOR TIMELINE FINISHED
		bAimActive = true;	// TO ACTIVATE TIMELINE TICK

		PlayAnimMontage(WeaponAimMontage, 0.01, FName("None"));
		
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("AimTimelineProgress"));
		AimTimeline.AddInterpFloat(TimelineCurve, ProgressFunction);
		AimTimeline.SetLooping(false);
		AimTimeline.Play();

		if (AimWidget != nullptr)
		{
			AimWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void AFlashlightCharacterBase::StopAim()
{
	if (bAiming)
	{
		bAiming = false;		// FOR TIMELINE FINISHED
		
		AimTimeline.Reverse();	// REVERSE AimTimeline FROM CURRENT POSITION

		// Get the animation instance of the character
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			// Stop the aim montage with a blend out time, e.g., 0.25 seconds
			AnimInstance->Montage_Stop(0.25f, WeaponAimMontage);
		}
		if (AimWidget != nullptr)
		{
			AimWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFlashlightCharacterBase::AimTimelineProgress(float Alpha)
{
	float NewFOV = FMath::Lerp(StartFOV, EndFOV, Alpha);
	FollowCamera->SetFieldOfView(NewFOV);

	float NewCameraPosition = FMath::Lerp(CameraBoom->SocketOffset.Y, EndCameraPosition, Alpha);
	CameraBoom->SocketOffset.Y = NewCameraPosition;
}

void AFlashlightCharacterBase::AimTimelineFinished()
{
	if (!bAiming)
		bAimActive = false;
}



////// FLASHLIGHT COMPONENT FUNCTIONALITY ////////

// AIM FLASHLIGHT START - activates flashlight component 
void AFlashlightCharacterBase::UseFlashlight()
{
	if (!bAttacking && !bAiming && FlashlightComponent && TimelineCurve)
	{
		bUsingFlashlight = true;
		bFlashlightActive = true;	// FOR TIMELINE TICK ACTIVATION
		
		PlayAnimMontage(FlashlightAimMontage, 0.01, FName("None"));
		
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("FlashlightTimelineProgress"));
		FlashlightTimeline.AddInterpFloat(TimelineCurve, ProgressFunction);
		FlashlightTimeline.SetLooping(false);
		FlashlightTimeline.Play();
	}
	
}

// AIM FLASHLIGHT END - deactivates flashlight component
void AFlashlightCharacterBase::StopUsingFlashlight()
{
	if (bUsingFlashlight && FlashlightComponent && TimelineCurve)
	{
		bUsingFlashlight = false;
		
		FlashlightComponent->Deactivate();
		FlashlightComponent->SetComponentTickEnabled(false);

		FlashlightTimeline.Reverse();	// REVERSE AimTimeline FROM CURRENT POSITION

		// Get the animation instance of the character
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			// Stop the aim montage with a blend out time, e.g., 0.25 seconds
			AnimInstance->Montage_Stop(0.25f, FlashlightAimMontage);
		}
	}
}

// UPDATES CAMERA & FLASHLIGHT PROPERTIES EVERY TICK OF AIM TIMELINE - called in Tick Function
void AFlashlightCharacterBase::FlashlightTimelineProgress(float Alpha)
{
	float NewFOV = FMath::Lerp(StartFOV, EndFOV, Alpha);
	FollowCamera->SetFieldOfView(NewFOV);
	
	float NewIntensity = FMath::Lerp(StartLightIntensity, EndLightIntensity, Alpha);
	FlashlightSpotLight->SetIntensity(NewIntensity);
	
	float NewOuterConeAngle = FMath::Lerp(StartOuterConeAngle, EndOuterConeAngle, Alpha);
	FlashlightSpotLight->SetOuterConeAngle(NewOuterConeAngle);
	
	float NewInnerConeAngle = FMath::Lerp(StartInnerConeAngle, EndInnerConeAngle, Alpha);
	FlashlightSpotLight->SetInnerConeAngle(NewInnerConeAngle);

	float NewCameraPosition = FMath::Lerp(CameraBoom->SocketOffset.Y, EndCameraPosition, Alpha);
	CameraBoom->SocketOffset.Y = NewCameraPosition;
}

// HANDLES WHEN AIM TIMELINE FULLY PLAYS/REVERSES
void AFlashlightCharacterBase::FlashlightTimelineFinished()
{
	if(bUsingFlashlight)				// ENABLES AIM TIMELINE TICK
	{
		FlashlightComponent->Activate();
		FlashlightComponent->SetComponentTickEnabled(true);
	}

	else
		bFlashlightActive = false;		// DISABLES AIM TIMELINE TICK
}
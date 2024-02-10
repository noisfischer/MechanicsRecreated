// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/FlashlightCharacterBase.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/FlashlightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SpotLightComponent.h"


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
	// FlashlightComponent->Deactivate();
	// FlashlightComponent->SetComponentTickEnabled(false);
	
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


	if (AimTimelineCurve)
	{
		FOnTimelineFloat onTimelineProgress;
		onTimelineProgress.BindUFunction(this, FName("HandleTimelineProgress"));
        
		AimTimeline.AddInterpFloat(AimTimelineCurve, onTimelineProgress);
        
		// Bind the finished function
		FOnTimelineEvent onTimelineFinished;
		onTimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));
		AimTimeline.SetTimelineFinishedFunc(onTimelineFinished);

		AimTimeline.SetLooping(false);
	}
	
}


void AFlashlightCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AimTimeline.TickTimeline(DeltaTime);
}


//////// INPUT ////////

void AFlashlightCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFlashlightCharacterBase::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFlashlightCharacterBase::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// BIND INPUT ACTIONS CREATE IN PROJECT SETTINGS TO FUNCTIONS
	PlayerInputComponent->BindAction("UseFlashlight", IE_Pressed, this, &AFlashlightCharacterBase::UseFlashlight);
	PlayerInputComponent->BindAction("UseFlashlight", IE_Released, this, &AFlashlightCharacterBase::StopUsingFlashlight);
	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &AFlashlightCharacterBase::Melee);
}

void AFlashlightCharacterBase::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
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



/////// FUNCTION IMPLEMENTATIONS ///////


// AIM FLASHLIGHT START - activates flashlight component 
void AFlashlightCharacterBase::UseFlashlight()
{
	if (!IsAttacking && FlashlightComponent && AimTimelineCurve)
	{
		IsAiming = true;
		PlayAnimMontage(AimMontage, 0.01, FName("None"));
		
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleTimelineProgress"));
		AimTimeline.AddInterpFloat(AimTimelineCurve, ProgressFunction);
		AimTimeline.SetLooping(false);
		AimTimeline.Play();
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Use Flashlight condition failed!"));
	}
	
}

// AIM FLASHLIGHT END - deactivates flashlight component
void AFlashlightCharacterBase::StopUsingFlashlight()
{
	if (IsAiming)
	{
		IsAiming = false;
		FlashlightComponent->Deactivate();
		FlashlightComponent->SetComponentTickEnabled(false);

		AimTimeline.Reverse();
	}
}


// FOR PLAYING MONTAGES USING PLAYER CHARACTER'S ANIMINSTANCE
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


// PLAY MELEE MONTAGE - after passing conditions
void AFlashlightCharacterBase::Melee()
{
	if(!IsAttacking && !IsAiming)
	{
		IsAttacking = true;
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
		IsAttacking = false;
}

// UPDATES CAMERA & FLASHLIGHT PROPERTIES EVERY TICK DURING AIM TIMELINE
void AFlashlightCharacterBase::HandleTimelineProgress(float Alpha)
{
	// LERP the FOV from start to end using the timeline's alpha
	float NewFOV = FMath::Lerp(StartFOV, EndFOV, Alpha);
	FollowCamera->SetFieldOfView(NewFOV);

	// LERP the flashlight intensity
	float NewIntensity = FMath::Lerp(StartLightIntensity, EndLightIntensity, Alpha);
	FlashlightSpotLight->SetIntensity(NewIntensity);

	// LERP the outer cone angle
	float NewOuterConeAngle = FMath::Lerp(StartOuterConeAngle, EndOuterConeAngle, Alpha);
	FlashlightSpotLight->SetOuterConeAngle(NewOuterConeAngle);
	
	float NewInnerConeAngle = FMath::Lerp(StartInnerConeAngle, EndInnerConeAngle, Alpha);
	FlashlightSpotLight->SetInnerConeAngle(NewInnerConeAngle);
}


void AFlashlightCharacterBase::OnTimelineFinished()
{
	if(IsAiming)
	{
		FlashlightComponent->Activate();
		FlashlightComponent->SetComponentTickEnabled(true);
	}
}

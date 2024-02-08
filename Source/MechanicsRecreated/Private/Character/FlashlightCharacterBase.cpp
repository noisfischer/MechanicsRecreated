// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/FlashlightCharacterBase.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SpotLightComponent.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AFlashlightCharacterBase::AFlashlightCharacterBase()
{

	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character uses controller rotation
	GetCharacterMovement()->bUseControllerDesiredRotation = true; // Character uses controller rotation
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Initialize the static mesh components and attach them to the skeletal mesh in left hand socket
	FlashlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightMesh"));
	FlashlightMesh->SetupAttachment(GetMesh(), "LeftHandSocket");

	// Initialize spotlight and attach it to front of flashlight mesh
	FlashlightSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightSpotlight"));
	FlashlightSpotLight->SetupAttachment(FlashlightMesh, "Bulb");
	
	// Default spotlight settings
	FlashlightSpotLight->Intensity = 10000.0f;
	FlashlightSpotLight->InnerConeAngle = 10.0f;
	FlashlightSpotLight->OuterConeAngle = 20.0f;
	FlashlightSpotLight->AttenuationRadius = 1000.0f;

	// Initialize weapon and attach it skeletal mesh in right hand socket
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), "RightHandSocket");
	
	// Initialize weapon capsule collision
	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);

	// Initialize flashlight component
	FlashlightComponent = CreateDefaultSubobject<UFlashlightComponent>(TEXT("Flashlight"));
}


void AFlashlightCharacterBase::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Deactivate flashlight component and weapon collision
	FlashlightComponent->Deactivate();
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Activate weapon collision
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AFlashlightCharacterBase::OnWeaponCollisionOverlap);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AFlashlightCharacterBase::OnMontageFinished);
	}
}




void AFlashlightCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


//////////////////////////////////////////////////////////////////////////
// Input

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

	// Bind the input actions
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
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


// Flashlight Ability using Left Click
void AFlashlightCharacterBase::UseFlashlight()
{
	if (!IsAttacking)
	{
		IsAiming = true;
		PlayAnimMontage(AimMontage, 0.01, FName("None"));
		FlashlightComponent->Activate();
	}
	
}

void AFlashlightCharacterBase::StopUsingFlashlight()
{
	if (FlashlightComponent->IsActive())
	{
		IsAiming = false;
		FlashlightComponent->Deactivate();
	}
}


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


/////////////////////////// MELEE ////////////////////////////////

// Melee using right click
void AFlashlightCharacterBase::Melee()
{
	if(!IsAttacking && !IsAiming)
	{
		WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		IsAttacking = true;
		PlayAnimMontage(MeleeMontage, MeleeMontageSpeed, FName("None"));
	}
}


// Bound to WeaponCollision capsule collision component on Weapon Mesh (look at BeginPlay)
void AFlashlightCharacterBase::OnWeaponCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor->ActorHasTag("shieldDown"))
	{
		Execute_MeleeDamage(OtherActor);
	}
}


void AFlashlightCharacterBase::OnMontageFinished(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == MeleeMontage || bInterrupted)
	{
		// Here you can set your variable, disable collision, or perform any other necessary actions.
		WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		IsAttacking = false;
	}
}


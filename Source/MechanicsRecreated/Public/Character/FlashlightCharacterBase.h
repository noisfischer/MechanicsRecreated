#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/FlashlightComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "Interfaces/DamageInterface.h"
#include "FlashlightCharacterBase.generated.h"

class UFlashlightComponent;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class USpotLightComponent;
class UCapsuleComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class MECHANICSRECREATED_API AFlashlightCharacterBase : public ACharacter, public IDamageInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* FlashlightMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UFlashlightComponent* FlashlightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* WeaponCollision;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	
public:
	// CONSTRUCTOR
	AFlashlightCharacterBase();
	

	
	/////// MEMBERS /////////
	
	// DEFAULT CAMERA PROPERTIES FOR AIM TIMELINE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float StartFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float EndFOV = 70.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float StartCameraPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float EndCameraPosition = 20.0f;

	
	// SPOTLIGHT FOR FLASHLIGHT MESH
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flashlight")
	USpotLightComponent* FlashlightSpotLight;

	

	// TIMELINE TO CONTROL CAMERA & FLASHLIGHT PROPERTIES
	UPROPERTY()
	FTimeline FlashlightTimeline;

	UPROPERTY()
	FTimeline AimTimeline;

	
	// FLOAT CURVE MADE IN UE EDITOR TO BE ASSIGNED IN CHILD BP
	UPROPERTY(EditAnywhere, Category="Timelines")
	UCurveFloat* TimelineCurve;
	
	// DEFAULT SPOTLIGHT PROPERTIES FOR AIM TIMELINE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float StartLightIntensity = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float EndLightIntensity = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float StartOuterConeAngle = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float EndOuterConeAngle = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float StartInnerConeAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float EndInnerConeAngle = 1.0f;


	
	// MONTAGE DECLARATIONS/INITIALIZATIONS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	float MeleeMontageSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* FlashlightAimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* WeaponAimMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FVector MuzzleFlashSize {.1, .1, .1};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TSubclassOf<UCameraShakeBase> PistolCameraShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	UParticleSystem* BulletImpact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FVector BulletImpactSize {1, 1, 1};
	

	// UI //
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> AimWidgetClass;
	
	UPROPERTY()
	UUserWidget* AimWidget;

	
	////////// FUNCTIONS /////////////
	
	// HANDLES WEAPON CAPSULE COLLISION COMPONENT - BOUND TO WeaponCollision COMPONENT ON BEGIN PLAY
	UFUNCTION()
	void OnWeaponCollisionOverlap
		(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	
	// BOUND TO UAnimInstance IN CPP - DEFINES WHAT HAPPENS WHEN MONTAGE IS FINISHED PLAYING
	UFUNCTION()
	void OnMontageFinished(UAnimMontage* Montage, bool bMontageInterrupted);
	

	// OVERRIDEN FUNCTION FOR PLAYING A MONTAGE ON A VALID UAnimInstance
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName) override;
	

	// ACTIVATE/DEACTIVATE WeaponCollision COMPONENT - CALLED IN MeleeAnimNotify.cpp
	virtual void ActivateWeapon();
	virtual void DeactivateWeapon();


	// TIMELINE FUNCTIONS
	UFUNCTION()
	void FlashlightTimelineProgress(float Alpha);	// CALLED IN TICK FUNCTION
	UFUNCTION()
	void FlashlightTimelineFinished();					// CALLED WHEN TIMELINE IS DONE PLAYING/REVERSING

	UFUNCTION()
	void AimTimelineProgress(float Alpha);	// CALLED IN TICK FUNCTION
	UFUNCTION()
	void AimTimelineFinished();	
	

	virtual void Tick(float DeltaTime) override;


private:

	// CONDITIONALS
	bool bAttacking = false;
	bool bAiming = false;
	bool bAimActive = false;
	bool bUsingFlashlight = false;
	bool bFlashlightActive = false;
	
	
	// INPUT ACTIONS SPECIFIED IN PROJECT INPUT SETTINGS
	// BINDINGS IN SetupPlayerInputComponent IMPLEMENTATION IN CPP
	void UseFlashlight();			// PRESSED MIDDLE MOUSE BUTTON
	void StopUsingFlashlight();		// RELEASED MIDDLE MOUSE BUTTON
	void Melee();					// PRESSED F BUTTON
	void Shoot();					// PRESSED LEFT MOUSE BUTTON
	void StartAim();				// PRESSED RIGHT MOUSE BUTTON
	void StopAim();					// RELEASED RIGHT MOUSE BUTTON
	
protected:
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	
public:	
	
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};

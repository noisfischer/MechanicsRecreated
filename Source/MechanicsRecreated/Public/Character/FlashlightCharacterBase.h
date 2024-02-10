#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/FlashlightComponent.h"
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
	float StartCameraPosition = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float EndCameraPosition = 70.0f;

	
	// SPOTLIGHT FOR FLASHLIGHT MESH
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flashlight")
	USpotLightComponent* FlashlightSpotLight;

	
	// DEFAULT SPOTLIGHT PROPERTIES FOR AIM TIMELINE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float StartLightIntensity = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float EndLightIntensity = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float StartOuterConeAngle = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float EndOuterConeAngle = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SpotLightProperties")
	float StartInnerConeAngle = 10.0f;

	
	// MONTAGE DECLARATIONS/INITIALIZATIONS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	float MeleeMontageSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* AimMontage;

	
	// CONDITIONALS TO PREVENT MONTAGE INTERRUPTIONS - no spamming melee 
	bool IsAttacking = false;
	bool IsAiming = false;

	

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

	virtual void Tick(float DeltaTime) override;


private:
	// INPUT ACTIONS SPECIFIED IN PROJECT INPUT SETTINGS
	// BINDINGS IN SetupPlayerInputComponent IMPLEMENTATION IN CPP
	void UseFlashlight();			// PRESSED LEFT MOUSE BUTTON
	void StopUsingFlashlight();		// RELEASED LEFT MOUSE BUTTON
	void Melee();					// PRESSED RIGHT MOUSE BUTTON

protected:
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	
public:	
	
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};

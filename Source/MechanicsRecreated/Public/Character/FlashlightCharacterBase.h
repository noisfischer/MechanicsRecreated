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
	// Sets default values for this character's properties
	AFlashlightCharacterBase();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Flashlight")
	USpotLightComponent* FlashlightSpotLight;

	// Variables for Blueprint Timeline for UseFlashlight trigger
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float StartFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float EndFOV = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float StartCameraPosition = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CameraProperties")
	float EndCameraPosition = 70.0f;

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

	// Assign Montages for Aim & Melee
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	float MeleeMontageSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* AimMontage;

	UFUNCTION()
	void OnWeaponCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnMontageFinished(UAnimMontage* Montage, bool bInterrupted);
	
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName) override;
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// Input actions specified in editor Project Settings
	void UseFlashlight();			// start left-click
	void StopUsingFlashlight();		// end left-click
	void Melee();					// right-click
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;


private:
	bool IsAttacking = false;
	bool IsAiming = false;
	
public:	
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};

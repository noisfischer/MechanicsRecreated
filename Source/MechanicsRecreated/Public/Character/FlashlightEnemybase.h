// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/DamageInterface.h"
#include "FlashlightEnemybase.generated.h"

UCLASS()
class MECHANICSRECREATED_API AFlashlightEnemybase : public ACharacter, public IDamageInterface
{
	GENERATED_BODY()

	// Declare widget component for shield health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* ShieldWidget;
	
public:
	// Sets default values for this character's properties
	AFlashlightEnemybase();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	float MaxShield = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	float CurrentShield = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	float MaxHealth = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	float CurrentHealth = 100;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")	// Reaction on shield break assigned in child class
	UAnimMontage* ShieldBreakMontage;

	UPROPERTY(VisibleAnywhere)
	bool EnemyDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	UMaterialInterface* BloodDecal;

private:
	UPROPERTY(VisibleAnywhere)
	UUserWidget* UserWidget;	// Actual widget displayed over enemy head. Assigned in child class's widget component


	// FOR DISMEMBERMENT //
	TArray<FName> BoneNames {"RightUpLeg", "LeftUpLeg", "RightLeg", "LeftLeg",
		"RightShoulder", "RightArm", "RightForeArm", "LeftShoulder", "LeftArm", "LeftForeArm",
		"Head", "HeadTop_End"};
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// Enemy receives shield damage as does widget health bar
	virtual void FlashlightDamage_Implementation(float Damage) override;

	// Enemy receives melee damage if widget health bar is depleted
	virtual void MeleeDamage_Implementation() override;

	// Enemy receives bullet damage if widget health bar is depleted
	virtual void BulletDamage_Implementation(FName ClosestBoneName, FVector ImpulseDirection) override;

	// Plays hit reaction when enemy's shield is broken
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName) override;

	virtual void SpawnDecal(FName BoneName);
	
	/*virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;*/

};

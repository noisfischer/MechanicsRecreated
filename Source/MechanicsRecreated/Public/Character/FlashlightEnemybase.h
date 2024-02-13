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
	float maxShield = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	float currentShield = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")	// Reaction on shield break assigned in child class
	UAnimMontage* shieldBreakMontage;

	UPROPERTY(VisibleAnywhere)
	bool EnemyDead = false;

private:
	UPROPERTY(VisibleAnywhere)
	UUserWidget* UserWidget;	// Actual widget displayed over enemy head. Assigned in child class's widget component

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
	virtual void BulletDamage_Implementation() override;

	// Plays hit reaction when enemy's shield is broken
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName) override;

	/*virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;*/

};

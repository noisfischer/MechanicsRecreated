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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	UAnimMontage* shieldBreakMontage;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1 - HEALTH")
	UUserWidget* UserWidget;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void FlashlightDamage_Implementation(float Damage) override;

	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName) override;

};

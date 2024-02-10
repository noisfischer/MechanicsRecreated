// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlashlightCharacterBase.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MeleeAnimNotify.generated.h"

// DECLARATIONS
class AFlashlightCharacterBase;

UCLASS()
class MECHANICSRECREATED_API UMeleeAnimNotify : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	// DECLARE CHARACTER POINTER
	UPROPERTY()
	AFlashlightCharacterBase* FlashlightCharacter;
	
	// OVERRIDEN FROM ANIMNOTIFY.H
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference) override;
	
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MeleeAnimNotify.h"

void UMeleeAnimNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	// ENSURE MESH IS VALID
	if (MeshComp && MeshComp->GetOwner())
	{
		// FOR SINGLE PLAYER ONLY - CAST TO PLAYER CHARACTER
		FlashlightCharacter = Cast<AFlashlightCharacterBase>(MeshComp->GetOwner());

		// IF PLAYER CHARACTER IS VALID, ACTIVATE WEAPON COLLISION
		if (FlashlightCharacter)
		{
			FlashlightCharacter->ActivateWeapon();	
		}
	}
}

void UMeleeAnimNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
		if (MeshComp && MeshComp->GetOwner())
		{
			FlashlightCharacter = Cast<AFlashlightCharacterBase>(MeshComp->GetOwner());

			if (FlashlightCharacter)
			{
				FlashlightCharacter->DeactivateWeapon();	
			}
		}
}

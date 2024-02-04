// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechanicsRecreatedGameMode.h"
#include "MechanicsRecreatedCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMechanicsRecreatedGameMode::AMechanicsRecreatedGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

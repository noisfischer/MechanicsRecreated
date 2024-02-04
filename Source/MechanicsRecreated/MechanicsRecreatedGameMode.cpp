// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechanicsRecreatedGameMode.h"
#include "UObject/ConstructorHelpers.h"

AMechanicsRecreatedGameMode::AMechanicsRecreatedGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

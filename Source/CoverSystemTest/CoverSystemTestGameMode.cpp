// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CoverSystemTestGameMode.h"
#include "CoverSystemTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACoverSystemTestGameMode::ACoverSystemTestGameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Player/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
}

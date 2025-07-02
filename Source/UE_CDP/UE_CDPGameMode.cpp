// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE_CDPGameMode.h"
#include "UE_CDPCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUE_CDPGameMode::AUE_CDPGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

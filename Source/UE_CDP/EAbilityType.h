#pragma once

#include "EAbilityType.generated.h"

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	DoubleJump	UMETA(DisplayName = "Double Jump"),
	Sprint		UMETA(DisplayName = "Sprint")
};

// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE_CDP : ModuleRules
{
	public UE_CDP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}

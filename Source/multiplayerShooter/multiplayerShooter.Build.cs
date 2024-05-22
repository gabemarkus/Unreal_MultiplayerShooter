// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class multiplayerShooter : ModuleRules
{
	public multiplayerShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "AkAudio", "WwiseSoundEngine" });
	}
}

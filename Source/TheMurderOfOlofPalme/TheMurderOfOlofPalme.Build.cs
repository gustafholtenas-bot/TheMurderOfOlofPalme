// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheMurderOfOlofPalme : ModuleRules
{
	public TheMurderOfOlofPalme(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
   			"Json",
    			"JsonUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TheMurderOfOlofPalme",
			"TheMurderOfOlofPalme/Variant_Platforming",
			"TheMurderOfOlofPalme/Variant_Platforming/Animation",
			"TheMurderOfOlofPalme/Variant_Combat",
			"TheMurderOfOlofPalme/Variant_Combat/AI",
			"TheMurderOfOlofPalme/Variant_Combat/Animation",
			"TheMurderOfOlofPalme/Variant_Combat/Gameplay",
			"TheMurderOfOlofPalme/Variant_Combat/Interfaces",
			"TheMurderOfOlofPalme/Variant_Combat/UI",
			"TheMurderOfOlofPalme/Variant_SideScrolling",
			"TheMurderOfOlofPalme/Variant_SideScrolling/AI",
			"TheMurderOfOlofPalme/Variant_SideScrolling/Gameplay",
			"TheMurderOfOlofPalme/Variant_SideScrolling/Interfaces",
			"TheMurderOfOlofPalme/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

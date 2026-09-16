// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Local_AdventureGame : ModuleRules
{
	public Local_AdventureGame(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Local_AdventureGame",
			"Local_AdventureGame/Variant_Horror",
			"Local_AdventureGame/Variant_Horror/UI",
			"Local_AdventureGame/Variant_Shooter",
			"Local_AdventureGame/Variant_Shooter/AI",
			"Local_AdventureGame/Variant_Shooter/UI",
			"Local_AdventureGame/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

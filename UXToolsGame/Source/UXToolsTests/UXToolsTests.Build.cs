// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using UnrealBuildTool;

public class UXToolsTests : ModuleRules
{
	public UXToolsTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Required to avoid errors about undefined preprocessor macros (C4668) when building DirectXMath.h
        bEnableUndefinedIdentifierWarnings = false;
	
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "LiveLinkInterface", "UXTools" });

		if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
	}
}


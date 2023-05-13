// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NeovimSourceCodeAccessor : ModuleRules
{
	public NeovimSourceCodeAccessor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
        // comment these if you need to debug
        //bUsePrecompiled = true;
        //PrecompileForTargets = PrecompileTargetsType.Any;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core" });

		PrivateDependencyModuleNames.AddRange(
                new string[] { 
                "CoreUObject",
                "Engine", 
                "Slate", 
                "SlateCore",
                "SourceCodeAccess"
                });
        
        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("GameProjectGeneration");
        }
        
	}
}

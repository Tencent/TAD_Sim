// Copyright 2024 Tencent Inc. All rights reserved.
//

using UnrealBuildTool;

// rtx lidar module
public class RTXLidar : ModuleRules
{
    // rtx lidar module target
    public RTXLidar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // include shared shader code
        PublicIncludePaths.AddRange(
            new string[]
            {
                "../Shaders/Shared"
            });

        // include runtime renderer code
        PrivateIncludePaths.AddRange(
            new string[]
            {
                EngineDirectory + "/Source/Runtime/Renderer/Private",
                EngineDirectory + "/Source/Runtime/RenderCore/Public",
            });

        // add module dependencies
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Projects"
            });

        // add module dependencies
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "RenderCore",
                "Renderer",
                "DeveloperSettings",
                "RHI",
            });

        // add module dependencies
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
        );
    }
}
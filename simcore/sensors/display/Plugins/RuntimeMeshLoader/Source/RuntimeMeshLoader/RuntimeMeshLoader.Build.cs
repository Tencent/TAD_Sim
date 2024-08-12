// Copyright 2024 Tencent Inc. All rights reserved.
//

using UnrealBuildTool;
using System.IO;

public class RuntimeMeshLoader : ModuleRules
{
    public RuntimeMeshLoader(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        var thirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty"));

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(thirdPartyPath, "assimp", "include")
                // ... add public include paths required here ...
            }
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
                // ... add other private include paths required here ...
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "RHI",
                "RenderCore",
                "ProceduralMeshComponent",
                "MeshDescription",
                "StaticMeshDescription",
                "Projects"
                // ... add other public dependencies that you statically link with here ...
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects"
                // ... add private dependencies that you statically link with here ...    
            }
        );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
        );

        PublicDelayLoadDLLs.Add("assimp-vc142-mt.dll");

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(thirdPartyPath, "assimp\\lib", "assimp-vc142-mt.lib"));

            RuntimeDependencies.Add(Path.Combine(thirdPartyPath, "assimp\\bin", "assimp-vc142-mt.dll"));
        }

        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            var AssimpPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "assimp", "lib");
            var AssimpLib = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "assimp", "lib", "libassimp.so");

            PublicAdditionalLibraries.Add(AssimpLib);
            //RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)/ubuntu18_20/", "libassimp.so"),
            //Path.Combine(AssimpPath, "libassimp.so"));
            RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)/ubuntu18_20/", "libassimp.so.5"),
                Path.Combine(AssimpPath, "libassimp.so.5"));
            RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)/ubuntu18_20/", "libminizip.so.1"),
                Path.Combine(AssimpPath, "libminizip.so.1"));
        }
    }
}
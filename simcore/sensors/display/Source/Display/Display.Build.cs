// Copyright 2024 Tencent Inc. All rights reserved.
//

using System.IO;
using UnrealBuildTool;

public class Display : ModuleRules
{
    public Display(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "XmlParser", "OnlineSubsystem",
            "OnlineSubsystemUtils", "CinematicCamera", "Networking", "Sockets", "PhysXVehicles",
            "ProceduralMeshComponent", "Json", "JsonUtilities", "PhysicsCore", "Slate", "SlateCore", "AutoRoad", "Json",
            "RuntimeMeshLoader"
        });

        //Plugin module
        PublicDependencyModuleNames.AddRange(new string[]
            { "HadMap", "Protobuf", "BoostLib", "SunPosition", "MyUDP", "CudaResource" });


        if (Target.Platform == UnrealTargetPlatform.Win64)
            PublicDependencyModuleNames.AddRange(new string[] { "RTXLidar" });

        PrivateDependencyModuleNames.Add("OnlineSubsystem");

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "SimMsg"));

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

        RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "../../Config", "DefaultAutoRoad.ini"));

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
        bEnableExceptions = true;
        // To include OnlineSubsystemSteam, add it to the plugins section
        // in your uproject file with the Enabled attribute set to true
        //bUseRTTI = true;

        //if (Target.Platform == UnrealTargetPlatform.Linux)
        //{
        //    RuntimeDependencies.Add("$(TargetOutputDir)/../../../Display.sh",
        // Path.Combine(Path.Combine(ModuleDirectory, "../../") + "Display.sh"), StagedFileType.NonUFS);
        //}
    }
}
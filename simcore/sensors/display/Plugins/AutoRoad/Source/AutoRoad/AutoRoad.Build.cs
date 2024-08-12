// Copyright 2024 Tencent Inc. All rights reserved.
//

using System.IO;
using UnrealBuildTool;

public class AutoRoad : ModuleRules
{
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    private string LibraryPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "./CoordTrans/Libraries/")); }
    }

    private string IncludesPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "./CoordTrans/Includes/")); }
    }

    public AutoRoad(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        //bUseRTTI = true;

        PublicIncludePaths.AddRange(
            new string[]
            {
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
                "Json",
                "JsonUtilities",
                "XmlParser",
                "ProceduralMeshComponent",
                "HadMap",
                "MeshDescription",
                "StaticMeshDescription",
                "MeshConversion",
                "RuntimeMeshLoader"
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

                // ... add private dependencies that you statically link with here ...    
            }
        );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
        );


        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            // Linking CoordTrans
            // Create CoordTransPath 
            // string CoordTransPath = Path.Combine(ThirdPartyPath, "CoordTrans");
            //
            // // Get Library Path 
            // string LibPath = "";
            //
            // LibPath = Path.Combine(CoordTransPath, "Libraries");
            //
            // //Add Include path 
            // PublicIncludePaths.AddRange(new string[] { Path.Combine(CoordTransPath, "Includes") });
            //
            // // Add Library Path 
            // PublicLibraryPaths.Add(LibPath);

            PublicIncludePaths.AddRange(new string[] { IncludesPath });
            // PublicLibraryPaths.Add(LibraryPath);
            //    PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/coord_trans.lib"));

            //Add Static Libraries
            // PublicAdditionalLibraries.Add("coord_trans.lib");

            // Definitions.Add("WITH_COORDTRANS_BINDING=1");
            PublicDefinitions.Add("WITH_COORDTRANS_BINDING=1");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            bEnableExceptions = true;

            PublicIncludePaths.AddRange(new string[] { IncludesPath });
            //    PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./Linux/Release/libcoord_trans.a"));
            PublicDefinitions.Add("WITH_COORDTRANS_BINDING=1");
        }
    }
}
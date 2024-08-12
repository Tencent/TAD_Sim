// Copyright 2024 Tencent Inc. All rights reserved.
//

using System;
using System.IO;
using UnrealBuildTool;
using Tools.DotNETCommon;

public class BoostLib : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    // get boost include
    public string getBoostInclude(ReadOnlyTargetRules Target)
    {
        // windows
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            // find vcpkg
            if (System.Environment.GetEnvironmentVariable("VCPKG_ROOT", EnvironmentVariableTarget.Machine) != null)
            {
                // vcpkg root
                string vcpkg_root =
                    System.Environment.GetEnvironmentVariable("VCPKG_ROOT", EnvironmentVariableTarget.Machine);

                return Path.Combine(vcpkg_root, "installed/x64-windows/include");
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            // linux
            return Path.GetFullPath(Path.Combine(ModulePath, "../../deps/"));
        }

        return "";
    }

    public BoostLib(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        string boost_path = getBoostInclude(Target);
        Log.TraceWarning("the boost path is : {0}", boost_path);

        PublicIncludePaths.AddRange(
            new string[]
            {
                //"BoostLib/Public",
                Path.Combine(ModulePath, "Public"),
                // ... add public include paths required here ...
            }
        );

        PrivateIncludePaths.AddRange(
            new string[]
            {
                //"BoostLib/Private",
                Path.Combine(ModulePath, "Private"),
                boost_path,
                // ... add other private include paths required here ...
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
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
        LoadLib(Target);
    }

    public bool LoadLib(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = true;
        bUseRTTI = true;
        bEnableExceptions = true;
        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            //PublicDefinitions.Add("BOOST_NO_RTTI");
            //PublicDefinitions.Add("BOOST_NO_TYPEID");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            isLibrarySupported = false;

            //string PlatformString = "Mac";
        }

        return isLibrarySupported;
    }
}
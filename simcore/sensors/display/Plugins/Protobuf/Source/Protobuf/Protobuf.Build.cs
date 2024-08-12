// Copyright 2024 Tencent Inc. All rights reserved.
//

using System.IO;
using UnrealBuildTool;

public class Protobuf : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    private string BinariesPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/")); }
    }

    private string ProtobufLibraryPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "./ProtobufSDK/Lib/")); }
    }

    private string SimmoduleLibraryPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "./SimModuleSDK/Lib/")); }
    }

    public Protobuf(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.AddRange(new string[] { Path.Combine(ThirdPartyPath, "ProtobufSDK", "Inc") });
        //PCHUsage = PCHUsageMode.UseSharedPCHs;
        PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI");
        PublicDefinitions.Add("GOOGLE_PROTOBUF_USE_UNALIGNED=0");
        

        PublicIncludePaths.AddRange(
            new string[]
            {
                //"Protobuf/Public",
                //"Protobuf/SimMsg",
                Path.Combine(ModulePath, "Public"),
                //Path.Combine(ThirdPartyPath, "SimModuleSDK", "Inc"),
                //Path.Combine(ThirdPartyPath, "SimModuleSDK", "Inc/utils")
                // ... add public include paths required here ...
            }
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ModulePath, "Private"),
                //Path.Combine(ThirdPartyPath, "SimModuleSDK", "Src")
                //"Protobuf/Private"
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


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
        );

        LoadLib(Target);

        //PublicDefinitions.Add("ZMQ_STATIC");

        ////Use key word 'noexcept  ' for fix protobuf build error.
        bEnableExceptions = true;
        bUseRTTI = true;
    }

    public bool LoadLib(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;
            // Protobuf
            //PublicLibraryPaths.Add(Path.Combine(ProtobufLibraryPath, "Win64/Release/"));
            PublicAdditionalLibraries.Add(Path.Combine(ProtobufLibraryPath, "Win64/Release/libprotobuf.lib"));
            //if (Target.Version.MinorVersion > 23)
            //{
            //    PublicSystemLibraries.Add("shlwapi.lib");
            //}
            //else
            //{
            //    PublicAdditionalLibraries.Add("shlwapi.lib");
            //}
            PublicSystemLibraries.Add("shlwapi.lib");
            string output_dir = Path.GetFullPath(Path.Combine(
                PluginDirectory, "../../Binaries", Target.Platform.ToString()));
            string x64_path = SimmoduleLibraryPath + "/Win64/Release/";
            string[] files = System.IO.Directory.GetFiles(x64_path);
            foreach (string filepath in files)
            {
                string filename = System.IO.Path.GetFileName(filepath);
                if (System.IO.Path.GetExtension(filename) == ".lib")
                {
                    PublicAdditionalLibraries.Add(Path.Combine(x64_path, filename));
                }
                else
                {
                    RuntimeDependencies.Add(Path.Combine(output_dir, filename),
                        Path.Combine(x64_path, filename));
                }
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            isLibrarySupported = true;

            //PublicLibraryPaths.Add(Path.Combine(ProtobufLibraryPath, "Linux/Release/"));
            // 
            PublicAdditionalLibraries.Add(ProtobufLibraryPath + "Linux/Release/" + "libprotobuf.a");
            PublicDelayLoadDLLs.Add(SimmoduleLibraryPath + "Linux/Release/" + "libtxsim-module-impl.so");
        }

        return isLibrarySupported;
    }
}
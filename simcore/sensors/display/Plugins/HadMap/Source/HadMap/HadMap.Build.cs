// Copyright 2024 Tencent Inc. All rights reserved.
//

using System.IO;
using UnrealBuildTool;
using Tools.DotNETCommon;

public class HadMap : ModuleRules
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

    private string BinariesThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/ThirdParty/")); }
    }

    private string LibraryPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "./HadMapSDK/Lib/")); }
    }

    private string LibraryPathFnn
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "./FLANN/lib/")); }
    }

    public HadMap(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        string MapSDKIncPath;
        MapSDKIncPath = Path.Combine(ThirdPartyPath, "HadMapSDK", "Inc");        

        PublicIncludePaths.AddRange(
            new string[]
            {
                //"HadMap/Public",
                Path.Combine(ModulePath, "Public"),
                //"ThirdParty/HadMapLibrary"
                MapSDKIncPath,
                Path.Combine(ThirdPartyPath, "FLANN", "include"),
                // ... add public include paths required here ...
            }
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
                //"HadMap/Private",
                Path.Combine(ModulePath, "Private"),
                // ... add other private include paths required here ...
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
                "CoreUObject",
                //"HadMapLibrary",
                "Projects"
                // ... add other public dependencies that you statically link with here ...
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine",
                "ProceduralMeshComponent",
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

        LoadLib(Target);
    }

    public bool LoadLib(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;
            bUseRTTI = true;
            bEnableExceptions = true;

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPathFnn, "./flann_s.lib"));

            string output_dir = Path.GetFullPath(Path.Combine(
                PluginDirectory, "../../Binaries", Target.Platform.ToString()));
            string x64_path = LibraryPath + "/x64/Release/";
            string[] files = System.IO.Directory.GetFiles(x64_path);
            foreach (string filepath in files)
            {
                string filename = System.IO.Path.GetFileName(filepath);
                if (System.IO.Path.GetExtension(filename) == ".lib")
                {
                    PublicAdditionalLibraries.Add(Path.Combine(x64_path, filename));
                }
                else if (System.IO.Path.GetExtension(filename) == ".dll")
                {
                    RuntimeDependencies.Add(Path.Combine(output_dir, filename),
                        Path.Combine(x64_path, filename));
                }

            }
            if (Target.bBuildEditor)
            {
                PublicAdditionalLibraries.Add(Path.Combine(x64_path, "./libcurl.dll.a"));
            }
            // MapSDK
            // PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/datamodel.lib"));
            // PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/map_engine.lib"));
            // PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/map_import.lib"));
            // PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/mapdb.lib"));
            // PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/route_plan.lib"));
            // PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/routingmap.lib"));
            // PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "./x64/Release/transmission.lib"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/freexl.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/freexl.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/geos_c.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/geos_c.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/iconv.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/iconv.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/libcrypto-1_1-x64.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/libcrypto-1_1-x64.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/libcurl-x64.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/libcurl-x64.dll"));
            // // RuntimeDependencies.Add("$(TargetOutputDir)/libfftw3f-3.dll",
            // // Path.Combine(LibraryPath, "./x64/Release/libfftw3f-3.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/libssl-1_1-x64.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/libssl-1_1-x64.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/libxml2.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/libxml2.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/proj.dll", 
            //     Path.Combine(LibraryPath, "./x64/Release/proj.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/spatialite.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/spatialite.dll"));
            // RuntimeDependencies.Add("$(TargetOutputDir)/sqlite3.dll",
            //     Path.Combine(LibraryPath, "./x64/Release/sqlite3.dll"));

        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            isLibrarySupported = true;
            bUseRTTI = true;
            bEnableExceptions = true;

            PublicAdditionalLibraries.Add(LibraryPathFnn + "libflann_s.a");


            ////PublicLibraryPaths.Add(Path.Combine(LibraryPathHmap, "./Linux/Release/"));
            ////PublicLibraryPaths.Add(Path.Combine(DepsLibraryPath, "./Linux/"));

            // MapSDK
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libhadmap_datamodel.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libhadmap_engine.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libhadmap_import.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libhadmap_mapdb.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libhadmap_routeplan.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libhadmap_routingmap.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libhadmap_transmission.a");

            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "liblzma.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libicudata.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libicutu.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libicuuc.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libicule.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libiculx.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libicuio.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libxml2.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libgeos.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libgeos_c.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libfreexl.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libspatialite.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "sqlite3.a");
            //PublicAdditionalLibraries.Add(LibraryPath + "Linux/Release/" + "libproj4.a");
            PublicAdditionalLibraries.Add(LibraryPath + "/Linux/Release/" + "libdatamodel.so");
            PublicAdditionalLibraries.Add(LibraryPath + "/Linux/Release/" + "libmapengine.so");
            PublicAdditionalLibraries.Add(LibraryPath + "/Linux/Release/" + "libmapimport.so");
            PublicAdditionalLibraries.Add(LibraryPath + "/Linux/Release/" + "libmapdb.so");
            PublicAdditionalLibraries.Add(LibraryPath + "/Linux/Release/" + "librouteplan.so");
            PublicAdditionalLibraries.Add(LibraryPath + "/Linux/Release/" + "libroutingmap.so");
            PublicAdditionalLibraries.Add(LibraryPath + "/Linux/Release/" + "libtransmission.so");

            //PublicDelayLoadDLLs.Add("libhadmap_datamodel.so");
            //PublicDelayLoadDLLs.Add("libhadmap_engine.so");
            //PublicDelayLoadDLLs.Add("libhadmap_import.so");
            //PublicDelayLoadDLLs.Add("libhadmap_mapdb.so");
            //PublicDelayLoadDLLs.Add("libhadmap_routeplan.so");
            //PublicDelayLoadDLLs.Add("libhadmap_routingmap.so");
            //PublicDelayLoadDLLs.Add("libhadmap_transmission.so");
            //PublicDelayLoadDLLs.Add("libproj.so");
            //PublicDelayLoadDLLs.Add("spatialite.so");
            //PublicDelayLoadDLLs.Add("sqlite3.so");

            // Ubuntu 18.04 && 20.04
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhadmap_datamodel.so",
            //Path.Combine(LibraryPath + "/Linux/Release/" + "libhadmap_datamodel.so"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhadmap_engine.so",
            //Path.Combine(LibraryPath + "/Linux/Release/" + "libhadmap_engine.so"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhadmap_import.so",
            //Path.Combine(LibraryPath + "/Linux/Release/" + "libhadmap_import.so"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhadmap_mapdb.so",
            //Path.Combine(LibraryPath + "/Linux/Release/" + "libhadmap_mapdb.so"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhadmap_routeplan.so",
            //Path.Combine(LibraryPath + "/Linux/Release/" + "libhadmap_routeplan.so"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhadmap_routingmap.so",
            //Path.Combine(LibraryPath + "/Linux/Release/" + "libhadmap_routingmap.so"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhadmap_transmission.so",
            //Path.Combine(LibraryPath + "/Linux/Release/" + "libhadmap_transmission.so"));

            string[] files = System.IO.Directory.GetFiles(LibraryPath + "/Linux/Release");
            foreach (string filepath in files)
            {
                Log.TraceInformation(Path.Combine("MapSDK dep path: " + filepath));
                string filename = System.IO.Path.GetFileName(filepath);

                RuntimeDependencies.Add(Path.Combine("$(TargetOutputDir)/ubuntu18_20/", filename),
                    Path.Combine(LibraryPath + "/Linux/Release/", filename));
            }

            /*
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libasn1.so.8", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libasn1.so.8"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcom_err.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libcom_err.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcrypt.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libcrypt.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcrypto.so.1.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libcrypto.so.1.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcurl.so.4", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libcurl.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libffi.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libffi.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libfreexl.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libfreexl.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libgeos_c.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgeos_c.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libgeos-3.6.2.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgeos-3.6.2.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libgmp.so.10", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgmp.so.10"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libgnutls.so.30", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgnutls.so.30"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libgssapi.so.3", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgssapi.so.3"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libgssapi_krb5.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgssapi_krb5.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhcrypto.so.4", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libhcrypto.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libheimbase.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libheimbase.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libheimntlm.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libheimntlm.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhogweed.so.4", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libhogweed.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libhx509.so.5", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libhx509.so.5"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libicudata.so.60", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libicudata.so.60"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libicuuc.so.60", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libicuuc.so.60"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libidn2.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libidn2.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libk5crypto.so.3", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libk5crypto.so.3"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libkeyutils.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libkeyutils.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libkrb5.so.3", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libkrb5.so.3"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libkrb5.so.26", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libkrb5.so.26"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libkrb5support.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libkrb5support.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/liblber-2.4.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "liblber-2.4.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libldap_r-2.4.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libldap_r-2.4.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/liblzma.so.5", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "liblzma.so.5"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnettle.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libnettle.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnghttp2.so.14", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libnghttp2.so.14"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libp11-kit.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libp11-kit.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libproj.so.12", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libproj.so.12"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libpsl.so.5", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libpsl.so.5"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libresolv.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libresolv.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libroken.so.18", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libroken.so.18"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/librtmp.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "librtmp.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libsasl2.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libsasl2.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libspatialite.so.7", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libspatialite.so.7"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libsqlite3.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libsqlite3.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libssl.so.1.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libssl.so.1.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libstdc++.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libstdc++.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libtasn1.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libtasn1.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libunistring.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libunistring.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libwind.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libwind.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libxml2.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libxml2.so.2"));


            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libdatamodel.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libdatamodel.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libmap_engine.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libmap_engine.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libroutingmap.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libroutingmap.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libtransmission.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libtransmission.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libmap_import.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libmap_import.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libmapdb.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libmapdb.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libproj.so.15", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libproj.so.15"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libgeos-3.8.0.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgeos-3.8.0.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/librouteplan.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "librouteplan.so"));



            RuntimeDependencies.Add("$(TargetOutputDir)/libz.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libz.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libbrotlicommon.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libbrotlicommon.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libbrotlidec.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libbrotlidec.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libc.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libc.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libssh.so.4", 
            
            Path.Combine(LibraryPath + "/Linux/Release/" + "libssh.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libpthread.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libpthread.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libproj.so.15", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libproj.so.15"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libnettle.so.7", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libnettle.so.7"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libm.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libm.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libicuuc.so.66", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libicuuc.so.66"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libicudata.so.66", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libicudata.so.66"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libhogweed.so.5", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libhogweed.so.5"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libgeos-3.8.0.so", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgeos-3.8.0.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libgcc_s.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libgcc_s.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libffi.so.7", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libffi.so.7"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/libcurl-gnutls.so.4", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libcurl-gnutls.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/libdl.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/" + "libdl.so.2"));

            // Ubuntu 16.04

            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhadmap_datamodel.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhadmap_datamodel.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhadmap_engine.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhadmap_engine.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhadmap_import.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhadmap_import.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhadmap_mapdb.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhadmap_mapdb.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhadmap_routeplan.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhadmap_routeplan.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhadmap_routingmap.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhadmap_routingmap.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhadmap_transmission.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhadmap_transmission.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libasn1.so.8", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libasn1.so.8"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libcom_err.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libcom_err.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libcrypt.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libcrypt.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libcrypto.so.1.0.0", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libcrypto.so.1.0.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libcurl.so.4", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libcurl.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libffi.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libffi.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libfreexl.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libfreexl.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libgeos_c.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libgeos_c.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libgeos-3.5.0.so", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libgeos-3.5.0.so"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libgmp.so.10", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libgmp.so.10"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libgnutls.so.30", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libgnutls.so.30"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libgssapi.so.3", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libgssapi.so.3"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libgssapi_krb5.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libgssapi_krb5.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhcrypto.so.4", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhcrypto.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libheimbase.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libheimbase.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libheimntlm.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libheimntlm.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhogweed.so.4", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhogweed.so.4"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libhx509.so.5", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libhx509.so.5"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libicudata.so.55", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libicudata.so.55"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libicuuc.so.55", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libicuuc.so.55"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libidn.so.11", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libidn.so.11"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libk5crypto.so.3", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libk5crypto.so.3"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libkeyutils.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libkeyutils.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libkrb5.so.3", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libkrb5.so.3"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libkrb5.so.26", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libkrb5.so.26"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libkrb5support.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libkrb5support.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/liblber-2.4.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "liblber-2.4.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libldap_r-2.4.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libldap_r-2.4.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/liblzma.so.5", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "liblzma.so.5"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libnettle.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libnettle.so.6"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libnghttp2.so.14",              
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libnghttp2.so.14"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libp11-kit.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libp11-kit.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libproj.so.9", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libproj.so.9"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libpsl.so.5",                   
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libpsl.so.5"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libresolv.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libresolv.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libroken.so.18", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libroken.so.18"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/librtmp.so.1", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "librtmp.so.1"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libsasl2.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libsasl2.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libspatialite.so.7", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libspatialite.so.7"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libsqlite3.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libsqlite3.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libssl.so.1.0.0", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libssl.so.1.0.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libstdc++.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libstdc++.so.6"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libtasn1.so.6", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libtasn1.so.6"));
            //RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libunistring.so.2",             
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libunistring.so.2"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libwind.so.0", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libwind.so.0"));
            RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu16/libxml2.so.2", 
            Path.Combine(LibraryPath + "/Linux/Release/ubuntu16/" + "libxml2.so.2"));*/
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            isLibrarySupported = true;

            //string PlatformString = "Mac";
            //PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, PlatformString, "libLeap.dylib"));
        }

        return isLibrarySupported;
    }
}
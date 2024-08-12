// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "HadmapModule.h"
// #include <WinBase.h>
#include "Core.h"
#include "Interfaces/IPluginManager.h"
// #include "ExampleLibrary.h"

#define LOCTEXT_NAMESPACE "FHadMapModule"

void FHadMapModule::StartupModule()
{
    // #if PLATFORM_LINUX
    //     FString Path = IPluginManager::Get().FindPlugin("Hadmap")->GetBaseDir();
    //     TArray<FString> LibraryPathArry;
    //     LibraryPathArry.Add(FPaths::Combine(Path,
    //     TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/libhadmap_transmission.so")));
    //     LibraryPathArry.Add(FPaths::Combine(Path,
    //     TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/libhadmap_routingmap.so")));
    //     LibraryPathArry.Add(FPaths::Combine(Path,
    //     TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/libhadmap_routeplan.so")));
    //     LibraryPathArry.Add(FPaths::Combine(Path,
    //     TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/libhadmap_mapdb.so")));
    //     LibraryPathArry.Add(FPaths::Combine(Path,
    //     TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/libhadmap_import.so")));
    //     LibraryPathArry.Add(FPaths::Combine(Path,
    //     TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/libhadmap_engine.so")));
    //     LibraryPathArry.Add(FPaths::Combine(Path,
    //     TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/libhadmap_datamodel.so")));
    //     //LibraryPathArry.Add(FPaths::Combine(Path, TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/proj.dll")));
    //     //LibraryPathArry.Add(FPaths::Combine(Path, TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/spatialite.dll")));
    //     //LibraryPathArry.Add(FPaths::Combine(Path, TEXT("/ThirdParty/HadMapSDK/Lib/Linux/Release/sqlite3.dll")));
    //     for (auto ELem : LibraryPathArry)
    //     {
    //         if (!FPlatformProcess::GetDllHandle(*ELem))
    //         {
    //             UE_LOG(LogTemp, Error, TEXT("Failed to load %s library."), *ELem);
    //         }
    //     }
    // #endif

    // TSharedPtr<IPlugin> Ptr = IPluginManager::Get().FindPlugin("Hadmap");
    // FString Path = IPluginManager::Get().FindPlugin("Hadmap")->GetBaseDir();

    //    // Get the base directory of this plugin
    //    FString BaseDir = IPluginManager::Get().FindPlugin("HadMapModule")->GetBaseDir();
    //
    //    // Add on the relative location of the third party dll and load it
    //    TArray<FString> LibraryPathArry;
    // #if PLATFORM_WINDOWS
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/freexl.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/geos_c.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/iconv.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/libcrypto-1_1-x64.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/libcurl-x64.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/libssl-1_1-x64.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/libxml2.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/proj.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/spatialite.dll")));
    //    LibraryPathArry.Add(FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/sqlite3.dll")));
    // #endif // PLATFORM_WINDOWS
    //    for (size_t i = 0; i < LibraryPathArry.Num(); i++)
    //    {
    //        if (!LibraryPathArry[i].IsEmpty())
    //        {
    //            LibraryHandleArry.Add(FPlatformProcess::GetDllHandle(*LibraryPathArry[i]));
    //        }
    //    }
    //
    //    //if (ExampleLibraryHandle)
    //    //{
    //    //    // Call the test function in the third party library that opens a message box
    //    //    ExampleLibraryFunction();
    //    //}
    //    //else
    //    //{
    //    //    FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third
    //    party library"));
    //    //}
}

void FHadMapModule::ShutdownModule()
{
    // for (size_t i = 0; i < LibraryHandleArry.Num(); i++)
    //{
    //     FPlatformProcess::FreeDllHandle(LibraryHandleArry[i]);
    // }
    //
    // LibraryHandleArry.Empty();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHadMapModule, HadMap)

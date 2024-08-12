// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "RuntimeMeshLoader.h"

#define LOCTEXT_NAMESPACE "FRuntimeMeshLoaderModule"

void FRuntimeMeshLoaderModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin
    // file per-module
    FString s = IPluginManager::Get().FindPlugin("RuntimeMeshLoader")->GetBaseDir();
    UE_LOG(LogTemp, Log, TEXT("%s"), *s);
    DllHandle = FPlatformProcess::GetDllHandle(*(IPluginManager::Get().FindPlugin("RuntimeMeshLoader")->GetBaseDir() +
                                                 "\\ThirdParty\\assimp\\bin\\assimp-vc142-mt.dll"));
}

void FRuntimeMeshLoaderModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.

    if (DllHandle)
    {
        FPlatformProcess::FreeDllHandle(DllHandle);
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRuntimeMeshLoaderModule, RuntimeMeshLoader)

// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CudaResource.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#if PLATFORM_LINUX
#include "VulkanRHIPrivate.h"
#include "VulkanRHIBridge.h"
#endif
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "CudaResourceModule"

void FCudaResourceModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin
    // file per-module

    // Get the base directory of this plugin
    FString BaseDir = IPluginManager::Get().FindPlugin("CudaResource")->GetBaseDir();
#if PLATFORM_LINUX
    if (FApp::CanEverRender())
    {
        const TArray<const ANSICHAR*> ExtentionsToAdd{
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME};
        VulkanRHIBridge::AddEnabledDeviceExtensionsAndLayers(ExtentionsToAdd, TArray<const ANSICHAR*>());
    }
#endif
}

void FCudaResourceModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCudaResourceModule, CudaResource)

// Copyright Epic Games, Inc. All Rights Reserved.

#include "RTXLidar.h"
#include "RTXLidarComponent.h"

#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FRTXLidarModule"

void FRTXLidarModule::StartupModule()
{
    // Get the base directory of this plugin
    FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("RTXLidar"))->GetBaseDir();

    // Register the shader directory
    FString PluginShaderDir = FPaths::Combine(BaseDir, TEXT("Shaders"));
    FString PluginMapping = TEXT("/Plugin/RTXLidar");
    AddShaderSourceDirectoryMapping(PluginMapping, PluginShaderDir);
    URTXLidarComponent::Startup();
}

void FRTXLidarModule::ShutdownModule()
{
    URTXLidarComponent::Shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRTXLidarModule, RTXLidar)

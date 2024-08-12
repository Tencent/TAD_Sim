// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Modules/ModuleManager.h"

#include "Interfaces/IPluginManager.h"
#include "HAL/PlatformProcess.h"
class FRuntimeMeshLoaderModule : public IModuleInterface
{
public:
    void* DllHandle;
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

protected:
    void* AssimpDllHandle = nullptr;
};

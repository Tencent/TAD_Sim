// Copyright (c) 2018 Tencent. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "DisplayGameInstance.h"
#include "NetworkModule.h"
#include "txsim_module.h"
// #include "txSimModuleServer.h"
/**
 *
 */
class DISPLAY_API DisplayNetworkManager : public FRunnable
{
public:
    DisplayNetworkManager(UDisplayGameInstance* gameInstance);

    virtual bool Init();
    virtual uint32 Run();
    virtual void Stop();

    void suspendThread();
    void resumeThread();
    void shutDown();

    ~DisplayNetworkManager();

    std::shared_ptr<NetworkModule> displayModule = nullptr;

private:
    FRunnableThread* thread = nullptr;
    UDisplayGameInstance* myGameInstance = nullptr;
    // TSharedPtr<tx_sim::SimModuleService> moduleServer;
    tx_sim::SimModuleService* moduleServer;
    // tx_sim::SimModuleServer* moduleServer = nullptr;

    // NetworkModule* displayModule = nullptr;
};

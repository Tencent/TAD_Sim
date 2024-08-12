// Copyright (c) 2018 Tencent. All rights reserved.

#include "DisplayNetworkManager.h"
#include "HAL/RunnableThread.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
// #include "Display.h"

// #ifdef _MSC_VER
// #include "AllowWindowsPlatformTypes.h"
// #endif
// #ifdef _MSC_VER
// #include "HideWindowsPlatformTypes.h"
// #endif

DisplayNetworkManager::DisplayNetworkManager(UDisplayGameInstance* gameInstance)
{
    myGameInstance = gameInstance;
    thread = FRunnableThread::Create(this, TEXT("DisplayNetworkManager"), 0, TPri_Normal);
    // JiangyuLin

    if (!displayModule)
    {
        displayModule.reset(new NetworkModule());
        displayModule->myGameInstance = myGameInstance;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("displayModule exist!"));
    }
}

bool DisplayNetworkManager::Init()
{
    UE_LOG(LogTemp, Log, TEXT("Init!"));
    // moduleServer = MakeShared< tx_sim::SimModuleService>(/*myGameInstance->getAddress()*/);
    moduleServer = new tx_sim::SimModuleService(/*myGameInstance->getAddress()*/);
    return true;
}

uint32 DisplayNetworkManager::Run()
{
    UE_LOG(LogTemp, Log, TEXT("Run!"));
    if (!moduleServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("moduleServer not exist!"));
    }
    if (!displayModule)
    {
        displayModule.reset(new NetworkModule());
        displayModule->myGameInstance = myGameInstance;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Run displayModule exist!"));
    }
    UE_LOG(LogTemp, Log, TEXT("moduleServer params: %s,%s"), UTF8_TO_TCHAR(myGameInstance->getName().c_str()),
        UTF8_TO_TCHAR(myGameInstance->getAddress().c_str()));
    try
    {
        moduleServer->Serve(myGameInstance->getName(), displayModule, myGameInstance->getAddress());
        moduleServer->Wait();
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Warning, TEXT("DisplayModule error: %s"), UTF8_TO_TCHAR(e.what()));
    }
    myGameInstance->NeedExit = true;
    myGameInstance->threadSuspendedEvent->Trigger();
    UKismetSystemLibrary::QuitGame(myGameInstance->GetWorld(), nullptr, EQuitPreference::Quit, true);
    return 0;
}

void DisplayNetworkManager::Stop()
{
    UE_LOG(LogTemp, Log, TEXT("Stop!"));
    if (moduleServer)
    {
        moduleServer->Shutdown();
        UE_LOG(LogTemp, Warning, TEXT("moduleServer Shutdown Return"));
    }
    UE_LOG(LogTemp, Log, TEXT("Stop Over"));
}

void DisplayNetworkManager::suspendThread()
{
    if (displayModule)
    {
        displayModule->getThreadSuspendedEvent()->Wait();
    }
}

void DisplayNetworkManager::resumeThread()
{
    if (displayModule)
    {
        displayModule->getThreadSuspendedEvent()->Trigger();
    }
}

void DisplayNetworkManager::shutDown()
{
    Stop();
    UE_LOG(LogTemp, Log, TEXT("DisplayNetworkManager::Begin delete thread!"));
    // if (thread)
    //{
    //     delete thread;
    //     thread = NULL;
    // }
    if (thread)
    {
        thread->WaitForCompletion();
        thread->Kill();
    }

    UE_LOG(LogTemp, Log, TEXT("DisplayNetworkManager::shutDown!"));
}

DisplayNetworkManager::~DisplayNetworkManager()
{
    UE_LOG(LogTemp, Log, TEXT("~DisplayNetworkManager Begin"));
    // if (moduleServer)
    //{
    //     //moduleServer->Shutdown();
    //     UE_LOG(LogTemp, Log, TEXT("~DisplayNetworkManager Delete moduleServer"));
    //     delete moduleServer;
    //     UE_LOG(LogTemp, Log, TEXT("~DisplayNetworkManager Delete moduleServer Done"));
    //     moduleServer = NULL;
    // }

    if (thread)
    {
        UE_LOG(LogTemp, Log, TEXT("~DisplayNetworkManager Delete thread"));
        delete thread;
        UE_LOG(LogTemp, Log, TEXT("~DisplayNetworkManager Delete thread Done"));
        thread = nullptr;
    }

    ////JiangyuLin
    // if (displayModule)
    //{
    //     delete displayModule;
    // }
    UE_LOG(LogTemp, Log, TEXT("DisplayNetworkManager Has Delete!"));
}

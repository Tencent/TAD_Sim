// Fill out your copyright notice in the Description page of Project Settings.

#include "DisplayGameStateBase.h"
#include "DisplayGameModeBase.h"
#include "Engine/World.h"
#include "DisplayPlayerController.h"
#include "Managers/TransportManager.h"
#include "../Managers/CreatureManager.h"
#include "../Managers/ObstacleManager.h"
#include "Managers/SensorManager.h"
#include "Managers/WeatherManager.h"
#include "Managers/EnvManager.h"
#include "Managers/SignalLightManager.h"
#include "Managers/TerrainManager.h"
#include "Objects/Transports/TransportPawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugGameState, Log, All);

ADisplayGameStateBase::ADisplayGameStateBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void ADisplayGameStateBase::Multicast_Reset_Implementation(FLocalResetIn _ResetData)
{
    UE_LOG(LogDebugGameState, Log, TEXT("Multicast_Reset"));

    // TODO: Delete all managers
    //..

    if (GetGameInstance()->IsDedicatedServerInstance())
    {
        // FLocalResetOut LocalResetOut;
        // GetWorld()->GetAuthGameMode<ADisplayGameModeBase>()->SimActionReturn(LocalResetOut, ESimState::SA_RESET);
        UE_LOG(LogDebugGameState, Log, TEXT("Execute Multicast_Reset Server."));
    }
    else
    {
        resetIn = _ResetData;
        if (!syncSystem)
        {
            syncSystem = GetWorld()->SpawnActor<ASyncSystem>();
        }
        syncSystem->SyncSimActors(resetIn);

        FLocalResetOut ResetOut;
        ResetOut.name = TEXT("RESET");
        GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->Server_SimResetOutput(ResetOut);
        // TODO: Respawn managers.
        //..

        // TODO: Init managers.
        //..
        // SimOutput(_ResetData);
        // Call PlayerController to reset private managers.
        // GetWorld()->GetFirstPlayerController<ADisplayGameStateBase>()->Reset(_ResetData);
        UE_LOG(LogDebugGameState, Log, TEXT("Execute Multicast_Reset client."));
    }
}

bool ADisplayGameStateBase::Multicast_Reset_Validate(FLocalResetIn _ResetData)
{
    // TODO: check value is legal
    // ..
    return true;
}

void ADisplayGameStateBase::Multicast_Update_Implementation(FLocalUpdateIn _UpdateData)
{
    if (GetGameInstance()->IsDedicatedServerInstance())
    {
        UE_LOG(LogDebugGameState, Log, TEXT("Execute Multicast_Update Server."));
    }
    else
    {
        if (_UpdateData.name == TEXT("UPDATE"))
        {
            updateIn = _UpdateData;
            FLocalUpdateOut UpdateOut = syncSystem->SyncSimActors(updateIn);
            UpdateOut.timeStamp = _UpdateData.timeStamp;
            // UpdateOut.timeStamp = 10000.f + _UpdateData.timeStamp;
            UpdateOut.name = TEXT("UPDATE");
            GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->Server_SimUpdateOutput(UpdateOut);
        }
        if (_UpdateData.name == TEXT("OUTPUT_SENSOR"))
        {
            FLocalUpdateOut UpdateOut = syncSystem->SyncSimActors(_UpdateData);
            UpdateOut.name = TEXT("OUTPUT_SENSOR");
            UpdateOut.timeStamp = _UpdateData.timeStamp;
            UpdateOut.timeStamp_ego = _UpdateData.timeStamp_ego;
            UpdateOut.timeStamp_tail = _UpdateData.timeStamp_tail;
            GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->Server_SimUpdateOutput(UpdateOut);
        }

        // UE_LOG(LogDebugGameState, Log, TEXT("Execute Multicast_Update client."));
    }
    // const FLocalData* Ptr = &_ResetData;
    // const FLocalResetIn* RPtr = static_cast<const FLocalResetIn*>(Ptr);
    // check(RPtr);
    // UE_LOG(LogDebugGameState, Log, TEXT("Multicast_Update, id: %d"), Ptr->id);
    // UE_LOG(LogDebugGameState, Log, TEXT("Multicast_Update, path: %s"),*RPtr->patch);
}

bool ADisplayGameStateBase::Multicast_Update_Validate(FLocalUpdateIn _UpdateData)
{
    // TODO: check value is legal
    // ..
    return true;
}

void ADisplayGameStateBase::SimInput(const FLocalData& _Data)
{
    if (_Data.name == TEXT("RESET"))
    {
        resetIn = *static_cast<const FLocalResetIn*>(&_Data);
        Multicast_Reset(resetIn);
    }
    else if (_Data.name == TEXT("UPDATE"))
    {
        updateIn = *static_cast<const FLocalUpdateIn*>(&_Data);
        Multicast_Update(updateIn);
    }
    else if (_Data.name == TEXT("OUTPUT_SENSOR"))
    {
        // updateIn = *static_cast<const FLocalUpdateIn*>(&_Data);
        FLocalUpdateIn InData;
        InData.timeStamp = _Data.timeStamp;
        InData.name = _Data.name;
        InData.timeStamp_ego = _Data.timeStamp_ego;
        InData.timeStamp_tail = _Data.timeStamp_tail;
        Multicast_Update(InData);
    }
}

void ADisplayGameStateBase::BeginPlay()
{
    Super::BeginPlay();
    if (!GetGameInstance()->IsDedicatedServerInstance())
    {
        if (!syncSystem)
        {
            syncSystem = GetWorld()->SpawnActor<ASyncSystem>();
        }
    }
}

FLocalUpdateOut ASyncSystem::SyncSimActors(const FLocalData& _Data)
{
    if (_Data.name == TEXT("RESET"))
    {
        resetIn = *static_cast<const FLocalResetIn*>(&_Data);
        SpawnAndInitAllManagers();
        ///* Focus to ego */
        // GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->SwitchPawnToEgo();

        // TODO: Need return FLocalUpdateOut
    }
    if (_Data.name == TEXT("UPDATE"))
    {
        updateIn = *static_cast<const FLocalUpdateIn*>(&_Data);
        updateOut = UpdateAllManagers(updateIn);
    }
    if (_Data.name == TEXT("OUTPUT_SENSOR"))
    {
        updateIn.sensorManager.timeStamp = _Data.timeStamp;
        updateIn.sensorManager.timeStamp_ego = _Data.timeStamp_ego;
        updateIn.sensorManager.timeStamp_tail = _Data.timeStamp_tail;
        SensorManager->Update(updateIn.sensorManager, updateOut.sensorManager);
        updateOut.message = TEXT("SUCCESS");
    }

    return updateOut;
}

bool ASyncSystem::SpawnAndInitAllManagers()
{
    bool AllManagerReady = true;

    /* Map */

    /* Transports */
    transportManager = GetWorld()->SpawnActor<ATransportManager>();
    if (transportManager.IsValid())
    {
        transportManager->Init(resetIn.transportManager);
    }
    else
    {
        AllManagerReady = false;
        UE_LOG(LogTemp, Error, TEXT("Can not spawn TransportManager!"));
    }

    /* Creatures */
    creatureManager = GetWorld()->SpawnActor<ACreatureManager>();
    if (creatureManager)
    {
        creatureManager->Init(resetIn.creatureManager);
    }
    else
    {
        AllManagerReady = false;
        UE_LOG(LogTemp, Error, TEXT("Can not spawn CreatureManager!"));
    }

    /* Obstacles */
    obstacleManager = GetWorld()->SpawnActor<AObstacleManager>();
    if (obstacleManager)
    {
        obstacleManager->Init(resetIn.obstacleManager);
    }
    else
    {
        AllManagerReady = false;
        UE_LOG(LogTemp, Error, TEXT("Can not spawn ObstacleManager!"));
    }

    /* SignalLights */
    signalligthManager = GetWorld()->SpawnActor<ASignalLightManager>();
    if (signalligthManager)
    {
        signalligthManager->Init(resetIn.signallightManager);
    }
    else
    {
        AllManagerReady = false;
        UE_LOG(LogTemp, Error, TEXT("Can not spawn signalligthManager!"));
    }

    /* Environment */
    envManager = GetWorld()->SpawnActor<AEnvManager>();
    if (envManager)
    {
        envManager->Init(resetIn.envManager);
    }
    else
    {
        AllManagerReady = false;
        UE_LOG(LogTemp, Error, TEXT("Can not spawn EnvManager!"));
    }

    /* Sensors */
    SensorManager = GetWorld()->SpawnActor<ASensorManager>();
    if (SensorManager)
    {
        SensorManager->Init(resetIn.sensorManager);
    }
    else
    {
        AllManagerReady = false;
        UE_LOG(LogTemp, Error, TEXT("Can not spawn SensorManager!"));
    }

    /* Terrain */
    terrainManager = GetWorld()->SpawnActor<ATerrainManager>();
    if (terrainManager)
    {
        terrainManager->Init(FManagerConfig());
    }
    else
    {
        AllManagerReady = false;
        UE_LOG(LogTemp, Error, TEXT("Can not spawn TerrainManager!"));
    }

    ///* UI */
    // UClass* MyWidgetClass = LoadClass<UOutlineWidget>(NULL,
    // TEXT("WidgetBlueprint'/Game/UI/WidgetBP_Outline.WidgetBP_Outline_C'")); if (MyWidgetClass)
    //{
    //     outlineWidget = CreateWidget<UOutlineWidget>(this, MyWidgetClass);
    //     if (outlineWidget)
    //     {
    //         outlineWidget->AddToViewport(1);
    //         outlineWidget->Init(this);
    //     }
    // }
    // else
    //{
    //     UE_LOG(LogTemp, Log, TEXT("DisplayPlayerController: Cant load /Game/Blueprints/UMG_ControlPad
    //     UWidgetBlueprint asset!"));
    // }

    OnAllManagersInit();
    return AllManagerReady;
}

FLocalUpdateOut ASyncSystem::UpdateAllManagers(const FLocalUpdateIn& _In)
{
    FLocalUpdateOut UpdateOut = FLocalUpdateOut();

    // check(transportManager);
    transportManager->Update(_In.transportManager, UpdateOut.transportManager);
    creatureManager->Update(_In.creatureManager, UpdateOut.creatureManager);
    obstacleManager->Update(_In.obstacleManager, UpdateOut.obstacleManager);
    signalligthManager->Update(_In.signallightManager, UpdateOut.signallightManager);
    envManager->Update(_In.envManager, UpdateOut.envManager);
    // SensorManager->Update(updateIn.sensorManager, updateOut.sensorManager);

    UpdateOut.message = TEXT("SUCCESS");

    OnAllManagersUpdate();
    return UpdateOut;
}

void ASyncSystem::OnAllManagersInit()
{
    // Possess to ego pawn

    bool SwitchCameraToEgo = false;
    if (transportManager.IsValid() && transportManager->vehicleManager)
    {
        if (transportManager->vehicleManager->egoArry.Num() > 0)
        {
            ATransportPawn* Ego = Cast<ATransportPawn>(transportManager->vehicleManager->egoArry[0]);
            if (Ego)
            {
                GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->SwitchPawnToEgo();
                // Ego->SwitchCamera(defaultCameraName);
                SwitchCameraToEgo = true;
            }
        }
    }
    if (!SwitchCameraToEgo)
    {
        GetWorld()->GetFirstPlayerController<ADisplayPlayerController>()->SwitchPawnToGhost();
    }
}

void ASyncSystem::OnAllManagersUpdate()
{
}

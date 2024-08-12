// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "SimInterface.h"
#include "GameFramework/GameStateBase.h"
#include "Managers/TransportManager.h"
#include "Managers/CreatureManager.h"
#include "Managers/ObstacleManager.h"
#include "Managers/SensorManager.h"
#include "Managers/WeatherManager.h"
#include "Managers/EnvManager.h"
#include "Managers/SignalLightManager.h"
#include "Managers/TerrainManager.h"

#include "Framework/DisplayGameModeBase.h"
#include "DisplayGameStateBase.generated.h"

// struct FLocalResetIn;
// struct FLocalUpdateIn;
// struct FLocalData;

UCLASS()
class DISPLAY_API ASyncSystem : public AActor
{
    GENERATED_BODY()
public:
    FLocalUpdateOut SyncSimActors(const FLocalData& _Data);
    // void SyncSensorActors(const FLocalData& _Data);
public:
    // TransportManager
    TWeakObjectPtr<ATransportManager> transportManager = NULL;

    // SensorManager
    ASensorManager* SensorManager = NULL;

    // CreatureManager
    ACreatureManager* creatureManager = NULL;

    // ObstacleManager
    AObstacleManager* obstacleManager = NULL;

    //// WeatherManager
    ASignalLightManager* signalligthManager = NULL;

    // EnvironmentManager
    AEnvManager* envManager = NULL;

    // Terrain
    ATerrainManager* terrainManager = NULL;

    //// UI
    // UOutlineWidget* outlineWidget = NULL;
protected:
    FLocalResetIn resetIn;
    FLocalUpdateIn updateIn;
    FLocalUpdateOut updateOut;

protected:
    /* Spawn all private managers */
    bool SpawnAndInitAllManagers();

    /* Init all private managers */
    FLocalUpdateOut UpdateAllManagers(const FLocalUpdateIn& _In);

    void OnAllManagersInit();

    void OnAllManagersUpdate();
};

/**
 *
 */
UCLASS()
class DISPLAY_API ADisplayGameStateBase : public AGameStateBase
{
    GENERATED_BODY()
public:
    /** Constructor for AActor that takes an ObjectInitializer for backward compatibility */
    ADisplayGameStateBase(const FObjectInitializer& ObjectInitializer);

public:
    /* Interface */

    // Gameinstance input simdata to gamestat
    virtual void SimInput(const FLocalData& _Data);

protected:
    FLocalResetIn resetIn;
    FLocalUpdateIn updateIn;

public:
    ASyncSystem* syncSystem = NULL;

protected:
protected:
    /**
     * Called by GameMode, run reset event.
     * NetMulticast function, create all public managers on server and client.
     */
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    virtual void Multicast_Reset(FLocalResetIn _ResetData);

    /**
     * Called by GameMode, run update event.
     * NetMulticast function, update all public managers on server and client.
     */
    UFUNCTION(NetMulticast, Reliable, WithValidation)
    virtual void Multicast_Update(FLocalUpdateIn _UpdateData);

protected:
    /* Engine Interface */
    /** Overridable native event for when play begins for this actor. */
    virtual void BeginPlay();
};

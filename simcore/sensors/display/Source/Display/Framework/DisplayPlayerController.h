// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SimInterface.h"
// #include "Managers/TransportManager.h"
// #include "../Managers/CreatureManager.h"
// #include "../Managers/ObstacleManager.h"
// #include "Managers/SensorManager.h"
// #include "Managers/WeatherManager.h"
// #include "Managers/EnvManager.h"
// #include "Managers/SignalLightManager.h"
// #include "Managers/TerrainManager.h"
#include "Framework/DisplayGameModeBase.h"
#include "Framework/DisplayGameStateBase.h"
#include "DisplayPlayerController.generated.h"

class UOutlineWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEgoViewChange, const FName& CameraName);

// USTRUCT()
// struct FLocalResetIn :public FLocalData
//{
//     GENERATED_BODY()
// public:
//     UPROPERTY()
//         float time;
//
//     UPROPERTY()
//         FString patch;
//
//     UPROPERTY()
//         FTransportManagerConfig transportManager;
//
//     UPROPERTY()
//         FCreatureManagerConfig creatureManager;
//
//     UPROPERTY()
//         FObstacleManagerConfig obstacleManager;
//
//     UPROPERTY()
//         FSignalLightManagerConfig signallightManager;
//
//     UPROPERTY()
//         FSensorManagerConfig sensorManager;
//
//     UPROPERTY()
//         FEnvManagerConfig envManager;
//
//     //struct FSensorManager* privateManagerArry;
// };
//
// USTRUCT()
// struct FLocalUpdateIn :public FLocalData
//{
//     GENERATED_BODY()
// public:
//     UPROPERTY()
//     double timeStamp = 0.f;
//
//     UPROPERTY()
//         FTransportManagerIn transportManager;
//
//     UPROPERTY()
//         FCreatureManagerIn creatureManager;
//
//     UPROPERTY()
//         FObstacleManagerIn obstacleManager;
//
//     UPROPERTY()
//         FSignalLightManagerIn signallightManager;
//
//     UPROPERTY()
//         FEnvManagerIn envManager;
//
//     UPROPERTY()
//         FSensorManagerIn sensorManager;
//     //struct FSensorManager* privateManagerArry;
// };
//
// USTRUCT()
// struct FLocalResetOut : public FLocalData
//{
//     GENERATED_BODY()
// public:
//     UPROPERTY()
//     FString message;
// };
//
// USTRUCT()
// struct FLocalUpdateOut : public FLocalData
//{
//     GENERATED_BODY()
// public:
//     UPROPERTY()
//     FString message;
//
//     UPROPERTY()
//         FTransportManagerOut transportManager;
//
//     UPROPERTY()
//         FCreatureManagerOut creatureManager;
//
//     UPROPERTY()
//         FObstacleManagerOut obstacleManager;
//
//     UPROPERTY()
//         FSignalLightManagerOut signallightManager;
//
//     UPROPERTY()
//         FEnvManagerOut envManager;
//
//     UPROPERTY()
//         FSensorManagerOut sensorManager;
// };

/**
 *
 */
UCLASS()
class DISPLAY_API ADisplayPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ADisplayPlayerController();

public:
    // UPROPERTY(Transient, ReplicatedUsing = OnSimResetInput)
    FLocalResetIn resetIn;

    FLocalResetOut resetOut;

    FLocalUpdateIn updateIn;

    FLocalUpdateOut updateOut;

    UFUNCTION()
    void OnSimResetInput();

    UFUNCTION()
    void OnSimUpdateInput();

    UFUNCTION()
    void OnSimUpdateInput_Deferred();

    virtual FString ConsoleCommand(const FString& Command, bool bWriteToLog = true);

public:
    //// TransportManager
    // ATransportManager* transportManager = NULL;

    // SensorManager
    ASensorManager* SensorManager = NULL;

    //// CreatureManager
    // ACreatureManager* creatureManager = NULL;

    //// ObstacleManager
    // AObstacleManager* obstacleManager = NULL;

    ////// WeatherManager
    // ASignalLightManager* signalligthManager = NULL;

    //// EnvironmentManager
    // AEnvManager* envManager = NULL;

    //// Terrain
    // ATerrainManager* terrainManager = NULL;

    // UI
    UOutlineWidget* outlineWidget = nullptr;
    UUserWidget* SwitchEgoWidget = nullptr;
    class UDrivingWidget* driving_widget = nullptr;

    class UDrivingWidget* GetDrivingWidget();

    UUserWidget* WanderInfoWidget = nullptr;

protected:
    FTimerHandle onSimTriggerTimer;

    FTimerHandle onSimTriggerTimer_Test;

    bool bSimTriggerFlag = false;

public:
    // UFUNCTION(Client, Reliable, WithValidation)
    //     virtual void Client_SimResetInput(FLocalResetIn _InData);

    UFUNCTION(Server, Reliable, WithValidation)
    virtual void Server_SimResetOutput(FLocalResetOut _ReturnData);

    // UFUNCTION(Client, Reliable, WithValidation)
    //     virtual void Client_SimUpdateInput(FLocalUpdateIn _InData);

    UFUNCTION(Server, Reliable, WithValidation)
    virtual void Server_SimUpdateOutput(FLocalUpdateOut _OutData);

    /** Overridable function called whenever this actor is being removed from a level */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Wrapper OnPossess function
    void PossessActor(AActor* _ActorToPossess);

    UFUNCTION(BlueprintCallable)
    void SetPossessEgoName(const FString& VehicleName);

    UFUNCTION(Exec)
    void PossessVehicleExec(int64 _Id);

    UFUNCTION(Exec)
    void DebugGate(bool bEnable);

    // UFUNCTION(Exec)
    // void ShowVehicle();

protected:
    /** Overridable native event for when play begins for this actor. */
    virtual void BeginPlay();

    /** Allows the PlayerController to set up custom input bindings. */
    virtual void SetupInputComponent();

    ///* Spawn all private managers */
    // bool SpawnAndInitAllManagers();

    ///* Init all private managers */
    // bool UpdateAllManagers();

    // void OnAllManagersInit();

    // void OnAllManagersUpdate();

private:
    UPROPERTY(Config)
    bool bShowMouseConfig = true;

    UPROPERTY(Config)
    bool bEnableGhost = false;

    UPROPERTY(Config)
    bool bEnableGod = false;

    // UPROPERTY(Config)
    //     FString defaultCameraName = TEXT("Camera_BirdView");

    class ADisplayPawn* ghostPawn = NULL;
    class AGodPawn* godPawn = NULL;

    bool bEnableConsole = false;

public:
    void ToggleUIVisibility();

    // Switch possess to ego vehicle
    void SwitchPawnToEgo();
    // Switch possess to ghost pawn
    void SwitchPawnToGhost();
    // Switch possess to god pawn
    void SwitchPawnToGod();

    void SwitchPawnToEgoOrGhost();

public:
    void ToggleDriveMode();
    int32 GetDrivingMode() const
    {
        return modeDrive;
    }
    void SetDrivingMode(int32 _Mode)
    {
        modeDrive = _Mode;
    }

protected:
    UPROPERTY(Config)
    int32 modeDrive = 0;

    UPROPERTY(Config)
    bool bAllowSwitchDriveMode = 0;

    UPROPERTY()
    class UOcculutionTraceComponent* OcculutionTraceComp;

public:
    UPROPERTY(Config)
    int32 id_controlled = 0;

    FOnEgoViewChange OnEgoViewChange;
};

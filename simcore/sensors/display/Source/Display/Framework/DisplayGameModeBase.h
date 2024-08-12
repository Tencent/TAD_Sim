// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SimInterface.h"
#include "Managers/TransportManager.h"
#include "../Managers/CreatureManager.h"
#include "../Managers/ObstacleManager.h"
#include "Managers/SensorManager.h"
#include "Managers/WeatherManager.h"
#include "Managers/EnvManager.h"
#include "Managers/SignalLightManager.h"
#include "Managers/TerrainManager.h"
#include "DisplayGameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSimGameMode, Log, All)

struct FSimData;
struct FSimOut;
// struct FLocalData;

// struct FLocalResetIn;
// struct FLocalResetOut;
// struct FLocalUpdateIn;
// struct FLocalUpdateOut;

USTRUCT()
struct FLocalIn : public FLocalData
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FLocalOut : public FLocalData
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FLocalResetIn : public FLocalIn
{
    GENERATED_BODY()
public:
    UPROPERTY()
    float time;

    UPROPERTY()
    FString patch;

    UPROPERTY()
    FTransportManagerConfig transportManager;

    UPROPERTY()
    FCreatureManagerConfig creatureManager;

    UPROPERTY()
    FObstacleManagerConfig obstacleManager;

    UPROPERTY()
    FSignalLightManagerConfig signallightManager;

    UPROPERTY()
    FSensorManagerConfig sensorManager;

    UPROPERTY()
    FEnvManagerConfig envManager;

    // struct FSensorManager* privateManagerArry;
};

USTRUCT()
struct FLocalUpdateIn : public FLocalIn
{
    GENERATED_BODY()
public:
    // UPROPERTY()
    //     double timeStamp = 0.f;

    UPROPERTY()
    FTransportManagerIn transportManager;

    UPROPERTY()
    FCreatureManagerIn creatureManager;

    UPROPERTY()
    FObstacleManagerIn obstacleManager;

    UPROPERTY()
    FSignalLightManagerIn signallightManager;

    UPROPERTY()
    FEnvManagerIn envManager;

    UPROPERTY()
    FSensorManagerIn sensorManager;
    // struct FSensorManager* privateManagerArry;
};

USTRUCT()
struct FLocalResetOut : public FLocalOut
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FString message;
};

USTRUCT()
struct FLocalUpdateOut : public FLocalOut
{
    GENERATED_BODY()
public:
    UPROPERTY()
    FString message;

    UPROPERTY()
    FTransportManagerOut transportManager;

    UPROPERTY()
    FCreatureManagerOut creatureManager;

    UPROPERTY()
    FObstacleManagerOut obstacleManager;

    UPROPERTY()
    FSignalLightManagerOut signallightManager;

    UPROPERTY()
    FEnvManagerOut envManager;

    UPROPERTY()
    FSensorManagerOut sensorManager;
};

/**
 *
 */
UCLASS()
class DISPLAY_API ADisplayGameModeBase : public AGameModeBase
{
    GENERATED_BODY()
public:
    ADisplayGameModeBase();

private:
    TArray<FUniqueNetIdWrapper> hasReturnClienIdArry;

public:
    /* Custom Interface */

    // virtual void SimResetInput(const FLocalResetIn& _Input);

    // virtual void SimResetOutput(const FLocalResetOut& _Output);

    // virtual void SimUpdateInput(const FLocalUpdateIn& _Input);

    // virtual void SimUpdateOutput(const FLocalUpdateOut& _Output);

    // Gameinstance input simdata to gamemode
    virtual void SimInput(const FSimData& _Data);

    // Playercontroller output simdata to gamemode
    virtual void SimOutput(const FLocalData& _Data, const FUniqueNetIdRepl& _ClientId);

protected:
    FString egoTypeName;

    TMap<FUniqueNetIdRepl, TSharedPtr<FSimOut>> clientOutputMap;

    TMap<int32, FString> trafficNames;

    // UPROPERTY(config)
    int64 id_controlled = 0;

protected:
    bool LoadSceneConfig(const FString& _FilePath);

    void ConvertData_SimToLocal(const FSimData& _SimData, FLocalData& _LocalData);

    void ConvertData_LocalToSim(const FLocalData& _LocalData, FSimData& _SimData);

    FString GetTrafficTypeDef(int32 _Id, ETrafficType _Type);
    FString GetTypeIdDef(int32 _Id, FString _Type);
    FString GetTypeIdDef(const FString& _TypeName);

    // FString GetTypeDef(int32 _Id, FString _Class);
    // virtual void SimInput(const FLocalData& _Input);

    // virtual void SimOutput(const FLocalData& _Output);

    // virtual void SimActionStart(const FSimData& _InputData);

    // virtual void SimActionReturn(const FLocalData& _OutputData, const ESimState& _State, const FUniqueNetIdRepl&
    // _NetId);

public:
    /* Engine Interface */

    /**
     * Accept or reject a player attempting to join the server.  Fails login if you set the ErrorMessage to a non-empty
     * string. PreLogin is called before Login.  Significant game time may pass before Login is called
     *
     * @param    Options                    The URL options (e.g. name/spectator) the player has passed
     * @param    Address                    The network address of the player
     * @param    UniqueId                The unique id the player has passed to the server
     * @param    ErrorMessage            When set to a non-empty value, the player will be rejected using the error
     * message set
     */
    virtual void PreLogin(
        const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);

    /**
     * Called to login new players by creating a player controller, overridable by the game
     *
     * Sets up basic properties of the player (name, unique id, registers with backend, etc) and should not be used to
     * do more complicated game logic.  The player controller is not fully initialized within this function as far as
     * networking is concerned. Save "game logic" for PostLogin which is called shortly afterward.
     *
     * @param NewPlayer pointer to the UPlayer object that represents this player (either local or remote)
     * @param RemoteRole the remote role this controller has
     * @param Portal desired portal location specified by the client
     * @param Options game options passed in by the client at login
     * @param UniqueId platform specific unique identifier for the logging in player
     * @param ErrorMessage [out] error message, if any, why this login will be failing
     *
     * If login is successful, returns a new PlayerController to associate with this player. Login fails if ErrorMessage
     * string is set.
     *
     * @return a new player controller for the logged in player, NULL if login failed for any reason
     */
    virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
        const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);

    /** Called after a successful login.  This is the first place it is safe to call replicated functions on the
     * PlayerController. */
    virtual void PostLogin(APlayerController* NewPlayer);

    /** Called when a Controller with a PlayerState leaves the game or is destroyed */
    virtual void Logout(AController* Exiting);

    /**
     * Handles reinitializing players that remained through a seamless level transition
     * called from C++ for players that finished loading after the server
     * @param C the Controller to handle
     */
    virtual void HandleSeamlessTravelPlayer(AController*& C);

    /**
     * Initialize the game.
     * The GameMode's InitGame() event is called before any other functions (including PreInitializeComponents() )
     * and is used by the GameMode to initialize parameters and spawn its helper classes.
     * @warning: this is called before actors' PreInitializeComponents.
     */
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage);

    /**
     *    Function called every frame on this Actor. Override this function to implement custom logic to be executed
     * every frame. Note that Tick is disabled by default, and you will need to check PrimaryActorTick.bCanEverTick is
     * set to true to enable it.
     *
     *    @param    DeltaSeconds    Game time elapsed during last frame modified by the time dilation
     */
    virtual void Tick(float DeltaSeconds);

public:
    // Check all client keep connect
    bool CheckAllClientKeepConnect();

    // Check all login client loaded server current world.
    bool CheckAllClientLoadedCurrentWorld();

    /* Cook simulator data, cast to local data */
    bool SimDataToLocalData(const FSimData& _SimData, FLocalData& _LocalData);

    /* Cook local data, cast to simulator data */
    bool LocalDataToSimData(const FLocalData& _LocalData, FSimData& _SimData);

protected:
    /* Engine Interface */
    /** Overridable native event for when play begins for this actor. */
    virtual void BeginPlay();
};

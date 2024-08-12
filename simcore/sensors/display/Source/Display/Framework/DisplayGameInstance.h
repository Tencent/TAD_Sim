// Copyright (c) 2019 Tencent. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include <map>

#ifdef _MSC_VER
#include "AllowWindowsPlatformTypes.h"
#endif
// Protobuf
#include "traffic.pb.h"
#include "location.pb.h"
#include "planOutput.pb.h"
#include "trajectory.pb.h"
#include "grading.pb.h"
#include "controlSim.pb.h"
#include "planStatus.pb.h"
#include "sensor_raw.pb.h"
#include "environment.pb.h"
#include "union.pb.h"
#ifdef _MSC_VER
#include "HideWindowsPlatformTypes.h"
#endif

#include "SimInterface.h"
#include "OnlineSessionInterface.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Objects/Transports/TransportInterface.h"
#include "DisplayGameInstance.generated.h"

#define MAPSDKMODE 1

DECLARE_LOG_CATEGORY_EXTERN(LogSimSystem, Log, All)
DECLARE_LOG_CATEGORY_EXTERN(LogSimDebug, Log, All)

namespace hadmapue4
{
class HadmapManager;
}

class DisplayNetworkManager;

struct FLocalInitIn;
struct FLocalInitOut;

struct FLocalResetIn;
struct FLocalResetOut;

struct FLocalUpdateIn;
struct FLocalUpdateOut;

struct FSensorManagerConfig;

// UENUM(BlueprintType)
// enum class EDrivingMode :uint8
//{
//     DM_AUTOPILOT,
//     DM_MANNED,
//     DM_MANNED_VR
// };

USTRUCT()
struct FMapInfo
{
    GENERATED_USTRUCT_BODY()
public:
    UPROPERTY()
    FString mapPath;
    UPROPERTY()
    FString mapName;
    UPROPERTY()
    double origin_Lon = 0;
    UPROPERTY()
    double origin_Lat = 0;
    UPROPERTY()
    double origin_Alt = 0;
    UPROPERTY()
    FString decryptFilePath;

    bool IsLegal() const
    {
        return true;
        // IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        // if (!PlatformFile.FileExists(*mapPath))
        // FString MapPath;
        // GConfig->GetString(TEXT("MapList"), *mapFileName, MapPath, GGameIni);
    }
};

struct FEgoInitInfo
{
    FString egoName = TEXT("suv");
    FString egoCategory = TEXT("car");
    FString egoType = "transport/Ego";
    int64 EgoID;
    double startLon;
    double startLat;
    double startAlt;
    double startTheta;
    double startSpeed;
};

USTRUCT()
struct FClientInfo
{
    GENERATED_BODY()
public:
    FUniqueNetIdRepl uniqueNetId;
    FString playerName;
    bool bIsSimActionComplete = false;
    bool bIsDeferredSimActionComplete = false;

    // FLocalResetIn resetIn;
    // FLocalResetOut resetOut;
    // FLocalUpdateIn updateIn;
    // FLocalUpdateOut updateOut;
    FString simStat = TEXT("DATA_SENT");
    ESimState state = ESimState::SA_DONE;
};

// USTRUCT()
struct FSimIn : public FSimData
{
    // GENERATED_BODY()
public:
    virtual ~FSimIn()
    {
    }
};

// USTRUCT()
struct FSimOut : public FSimData
{
    // GENERATED_BODY()
public:
    bool bIsSent = false;
};

// USTRUCT()
struct FSimInitIn : public FSimIn
{
    // GENERATED_BODY()
public:
    int32 clientNum = 1;
};

// USTRUCT()
struct FSimInitOut : public FSimOut
{
    // GENERATED_BODY()
public:
    virtual ~FSimInitOut()
    {
    }

    FString message;

    std::vector<std::string> sensor_topic;
};

// USTRUCT()
struct FSimResetIn : public FSimIn
{
    // GENERATED_BODY()
public:
    FString tadsimPath;
    FString configFilePath;
    double mapOriginLon = 0.f;
    double mapOriginLat = 0.f;
    double mapOriginAlt = 0.f;

    TArray<FEgoInitInfo> EgoInitInfoArry;
    FString egoName = TEXT("suv");
    FString egoCategory = TEXT("car");
    FString egoType = "transport/Ego";
    double startLon;
    double startLat;
    double startAlt;
    double startTheta;
    double startSpeed;
    double endLon;
    double endLat;
    double endAlt;

    int32 mapIndex;
    FString mapDataBaseName;
    FString mapDataBasePath;

    FString sensorConfigPath;
    FString envConfigPath;

    FString mapName;
    FString mapPath;
    FString decryptFilePath;
    FString ModelPath;

    std::string sceneBuffer;

    FString SceneTrafficPath;
};

// USTRUCT()
struct FSimResetOut : public FSimOut
{
    // GENERATED_BODY()

public:
    virtual ~FSimResetOut()
    {
    }

    FString message;
};

// USTRUCT()
struct FSimUpdateIn : public FSimIn
{
    // GENERATED_BODY()
public:
    virtual ~FSimUpdateIn()
    {
    }

    int32 frameID = 0;
    TMap<FString, sim_msg::Location> egoData;
    TMap<FString, sim_msg::Location> egoContainerData;
    sim_msg::Location overrideEgoLocation;
    sim_msg::Traffic trafficData;
    sim_msg::Trajectory trajectoryData;
    sim_msg::PlanOutput planOutputData;
    sim_msg::ControlSim controlSimData;
    sim_msg::PlanStatus planStatusData;
    sim_msg::EnvironmentalConditions environmentData;
};

// USTRUCT()
struct FSimUpdateOut : public FSimOut
{
    // GENERATED_BODY()
public:
    FSimUpdateOut()
    {
        datatype = 1;
    }
    virtual ~FSimUpdateOut()
    {
    }

    int32 frameID = 0;

    sim_msg::Location egoData;
    sim_msg::Location overrideEgoLocation;
    sim_msg::Traffic trafficData;
    sim_msg::Trajectory trajectoryData;
    sim_msg::PlanOutput planOutputData;
    sim_msg::ControlSim controlSimData;
    sim_msg::PlanStatus planStatusData;
    sim_msg::ManualVehicleControlInfoArray manualTrafficData;
    sim_msg::DisplayPose trafficPose;

    std::string topic_egoData = "LOCATION";
    std::string topic_overrideEgoLocation = "LOCATION_TRAILER";
    std::string topic_trafficData = "TRAFFIC";
    std::string topic_trajectoryData = "TRAJECTORY";
    std::string topic_planOutputData = "PLAN_OUTPUT";
    std::string topic_controlSimData = "CONTROL";
    std::string topic_planStatusData = "PLAN_STATUS";
    std::string topic_manualTrafficData = "TX_Manual_Vehicle";

    // FString topic_egoData = TEXT("LOCATION");
    // FString topic_overrideEgoLocation = TEXT("LOCATION_TRAILER");
    // FString topic_trafficData = TEXT("TRAFFIC");
    // FString topic_trajectoryData = TEXT("TRAJECTORY");
    // FString topic_planOutputData = TEXT("PLAN_OUTPUT");
    // FString topic_controlSimData = TEXT("CONTROL");
    // FString topic_planStatusData = TEXT("PLAN_STATUS");
    // FString topic_manualTrafficData = TEXT("TX_Manual_Vehicle");

    //// TODO: Sensor
    // FSimUpdateOut& operator = (const FSimUpdateOut& A)
    //{
    //     egoData.CopyFrom(A.egoData);
    //     overrideEgoLocation.CopyFrom(A.overrideEgoLocation);
    //     trafficData.CopyFrom(A.trafficData);
    //     trajectoryData.CopyFrom(A.trajectoryData);
    //     planOutputData.CopyFrom(A.planOutputData);
    //     controlSimData.CopyFrom(A.controlSimData);
    //     planStatusData.CopyFrom(A.planStatusData);

    //    topic_egoData = A.topic_egoData;
    //    topic_overrideEgoLocation = A.topic_overrideEgoLocation;
    //    topic_trafficData = A.topic_trafficData;
    //    topic_trajectoryData = A.topic_trajectoryData;
    //    topic_planOutputData = A.topic_planOutputData;
    //    topic_controlSimData = A.topic_controlSimData;
    //    topic_planStatusData = A.topic_planStatusData;
    //    topic_manualTrafficData = A.topic_manualTrafficData;

    //    return *this;
    //}
};

struct FSimSensorUpdateOut : public FSimOut
{
    // GENERATED_BODY()
public:
    FSimSensorUpdateOut()
    {
        datatype = 2;
    }
    virtual ~FSimSensorUpdateOut()
    {
    }

    // 具体传感器的序列化改为传感器内部进行
    sim_msg::SensorRaw sensorData;
};

// USTRUCT()
struct FSimStopIn : public FSimIn
{
    // GENERATED_BODY()
public:
};

// class ASceneManager;
// class AManager;
// class AVehicleManager;
// class ATrafficLightManager;
// class APedestrianManager;
// enum class EDrivingMode :uint8;
// UENUM()
// enum class ESyncConfigTypeEnum : uint8
//{
//     SYNCCONFIG_NONE,
//     SYNCCONFIG_VEHICLEMANAGER,
//     SYNCCONFIG_TRAFFICLIGHTMANAGER,
//     SYNCCONFIG_PEDESTRIANMANAGER
//     //SYNCCONFIG_DATAVISIUALMANAGER
// };
//
// UENUM()
// enum class ESyncInDataTypeEnum : uint8
//{
//     SYNCINDATA_NONE,
//     SYNCINDATA_VEHICLEMANAGER,
//     SYNCCONFIG_TRAFFICLIGHTMANAGER,
//     SYNCCONFIG_PEDESTRIANMANAGER
// };
//
// UENUM()
// enum class ESyncOutDataTypeEnum : uint8
//{
//     SYNCOUTDATA_NONE,
//     SYNCOUTDATA_VEHICLEMANAGER,
//     SYNCCONFIG_TRAFFICLIGHTMANAGER,
//     SYNCCONFIG_PEDESTRIANMANAGER
// };
//
// USTRUCT()
// struct FSyncActorStruct
//{
//     GENERATED_BODY();
//     AActor* pointer;
//     ESyncConfigTypeEnum configType;
//     ESyncInDataTypeEnum inDataType;
//     ESyncOutDataTypeEnum outDataType;
//
//     bool operator ==(const FSyncActorStruct &b) const
//     {
//         if (this->pointer == b.pointer && this->configType == b.configType && this->inDataType == b.inDataType &&
//         this->outDataType == b.outDataType)
//         {
//             return true;
//         }
//         return false;
//     }
//
// };
//
// UENUM()
// enum class ELevelStateEnum : uint8
//{
//     ASYNCLOADING,
//     ASYNCLOADED,
//     OPENING,
//     OPENED,
//     INITING,
//     INITED
// };

UCLASS(config = Game)
class DISPLAY_API UDisplayGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    // Simulator state
    ESimState simState = ESimState::SA_DONE;

    // Init data from simulator
    FSimInitIn* simInitIn = NULL;
    // Init data to return simulator
    FSimInitOut* simInitOut = NULL;

    // Reset data from simulator
    FSimResetIn* simRestIn = NULL;
    // Reset data to return simulator
    FSimResetOut* simRestOut = NULL;

    // Update data from simulator
    FSimUpdateIn* simUpdateIn = NULL;
    // Update data to return simulator
    FSimUpdateOut* simUpdateOut = NULL;

    FString ResetFaildStr;

    // Current simulator action state.
    bool bIsSimActionSuccessed = true;

    bool bAllowSync;

    // Init data in Local
    FLocalInitIn* localInitIn = NULL;
    // Init data in Local to return simulator
    FLocalInitOut* localInitOut = NULL;

    // Reset data in Local
    FLocalResetIn* localRestIn = NULL;
    // Reset data in Local to return simulator
    // TArray<FLocalResetOut> clientResetOutArry;
    // FLocalResetOut* localRestOut = NULL;

    // Update data in Local
    FLocalUpdateIn* localUpdateIn = NULL;
    // Update data in Local to return simulator
    // TArray<FLocalUpdateOut> clientUpdateOutArry;
    // FLocalUpdateOut* localUpdateOut = NULL;

    hadmapue4::HadmapManager* GetHadmapHandle()
    {
        return hadmapHandle;
    }
    void StepUpdateImmediately(const FSimUpdateIn& simIn, TArray<FSimUpdateOut>& simOuts);
    FString GetGameConfig(const TCHAR* Section, const TCHAR* Key);

    void SetAsynchronousMode(bool _Active);

    // SimTrigger flag
    bool bIsSimTrigger = false;

    // Does hadmap need loaded
    bool DoesHadmapNeedLoaded() const
    {
        return bNeedToLoadHadmap;
    }

    void SimOutput(const FSimData& _Data);

    void OnAllClientLevelLoaded();

private:
    //
    int32 clientNum = 1;

    //
    FTimerHandle onTriggerSimTimer;

    // Module Handle
    hadmapue4::HadmapManager* hadmapHandle = NULL;

    // Load hadmap flag
    bool bNeedToLoadHadmap = false;

    // Map decrypt
    bool bUseMapDecrypt = false;
    FString decryptFilePath;

    FString mapPath_SQL;

    double startLon = 0;
    double startLat = 0;
    double startAlt = 0;
    double endLon = 0;
    double endLat = 0;
    double endAlt = 0;

    double theta = 0;

    double startSpeed = 0;
    // sim_msg::Location* overrideEgoLocation;
    //  Control send data.
    // sim_msg::Sim2Car* simToCarData;

    // Level index
    int32 mapIndex = 0;
    // FString levelName;
    FString mapName;
    // level map path
    FString mapPaht;

    TMap<FString, int64> EgoName_ID_Mapping;

    // TArray<FSyncActorStruct>syncActorArry;
    // FVehicleManagerConfigStruct vehicleConfig;
    // void UnregisterAllSyncActor();
    // void SyncAllActor();

    // DECLARE_DYNAMIC_DELEGATE(FOnPackageLoaded);
    // UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "TransitionLevel")
    //     static bool LoadLevelAsync(const FString& InName, TArray<FString> Packages, FOnPackageLoaded
    //     OnPackageLoaded);
    // ELevelStateEnum levelState = ELevelStateEnum::OPENING;
    // UFUNCTION()
    //     void TravelLevel();

    //// save data thread
    // class SaveDataThread* savedataThreadHandle = NULL;

    // const int k_TypePedestrain = 2;

    // Init from simulator
    void Sim_InitBeginLoadWorld();

    // Init from simulator
    void Sim_InitAfterLoadedWorld();

    // Reset from simulator
    void Sim_ResetBeginLoadWorld();

    // Reset from simulator
    void Sim_ResetAfterLoadedWorld();

    // Update from simulator
    void Sim_Update();

    void Sim_Update_Defferred();

    std::string ipAddress;

    std::string moduleName = "Display";

    FDelegateHandle TickDelegateHandle;

    // TArray<FString> mapArray;

    void ReadSceneFileAndConfig(FSimIn& _InData);

    // std::map<int, AActor*> obstacleMap;

    // int levelIndex = -1;

    TSharedPtr<DisplayNetworkManager> displayNetworkManager;

    // ASceneManager* sceneManager = nullptr;

    // double timeStamp = 0;

    // Init complete flag
    bool bInitActionComplete = false;
    // Reset complete flag
    bool bResetActionComplete = false;

    std::string modeName = "FrameAsync";

    bool sendVilMsg = false;
    // UPROPERTY(config)
    //     bool bUseEgoDataPath = false;

    // uint64 currentDataPathIndex = 0;

    // save data thread
    TSharedPtr<class SaveDataThread> savedataThreadHandle;

    FString getTrafficName(int32 id, int32 type);

protected:
    /* Delegate called when session created */
    FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
    /* Delegate called when session started */
    FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

    /** Handles to registered delegates for creating/starting a session */
    FDelegateHandle OnCreateSessionCompleteDelegateHandle;
    FDelegateHandle OnStartSessionCompleteDelegateHandle;

    TSharedPtr<class FOnlineSessionSettings> SessionSettings;

    /** Delegate for searching for sessions */
    FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

    /** Handle to registered delegate for searching a session */
    FDelegateHandle OnFindSessionsCompleteDelegateHandle;

    // A variable for our SearchSettings which will also contain our SearchResults, once this search is complete and
    // successful
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    /** Delegate for joining a session */
    FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

    /** Handle to registered delegate for joining a session */
    FDelegateHandle OnJoinSessionCompleteDelegateHandle;

    /** Delegate for destroying a session */
    FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

    /** Handle to registered delegate for destroying a session */
    FDelegateHandle OnDestroySessionCompleteDelegateHandle;

    // Check SimTrigger High Frequency by timer system.
    void CheckSimTriggerHighFrequency();

    // On SimInput
    void OnTriggerSim();

    // Is all clients loaded world.
    bool bIsAllClientsLoadedWorld = false;

    // Transform data from Sim to Local
    FLocalResetIn TransformResetData(const FSimResetIn& _SimResetIn);

    // Transform data from Sim to Local
    FLocalUpdateIn TransformUpdateData(const FSimUpdateIn& _SimUpdateIn);

    FString sensorConfigPath;
    FString envConfigPath;

    // FSensorManagerConfig ReadSensorFile(FString _Path);

    FString egoNames;
    TMap<int32, FString> trafficNames;

    UPROPERTY()
    class UCatalogDataSource* CatalogDataSource;

    UPROPERTY()
    class URuntimeMeshLoader* RuntimeMeshLoader;

    // Map info
    UPROPERTY()
    FMapInfo currentMapInfo;

    bool GetMapInfo(int32 _MapIndex, const FString& _MapFileName, FMapInfo& _MapInfo, FString& _ErrorMessage);

    // Reset to init
    void Reset();

public:
    UDisplayGameInstance(const FObjectInitializer& ObjectInitializer);

    virtual void Init() override;

    bool Tick(float DeltaSeconds);

    virtual void Shutdown() override;

    virtual void LoadComplete(const float LoadTime, const FString& MapName);

    /**
     *    Function to host a game!
     *
     *    @Param        UserID            User that started the request
     *    @Param        SessionName        Name of the Session
     *    @Param        bIsLAN            Is this is LAN Game?
     *    @Param        bIsPresence        "Is the Session to create a presence Session"
     *    @Param        MaxNumPlayers            Number of Maximum allowed players on this "Session" (Server)
     */
    bool HostSession(
        TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);

    /**
     *    Function fired when a session create request has completed
     *
     *    @param SessionName the name of the session this callback is for
     *    @param bWasSuccessful true if the async action completed without error, false if there was an error
     */
    virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

    /**
     *    Function fired when a session start request has completed
     *
     *    @param SessionName the name of the session this callback is for
     *    @param bWasSuccessful true if the async action completed without error, false if there was an error
     */
    void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

    /**
     *    Find an online session
     *
     *    @param UserId user that initiated the request
     *    @param bIsLAN are we searching LAN matches
     *    @param bIsPresence are we searching presence sessions
     */
    void FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence);

    /**
     *    Delegate fired when a session search query has completed
     *
     *    @param bWasSuccessful true if the async action completed without error, false if there was an error
     */
    void OnFindSessionsComplete(bool bWasSuccessful);

    /**
     *    Joins a session via a search result
     *
     *    @param SessionName name of session
     *    @param SearchResult Session to join
     *
     *    @return bool true if successful, false otherwise
     */
    bool JoinSession2(
        TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

    /**
     *    Delegate fired when a session join request has completed
     *
     *    @param SessionName the name of the session this callback is for
     *    @param bWasSuccessful true if the async action completed without error, false if there was an error
     */
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    /**
     *    Delegate fired when a destroying an online session has completed
     *
     *    @param SessionName the name of the session this callback is for
     *    @param bWasSuccessful true if the async action completed without error, false if there was an error
     */
    virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    UFUNCTION(BlueprintCallable, Category = "Network|Test")
    void StartOnlineGame();

    UFUNCTION(BlueprintCallable, Category = "Network|Test")
    void FindOnlineGames();

    UFUNCTION(BlueprintCallable, Category = "Network|Test")
    void JoinOnlineGame();

    UFUNCTION(BlueprintCallable, Category = "Network|Test")
    void DestroySessionAndLeaveGame();

    // Create thread to communicate with coordinator
    bool CreateSimModuleThread();

    // Shutdown thread that communicate with coordinator
    void ShutdownSimModuleThread();

    void SimInitOut();

    void SimResetOut(const FLocalResetOut& _ResetOutput, const FUniqueNetIdRepl& _ClientNetId);

    void SimUpdateOut(const FLocalUpdateOut& _UpdateOutput, const FUniqueNetIdRepl& _ClientNetId);

    void SimUpdateOut_Deferred(const FLocalUpdateOut& _UpdateOutput, const FUniqueNetIdRepl& _ClientNetId);

    /**
     * Sim interface
     * Trigger simulator action to GameInstance from SimModule.
     */
    virtual void SimInput(const FSimData& _Data);

    ///**
    // * Sim interface
    // * Receive action`s result to GameInstance from GameMode.
    // */
    // virtual void SimOutput(const FLocalData& _Output);

    // Get client config array
    FORCEINLINE TArray<FClientInfo> GetAllClientConfig() const
    {
        return clientConfigArry;
    };

    // Register client to simulator when client is login.
    bool RegisterClientToSim(APlayerController* NewPlayer);

    // Unregister client to simulator when client is login.
    bool UnregisterClientFromSim(AController* Player);

    //
    void OnAllClientLoadedWorld();

    // Check current simulator action is successed.
    FORCEINLINE bool IsSimActionSuccessed() const
    {
        return bIsSimActionSuccessed;
    };

    std::string getAddress();

    std::string getName();

    FORCEINLINE bool GetFlag_SendVilMsg() const
    {
        return sendVilMsg;
    };

    class UCatalogDataSource* GetCatalogDataSource()
    {
        return CatalogDataSource;
    }

    class URuntimeMeshLoader* GetRuntimeMeshLoader()
    {
        return RuntimeMeshLoader;
    }

    // void localToLonLat(double& x, double& y, double& z);
    // void LonLatToLocal(double& _Lon, double& _Lat, double& _Alt);

    // Init flag that execute simulator init.
    bool initFlag = false;

    // Reset flag that execute simulator reset.
    UPROPERTY(BlueprintReadOnly)
    bool resetFlag = false;

    // Update flag that execute simulator update.
    bool updateFlag = false;

    std::string configFilePath;

    bool IsLockStep();

    int frameID = -1;

    EDrivingMode drivingMode = EDrivingMode::DM_AUTOPILOT;

    // timeStamp getter and setter
    double getTimeStamp();
    void setTimeStamp(double timeStamp);

    FEvent* threadSuspendedEvent;

    sim_msg::Location getOverrideLocation();
    sim_msg::Sim2Car GetControlSendData();

    double mapOriginLon = 0;
    double mapOriginLat = 0;
    double mapOriginAlt = 0;

    UPROPERTY(BlueprintReadWrite)    // Jiangyu
    int cameraIndex = 0;

    bool IsWaitToReturnData();

    // void UpdateLevelState(ELevelStateEnum _NewState);
    // int RegisterSyncActor(FSyncActorStruct _NewSyncActor);
    // bool GetSyncConfig(int _ID, FSyncConfigStruct& _Config);

    // SaveDataThread* GetSaveDataHandle()const;
    SaveDataThread* GetSaveDataHandle() const;

    // fpsPrint
    double ETime = 0;
    double fps = 0;

    // store simdata
    TArray<TSharedPtr<FSimIn>> simInDataArry;
    TArray<TSharedPtr<FSimOut>> simOutDataArry;
    TSharedPtr<FSimIn> currentSimInData;
    TSharedPtr<FSimOut> currentSimOutData;
    TSharedPtr<FSimOut> currentSimSensorOutData;
    // update data flag
    bool bSimInDataRefreshed = false;
    bool bSimOutDataProduced = false;

    bool bIsLockStep = true;

    bool asynchronousMode = true;
    bool syncOneFrame = false;

    bool bIsFrameSync = false;
    int SyncModeWait = 0;
    bool NeedExit = false;

    FString ModuleGroupName;

    FVector2D nHILpos{-1, -1};

    // Gameinstance input simdata to gamemode
    virtual void SimPull(const FString _Command);

    void SendSimData();
    void ReceiveSimData();
    void SyncSimData();
    void OutputData();

    FString mapPath_Lobby = TEXT("/Game/Maps/LobbyMap");

    // bool CheckAllClientsLogin();
    bool bAllClientsLogin = false;

    // All client configuration, include uniqueNetId, sensor config and so on.
    TArray<FClientInfo> clientConfigArry;

    int32 getMapIndex(const FString& mapname);

    // if EgoGrouName is null, then find current possessed ego
    int64 GetEgoIDByGroupName(FString EgoGroupName = TEXT(""));

    UFUNCTION(BlueprintCallable)
    TArray<FString> GetEgoNameList();

    FORCEINLINE void SetCurrentModuleGroup(const FString NewGroupName)
    {
        ModuleGroupName = NewGroupName;
    }
};

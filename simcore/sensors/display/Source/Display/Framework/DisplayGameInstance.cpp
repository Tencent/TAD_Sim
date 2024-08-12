// Copyright (c) 2018 Tencent. All rights reserved.

#include "DisplayGameInstance.h"
// #include "Display.h"
#include <string>
#include <stdio.h>
#include <limits.h>
#include <algorithm>
#include "XmlParser.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Objects/Transports/Vehicle/VehiclePawn.h"

// #include "common/coord_trans.h"
// #include "structs/base_struct.h"

// hadmap
#include "HadmapManager.h"

#include "DisplayNetworkManager.h"
// #include "EgoVehicle.h"
// #include "SplineVehicle.h"

// Jiangyu Lin
// #include "TrafficLightManager.h" //Jiangyu Lin
// #include "PedestrianManager.h" //Jiangyu Lin

// #include "VehicleManager.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Ticker.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Engine.h"
#include "CommandLine.h"
#include "DisplayGameModeBase.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Engine/LocalPlayer.h"
#include "DisplayPlayerState.h"
#include "DisplayPlayerController.h"
#include "Display/Framework/DisplayGameStateBase.h"
#include "Objects/Transports/Vehicle/VehicleInterface.h"
#include "Objects/Obstacles/ObstacleInterface.h"
#include "Managers/SensorManager.h"
#include "Framework/DisplayGameModeBase.h"
#include "FovFilter.h"
#include <chrono>
#include <ctime>
// SaveData
#include "SaveDataThread.h"

#include "Misc/MessageDialog.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "scene.pb.h"
#include "LoaderBPFunctionLibrary.h"
#include "Data/CatalogDataSource.h"
#include "Internationalization/Regex.h"
// #include "AsyncLoading.h"
// #include "Async.h"

// SaveData
// #include "SaveDataThread.h"

// #define VEHICLE_LENGTH 4.8
// #define VEHICLE_WIDTH 1.8
// #define VEHICLE_HEIGHT 1.5

// #define PI 3.1415926

#define CONSUMED_MAXTICK 1000000

DEFINE_LOG_CATEGORY(LogSimSystem);
DEFINE_LOG_CATEGORY(LogSimDebug);
#define GETENUMSTRING(etype, evalue)                                                         \
    ((FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr)                          \
            ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetEnumName((int32) evalue) \
            : FString("Invalid - are you sure enum uses UENUM() macro?"))
DEFINE_LOG_CATEGORY_STATIC(LogSimGameInstance, Log, All);

void UDisplayGameInstance::Reset()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, true);
    // TODO: reset gameinstance
    // simActionState = TEXT("SCENE_ERROR");
    // if (!GetWorld()->ServerTravel(mapPath_Lobby, false, false))
    //{
    //    UE_LOG(LogSimGameMode, Warning, TEXT("Server Travel Level Failed!"));
    //}

    // bIsAllClientsLoadedWorld = false;
}

UDisplayGameInstance::UDisplayGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    /** Bind function for CREATING a Session */
    OnCreateSessionCompleteDelegate =
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UDisplayGameInstance::OnCreateSessionComplete);
    OnStartSessionCompleteDelegate =
        FOnStartSessionCompleteDelegate::CreateUObject(this, &UDisplayGameInstance::OnStartOnlineGameComplete);

    /** Bind function for FINDING a Session */
    OnFindSessionsCompleteDelegate =
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &UDisplayGameInstance::OnFindSessionsComplete);

    /** Bind function for JOINING a Session */
    OnJoinSessionCompleteDelegate =
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &UDisplayGameInstance::OnJoinSessionComplete);

    /** Bind function for DESTROYING a Session */
    OnDestroySessionCompleteDelegate =
        FOnDestroySessionCompleteDelegate::CreateUObject(this, &UDisplayGameInstance::OnDestroySessionComplete);

    // Apply for network thread
    threadSuspendedEvent = FPlatformProcess::GetSynchEventFromPool();

    drivingMode = EDrivingMode::DM_AUTOPILOT;
}

void UDisplayGameInstance::Init()
{
    UE_LOG(LogSimSystem, Log, TEXT("Build time: %s %s"), TEXT(__DATE__), TEXT(__TIME__));
    // add tick function for game instance
    TickDelegateHandle =
        FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UDisplayGameInstance::Tick));
    UGameInstance::Init();

    // UE_LOG(LogSimSystem, Log, TEXT("Device id: %s "), *FGenericPlatformMisc::GetDeviceId());

    // simActionStat = TEXT("GAME_INITING");

    // Create SaveData thread
    savedataThreadHandle = MakeShared<SaveDataThread>();

    /* Create CatalogDataSource instance */
    CatalogDataSource = NewObject<UCatalogDataSource>();
    if (CatalogDataSource)
    {
        CatalogDataSource->AddToRoot();
    }

    RuntimeMeshLoader = NewObject<URuntimeMeshLoader>();
    if (RuntimeMeshLoader)
    {
        RuntimeMeshLoader->AddToRoot();
        RuntimeMeshLoader->Init();
        URuntimeMeshLoader::SetInstance(RuntimeMeshLoader);
    }

    /* ======Processing Command Arguments===== */

    // Get user directory
    FString UserDirStr;
    if (FParse::Value(FCommandLine::Get(), TEXT("UserDir="), UserDirStr))
    {
        UE_LOG(LogSimSystem, Log, TEXT("UserDirStr:%s"), *UserDirStr);
    }

    // Get framesync mode
    FString ModeStr;
    if (FParse::Value(FCommandLine::Get(), TEXT("-mode="), ModeStr))    // TODO: check server
    {
        modeName = std::string(TCHAR_TO_UTF8(*ModeStr));
    }
    else
    {
        UE_LOG(LogSimSystem, Log, TEXT("-mode is null."));
    }
    if (modeName == "FrameSync")
    {
        bIsFrameSync = true;
    }
    else if (modeName == "FrameAsync")
    {
        bIsFrameSync = false;
    }
    UE_LOG(LogSimSystem, Log, TEXT("-mode use %s"), UTF8_TO_TCHAR(modeName.c_str()));

    GConfig->GetInt(TEXT("Mode"), TEXT("SyncModeWait"), SyncModeWait, GGameIni);
    UE_LOG(LogSimSystem, Log, TEXT("SyncModeWait is %u"), SyncModeWait);

    // Get VIL flag
    FString FlagVilSendMsg;
    if (FParse::Value(FCommandLine::Get(), TEXT("-vilmsg"), FlagVilSendMsg))
    {
        sendVilMsg = true;
    }
    UE_LOG(LogSimSystem, Log, TEXT("-novilmsg use, forbid send topic msg about vil!"));

    // Get HIL flag
    FString FlagHIL;
    if (FParse::Value(FCommandLine::Get(), TEXT("-hil="), FlagHIL))
    {
        FString xleft, yright;
        FlagHIL.Split(TEXT("x"), &xleft, &yright);
        nHILpos.X = FCString::Atof(*xleft);
        nHILpos.Y = FCString::Atof(*yright);
    }
    UE_LOG(LogSimSystem, Log, TEXT("hil pos = %f,%f"), nHILpos.X, nHILpos.Y);

    // Get Enviroment var
    FString EnviromentVar, EnglishValue;
    GConfig->GetString(TEXT("Language"), TEXT("EnviromentVar"), EnviromentVar, GGameIni);
    GConfig->GetString(TEXT("Language"), TEXT("EnglishValue"), EnglishValue, GGameIni);
    FString Language = FPlatformMisc::GetEnvironmentVariable(*EnviromentVar);

    if (Language.Equals(EnglishValue, ESearchCase::IgnoreCase))
    {
        UKismetInternationalizationLibrary::SetCurrentCulture(TEXT("en"), false);
    }
    else    // default chinese
    {
        UKismetInternationalizationLibrary::SetCurrentCulture(TEXT("ch"), false);
    }

    /* =====Load Config===== */

    // Get sync mode
    GConfig->GetBool(TEXT("Mode"), TEXT("Asynchronous"), asynchronousMode, GGameIni);
    GConfig->GetBool(TEXT("Mode"), TEXT("SyncOneFrame"), syncOneFrame, GGameIni);

    // Get lobby map path
    GConfig->GetString(TEXT("GlobalSettings"), TEXT("LobbyMapPath"), mapPath_Lobby, GGameIni);

    // if (GConfig)
    //{
    //     GConfig->Flush(true, GGameIni);
    //     GConfig->GetArray(TEXT("MapIndex"), TEXT("MapArray"), mapArray, GGameIni);
    // }
    //  Ego data path function
    // UE_LOG(LogTemp, Log, TEXT("bUseEgoDataPath is: %d"), bUseEgoDataPath);

    // if (bUseEgoDataPath)
    //{
    //     // read data path..
    //     UE_LOG(LogTemp, Log, TEXT("Path is: %s"), *(FPaths::ProjectDir() +
    //     TEXT("Saved/SensorData/EgoDataPath.csv"))); if
    //     (FPlatformFileManager::Get().GetPlatformFile().FileExists(*(FPaths::ProjectDir() +
    //     TEXT("Saved/SensorData/EgoDataPath.csv"))))
    //     {
    //         UE_LOG(LogTemp, Log, TEXT("EgoDataPath file has exists!"));
    //         FString String;
    //         FFileHelper::LoadFileToString(String, *(FPaths::ProjectDir() +
    //         TEXT("Saved/SensorData/EgoDataPath.csv"))); FString Left; FString Right; while
    //         (String.Split(FString("\r\n"), &Left, &Right))
    //         {
    //             FVehicleInData NewData;
    //             for (size_t i = 0; i < 6; i++)
    //             {
    //                 FString LeftTable;
    //                 FString RightTable;
    //                 if (Left.Split(FString(","), &LeftTable, &RightTable))
    //                 {
    //                     if (i == 0)
    //                     {
    //                         NewData.location.X = FCString::Atof(*LeftTable);
    //                     }
    //                     else if (i == 1)
    //                     {
    //                         NewData.location.Y = FCString::Atof(*LeftTable);
    //                     }
    //                     else if (i == 2)
    //                     {
    //                         //NewData.location.Z = FCString::Atof(*LeftTable);
    //                         NewData.location.Z = 4.f;
    //                     }
    //                     else if (i == 3)
    //                     {
    //                         NewData.rotation.Roll = FCString::Atof(*LeftTable);
    //                     }
    //                     else if (i == 4)
    //                     {
    //                         NewData.rotation.Pitch = FCString::Atof(*LeftTable);
    //                     }
    //                     else if (i == 5)
    //                     {
    //                         NewData.rotation.Yaw = FCString::Atof(*LeftTable);
    //                         NewData.timeStamp = (int)FCString::Atoi(*RightTable);
    //                     }
    //                     Left = RightTable;
    //                 }
    //             }
    //             //egoDataPathArry.Add(NewData);
    //             String = Right;
    //         }

    //    }
    //    else
    //    {
    //        bUseEgoDataPath = false;
    //    }

    //}
}

void UDisplayGameInstance::Sim_InitBeginLoadWorld()
{
    bInitActionComplete = false;

    check(GetWorld());
    if (!GetWorld()->ServerTravel(mapPath_Lobby, false, false))
    {
        UE_LOG(LogSimGameInstance, Warning, TEXT("Server Travel Level Failed!"));
    }
    bIsAllClientsLoadedWorld = false;
    // TODO: Init GameInstance

    // TODO: Config client
    // ..

    // check(GetWorld() && GetWorld()->GetAuthGameMode());
    // ADisplayGameModeBase* GM = Cast<ADisplayGameModeBase>(GetWorld()->GetAuthGameMode());
    // check(GM);
    // GM->SimActionStart(*simInitIn, simState);
}

void UDisplayGameInstance::Sim_InitAfterLoadedWorld()
{
    bInitActionComplete = true;
    SimInitOut();
}

void UDisplayGameInstance::Sim_ResetBeginLoadWorld()
{
    // TODO: Set Data after load map?
    bResetActionComplete = false;

    UE_LOG(LogSimGameInstance, Log, TEXT("Reset Begin Load World!"));

    if (CatalogDataSource)
        CatalogDataSource->ClearData();

    ReadSceneFileAndConfig(*currentSimInData);

    FSimResetIn* ResetIn = StaticCast<FSimResetIn*>(currentSimInData.Get());

    FMapInfo NewMapInfo;
    FString ErrorMessage;
    if (GetMapInfo(ResetIn->mapIndex, ResetIn->mapDataBaseName, NewMapInfo, ErrorMessage))
    {
        UE_LOG(LogSimSystem, Log, TEXT("Sim_Reset mapIndex : %d"), ResetIn->mapIndex);
        ResetIn->mapOriginLon = NewMapInfo.origin_Lon;
        ResetIn->mapOriginLat = NewMapInfo.origin_Lat;
        ResetIn->mapOriginAlt = NewMapInfo.origin_Alt;
        ResetIn->mapPath = NewMapInfo.mapPath;
        ResetIn->mapName = NewMapInfo.mapName;
        ResetIn->decryptFilePath = NewMapInfo.decryptFilePath;
        // currentMapInfo = NewMapInfo;
    }
    else
    {
        ShutdownSimModuleThread();
        FGenericPlatformMisc::RequestExit(false);
        return;
        // FMapInfo RestDefault;
        // currentMapInfo = RestDefault;
#ifdef WIN32
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ErrorMessage));
#endif
        // TODO: reset
        Reset();
        return;
    }

    // ReLoad config from game.ini
    int32 MapModeIndex = 0;
    GConfig->GetBool(TEXT("Mode"), TEXT("bLoadHadmap"), bNeedToLoadHadmap, GGameIni);
    GConfig->GetInt(TEXT("Mode"), TEXT("MapMode"), MapModeIndex, GGameIni);
    hadmapue4::MapMode MapMode = hadmapue4::MapMode::ROUTINGMAP;
    if (MapModeIndex == 0)
    {
        MapMode = hadmapue4::MapMode::ROUTINGMAP;
    }
    if (MapModeIndex == 1)
    {
        MapMode = hadmapue4::MapMode::MAPENGINE;
    }

    // Reset HadMap
    hadmapHandle = SHadmap;
    if (bNeedToLoadHadmap)
    {
        UE_LOG(LogSimSystem, Log, TEXT("Loading hadmap data file."));
        if (hadmapHandle)
        {
            if (hadmapHandle->Init(MapMode, ResetIn->mapDataBasePath, ResetIn->mapOriginLon, ResetIn->mapOriginLat,
                    ResetIn->mapOriginAlt, ResetIn->decryptFilePath))
            {
                UE_LOG(LogSimSystem, Log, TEXT("Init hadmap success!"));
            }
            else
            {
                UE_LOG(LogSimSystem, Warning, TEXT("Init hadmap failed!"));
                ResetFaildStr = TEXT("Init hadmap failed!");
                ShutdownSimModuleThread();
                FGenericPlatformMisc::RequestExit(false);
                return;
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString(TEXT("Init hadmap failed!"))));
            }
        }
        else
        {
            UE_LOG(LogSimSystem, Warning, TEXT("Init hadmap failed! hadmapHandle is null!"));
            ResetFaildStr = TEXT("Init hadmap failed! hadmapHandle is null!");
            ShutdownSimModuleThread();
            FGenericPlatformMisc::RequestExit(false);
            return;
            FMessageDialog::Open(
                EAppMsgType::Ok, FText::FromString(FString(TEXT("Init hadmap failed! hadmapHandle is null!"))));
        }
    }
    else
    {
        UE_LOG(LogSimSystem, Log, TEXT("Dont need load hadmap data file."));
        if (hadmapHandle)
        {
            if (hadmapHandle->Init(
                    ResetIn->mapOriginLon, ResetIn->mapOriginLat, ResetIn->mapOriginAlt, ResetIn->decryptFilePath))
            {
                UE_LOG(LogSimSystem, Log, TEXT("Init hadmap success!"));
            }
            else
            {
                UE_LOG(LogSimSystem, Warning, TEXT("Init hadmap failed!"));
                ResetFaildStr = TEXT("Init hadmap failed!");
                ShutdownSimModuleThread();
                FGenericPlatformMisc::RequestExit(false);
                return;
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString(TEXT("Init hadmap failed!"))));
            }
        }
        else
        {
            UE_LOG(LogSimSystem, Warning, TEXT("Init hadmap failed! hadmapHandle is null!"));
            ResetFaildStr = TEXT("Init hadmap failed! hadmapHandle is null!");
            ShutdownSimModuleThread();
            FGenericPlatformMisc::RequestExit(false);
            return;
            FMessageDialog::Open(
                EAppMsgType::Ok, FText::FromString(FString(TEXT("Init hadmap failed! hadmapHandle is null!"))));
        }
    }

    // Check map asset exist
    if (!FPackageName::DoesPackageExist(ResetIn->mapPath))
    {
        UE_LOG(LogSimSystem, Warning, TEXT("The map resource does not exist in the package file! (MapPath:%s)"),
            *ResetIn->mapPath);
        ResetFaildStr = TEXT("The map resource does not exist in the package file!");
        ShutdownSimModuleThread();
        FGenericPlatformMisc::RequestExit(false);
        return;
        FMessageDialog::Open(EAppMsgType::Ok,
            FText::FromString(FString(TEXT("The map resource does not exist in the package file!\n(Path:")) +
                              ResetIn->mapPath + TEXT("\nName:") + ResetIn->mapName + TEXT(")")));
        Reset();
    }
    else
    {
        // Travel map
        check(GetWorld() && GetWorld()->GetAuthGameMode());
        // LoadPackageAsync(mapPaht,
        // FLoadPackageAsyncDelegate::CreateLambda([=](const FName& PackageName, UPackage* LoadedPackage,
        // EAsyncLoadingResult::Type Result)
        //{
        //     if (Result == EAsyncLoadingResult::Succeeded)
        //     {
        //         if (!GetWorld()->ServerTravel(mapPaht, false, false))
        //         {
        //             UE_LOG(LogSimGameInstance, Warning, TEXT("Server Travel Level Failed!"));
        //         }
        //         bIsAllClientsLoadedWorld = false;
        //     }
        // }),
        // 0,
        // PKG_ContainsMap);

        // TODO: Show percentage of loading progress
        GetAsyncLoadPercentage(FName(*ResetIn->mapPath));
        if (!GetWorld()->ServerTravel(ResetIn->mapPath, false, false))
        {
            UE_LOG(LogSimGameInstance, Warning, TEXT("Server Travel Level Failed!"));
        }
        bIsAllClientsLoadedWorld = false;

        //// If not all client loaded world, wait for them loaded.
        // ADisplayGameModeBase* GM = Cast<ADisplayGameModeBase>(GetWorld()->GetAuthGameMode());
        // check(GM);
        // if (GM->CheckAllClientLoadedCurrentWorld())
        //{
        //     Sim_ResetAfterLoadedWorld();
        // }

        // TODO:check player

        ////Get Level name.
        // if (!GConfig->GetString(TEXT("MapIndex"), *mapName, levelName, GGameIni))
        //{
        //     levelName.Empty();
        // }
        // UE_LOG(LogTemp, Log, TEXT("%s: levelName is: %s"), *this->GetName(), *levelName);

        // levelState = ELevelStateEnum::ASYNCLOADING;
        ////ShowLoadUI
        // APlayerController* PC = GetWorld()->GetFirstPlayerController();
        // if (PC)
        //{
        //     ASimPlayerController* SPC = Cast<ASimPlayerController>(PC);
        //     if (SPC)
        //     {
        //         SPC->ShowLoadUI();
        //     }
        // }
    }
}

void UDisplayGameInstance::Sim_ResetAfterLoadedWorld()
{
    UE_LOG(LogSimGameInstance, Log, TEXT("Reset After Load World!"));

    // Update hadmap
    if (hadmapHandle && hadmapHandle->IsMapReady() && hadmapHandle->GetMapMode() == hadmapue4::MapMode::ROUTINGMAP)
    {
        hadmapHandle->UpdateRoutingmap(StaticCast<FSimResetIn*>(currentSimInData.Get())->startLon,
            StaticCast<FSimResetIn*>(currentSimInData.Get())->startLat,
            StaticCast<FSimResetIn*>(currentSimInData.Get())->startAlt);
    }

    // TODO: Async cook data
    // Cook data
    // FLocalResetIn NewResetIn = TransformResetData(*static_cast<FSimResetIn*>(currentSimData));
    GetWorld()->GetAuthGameMode<ADisplayGameModeBase>()->SimInput(*currentSimInData);

    //// All client have loaded world, begin send reset data to client.
    // for (size_t i = 0; i < clientConfigArry.Num(); i++)
    //{
    //     APlayerController* PC = GetPlayerControllerFromNetId(GetWorld(),
    //     *clientConfigArry[i].uniqueNetId.GetUniqueNetId().Get()); ADisplayPlayerController* DPC =
    //     Cast<ADisplayPlayerController>(PC); if (DPC)
    //     {
    //         UE_LOG(LogSimGameInstance, Log, TEXT("Send Data To PlayerController, client name: %s"),
    //         *clientConfigArry[i].playerName); DPC->Client_SimResetInput(NewResetIn);
    //         //DPC->resetIn = NewResetIn;
    //         clientConfigArry[i].state = ESimState::SA_RESET;
    //         clientConfigArry[i].simStat = TEXT("RESET");
    //     }
    //     else
    //     {
    //         UE_LOG(LogSimGameInstance, Error, TEXT("Can not find PlayerController from UniqueNetId, client name:
    //         %s"), *clientConfigArry[i].playerName);
    //     }
    // }

    // APlayerController* LocalPC = GetFirstLocalPlayerController(GetWorld());
    // UE_LOG(LogSimGameInstance, Log, TEXT("Trigger Loacl PlayerController, Name: %s"),
    // *LocalPC->GetPlayerState<ADisplayPlayerState>()->GetPlayerName()); ADisplayPlayerController* LocalDisplayPC =
    // Cast<ADisplayPlayerController>(LocalPC);
    ////UE_LOG(LogSimGameInstance, Log, TEXT("Local PlayerController`s ResetIn Data Id: %d"),
    /// LocalDisplayPC->resetIn.transportManager.vehicleManagerConfig.trafficConfigArry[0].id);
    // if (LocalDisplayPC)
    //{
    //     LocalDisplayPC->resetIn = NewResetIn;
    //     LocalDisplayPC->OnSimResetInput();
    // }

    // UE_LOG(LogSimGameInstance, Log, TEXT("simRestIn->configFilePath: %s"), *simRestIn->configFilePath);
    // GM->SimActionStart(*simRestIn, simState);

    bResetActionComplete = true;
    UE_LOG(LogSimGameInstance, Log, TEXT("Execute ResetAfterLoadedWorld Over!"));
}

void UDisplayGameInstance::Sim_Update()
{
    if (true)
    {
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> Update;
        Update = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        // UE_LOG(LogSimGameInstance, Log, TEXT("Begin Updat: %s seconds"), *FString::Printf(TEXT("%lld"),
        // Update.time_since_epoch().count()));
    }

    // if (GetWorld()->GetTimeSeconds() - ETime > 5.0)
    //{
    //     extern ENGINE_API float GAverageFPS;
    //     UE_LOG(LogSimGameInstance, Log, TEXT("FPS: %f "), GAverageFPS);
    //     ETime = GetWorld()->GetTimeSeconds();
    // }

    //// Update hadmap
    // if (hadmapHandle && hadmapHandle->IsMapReady() && hadmapHandle->GetMapMode() == hadmapue4::MapMode::ROUTINGMAP &&
    // bNeedToLoadHadmap)
    //{
    //     hadmapHandle->UpdateRoutingmap(simUpdateIn->egoData.position().x(), simUpdateIn->egoData.position().y(),
    //     simUpdateIn->egoData.position().z());
    // }

    // Cook data
    FLocalUpdateIn NewInData = TransformUpdateData(*StaticCast<FSimUpdateIn*>(currentSimInData.Get()));

    // Local PlayerController not using RPC
    if (clientConfigArry.Num() > 0)
    {
        APlayerController* PC =
            GetPlayerControllerFromNetId(GetWorld(), *clientConfigArry[0].uniqueNetId.GetUniqueNetId().Get());
        ADisplayPlayerController* DPC = Cast<ADisplayPlayerController>(PC);
        if (DPC)
        {
            // UE_LOG(LogSimGameInstance, Log, TEXT("Update Data To Local PlayerController, client name: %s"),
            // *clientConfigArry[0].playerName);
            DPC->updateIn = NewInData;
            DPC->OnSimUpdateInput();
            // UE_LOG(LogSimGameInstance, Log, TEXT("Copy Data To Local PlayerController Complete, client name: %s"),
            // *clientConfigArry[0].playerName);
            clientConfigArry[0].state = ESimState::SA_UPDATE;
            clientConfigArry[0].simStat = TEXT("UPDATE");
        }
        else
        {
            UE_LOG(LogSimGameInstance, Warning,
                TEXT("Can not find Local PlayerController from UniqueNetId, client name: %s"),
                *clientConfigArry[0].playerName);
        }
    }

    // Update scene
    for (size_t i = 1; i < clientConfigArry.Num(); i++)
    {
        APlayerController* PC =
            GetPlayerControllerFromNetId(GetWorld(), *clientConfigArry[i].uniqueNetId.GetUniqueNetId().Get());
        ADisplayPlayerController* DPC = Cast<ADisplayPlayerController>(PC);
        if (DPC)
        {
            // UE_LOG(LogSimGameInstance, Log, TEXT("Update Data To PlayerController, client name: %s"),
            // *clientConfigArry[i].playerName);

            std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> Update;
            Update = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            // UE_LOG(LogSimGameInstance, Log, TEXT("Begin Updat PlayerController: %s seconds"),
            // *FString::Printf(TEXT("%lld"), Update.time_since_epoch().count()));

            // DPC->Client_SimUpdateInput(NewInData);
            // UE_LOG(LogSimGameInstance, Log, TEXT("Copy Data To PlayerController Complete, client name: %s"),
            // *clientConfigArry[i].playerName);
            clientConfigArry[i].state = ESimState::SA_UPDATE;
            clientConfigArry[i].simStat = TEXT("UPDATE");
        }
        else
        {
            UE_LOG(LogSimGameInstance, Warning, TEXT("Can not find PlayerController from UniqueNetId, client name: %s"),
                *clientConfigArry[i].playerName);
        }
    }
}

void UDisplayGameInstance::Sim_Update_Defferred()
{
    // if (true)
    //{
    //     std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> Update;
    //     Update = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    //     //UE_LOG(LogSimGameInstance, Log, TEXT("Begin Updat: %s seconds"), *FString::Printf(TEXT("%lld"),
    //     Update.time_since_epoch().count()));
    // }

    // if (GetWorld()->GetTimeSeconds() - ETime > 5.0)
    //{
    //     extern ENGINE_API float GAverageFPS;
    //     UE_LOG(LogSimGameInstance, Log, TEXT("FPS: %f "), GAverageFPS);
    //     ETime = GetWorld()->GetTimeSeconds();
    // }

    //// Update hadmap
    // if (hadmapHandle && hadmapHandle->GetMapMode() == hadmapue4::MapMode::ROUTINGMAP)
    //{
    //     hadmapHandle->UpdateRoutingmap(simUpdateIn->egoData.position().x(), simUpdateIn->egoData.position().y(),
    //     simUpdateIn->egoData.position().z());
    // }

    //// Cook data
    // FLocalUpdateIn NewInData = TransformUpdateData(*simUpdateIn);

    // Local PlayerController not using RPC
    if (clientConfigArry.Num() > 0)
    {
        APlayerController* PC =
            GetPlayerControllerFromNetId(GetWorld(), *clientConfigArry[0].uniqueNetId.GetUniqueNetId().Get());
        ADisplayPlayerController* DPC = Cast<ADisplayPlayerController>(PC);
        if (DPC)
        {
            // UE_LOG(LogSimGameInstance, Log, TEXT("Update Data To Local PlayerController, client name: %s"),
            // *clientConfigArry[0].playerName); DPC->updateIn = NewInData;
            DPC->OnSimUpdateInput_Deferred();
            // UE_LOG(LogSimGameInstance, Log, TEXT("Copy Data To Local PlayerController Complete, client name: %s"),
            // *clientConfigArry[0].playerName);
            clientConfigArry[0].state = ESimState::SA_UPDATE;
            clientConfigArry[0].simStat = TEXT("UPDATE");
        }
        else
        {
            UE_LOG(LogSimGameInstance, Error,
                TEXT("Can not find Local PlayerController from UniqueNetId, client name: %s"),
                *clientConfigArry[0].playerName);
        }
    }

    //// Update scene
    // for (size_t i = 1; i < clientConfigArry.Num(); i++)
    //{
    //     APlayerController* PC = GetPlayerControllerFromNetId(GetWorld(),
    //     *clientConfigArry[i].uniqueNetId.GetUniqueNetId().Get()); ADisplayPlayerController* DPC =
    //     Cast<ADisplayPlayerController>(PC); if (DPC)
    //     {
    //         //UE_LOG(LogSimGameMode, Log, TEXT("Update Data To PlayerController, client name: %s"),
    //         *clientConfigArry[i].playerName);

    //        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> Update;
    //        Update = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    //        //UE_LOG(LogSimGameMode, Log, TEXT("Begin Updat PlayerController: %s seconds"),
    //        *FString::Printf(TEXT("%lld"), Update.time_since_epoch().count()));

    //        DPC->Client_SimUpdateInput(NewInData);
    //        //UE_LOG(LogSimGameMode, Log, TEXT("Copy Data To PlayerController Complete, client name: %s"),
    //        *clientConfigArry[i].playerName); clientConfigArry[i].state = ESimState::SA_UPDATE;

    //    }
    //    else
    //    {
    //        UE_LOG(LogSimGameMode, Error, TEXT("Can not find PlayerController from UniqueNetId, client name: %s"),
    //        *clientConfigArry[i].playerName);
    //    }
    //}
}

bool UDisplayGameInstance::Tick(float DeltaSeconds)
{
    // check clients all connected
    if (!bAllClientsLogin)
    {
        // TODO: process unconnet
        return true;
    }
    if (syncOneFrame)    // sigle frame
    {
        OutputData();    // SensorManger update
        SendSimData();
        ReceiveSimData();    //
        SyncSimData();       // location update.
    }
    else
    {
        OutputData();        // SensorManger update
        ReceiveSimData();    // read location, begin of step and waiting for step
        SyncSimData();       // location update.
        SendSimData();       // public sensor
    }

    if (GetWorld()->GetTimeSeconds() - ETime > 15.0)
    {
        extern ENGINE_API float GAverageFPS;
        UE_LOG(LogSimGameInstance, Log, TEXT("FPS: %f "), GAverageFPS);
        ETime = GetWorld()->GetTimeSeconds();
    }
    return true;
}

bool UDisplayGameInstance::HostSession(
    TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
    // Get the Online Subsystem to work with
    IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

    if (OnlineSub)
    {
        // Get the Session Interface, so we can call the "CreateSession" function on it
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

        if (Sessions.IsValid() && UserId.IsValid())
        {
            /*
                Fill in all the Session Settings that we want to use.

                There are more with SessionSettings.Set(...);
                For example the Map or the GameMode/Type.
            */
            SessionSettings = MakeShareable(new FOnlineSessionSettings());

            SessionSettings->bIsLANMatch = bIsLAN;
            SessionSettings->bUsesPresence = bIsPresence;
            SessionSettings->NumPublicConnections = MaxNumPlayers;
            SessionSettings->NumPrivateConnections = 0;
            SessionSettings->bAllowInvites = true;
            SessionSettings->bAllowJoinInProgress = true;
            SessionSettings->bShouldAdvertise = true;
            SessionSettings->bAllowJoinViaPresence = true;
            SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

            SessionSettings->Set(SETTING_MAPNAME, FString("LobbyMap"), EOnlineDataAdvertisementType::ViaOnlineService);

            // Set the delegate to the Handle of the SessionInterface
            OnCreateSessionCompleteDelegateHandle =
                Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

            // Our delegate should get called when this is complete (doesn't need to be successful!)
            return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
    }

    return false;
}

void UDisplayGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
        FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

    // Get the OnlineSubsystem so we can get the Session Interface
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        // Get the Session Interface to call the StartSession function
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

        if (Sessions.IsValid())
        {
            // Clear the SessionComplete delegate handle, since we finished this call
            Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
            if (bWasSuccessful)
            {
                // Set the StartSession delegate handle
                OnStartSessionCompleteDelegateHandle =
                    Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

                // Our StartSessionComplete delegate should get called after this
                Sessions->StartSession(SessionName);
            }
        }
    }
}

void UDisplayGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
        FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

    // Get the Online Subsystem so we can get the Session Interface
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        // Get the Session Interface to clear the Delegate
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            // Clear the delegate, since we are done with this call
            Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
        }
    }

    // If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
    if (bWasSuccessful)
    {
        UGameplayStatics::OpenLevel(GetWorld(), "LobbyMap", true, "listen");
    }
}

void UDisplayGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, bool bIsLAN, bool bIsPresence)
{
    // Get the OnlineSubsystem we want to work with
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

    if (OnlineSub)
    {
        // Get the SessionInterface from our OnlineSubsystem
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

        if (Sessions.IsValid() && UserId.IsValid())
        {
            /*
                Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to
               have!
            */
            SessionSearch = MakeShareable(new FOnlineSessionSearch());

            SessionSearch->bIsLanQuery = bIsLAN;
            SessionSearch->MaxSearchResults = 20;
            SessionSearch->PingBucketSize = 50;

            // We only want to set this Query Setting if "bIsPresence" is true
            if (bIsPresence)
            {
                SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
            }

            TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

            // Set the Delegate to the Delegate Handle of the FindSession function
            OnFindSessionsCompleteDelegateHandle =
                Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

            // Finally call the SessionInterface function. The Delegate gets called once this is finished
            Sessions->FindSessions(*UserId, SearchSettingsRef);
        }
    }
    else
    {
        // If something goes wrong, just call the Delegate Function directly with "false".
        OnFindSessionsComplete(false);
    }
}

void UDisplayGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    GEngine->AddOnScreenDebugMessage(
        -1, 10.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful));

    // Get OnlineSubsystem we want to work with
    IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        // Get SessionInterface of the OnlineSubsystem
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
        if (Sessions.IsValid())
        {
            // Clear the Delegate handle, since we finished this call
            Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

            // Just debugging the Number of Search results. Can be displayed in UMG or something later on
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
                FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));

            // If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG
            // Widgets, like it is done in the BP version!
            if (SessionSearch->SearchResults.Num() > 0)
            {
                // "SessionSearch->SearchResults" is an Array that contains all the information. You can access the
                // Session in this and get a lot of information. This can be customized later on with your own classes
                // to add more information that can be set and displayed
                for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
                {
                    // OwningUserName is just the SessionName for now. I guess you can create your own Host Settings
                    // class and GameSession Class and add a proper GameServer Name here. This is something you can't do
                    // in Blueprint for example!
                    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
                        FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1,
                            *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName)));
                }
            }
        }
    }
}

bool UDisplayGameInstance::JoinSession2(
    TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
    // Return bool
    bool bSuccessful = false;

    // Get OnlineSubsystem we want to work with
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

    if (OnlineSub)
    {
        // Get SessionInterface from the OnlineSubsystem
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

        if (Sessions.IsValid() && UserId.IsValid())
        {
            // Set the Handle again
            OnJoinSessionCompleteDelegateHandle =
                Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

            // Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be
            // used to get such a "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
            bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
        }
    }

    return bSuccessful;
}

void UDisplayGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
        FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(Result)));

    // Get the OnlineSubsystem we want to work with
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        // Get SessionInterface from the OnlineSubsystem
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

        if (Sessions.IsValid())
        {
            // Clear the Delegate again
            Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

            // Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
            // This is something the Blueprint Node "Join Session" does automatically!
            APlayerController* const PlayerController = GetFirstLocalPlayerController();

            // We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
            // String for us by giving him the SessionName and an empty String. We want to do this, because
            // Every OnlineSubsystem uses different TravelURLs
            FString TravelURL;

            if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
            {
                // Finally call the ClienTravel. If you want, you could print the TravelURL to see
                // how it really looks like
                PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
            }
        }
    }
}

void UDisplayGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
        FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

    // Get the OnlineSubsystem we want to work with
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        // Get the SessionInterface from the OnlineSubsystem
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

        if (Sessions.IsValid())
        {
            // Clear the Delegate
            Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

            // If it was successful, we just load another level (could be a MainMenu!)
            if (bWasSuccessful)
            {
                UGameplayStatics::OpenLevel(GetWorld(), "LobbyMap", true);
            }
        }
    }
}

void UDisplayGameInstance::StartOnlineGame()
{
    // Creating a local player where we can get the UserID from
    ULocalPlayer* const Player = GetFirstGamePlayer();

    // Call our custom HostSession function. GameSessionName is a GameInstance variable
    HostSession(Player->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, true, true, 4);
}

void UDisplayGameInstance::FindOnlineGames()
{
    ULocalPlayer* const Player = GetFirstGamePlayer();

    FindSessions(Player->GetPreferredUniqueNetId().GetUniqueNetId(), true, true);
}

void UDisplayGameInstance::JoinOnlineGame()
{
    ULocalPlayer* const Player = GetFirstGamePlayer();

    // Just a SearchResult where we can save the one we want to use, for the case we find more than one!
    FOnlineSessionSearchResult SearchResult;

    // If the Array is not empty, we can go through it
    if (SessionSearch->SearchResults.Num() > 0)
    {
        for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
        {
            // To avoid something crazy, we filter sessions from ourself
            if (SessionSearch->SearchResults[i].Session.OwningUserId != Player->GetPreferredUniqueNetId())
            {
                SearchResult = SessionSearch->SearchResults[i];

                // Once we found a Session that is not ours, just join it. Instead of using a for loop, you could
                // use a widget where you click on and have a reference for the GameSession it represents which you can
                // use here
                JoinSession2(Player->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, SearchResult);
                break;
            }
        }
    }
}

void UDisplayGameInstance::DestroySessionAndLeaveGame()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (OnlineSub)
    {
        IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

        if (Sessions.IsValid())
        {
            Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

            Sessions->DestroySession(GameSessionName);
        }
    }
}

std::string UDisplayGameInstance::getAddress()
{
    return ipAddress;
}

std::string UDisplayGameInstance::getName()
{
    return moduleName;
}

// void UDisplayGameInstance::UnregisterAllSyncActor()
//{
//     syncActorArry.Empty();
//     vehicleConfig = FVehicleManagerConfigStruct();
// }

// void UDisplayGameInstance::SyncAllActor()
//{
//     for (auto &Elem : syncActorArry)
//     {
//         //Data
//         FVehicleManagerInDataStruct NewVehicleManagerInData;
//         FVehicleManagerOutDataStruct NewVehicleManagerOutData;
//         FTrafficLightManagerInDataStruct NewTrafficLightManagerInData;
//         FTrafficLightManagerOutDataStruct NewTrafficLightManagerOutData;
//         FPedestrianManagerInDataStruct NewPedestrianManagerInData;
//         FPedestrianManagerOutDataStruct NewPedestrianManagerOutData;
//         //FSyncInDataStruct* NewSyncInData = NULL;
//         //FSyncOutDataStruct* NewSynOutData = NULL;
//
//         //SyncInData
//         switch (Elem.inDataType)
//         {
//         case ESyncInDataTypeEnum::SYNCINDATA_NONE:
//             UE_LOG(LogTemp, Log, TEXT("Sync Type is: NONE."));
//             break;
//         case ESyncInDataTypeEnum::SYNCINDATA_VEHICLEMANAGER:
//         {
//             //FVehicleManagerInDataStruct& NewVehicleManagerInData = *(new FVehicleManagerInDataStruct());
//             //NewSyncInData = &NewVehicleManagerInData;
//
//             //TimeStamp
//             NewVehicleManagerInData.timeStamp = timeStamp;
//
//             //if (((int)timeStamp) % 100 == 0)
//             //{
//             //    //UE_LOG(LogTemp, Log, TEXT("trajectory begin: %s,%s"),
//             *FString::SanitizeFloat(trajectoryData->point()[0].x(), 9),
//             *FString::SanitizeFloat(trajectoryData->point()[0].y(), 9));
//             //    //UE_LOG(LogTemp, Log, TEXT("trajectory end: %s,%s"),
//             *FString::SanitizeFloat(trajectoryData->point()[trajectoryData->point().size()-1].x(), 9),
//             *FString::SanitizeFloat(trajectoryData->point()[trajectoryData->point().size() - 1].y(), 9));
//             //    //UE_LOG(LogTemp, Log, TEXT("time: %f"), timeStamp);
//             //}
//
//             //EgoVehicle
//             if (egoData && egoData->has_position())
//             {
//                 FVector egoVehicleLocation;
//
//                 double x = egoData->position().x();
//                 double y = egoData->position().y();
//                 double z = egoData->position().z();
//                 NewVehicleManagerInData.egoVehicleInData.lon = x;
//                 NewVehicleManagerInData.egoVehicleInData.lat = y;
//                 NewVehicleManagerInData.egoVehicleInData.alt = z;
//                 coord_trans_api::lonlat2local(x, y, z, mapOriginLon, mapOriginLat, mapOriginAlt);
//                 egoVehicleLocation.X = (float)(x * 100);
//                 //use negative y here as using different coordinates systems
//                 egoVehicleLocation.Y = -(float)(y * 100);
//                 //use fixed height as ego height is not correct from planner
//                 //egoVehicleLocation.Z = (float)(z * 100);
//                 egoVehicleLocation.Z = 4;
//                 //UE_LOG(LogTemp, Log, TEXT("Received egoVehicleLocation is: %s"), *egoVehicleLocation.ToString());
//                 //UE_LOG(LogTemp, Log, TEXT("Location.X is: %f"), (float)x);
//                 //UE_LOG(LogTemp, Log, TEXT("Location.Y is: %f"), (float)y);
//                 //UE_LOG(LogTemp, Log, TEXT("Location.Z is: %f"), (float)z);
//
//                 NewVehicleManagerInData.egoVehicleInData.location = egoVehicleLocation;
//                 //UE_LOG(LogTemp, Log, TEXT("GameInstance Ego location: %s"), *egoVehicleLocation.ToString());
//                 //UE_LOG(LogTemp, Log, TEXT("GameInstance Ego GPS: lon %.9f lat %.9f alt %.9f"),
//                 egoData->position().x(), egoData->position().y(), egoData->position().z());
//             }
//             if (egoData && egoData->has_rpy())
//             {
//                 //..
//                 FRotator egoVehicleRotation;
//                 egoVehicleRotation.Roll = (float)(egoData->rpy().x() * 180 / PI);
//                 egoVehicleRotation.Pitch = (float)(-egoData->rpy().y() * 180 / PI);
//                 egoVehicleRotation.Yaw = (float)(-egoData->rpy().z() * 180 / PI - 90);
//                 NewVehicleManagerInData.egoVehicleInData.rotation = egoVehicleRotation;
//                 //UE_LOG(LogTemp, Log, TEXT("GameInstance Ego rotation: %s"), *egoVehicleRotation.ToString());
//                 //UE_LOG(LogTemp, Log, TEXT("GameInstance Ego rpy: %s"), *FRotator(egoData->rpy().x(),
//                 egoData->rpy().y(), egoData->rpy().z()).ToString());
//                 //UE_LOG(LogTemp, Log, TEXT("GameInstance Ego timeStamp: %f"), timeStamp);
//             }
//             if (egoData && egoData->has_velocity())
//             {
//                 FVector Velocity = FVector(egoData->velocity().x(), egoData->velocity().y(),
//                 egoData->velocity().z()); NewVehicleManagerInData.egoVehicleInData.velocity = Velocity;
//                 //UE_LOG(LogTemp, Log, TEXT("Velocity Received is: %f, Timestamp is: %f"), Velocity.Size(),
//                 timeStamp);
//             }
//
//             if (egoData && planStatusData->has_expect_steering())
//             {
//                 NewVehicleManagerInData.egoVehicleInData.steeringAngle = planStatusData->expect_steering().angle();
//
//                 GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("expect_steering:
//                 %f"), NewVehicleManagerInData.egoVehicleInData.steeringAngle));
//                 GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("angular().z() is:
//                 %f"), egoData->angular().z()));
//             }
//
//             //TODO: not need to send timestamp data.
//             NewVehicleManagerInData.egoVehicleInData.timeStamp = timeStamp;
//             // Traffic Vehicle
//             if (trafficData)
//             {
//                 for (auto &Elem : trafficData->cars())
//                 {
//                     FVehicleInData NewVehicleInData;
//                     // Type
//                     //Elem.type();
//                     //UE_LOG(LogTemp, Log, TEXT("Vehicle ID is: %d"), Elem.id());
//                     //UE_LOG(LogTemp, Log, TEXT("Vehicle Type is: %d"), Elem.type());
//                     //ID
//                     NewVehicleInData.id = Elem.id();
//                     //Location
//                     FVector NewTrafficVehicleLocation;
//                     double x = Elem.x();
//                     double y = Elem.y();
//                     double z = mapOriginAlt;
//                     NewVehicleInData.lon = x;
//                     NewVehicleInData.lon = y;
//                     NewVehicleInData.lon = z;
//                     coord_trans_api::lonlat2local(x, y, z, mapOriginLon, mapOriginLat, mapOriginAlt);
//                     NewTrafficVehicleLocation.X = (float)(x * 100);
//                     NewTrafficVehicleLocation.Y = -(float)(y * 100);
//                     NewTrafficVehicleLocation.Z = 4;
//                     NewVehicleInData.location = NewTrafficVehicleLocation;
//                     //Rotation
//                     FRotator NewTrafficVehicleRotation;
//                     NewTrafficVehicleRotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
//                     NewVehicleInData.rotation = NewTrafficVehicleRotation;
//                     //TimeStamp
//                     NewVehicleInData.timeStamp = timeStamp;
//
//                     NewVehicleManagerInData.trafficVehicleInDataArry.Add(NewVehicleInData);
//                 }
//             }
//             // ControlSim
//             if (controlSimData && controlSimData->has_car2sim())
//             {
//                 NewVehicleManagerInData.egoVehicleMannedInData.drive_mode = controlSimData->car2sim().drive_mode();
//                 NewVehicleManagerInData.egoVehicleMannedInData.steering_angle =
//                 controlSimData->car2sim().steering_angle();
//                 NewVehicleManagerInData.egoVehicleMannedInData.throttle_value =
//                 controlSimData->car2sim().throttle_value();
//                 NewVehicleManagerInData.egoVehicleMannedInData.brake_pressure =
//                 controlSimData->car2sim().brake_pressure(); NewVehicleManagerInData.egoVehicleMannedInData.gearMode =
//                 controlSimData->car2sim().gear();
//
//                 ////GetWorld()->Exec(GetWorld(), TEXT("ENABLEALLSCREENMESSAGES"));
//                 //UE_LOG(LogTemp, Log, TEXT("drivemode is: %d"), controlSimData->car2sim().drive_mode());
//                 //UE_LOG(LogTemp, Log, TEXT("process_mode is: %d"), controlSimData->car2sim().process_mode());
//                 //UE_LOG(LogTemp, Log, TEXT("steering_angle is: %f"), controlSimData->car2sim().steering_angle());
//                 //UE_LOG(LogTemp, Log, TEXT("steering_torque is: %f"), controlSimData->car2sim().steering_torque());
//                 //UE_LOG(LogTemp, Log, TEXT("brake_pressure is: %f"), controlSimData->car2sim().brake_pressure());
//                 //UE_LOG(LogTemp, Log, TEXT("throttle_value is: %f"), controlSimData->car2sim().throttle_value());
//                 //UE_LOG(LogTemp, Log, TEXT("gear is: %d"), controlSimData->car2sim().gear());
//                 ////GetWorld()->Exec(GetWorld(), TEXT("DISABLEALLSCREENMESSAGES"));
//                 GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("drivemode: %d"),
//                 controlSimData->car2sim().drive_mode())); GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green,
//                 FString::Printf(TEXT("steering_angle: %f"), controlSimData->car2sim().steering_angle()));
//                 GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("brake_pressure: %f"),
//                 controlSimData->car2sim().brake_pressure())); GEngine->AddOnScreenDebugMessage(-1, 1.f,
//                 FColor::Green, FString::Printf(TEXT("throttle_value: %f"),
//                 controlSimData->car2sim().throttle_value()));
//             }
//
//             // Transform data
//             if (Elem.pointer)
//             {
//                 ISyncInterface* SyncObject = Cast<ISyncInterface>(Elem.pointer);
//                 if (SyncObject)
//                 {
//                     //ISyncInterface::Execute_Sync(SyncObject, NewVehicleManagerInData, NewVehicleManagerOutData);
//                     SyncObject->Sync(NewVehicleManagerInData, NewVehicleManagerOutData);
//                 }
//             }
//             break;
//         }
//         case ESyncInDataTypeEnum::SYNCCONFIG_TRAFFICLIGHTMANAGER:
//         {
//             // Traffic light
//             if (trafficData)
//             {
//                 if (trafficData->trafficlights_size() > 0)
//                 {
//                     for (size_t i = 0; i < trafficData->trafficlights().size(); i++)
//                     {
//                         FSignLightInDataStruct NewSignLightInData;
//                         NewSignLightInData.id = i;
//                         NewSignLightInData.color =
//                         static_cast<ELightColorEnum>(trafficData->trafficlights()[i].color()); FVector2D Location;
//                         double x = trafficData->trafficlights()[i].x();
//                         double y = trafficData->trafficlights()[i].y();
//                         double z = mapOriginAlt;
//                         coord_trans_api::lonlat2local(x, y, z, mapOriginLon, mapOriginLat, mapOriginAlt);
//                         Location.X = (float)(x * 100);
//                         Location.Y = -(float)(y * 100);
//                         NewSignLightInData.location = Location;
//                         NewSignLightInData.heading = trafficData->trafficlights()[i].heading();
//
//                         //UE_LOG(LogTemp, Log, TEXT("NewSignLightInData id: %d"), i);
//                         //UE_LOG(LogTemp, Log, TEXT("NewSignLightInData color: %d"),
//                         static_cast<int32>(trafficData->trafficlights()[i].color()));
//                         //UE_LOG(LogTemp, Log, TEXT("NewSignLightInData location: %s"),
//                         *NewSignLightInData.location.ToString());
//                         //UE_LOG(LogTemp, Log, TEXT("NewSignLightInData heading: %f"),
//                         trafficData->trafficlights()[i].heading());
//
//                         NewTrafficLightManagerInData.signLightInDataArry.Add(NewSignLightInData);
//                         NewTrafficLightManagerInData.timeStamp = timeStamp;
//                     }
//                     //for (auto& Elem : trafficData->trafficlights())
//                     //{
//
//                     //}
//                 }
//             }
//
//             // Transform data
//             if (Elem.pointer)
//             {
//                 ISyncInterface* SyncObject = Cast<ISyncInterface>(Elem.pointer);
//                 if (SyncObject)
//                 {
//                     //ISyncInterface::Execute_Sync(SyncObject, NewVehicleManagerInData, NewVehicleManagerOutData);
//                     SyncObject->Sync(NewTrafficLightManagerInData, NewTrafficLightManagerOutData);
//                 }
//             }
//             break;
//         }
//         case ESyncInDataTypeEnum::SYNCCONFIG_PEDESTRIANMANAGER:
//         {
//             if (trafficData)
//             {
//                 for (size_t i = 0; i < trafficData->staticobstacles().size(); i++)
//                 {
//                     if (trafficData->staticobstacles()[i].type() == k_TypePedestrain)
//                     {
//                         FPedestrianInDataStruct NewInPedestrian;
//                         NewInPedestrian.id = trafficData->staticobstacles()[i].id();
//                         NewInPedestrian.timeStamp = trafficData->staticobstacles()[i].t();
//                         NewInPedestrian.speed = 0;
//                         FVector Location;
//                         double x = trafficData->staticobstacles()[i].x();
//                         double y = trafficData->staticobstacles()[i].y();
//                         double z = mapOriginAlt;
//                         coord_trans_api::lonlat2local(x, y, z, mapOriginLon, mapOriginLat, mapOriginAlt);
//                         Location.X = (float)(x * 100);
//                         Location.Y = -(float)(y * 100);
//                         Location.Z = 4;
//                         NewInPedestrian.location = Location;
//                         FRotator Rotation;
//                         Rotation = FRotator(0, -trafficData->staticobstacles()[i].heading() * 180 / PI - 90, 0);
//                         NewInPedestrian.rotation = Rotation;
//
//                         NewPedestrianManagerInData.pedestrianInDataArry.Add(NewInPedestrian);
//                     }
//                 }
//             }
//
//             // Transform data
//             if (Elem.pointer)
//             {
//                 ISyncInterface* SyncObject = Cast<ISyncInterface>(Elem.pointer);
//                 if (SyncObject)
//                 {
//                     //ISyncInterface::Execute_Sync(SyncObject, NewVehicleManagerInData, NewVehicleManagerOutData);
//                     SyncObject->Sync(NewPedestrianManagerInData, NewPedestrianManagerOutData);
//                 }
//             }
//             break;
//         }
//
//         }
//
//         //// Transform data
//         //if (Elem.pointer)
//         //{
//         //    ISyncInterface* SyncObject = Cast<ISyncInterface>(Elem.pointer);
//         //    if (SyncObject)
//         //    {
//         //        //ISyncInterface::Execute_Sync(SyncObject, NewVehicleManagerInData, NewVehicleManagerOutData);
//         //        SyncObject->Sync(NewVehicleManagerInData, NewVehicleManagerOutData);
//         //    }
//         //}
//
//         //SyncOutData
//         //FSyncOutDataStruct* NewSyncOutData;
//         switch (Elem.outDataType)
//         {
//         case ESyncOutDataTypeEnum::SYNCOUTDATA_NONE:
//             break;
//         case ESyncOutDataTypeEnum::SYNCOUTDATA_VEHICLEMANAGER:
//         {
//             //if (!NewSynOutData)
//             //{
//             //    break;
//             //}
//             //FVehicleManagerOutDataStruct* NewOutDataPtr =
//             static_cast<FVehicleManagerOutDataStruct*>(NewSynOutData);
//             //if (!NewOutDataPtr)
//             //{
//             //    break;
//             //}
//             //FVehicleManagerOutDataStruct& NewVehicleManagerOutData = *NewOutDataPtr;
//
//             switch (NewVehicleManagerOutData.egoVehicleOutData.drivingMode)
//             {
//             case EDrivingMode::DM_AUTOPILOT:
//             {
//                 sim_msg::Sim2Car& NewSim2Car = *controlSimData->mutable_sim2car();
//                 NewSim2Car.set_en_angle(NewVehicleManagerOutData.egoVehicleMannedOutData.en_angle);
//                 NewSim2Car.set_angle(NewVehicleManagerOutData.egoVehicleMannedOutData.angle * 2);
//                 bIsLockStep = false;
//                 break;
//             }
//             case EDrivingMode::DM_MANNED:
//             {
//                 //SEND DATA
//                 // control
//                 sim_msg::Sim2Car& NewSim2Car = *controlSimData->mutable_sim2car();
//                 NewSim2Car.set_en_angle(NewVehicleManagerOutData.egoVehicleMannedOutData.en_angle);
//
//                 // planning
//                 sim_msg::Vec3& NewLocation = *overrideEgoLocation->mutable_position();
//                 sim_msg::Vec3& NewRotation = *overrideEgoLocation->mutable_rpy();
//                 sim_msg::Vec3& NewVelocity = *overrideEgoLocation->mutable_velocity();
//                 //position
//                 double X = NewVehicleManagerOutData.egoVehicleOutData.location.X / 100;
//                 double Y = NewVehicleManagerOutData.egoVehicleOutData.location.Y / -100;
//                 double Z = NewVehicleManagerOutData.egoVehicleOutData.location.Z / 100;
//                 coord_trans_api::local2lonlat(X, Y, Z, mapOriginLon, mapOriginLat, mapOriginAlt);
//                 NewLocation.set_x(X);
//                 NewLocation.set_y(Y);
//                 NewLocation.set_z(mapOriginAlt);
//                 //rpy
//                 NewRotation.set_x(NewVehicleManagerOutData.egoVehicleOutData.rotation.Roll*PI / 180);
//                 NewRotation.set_y(NewVehicleManagerOutData.egoVehicleOutData.rotation.Pitch*PI / 180);
//                 NewRotation.set_z(-(NewVehicleManagerOutData.egoVehicleOutData.rotation.Yaw + 90) *PI / 180);
//                 //angular
//                 //..
//                 //t
//                 overrideEgoLocation->set_t(timeStamp);
//                 //velocity
//                 NewVelocity.set_x(NewVehicleManagerOutData.egoVehicleOutData.velocity.X / 100.f);//cm 2 m
//                 NewVelocity.set_y(NewVehicleManagerOutData.egoVehicleOutData.velocity.Y / 100.f);//cm 2 m
//                 NewVelocity.set_z(NewVehicleManagerOutData.egoVehicleOutData.velocity.Z / 100.f);//cm 2 m
//
//                 //UE_LOG(LogTemp, Log, TEXT("MANNED position is: %s"), *FVector(overrideEgoLocation->position().x(),
//                 overrideEgoLocation->position().y(), overrideEgoLocation->position().z()).ToString());
//                 //UE_LOG(LogTemp, Log, TEXT("MANNED rpy is: %s"), *FVector(overrideEgoLocation->rpy().x(),
//                 overrideEgoLocation->rpy().y(), overrideEgoLocation->rpy().z()).ToString());
//                 //UE_LOG(LogTemp, Log, TEXT("MANNED velocity is: %s"), *FVector(overrideEgoLocation->velocity().x(),
//                 overrideEgoLocation->velocity().y(), overrideEgoLocation->velocity().z()).ToString());
//                 //UE_LOG(LogTemp, Log, TEXT("MANNED timestamp is: %f"), (float)overrideEgoLocation->t());
//                 bIsLockStep = true;
//                 break;
//             }
//             case EDrivingMode::DM_MANNED_VR:
//                 break;
//             default:
//                 break;
//             }
//             break;
//         }
//         case ESyncOutDataTypeEnum::SYNCCONFIG_TRAFFICLIGHTMANAGER:
//         {
//             break;
//         }
//         case ESyncOutDataTypeEnum::SYNCCONFIG_PEDESTRIANMANAGER:
//         {
//             break;
//         }
//
//         }
//     }
// }

// bool UDisplayGameInstance::LoadLevelAsync(const FString & InName, TArray<FString> Packages, FOnPackageLoaded
// OnPackageLoaded)
//{
//     FTimerHandle* TimerHandle = nullptr;
//
//     LoadPackageAsync(InName, FLoadPackageAsyncDelegate::CreateLambda([=](const FName& PackageName, UPackage*
//     LoadedPackage, EAsyncLoadingResult::Type Result) {
//         OnPackageLoaded.ExecuteIfBound(); }),
//         0, PKG_ContainsMap);
//
//     //StreamableManager
//     //FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
//     //Streamable.RequestAsyncLoad(,)
//
//     return true;
// }
//
// void UDisplayGameInstance::TravelLevel()
//{
//     //Init register data
//     UnregisterAllSyncActor();
//     //Open level.
//     UGameplayStatics::OpenLevel(GetWorld(), FName(*levelName));
// }
//
// void UDisplayGameInstance::UpdateLevelState(ELevelStateEnum _NewState)
//{
//     //UE_LOG(LogTemp,Log,TEXT(""))
//     levelState = _NewState;
//     if (levelState == ELevelStateEnum::OPENED)
//     {
//         //ForceGC
//         GetWorld()->ForceGarbageCollection(true);
//     }
//     if (levelState == ELevelStateEnum::INITED)
//     {
//         //Resume simulator thread.
//         if (displayNetworkManager)
//         {
//             displayNetworkManager->resumeThread();
//         }
//     }
// }

// int UDisplayGameInstance::RegisterSyncActor(FSyncActorStruct _NewSyncActor)
//{
//     if (!_NewSyncActor.pointer)
//     {
//         return -1;
//     }
//     if (!_NewSyncActor.pointer->GetClass()->ImplementsInterface(USyncInterface::StaticClass()))
//     {
//         return -1;
//     }
//
//     return syncActorArry.Add(_NewSyncActor);;
// }
// bool UDisplayGameInstance::GetSyncConfig(int _ID, FSyncConfigStruct& _Config)
//{
//     if (syncActorArry.Num() <= _ID && _ID < 0)
//     {
//         return false;
//     }
//     switch (syncActorArry[_ID].configType)
//     {
//     case ESyncConfigTypeEnum::SYNCCONFIG_NONE:
//         break;
//     case ESyncConfigTypeEnum::SYNCCONFIG_VEHICLEMANAGER:
//     {
//         vehicleConfig.egoVehicleConfig.id = 1;
//         //UE_LOG(LogTemp, Log, TEXT("GameInstance StartLon is: %f, StartLat is: %f"), StartLon, StartLat);
//         double Lon = 113.933024;
//         double Lat = 22.525981;
//         double Alt = 0.342232696;
//         LonLatToLocal(Lon, Lat, Alt);
//         vehicleConfig.egoVehicleConfig.location = FVector(Lon, Lat, 0.f);
//         UE_LOG(LogTemp, Log, TEXT("egoVehicleConfig.startLocation is: %s"),
//         *vehicleConfig.egoVehicleConfig.location.ToString()); vehicleConfig.egoVehicleConfig.rotation = FRotator(0,
//         0, 0);
//         //_Config = vehicleConfig;
//         FSyncConfigStruct* Config = &_Config;
//         FVehicleManagerConfigStruct* VMConfig = static_cast<FVehicleManagerConfigStruct*>(Config);
//         *VMConfig = vehicleConfig;
//         return true;
//         break;
//     }
//     case ESyncConfigTypeEnum::SYNCCONFIG_TRAFFICLIGHTMANAGER:
//     {
//         break;
//     }
//     case ESyncConfigTypeEnum::SYNCCONFIG_PEDESTRIANMANAGER:
//     {
//         break;
//     }
//
//     }
//     return false;
// }

// SaveDataThread * UDisplayGameInstance::GetSaveDataHandle() const
//{
//     return savedataThreadHandle;
// }

double UDisplayGameInstance::getTimeStamp()
{
    return currentSimInData->timeStamp;
}

void UDisplayGameInstance::setTimeStamp(double setTimeStamp)
{
    currentSimInData->timeStamp = setTimeStamp;
}

bool UDisplayGameInstance::IsLockStep()
{
    return bIsLockStep;
}

sim_msg::Location UDisplayGameInstance::getOverrideLocation()
{
    if (StaticCast<FSimUpdateIn*>(currentSimInData.Get()))
    {
        return StaticCast<FSimUpdateIn*>(currentSimInData.Get())->overrideEgoLocation;
    }
    return sim_msg::Location();
}

sim_msg::Sim2Car UDisplayGameInstance::GetControlSendData()
{
    if (StaticCast<FSimUpdateIn*>(currentSimInData.Get()))
    {
        return StaticCast<FSimUpdateIn*>(currentSimInData.Get())->controlSimData.sim2car();
    }
    return sim_msg::Sim2Car();
}

void UDisplayGameInstance::ReadSceneFileAndConfig(FSimIn& _InData)
{
    // read scene xml
    FSimResetIn* SimResetInPtr = static_cast<FSimResetIn*>(&_InData);
    if (!SimResetInPtr || SimResetInPtr->name != TEXT("RESET"))
    {
        UE_LOG(LogSimGameInstance, Error, TEXT("Input data is illegal!"));
        return;
    }

    SimResetInPtr->egoType = TEXT("transport/Type-1");
    SimResetInPtr->egoName = TEXT("suv");

    if (SimResetInPtr->sceneBuffer.empty())
    {
        if (CatalogDataSource)
        {
            /*
            CatalogDataSource->LoadConfig(SimResetInPtr->tadsimPath +
            TEXT("/data/scenario/Catalogs/Vehicles/default.xosc"), ECatalogType::CT_EgoVehicle);
            CatalogDataSource->LoadConfig(SimResetInPtr->tadsimPath +
            TEXT("/data/scenario/Catalogs/Vehicles/VehicleCatalog.xosc"), ECatalogType::CT_TrafficVehicle);
            CatalogDataSource->LoadConfig(SimResetInPtr->tadsimPath +
            TEXT("/data/scenario/Catalogs/Pedestrians/PedestrianCatalog.xosc"), ECatalogType::CT_Creature);
            CatalogDataSource->LoadConfig(SimResetInPtr->tadsimPath +
            TEXT("/data/scenario/Catalogs/MiscObjects/MiscObjectCatalog.xosc"), ECatalogType::CT_Obstacle);
             */
        }

        if (!SimResetInPtr->configFilePath.IsEmpty() && FPaths::FileExists(SimResetInPtr->configFilePath))
        {
            UE_LOG(LogSimGameMode, Log, TEXT("ConfigFilePath: %s"),
                *SimResetInPtr
                     ->configFilePath);    // C:\Users\jiangyulin\AppData\Roaming\TADSim\scenario\scene\boyuanlu_sql_20191120_001.sim
            const FString MapFilePath(SimResetInPtr->configFilePath);

            // read environment
            {
                FString jpath = SimResetInPtr->tadsimPath + TEXT("/data/scenario/sensors/config.json");
                if (FPaths::FileExists(jpath))
                {
                    FString jbuf;
                    if (FFileHelper::LoadFileToString(jbuf, *jpath))
                    {
                        TSharedPtr<FJsonObject> json;
                        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
                        if (FJsonSerializer::Deserialize(Reader, json))
                        {
                            if (json->GetBoolField(TEXT("environmentUseGlobal")))
                            {
                                SimResetInPtr->envConfigPath = SimResetInPtr->tadsimPath +
                                                               TEXT("/data/scenario/sensors/") +
                                                               json->GetStringField(TEXT("environmentGlobalFilePath"));
                            }
                            else
                            {
                                SimResetInPtr->envConfigPath =
                                    SimResetInPtr->tadsimPath + TEXT("/data/scenario/scene/") +
                                    FPaths::GetBaseFilename(MapFilePath) + TEXT("_environment.xml");
                            }
                        }
                        else
                        {
                            UE_LOG(LogSimGameMode, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
                        }
                    }
                }
            }

            // read .sim file
            FString Extension = FPaths::GetExtension(MapFilePath);
            if (Extension.Equals(TEXT("sim"), ESearchCase::IgnoreCase))
            {
                UE_LOG(LogTemp, Log, TEXT("Got .sim scene file"));
                FXmlFile SceneSimFile(MapFilePath);
                if (SceneSimFile.IsValid())
                {
                    UE_LOG(LogTemp, Log, TEXT("Got Scene xml"));
                    FXmlNode* Simulation = SceneSimFile.GetRootNode();
                    if (Simulation)
                    {
                        FXmlNode* Traffic = Simulation->FindChildNode(TEXT("traffic"));
                        FXmlNode* Planner = Simulation->FindChildNode(TEXT("planner"));
                        FXmlNode* mapfile = Simulation->FindChildNode(TEXT("mapfile"));
                        FXmlNode* Sensor = Simulation->FindChildNode(TEXT("sensor"));
                        FXmlNode* Env = Simulation->FindChildNode(TEXT("environment"));
                        FXmlNode* CatalogLocations = Simulation->FindChildNode(TEXT("CatalogLocations"));

                        if (mapfile)
                        {
                            // simRestIn->mapOriginLon = FCString::Atod(*mapfile->GetAttribute("lon"));
                            // simRestIn->mapOriginLat = FCString::Atod(*mapfile->GetAttribute("lat"));
                            // simRestIn->mapOriginAlt = FCString::Atod(*mapfile->GetAttribute("alt"));
                            SimResetInPtr->mapIndex = FCString::Atoi(*mapfile->GetAttribute("unrealLevelIndex"));
                            FString MapPathStr = mapfile->GetContent();
                            UE_LOG(LogTemp, Log, TEXT("INFO mapfile is: %s"), *MapPathStr);
                            FString LeftString;
                            FString RightString;
                            if (MapPathStr.Split(
                                    TEXT("/"), &LeftString, &RightString, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
                            {
                                SimResetInPtr->mapDataBaseName = RightString;
                                //// Load map path
                                ///*FString MapPath;*/
                                // FJsonSerializableArray MapOriginStr;
                                // if (GConfig->GetString(TEXT("MapList"), *mapName, mapPaht, GGameIni))
                                //{
                                //     if (GConfig->GetSingleLineArray(TEXT("MapList"), *mapPaht, MapOriginStr,
                                //     GGameIni))
                                //     {
                                //         // Must be befor set start location!
                                //         mapOriginLon = FCString::Atod(*MapOriginStr[0]);
                                //         mapOriginLat = FCString::Atod(*MapOriginStr[1]);
                                //         mapOriginAlt = FCString::Atod(*MapOriginStr[2]);
                                //     }
                                // }
                                // else
                                //{
                                //     UE_LOG(LogSimSystem, Error, TEXT("The map is not in list: %s"), *mapName);
                                //     FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString(TEXT("The map is
                                //     not in list"))));
                                // }
                            }
                            FString cfgPathStr(SimResetInPtr->configFilePath);
                            cfgPathStr = cfgPathStr.Replace(TEXT("\\"), TEXT("/"));
                            // if (cfgPathStr.Split(TEXT("/scene/"), &LeftString, &RightString, ESearchCase::IgnoreCase,
                            // ESearchDir::FromEnd))
                            //{
                            //     SimResetInPtr->mapDataBasePath = LeftString + MapPathStr;
                            //     SimResetInPtr->mapDataBasePath = mapPath_SQL.Replace(TEXT("\\"), TEXT("/"));
                            //     SimResetInPtr->mapDataBasePath = mapPath_SQL.Replace(TEXT(".."), TEXT(""));
                            //     UE_LOG(LogSimGameMode, Log, TEXT("mapPath_SQL: %s"),
                            //     *SimResetInPtr->mapDataBasePath);
                            // }
                            UE_LOG(LogSimGameMode, Log, TEXT("mapPath_SQL: %s"), *SimResetInPtr->mapDataBasePath);
                        }
                        if (Planner)
                        {
                            FXmlNode* EgoInfo = Planner->FindChildNode(TEXT("ego"));
                            FXmlNode* Route = Planner->FindChildNode(TEXT("route"));
                            FXmlNode* Theta = Planner->FindChildNode(TEXT("theta"));
                            FXmlNode* Start_v = Planner->FindChildNode(TEXT("start_v"));
                            FXmlNode* Altitude = Planner->FindChildNode(TEXT("altitude"));
                            if (EgoInfo)
                            {
                                FString egoCategory = EgoInfo->GetAttribute(TEXT("type"));
                                FString egoName = EgoInfo->GetAttribute(TEXT("name"));
                                if (!egoCategory.IsEmpty() && !egoName.IsEmpty())
                                {
                                    SimResetInPtr->egoCategory = egoCategory;
                                    SimResetInPtr->egoName = egoName;
                                }

                                if (SimResetInPtr->egoName.IsEmpty())
                                {
                                    SimResetInPtr->egoName = SimResetInPtr->egoCategory;
                                }

                                if (!SimResetInPtr->egoName.IsEmpty())
                                {
                                    if (SimResetInPtr->egoName.Contains(TEXT("AIV_FullLoad_001")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type503");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("AIV_Empty_001")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type504");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("MPV_001")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type505");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("AIV_FullLoad_002")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type506");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("AIV_Empty_002")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type507");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("MIFA_01")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type508");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("MiningDumpTruck_001")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type512");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("AIV_V5_FullLoad_001")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type513");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("AIV_V5_Empty_001")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type514");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("Unloaded_Trailer")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type515");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("Half_loaded_Trailer")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type516");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("Full_loaded_Trailer")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type517");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("suv")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type-1");
                                    }
                                    else if (SimResetInPtr->egoName.Contains(TEXT("truck")))
                                    {
                                        SimResetInPtr->egoType = TEXT("transport/Type-2");
                                    }
                                }
                            }
                            if (Route)
                            {
                                FString StartLocStr = Route->GetAttribute(TEXT("start"));
                                FString EndLocStr = Route->GetAttribute(TEXT("end"));
                                FString LeftString;
                                FString RightString;
                                if (StartLocStr.Split(TEXT(","), &LeftString, &RightString, ESearchCase::IgnoreCase,
                                        ESearchDir::FromEnd))
                                {
                                    SimResetInPtr->startLon = FCString::Atod(*LeftString);
                                    SimResetInPtr->startLat = FCString::Atod(*RightString);
                                    // simRestIn->startLon = startLon;
                                    // simRestIn->startLat = startLat;
                                    UE_LOG(LogTemp, Log, TEXT("Start lon is: %.9f"), SimResetInPtr->startLon);
                                    UE_LOG(LogTemp, Log, TEXT("Start lat is: %.9f"), SimResetInPtr->startLat);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("ERROR Cant get Start GPS!"));
                                }
                                if (EndLocStr.Split(TEXT(","), &LeftString, &RightString, ESearchCase::IgnoreCase,
                                        ESearchDir::FromEnd))
                                {
                                    SimResetInPtr->endLon = FCString::Atod(*LeftString);
                                    SimResetInPtr->endLat = FCString::Atod(*RightString);
                                    // simRestIn->endLon = endLon;
                                    // simRestIn->endLat = endLat;
                                    UE_LOG(LogTemp, Log, TEXT("EndLon lon is: %.9f"), SimResetInPtr->endLon);
                                    UE_LOG(LogTemp, Log, TEXT("EndLat lat is: %.9f"), SimResetInPtr->endLat);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("ERROR Cant get End GPS!"));
                                }
                            }
                            if (Altitude)
                            {
                                FString StartAltStr = Altitude->GetAttribute(TEXT("start"));
                                FString EndAltStr = Altitude->GetAttribute(TEXT("end"));
                                SimResetInPtr->startAlt = FCString::Atod(*StartAltStr);
                                SimResetInPtr->endAlt = FCString::Atod(*EndAltStr);
                                // simRestIn->startAlt = startAlt;
                                // simRestIn->endAlt = endAlt;
                            }
                            else
                            {
                                // simRestIn->startAlt = mapOriginAlt;
                                // simRestIn->endAlt = mapOriginAlt;
                                UE_LOG(LogTemp, Log, TEXT("Start altitude is null! Use map origin altitude."));
                            }
                            if (Theta)
                            {
                                SimResetInPtr->startTheta = FCString::Atod(*Theta->GetContent());
                                // simRestIn->startTheta = theta;
                                UE_LOG(LogTemp, Log, TEXT("Theta Is: %f"), SimResetInPtr->startTheta);
                            }
                            else
                            {
                                UE_LOG(LogTemp, Warning, TEXT("ERROR Cant Get Theta!"));
                            }
                            if (Start_v)
                            {
                                SimResetInPtr->startSpeed = FCString::Atod(*Start_v->GetContent());
                                // simRestIn->startSpeed = FCString::Atod(*Start_v->GetContent());
                            }
                        }

                        if (false /*Sensor || Env*/)
                        {
                            FString sensorGlobal;
                            FString envGlobal;
                            FString jpath = SimResetInPtr->tadsimPath + TEXT("/data/scenario/sensors/config.json");
                            if (FPaths::FileExists(jpath))
                            {
                                FString jbuf;
                                if (FFileHelper::LoadFileToString(jbuf, *jpath))
                                {
                                    TSharedPtr<FJsonObject> json;
                                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
                                    if (FJsonSerializer::Deserialize(Reader, json))
                                    {
                                        if (json->GetBoolField(TEXT("sensorUseGlobal")))
                                        {
                                            SimResetInPtr->sensorConfigPath =
                                                SimResetInPtr->tadsimPath + TEXT("/data/scenario/sensors/") +
                                                json->GetStringField(TEXT("sensorGlobalFilePath"));
                                        }
                                        else if (Sensor)
                                        {
                                            SimResetInPtr->sensorConfigPath = SimResetInPtr->tadsimPath +
                                                                              TEXT("/data/scenario/scene/") +
                                                                              Sensor->GetContent();
                                        }
                                        if (json->GetBoolField(TEXT("environmentUseGlobal")))
                                        {
                                            SimResetInPtr->envConfigPath =
                                                SimResetInPtr->tadsimPath + TEXT("/data/scenario/sensors/") +
                                                json->GetStringField(TEXT("environmentGlobalFilePath"));
                                        }
                                        else if (Env)
                                        {
                                            SimResetInPtr->envConfigPath = SimResetInPtr->tadsimPath +
                                                                           TEXT("/data/scenario/scene/") +
                                                                           Env->GetContent();
                                        }
                                    }
                                    else
                                    {
                                        UE_LOG(LogSimGameMode, Warning, TEXT("Can`t read json: %s"),
                                            *Reader->GetErrorMessage());
                                    }
                                }
                            }
                        }
                        else
                        {
                            SimResetInPtr->sensorConfigPath =
                                SimResetInPtr->tadsimPath + TEXT("/data/scenario/Catalogs/Sensors/SensorCatalog.xosc");
                        }
                    }
                }
            }
            // read .xosc file
            if (Extension.Equals(TEXT("xosc"), ESearchCase::IgnoreCase))
            {
                UE_LOG(LogTemp, Log, TEXT("Got .xosc scene file"));
                FXmlFile SceneSimFile(MapFilePath);
                if (SceneSimFile.IsValid())
                {
                    UE_LOG(LogTemp, Log, TEXT("Got Scene xml"));
                    FXmlNode* OpenScenario = SceneSimFile.GetRootNode();
                    if (OpenScenario)
                    {
                        FXmlNode* Params = OpenScenario->FindChildNode(TEXT("ParameterDeclarations"));
                        FXmlNode* RoadNetwork = OpenScenario->FindChildNode(TEXT("RoadNetwork"));
                        FXmlNode* Storyboard = OpenScenario->FindChildNode(TEXT("Storyboard"));
                        FXmlNode* Entities = OpenScenario->FindChildNode(TEXT("Entities"));

                        if (Params)
                        {
                            for (auto& Elem : Params->GetChildrenNodes())
                            {
                                if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("$UnrealLevelIndex")))
                                {
                                    SimResetInPtr->mapIndex = FCString::Atoi(*Elem->GetAttribute(TEXT("value")));
                                }
                                // if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("Speed_Ini_Ego")))
                                //{
                                //     SimResetInPtr->startSpeed = FCString::Atod(*Elem->GetAttribute(TEXT("value")));
                                // }
                                ////if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("Location_StartX_Ego")))
                                ////{
                                ////    SimResetInPtr->startLon = FCString::Atod(*Elem->GetAttribute(TEXT("value")));
                                ////    UE_LOG(LogTemp, Log, TEXT("Start lon is: %.9f"), SimResetInPtr->startLon);
                                ////}
                                ////if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("Location_StartY_Ego")))
                                ////{
                                ////    SimResetInPtr->startLat = FCString::Atod(*Elem->GetAttribute(TEXT("value")));
                                ////    UE_LOG(LogTemp, Log, TEXT("Start lat is: %.9f"), SimResetInPtr->startLat);
                                ////}
                            }
                            // UE_LOG(LogTemp, Log, TEXT("mapIndex is: %d"), SimResetInPtr->mapIndex);
                            // UE_LOG(LogTemp, Log, TEXT("mapIndex is: %f"), SimResetInPtr->startSpeed);
                        }
                        if (RoadNetwork)
                        {
                            FXmlNode* LogicFile = RoadNetwork->FindChildNode(TEXT("LogicFile"));
                            if (LogicFile)
                            {
                                FString MapPathStr = LogicFile->GetAttribute(TEXT("filepath"));
                                UE_LOG(LogTemp, Log, TEXT("INFO mapfile is: %s"), *MapPathStr);
                                FString LeftString;
                                FString RightString;
                                if (MapPathStr.Split(TEXT("/"), &LeftString, &RightString, ESearchCase::IgnoreCase,
                                        ESearchDir::FromEnd))
                                {
                                    SimResetInPtr->mapDataBaseName = RightString;
                                }
                                FString cfgPathStr(SimResetInPtr->configFilePath);
                                cfgPathStr = cfgPathStr.Replace(TEXT("\\"), TEXT("/"));
                                // if (cfgPathStr.Split(TEXT("/scene/"), &LeftString, &RightString,
                                // ESearchCase::IgnoreCase, ESearchDir::FromEnd))
                                //{
                                //     SimResetInPtr->mapDataBasePath = LeftString + MapPathStr;
                                //     SimResetInPtr->mapDataBasePath =
                                //     SimResetInPtr->mapDataBasePath.Replace(TEXT("\\"), TEXT("/"));
                                //     SimResetInPtr->mapDataBasePath =
                                //     SimResetInPtr->mapDataBasePath.Replace(TEXT(".."), TEXT(""));
                                //     UE_LOG(LogSimGameMode, Log, TEXT("mapPath_SQL: %s"),
                                //     *SimResetInPtr->mapDataBasePath);
                                // }
                                UE_LOG(LogSimGameMode, Log, TEXT("mapPath_SQL: %s"), *SimResetInPtr->mapDataBasePath);
                            }
                            else
                            {
                                UE_LOG(LogTemp, Warning, TEXT("ERROR Can not get map file path!"));
                            }
                        }
                        if (Storyboard)
                        {
                            // FXmlNode* Actions =
                            // Storyboard->FindChildNode(TEXT("Init"))->FindChildNode(TEXT("Actions")); for (auto
                            // &Private : Actions->GetChildrenNodes())
                            //{
                            //     if (Private->GetAttribute(TEXT("entityRef")).Equals(TEXT("Ego")))
                            //     {
                            //         for (auto &PrivateAction : Private->GetChildrenNodes())
                            //         {
                            //             if (PrivateAction->FindChildNode(TEXT("RoutingAction")))
                            //             {
                            //                 FXmlNode* Route =
                            //                 PrivateAction->FindChildNode(TEXT("RoutingAction"))->FindChildNode(
                            //    TEXT("AssignRouteAction"))->FindChildNode(TEXT("Route"));
                            //                 if (Route)
                            //                 {
                            //                     const FXmlNode* Waypoint = Route->GetFirstChildNode();
                            //                     if (Waypoint)
                            //                     {
                            //                         const FXmlNode* WorldPosition =
                            //                         Waypoint->FindChildNode(TEXT("Position"))->FindChildNode(TEXT("WorldPosition"));
                            //                         if (WorldPosition)
                            //                         {
                            //                             SimResetInPtr->startLon =
                            //                             FCString::Atod(*WorldPosition->GetAttribute(TEXT("x")));
                            //                             SimResetInPtr->startLat =
                            //                             FCString::Atod(*WorldPosition->GetAttribute(TEXT("y")));
                            //                             SimResetInPtr->startAlt =
                            //                             FCString::Atod(*WorldPosition->GetAttribute(TEXT("z")));
                            //                             SimResetInPtr->startTheta =
                            //                             FCString::Atod(*WorldPosition->GetAttribute(TEXT("h")));
                            //                         }
                            //                     }
                            //                 }
                            //             }
                            //         }
                            //     }
                            // }
                            // UE_LOG(LogTemp, Log, TEXT("Start lon is: %.9f"), SimResetInPtr->startLon);
                            // UE_LOG(LogTemp, Log, TEXT("Start lat is: %.9f"), SimResetInPtr->startLat);
                            // UE_LOG(LogTemp, Log, TEXT("Theta Is: %f"), SimResetInPtr->startTheta);
                        }
                        if (Entities)
                        {
                            for (auto& Elem : Params->GetChildrenNodes())
                            {
                                if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("Ego")))
                                {
                                    FXmlNode* S_Vehicle = Elem->FindChildNode(TEXT("Vehicle"));
                                    if (S_Vehicle)
                                    {
                                        SimResetInPtr->egoName = S_Vehicle->GetAttribute(TEXT("name"));
                                        SimResetInPtr->egoCategory = S_Vehicle->GetAttribute(TEXT("vehicleCategory"));
                                        if (SimResetInPtr->egoName.Equals(TEXT("suv")))
                                        {
                                            SimResetInPtr->egoType = TEXT("transport/Type-1");
                                        }
                                        else if (SimResetInPtr->egoName.Equals(TEXT("truck")))
                                        {
                                            SimResetInPtr->egoType = TEXT("transport/Type-2");
                                        }
                                        else if (SimResetInPtr->egoName.Equals(TEXT("AIV_FullLoad_001")))
                                        {
                                            SimResetInPtr->egoType = "transport/Type503";
                                        }
                                        else if (SimResetInPtr->egoName.Equals(TEXT("AIV_Empty_001")))
                                        {
                                            SimResetInPtr->egoType = "transport/Type504";
                                        }
                                        else if (SimResetInPtr->egoName.Equals(TEXT("MPV_001")))
                                        {
                                            SimResetInPtr->egoType = "transport/Type505";
                                        }
                                    }
                                }
                            }
                        }

                        UE_LOG(LogTemp, Log, TEXT("mapIndex is: %d"), SimResetInPtr->mapIndex);
                        UE_LOG(LogSimGameMode, Log, TEXT("mapPath_SQL: %s"), *SimResetInPtr->mapDataBasePath);
                        UE_LOG(LogTemp, Log, TEXT("Start lon is: %.9f"), SimResetInPtr->startLon);
                        UE_LOG(LogTemp, Log, TEXT("Start lat is: %.9f"), SimResetInPtr->startLat);
                        UE_LOG(LogTemp, Log, TEXT("Start alt is: %.9f"), SimResetInPtr->startAlt);
                        UE_LOG(LogTemp, Log, TEXT("Theta Is: %f"), SimResetInPtr->startTheta);

                        SimResetInPtr->sensorConfigPath =
                            SimResetInPtr->tadsimPath + TEXT("/data/scenario/Catalogs/Sensors/SensorCatalog.xosc");
                        // read sensor and environment  xml path
                        {
                            FString sensorGlobal;
                            FString envGlobal;
                            FString jpath = SimResetInPtr->tadsimPath + TEXT("/data/scenario/sensors/config.json");
                            if (FPaths::FileExists(jpath))
                            {
                                FString jbuf;
                                if (FFileHelper::LoadFileToString(jbuf, *jpath))
                                {
                                    TSharedPtr<FJsonObject> json;
                                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
                                    if (FJsonSerializer::Deserialize(Reader, json))
                                    {
                                        if (false /*json->GetBoolField(TEXT("sensorUseGlobal"))*/)
                                        {
                                            SimResetInPtr->sensorConfigPath =
                                                json->GetStringField(TEXT("sensorGlobalFilePath"));
                                        }
                                        if (json->GetBoolField(TEXT("environmentUseGlobal")))
                                        {
                                            SimResetInPtr->envConfigPath =
                                                json->GetStringField(TEXT("environmentGlobalFilePath"));
                                        }
                                    }
                                    else
                                    {
                                        UE_LOG(LogSimGameMode, Warning, TEXT("Can`t read json: %s"),
                                            *Reader->GetErrorMessage());
                                    }
                                }
                            }
                        }
                    }
                }
            }

            SimResetInPtr->SceneTrafficPath =
                MapFilePath.Left(MapFilePath.Len() - Extension.Len() - 1) + TEXT("_traffic.xml");
            FString SceneTrafficPath = SimResetInPtr->SceneTrafficPath;
            {
                FXmlFile SceneTrafficFile(SceneTrafficPath);
                if (SceneTrafficFile.IsValid())
                {
                    UE_LOG(LogTemp, Log, TEXT("Got SceneTraffic xml"));
                    FXmlNode* Traffic = SceneTrafficFile.GetRootNode();
                    if (Traffic)
                    {
                        FXmlNode* signlights = Traffic->FindChildNode(TEXT("signlights"));

                        if (signlights)
                        {
                            for (auto& Elem : signlights->GetChildrenNodes())
                            {
                                if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("$UnrealLevelIndex")))
                                {
                                    SimResetInPtr->mapIndex = FCString::Atoi(*Elem->GetAttribute(TEXT("value")));
                                }

                                // if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("Speed_Ini_Ego")))
                                //{
                                //     SimResetInPtr->startSpeed = FCString::Atod(*Elem->GetAttribute(TEXT("value")));
                                // }
                                ////if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("Location_StartX_Ego")))
                                ////{
                                ////    SimResetInPtr->startLon = FCString::Atod(*Elem->GetAttribute(TEXT("value")));
                                ////    UE_LOG(LogTemp, Log, TEXT("Start lon is: %.9f"), SimResetInPtr->startLon);
                                ////}
                                ////if (Elem->GetAttribute(TEXT("name")).Equals(TEXT("Location_StartY_Ego")))
                                ////{
                                ////    SimResetInPtr->startLat = FCString::Atod(*Elem->GetAttribute(TEXT("value")));
                                ////    UE_LOG(LogTemp, Log, TEXT("Start lat is: %.9f"), SimResetInPtr->startLat);
                                ////}
                            }
                        }

                        UE_LOG(LogTemp, Log, TEXT("mapIndex is: %d"), SimResetInPtr->mapIndex);
                        UE_LOG(LogSimGameMode, Log, TEXT("mapPath_SQL: %s"), *SimResetInPtr->mapDataBasePath);
                        UE_LOG(LogTemp, Log, TEXT("Start lon is: %.9f"), SimResetInPtr->startLon);
                        UE_LOG(LogTemp, Log, TEXT("Start lat is: %.9f"), SimResetInPtr->startLat);
                        UE_LOG(LogTemp, Log, TEXT("Start alt is: %.9f"), SimResetInPtr->startAlt);
                        UE_LOG(LogTemp, Log, TEXT("Theta Is: %f"), SimResetInPtr->startTheta);

                        SimResetInPtr->sensorConfigPath =
                            SimResetInPtr->tadsimPath + TEXT("/data/scenario/Catalogs/Sensors/SensorCatalog.xosc");
                        // read sensor and environment  xml path
                        {
                            FString sensorGlobal;
                            FString envGlobal;
                            FString jpath = SimResetInPtr->tadsimPath + TEXT("/data/scenario/sensors/config.json");
                            if (FPaths::FileExists(jpath))
                            {
                                FString jbuf;
                                if (FFileHelper::LoadFileToString(jbuf, *jpath))
                                {
                                    TSharedPtr<FJsonObject> json;
                                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
                                    if (FJsonSerializer::Deserialize(Reader, json))
                                    {
                                        if (false /*json->GetBoolField(TEXT("sensorUseGlobal"))*/)
                                        {
                                            SimResetInPtr->sensorConfigPath =
                                                json->GetStringField(TEXT("sensorGlobalFilePath"));
                                        }
                                        if (json->GetBoolField(TEXT("environmentUseGlobal")))
                                        {
                                            SimResetInPtr->envConfigPath =
                                                json->GetStringField(TEXT("environmentGlobalFilePath"));
                                        }
                                    }
                                    else
                                    {
                                        UE_LOG(LogSimGameMode, Warning, TEXT("Can`t read json: %s"),
                                            *Reader->GetErrorMessage());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        sim_msg::Scene scene;
        if (!scene.ParseFromString(SimResetInPtr->sceneBuffer))
        {
            UE_LOG(LogSimGameMode, Warning, TEXT("ParseFromString faild."));

            return;
        }
        UE_LOG(LogSimGameInstance, Log, TEXT("scenesceneBuffer : %s"), UTF8_TO_TCHAR(scene.DebugString().c_str()));
        SimResetInPtr->mapIndex = getMapIndex(SimResetInPtr->mapDataBaseName);
        SimResetInPtr->ModelPath = UTF8_TO_TCHAR(scene.setting().model3d_pathdir().c_str());
        if (SimResetInPtr->mapIndex == 0 && !FPaths::FileExists(SimResetInPtr->mapDataBasePath))
        {
            const auto& hadbuff = scene.setting().hadmap_data();
            if (!hadbuff.empty())
            {
                TArray<uint8> databuffer((uint8*) hadbuff.data(), hadbuff.size());
                SimResetInPtr->mapDataBasePath = FPaths::ProjectLogDir() + SimResetInPtr->mapDataBaseName;
                if (FPaths::FileExists(SimResetInPtr->mapDataBasePath))
                {
                    IFileManager::Get().Delete(*SimResetInPtr->mapDataBasePath);
                }
                FFileHelper::SaveArrayToFile(databuffer, *SimResetInPtr->mapDataBasePath);
                UE_LOG(LogSimGameMode, Warning, TEXT("Saved hadmap to %s."), *SimResetInPtr->mapDataBasePath);
            }
        }

        FString Type = "";
        std::string DebugStr = scene.DebugString();
        EgoName_ID_Mapping.Empty(scene.egos().size());
        for (int32 i = 0; i < scene.egos().size(); i++)
        {
            const auto& EgoData = scene.egos(i);
            if (EgoData.initial().common().waypoints().size() == 0)
            {
                continue;
            }
            FEgoInitInfo InitInfo;

            FString ElemGroupName = UTF8_TO_TCHAR(EgoData.group().c_str());
            int64 ElemID = i;    // EgoData.id();

            EgoName_ID_Mapping.Add(ElemGroupName, ElemID);
            UE_LOG(LogSimGameInstance, Log, TEXT("EgoName_ID_Mapping : %s_%ld"), *ElemGroupName, ElemID);

            InitInfo.EgoID = ElemID;
            InitInfo.startLon = EgoData.initial().common().waypoints(0).position().world().x();
            InitInfo.startLat = EgoData.initial().common().waypoints(0).position().world().y();
            InitInfo.startAlt = EgoData.initial().common().waypoints(0).position().world().z();
            InitInfo.startSpeed = EgoData.initial().common().waypoints(0).speed().value();
            InitInfo.startTheta = EgoData.initial().common().waypoints(0).heading_angle().value();

            Type = ANSI_TO_TCHAR(EgoData.physicles(0).common().model_3d().c_str());

            InitInfo.egoName = Type;
            if (!Type.IsEmpty()) {
                if (Type.Contains(TEXT("aiv/sm_tuobanche_jzx.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type503");
                } else if (Type.Contains(TEXT("aiv/sm_tubanche.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type504");
                } else if (Type.Contains(TEXT("mpv/BP_V_WLHG_B_lod_jihzxtd.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type505");
                } else if (Type.Contains(TEXT("sm_AIV_FullLoad_002/SM_AIV_FullLoad_002.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type506");
                } else if (Type.Contains(TEXT("sm_AIV_Empty_002/SM_AIV_Empty_002.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type507");
                } else if (Type.Contains(TEXT("sm_MIFA_01/sm_MIFA_01.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type508");
                } else if (Type.Contains(TEXT("maintruck/maintruk_jiaotongche.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type512");
                } else if (Type.Contains(TEXT("aiv002/sm_aiv_new_full.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type513");
                } else if (Type.Contains(TEXT("aiv002/sm_aiv_new.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type514");
                } else if (Type.Contains(TEXT("long-truck/sm_cgc_red_wuxiang_dm.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type515");
                } else if (Type.Contains(TEXT("long-truck/sm_cgc_red_xiaoxiang_dm.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type516");
                } else if (Type.Contains(TEXT("long-truck/sm_cgc_red_daxiang_dm.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type517");
                } else if (Type.Contains(TEXT("maintruck/maintruk_jiaotongche.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type5121");
                } else if (Type.Contains(TEXT("sm_baisifu/sm_baisifu.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type52");
                } else if (Type.Contains(TEXT("mainsuv/sm_mainsuv1.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type-1");
                } else if (Type.Contains(TEXT("main-combination/sk_hyblue_web.fbx"))) {
                    InitInfo.egoType = TEXT("transport/Type-2");
                }
            }

            SimResetInPtr->EgoInitInfoArry.Add(InitInfo);
        }
        if (SimResetInPtr->EgoInitInfoArry.Num() == 0)
        {
            UE_LOG(LogSimGameInstance, Error, TEXT("sceneBuffer EgoInitInfoArry is null"));
        }
    }
}

FString UDisplayGameInstance::getTrafficName(int32 id, int32 type)
{
    auto fd = trafficNames.Find(id);
    if (fd)
    {
        return *fd;
    }
    fd = &trafficNames.Add(id);

    FString typeKey = TEXT("transport/Type");
    typeKey += FString::FromInt(type);

    FString typeName;
    if (!GConfig->GetString(TEXT("TypeDef"), *typeKey, typeName, GGameIni))
    {
        GConfig->GetString(TEXT("TypeDef"), TEXT("transport/Type0"), typeName, GGameIni);
    }
    if (typeName.IsEmpty())
    {
        UE_LOG(LogSimSystem, Error, TEXT("Can not get TEYPE define"));
    }

    typeName = typeName.Replace(*FString(" "), *FString(""));    // Remove space
    FString LeftStr;
    FString RightStr;
    TArray<FString> names;
    while (typeName.Split(",", &LeftStr, &RightStr))
    {
        names.Add(LeftStr);
        typeName = RightStr;
    }
    names.Add(typeName);
    if (names.Num() > 1)
        typeName = names[FMath::RandRange(0, names.Num() - 1)];

    *fd = typeName;

    return *fd;
}

bool UDisplayGameInstance::IsWaitToReturnData()
{
    // TODO:Update bIsLockStep;
    return bIsLockStep;
}

SaveDataThread* UDisplayGameInstance::GetSaveDataHandle() const
{
    return savedataThreadHandle.Get();
}

void UDisplayGameInstance::SimPull(const FString _Command)
{
    // TODO
    // GetWorld()->GetAuthGameMode<ADisplayGameModeBase>()->SimInput(*currentSimData);
}

void UDisplayGameInstance::ReceiveSimData()
{
    if (currentSimInData && currentSimInData->bIsConsumed < CONSUMED_MAXTICK)
    {
        currentSimInData->bIsConsumed += 1;
    }

    if (bIsFrameSync)
    {
        uint32 waitTime = SyncModeWait <= 0 ? MAX_uint32 : SyncModeWait;
        if (!currentSimInData)
        {
            // UE_LOG(LogSimSystem, Log, TEXT("ReceiveSimData waiting"));
            threadSuspendedEvent->Wait(waitTime);
        }
        else
        {
            if (currentSimInData->name == TEXT("UPDATE"))
            {
                // UE_LOG(LogSimSystem, Log, TEXT("ReceiveSimData waiting"));
                threadSuspendedEvent->Wait(waitTime);
            }
            else if (currentSimInData->name == TEXT("RESET"))
            {
                if (currentSimInData->bIsConsumed > CONSUMED_MAXTICK)
                {
                    // UE_LOG(LogSimSystem, Log, TEXT("ReceiveSimData waiting"));
                    threadSuspendedEvent->Wait(waitTime);
                }
            }
            else if (currentSimInData->name == TEXT("STOP"))
            {
                if (!NeedExit)
                {
                    threadSuspendedEvent->Wait(waitTime);
                }
            }
        }
    }
    FScopeLock ScopeLock(&displayNetworkManager->displayModule->mutex_Input);
    if (simInDataArry.Num() > 0 && currentSimInData != simInDataArry.Top())
    {
        for (int i = simInDataArry.Num() - 2; i >= 2; i--)
        {
            simInDataArry.RemoveAt(i);
        }

        currentSimInData = simInDataArry.Top();
        currentSimInData->bIsConsumed = 0;
    }

    // UE_LOG(LogSimSystem, Log, TEXT("ReceiveSimData ok"));
}

void UDisplayGameInstance::SyncSimData()
{
    if (!currentSimInData || currentSimInData->bIsConsumed > 0)
    {
        return;
    }

    if (currentSimInData->name == TEXT("INIT"))
    {
        Sim_InitBeginLoadWorld();
    }
    if (currentSimInData->name == TEXT("RESET"))
    {
        Sim_ResetBeginLoadWorld();
    }
    if (currentSimInData->name == TEXT("UPDATE"))
    {
        SimInput(*currentSimInData);
        // Sim_Update();
    }
    if (currentSimInData->name == TEXT("STOP"))
    {
        // TODO: do stop
    }
}

void UDisplayGameInstance::OutputData()
{
    if (!currentSimInData || currentSimInData->bIsConsumed > 0)
    {
        return;
    }

    if (currentSimInData->name == TEXT("UPDATE"))
    {
        FSimIn SensorInData;
        SensorInData.name = TEXT("OUTPUT_SENSOR");
        SensorInData.timeStamp = currentSimInData->timeStamp;
        FSimUpdateIn* UpdateIn = StaticCast<FSimUpdateIn*>(currentSimInData.Get());
        if (sim_msg::Location* Location = UpdateIn->egoData.Find(ModuleGroupName))
        {
            SensorInData.timeStamp_ego = Location->t() * 1000;
        }
        if (sim_msg::Location* LocationContainer = UpdateIn->egoContainerData.Find(ModuleGroupName))
        {
            SensorInData.timeStamp_tail = LocationContainer->t() * 1000;
        }

        SimInput(SensorInData);
    }
}

void UDisplayGameInstance::SendSimData()
{
    if (!currentSimInData || currentSimInData->bIsConsumed > 0)
    {
        return;
    }

    if (currentSimInData->name == TEXT("INIT"))
    {
        {
            // FScopeLock ScopeLock(&displayNetworkManager->displayModule->mutex_Output);
            //  Write output data
        }
        bAllowSync = false;
        // displayNetworkManager->resumeThread();
    }
    else if (currentSimInData->name == TEXT("RESET"))
    {
        {
            // FScopeLock ScopeLock(&displayNetworkManager->displayModule->mutex_Output);
            //  Write output data
        }
        bAllowSync = false;
        // displayNetworkManager->resumeThread();
    }
    else if (currentSimInData->name == TEXT("UPDATE"))
    {
        {
            FScopeLock ScopeLock(&displayNetworkManager->displayModule->mutex_Output);
            // Write output data
            if (currentSimOutData.IsValid())
                simOutDataArry.Add(currentSimOutData);
            if (currentSimSensorOutData.IsValid())
                simOutDataArry.Add(currentSimSensorOutData);
            currentSimOutData = nullptr;
            currentSimSensorOutData = nullptr;
        }
        bAllowSync = true;

        if (!asynchronousMode)
        {
            // UE_LOG(LogSimSystem, Log, TEXT("displayNetworkManager resume"));
            displayNetworkManager->resumeThread();
        }
    }
    else if (currentSimInData->name == TEXT("STOP"))
    {
        {
            FScopeLock ScopeLock(&displayNetworkManager->displayModule->mutex_Output);
            // Write output data
        }
        bAllowSync = false;
        // displayNetworkManager->resumeThread();
    }
}

void UDisplayGameInstance::Shutdown()
{
    // ShutdownSimModuleThread();
    if (displayNetworkManager)
    {
        displayNetworkManager.Reset();
        UE_LOG(LogSimSystem, Log, TEXT("GameInstance Shutdown: Shut down SimModule Thread Successed."));
    }
    else
    {
        UE_LOG(LogSimSystem, Log, TEXT("GameInstance Shutdown: Shut down SimModule Thread Not Exist."));
    }

    FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

    FPlatformProcess::ReturnSynchEventToPool(threadSuspendedEvent);
    threadSuspendedEvent = nullptr;

    // kill savedata thread (byjiangyuLin
    if (savedataThreadHandle)
    {
        savedataThreadHandle->EnsureCompletion();
        savedataThreadHandle.Reset();
    }

    if (CatalogDataSource && CatalogDataSource->IsValidLowLevel())
    {
        CatalogDataSource->RemoveFromRoot();
        CatalogDataSource = nullptr;
    }

    if (RuntimeMeshLoader && RuntimeMeshLoader->IsValidLowLevel())
    {
        RuntimeMeshLoader->RemoveFromRoot();
        RuntimeMeshLoader = nullptr;
    }

    ////// kill hadmap
    // if (hadmapHandle)
    //{
    //     delete hadmapHandle;
    //     hadmapHandle = NULL;
    // }

    // if (routingMap)
    //{
    //     delete routingMap;
    //     routingMap = NULL;
    //     mapInterface = NULL;
    // }

    UGameInstance::Shutdown();
}

void UDisplayGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
    Super::LoadComplete(LoadTime, MapName);

    UE_LOG(LogSimSystem, Log, TEXT("LoadMapComplete! LoadTime: %f, MapName: %s"), LoadTime, *MapName);
}

bool UDisplayGameInstance::CreateSimModuleThread()
{
    // If exist return
    if (displayNetworkManager)
    {
        UE_LOG(LogSimSystem, Log, TEXT("GI: Create SimModule Thread Failed, SimModule Thread Is Exist."));
        return false;
    }

    //// Custom tick
    // GetTimerManager().ClearTimer(onTriggerSimTimer);
    // GetTimerManager().SetTimer(onTriggerSimTimer, this, &UDisplayGameInstance::CheckSimTriggerHighFrequency, 0.001f,
    // true);

    FString fAddress;
    if (FParse::Value(FCommandLine::Get(), TEXT("-address="), fAddress))    // TODO: check server
    {
        ipAddress = std::string(TCHAR_TO_UTF8(*fAddress));
    }
    else
    {
        UE_LOG(LogSimSystem, Log, TEXT("-address is null."));
    }

    FString fName;
    if (FParse::Value(FCommandLine::Get(), TEXT("-name="), fName))
    {
        moduleName = std::string(TCHAR_TO_UTF8(*fName));
    }
    else
    {
        UE_LOG(LogSimSystem, Log, TEXT("-name is null, use <Display>."));
    }
    // else
    //{
    //     UE_LOG(LogSimSystem, Log, TEXT("GI: Create SimModule Thread Failed, Can Not Get <-address=>."));
    //     return false;
    // }
    displayNetworkManager = MakeShared<DisplayNetworkManager>(this);
    UE_LOG(LogSimSystem, Log, TEXT("GI: Create SimModule Thread Successed."));
    return true;
}

void UDisplayGameInstance::ShutdownSimModuleThread()
{
    if (displayNetworkManager)
    {
        displayNetworkManager->resumeThread();
        displayNetworkManager->shutDown();
        displayNetworkManager.Reset();
        UE_LOG(LogSimSystem, Log, TEXT("GI: Shut down SimModule Thread Successed."));
    }
    else
    {
        UE_LOG(LogSimSystem, Log, TEXT("GI: Shut down SimModule Thread Successed, Thread Not Exist."));
    }
}

void UDisplayGameInstance::SimInitOut()
{
    // if (displayNetworkManager)
    //{
    //     displayNetworkManager->resumeThread();
    // }
    UE_LOG(LogSimSystem, Log, TEXT("Init succussed"));

    bIsSimActionSuccessed = true;
    // simState = ESimState::SA_DONE;

    // GetTimerManager().SetTimer(onTriggerSimTimer, this, &UDisplayGameInstance::CheckSimTriggerHighFrequency, 0.001f,
    // true);
}

void UDisplayGameInstance::SimResetOut(const FLocalResetOut& _ResetOutput, const FUniqueNetIdRepl& _ClientNetId)
{
    bool IsAllClientsReturn = true;
    for (auto& Elem : clientConfigArry)
    {
        if (Elem.uniqueNetId == _ClientNetId)
        {
            if (_ResetOutput.message == TEXT("SUCCESS"))
            {
                Elem.state = ESimState::SA_DONE;
                UE_LOG(LogSimSystem, Log, TEXT("Client %s Reset Successed, message: %s"), *Elem.playerName,
                    *_ResetOutput.message);
            }
            else
            {
                UE_LOG(LogSimSystem, Error, TEXT("Client %s Reset failed, message: %s"), *Elem.playerName,
                    *_ResetOutput.message);
            }
        }
        if (Elem.state != ESimState::SA_DONE)
        {
            IsAllClientsReturn = false;
        }
    }

    if (IsAllClientsReturn)
    {
        if (displayNetworkManager)
        {
            // displayNetworkManager->resumeThread();
            // UE_LOG(LogSimSystem, Log, TEXT("resumeThread!"));
        }
        UE_LOG(LogSimSystem, Log, TEXT("Reset succussed"));
        bIsSimActionSuccessed = true;
        // simState = ESimState::SA_DONE;
        // GetTimerManager().SetTimer(onTriggerSimTimer, this, &UDisplayGameInstance::CheckSimTriggerHighFrequency,
        // 0.001f, true);
    }
    // TODO: Combine ResetOut Data.
}

void UDisplayGameInstance::SimUpdateOut(const FLocalUpdateOut& _UpdateOutput, const FUniqueNetIdRepl& _ClientNetId)
{
    bool IsAllClientsReturn = true;
    for (auto& Elem : clientConfigArry)
    {
        if (Elem.uniqueNetId == _ClientNetId)
        {
            if (_UpdateOutput.message == TEXT("SUCCESS"))
            {
                Elem.bIsSimActionComplete = true;
                if (Elem.bIsSimActionComplete && Elem.bIsDeferredSimActionComplete)
                {
                    Elem.state = ESimState::SA_DONE;
                }
                // UE_LOG(LogSimSystem, Log, TEXT("Client %s Update Successed, message: %s"), *Elem.playerName,
                // *_UpdateOutput.message);
            }
            else
            {
                UE_LOG(LogSimSystem, Error, TEXT("Client %s Update failed, message: %s"), *Elem.playerName,
                    *_UpdateOutput.message);
            }
        }
        if (Elem.state != ESimState::SA_DONE)
        {
            IsAllClientsReturn = false;
        }
    }

    if (IsAllClientsReturn)
    {
        if (displayNetworkManager)
        {
            // displayNetworkManager->resumeThread();
            if (IsLockStep() && !asynchronousMode)
            {
                // displayNetworkManager->resumeThread();
                // UE_LOG(LogSimSystem, Log, TEXT("resumeThread!"));
            }
        }
        // UE_LOG(LogSimSystem, Log, TEXT("Update succussed"));
        bIsSimActionSuccessed = true;
        // simState = ESimState::SA_DONE;
        // GetTimerManager().SetTimer(onTriggerSimTimer, this, &UDisplayGameInstance::CheckSimTriggerHighFrequency,
        // 0.001f, true);
    }
    // threadSuspendedEvent->Wait();
}

void UDisplayGameInstance::SimUpdateOut_Deferred(
    const FLocalUpdateOut& _UpdateOutput, const FUniqueNetIdRepl& _ClientNetId)
{
    bool IsAllClientsReturn = true;
    for (auto& Elem : clientConfigArry)
    {
        if (Elem.uniqueNetId == _ClientNetId)
        {
            if (_UpdateOutput.message == TEXT("SUCCESS"))
            {
                Elem.bIsDeferredSimActionComplete = true;
                if (Elem.bIsSimActionComplete && Elem.bIsDeferredSimActionComplete)
                {
                    Elem.state = ESimState::SA_DONE;
                }
                // UE_LOG(LogSimSystem, Log, TEXT("Client %s Update Successed, message: %s"), *Elem.playerName,
                // *_UpdateOutput.message);
            }
            else
            {
                UE_LOG(LogSimSystem, Error, TEXT("Client %s Update failed, message: %s"), *Elem.playerName,
                    *_UpdateOutput.message);
            }
        }
        if (Elem.state != ESimState::SA_DONE)
        {
            IsAllClientsReturn = false;
        }
    }

    if (IsAllClientsReturn)
    {
        if (displayNetworkManager)
        {
            // displayNetworkManager->resumeThread();
            if (IsLockStep() && !asynchronousMode)
            {
                // displayNetworkManager->resumeThread();
                // UE_LOG(LogSimSystem, Log, TEXT("resumeThread!"));
            }
        }
        // UE_LOG(LogSimSystem, Log, TEXT("Update succussed"));
        bIsSimActionSuccessed = true;
        // simState = ESimState::SA_DONE;
        // GetTimerManager().SetTimer(onTriggerSimTimer, this, &UDisplayGameInstance::CheckSimTriggerHighFrequency,
        // 0.001f, true);
        if (!asynchronousMode)
        {
            displayNetworkManager->resumeThread();
        }
    }
    // threadSuspendedEvent->Wait();
}

bool UDisplayGameInstance::RegisterClientToSim(APlayerController* NewPlayer)
{
    if (!NewPlayer || !NewPlayer->GetPlayerState<ADisplayPlayerState>())
    {
        UE_LOG(LogSimSystem, Log, TEXT("Register Client To Sim Failed! Player Is Illegal."));
        return false;
    }
    if (clientConfigArry.Num() < clientNum)
    {
        FClientInfo NewClient;
        NewClient.uniqueNetId = NewPlayer->GetPlayerState<ADisplayPlayerState>()->GetUniqueId();
        NewClient.playerName = NewPlayer->GetPlayerState<ADisplayPlayerState>()->GetPlayerName();
        clientConfigArry.Add(NewClient);

        // FLocalResetOut NewResetOut;
        // clientResetOutArry.Add(NewResetOut);
        // FLocalUpdateOut NewUpdateOut;
        // clientUpdateOutArry.Add(NewUpdateOut);

        UE_LOG(LogSimSystem, Log, TEXT("Register Client To Sim Successed! Client Name: %s, Client Num: %d/%d"),
            *(clientConfigArry.Last().playerName), clientConfigArry.Num(), clientNum);

        // Connect is full
        if (clientConfigArry.Num() == clientNum)
        {
            bAllClientsLogin = true;
            // Create SimModuleThread to connect coordinator.
            CreateSimModuleThread();
        }
        return true;
    }
    else
    {
        UE_LOG(LogSimSystem, Warning,
            TEXT("Register Client To Sim Reject! Only Allow %d Client To Connect, Client Name: %s"), clientNum,
            *(NewPlayer->GetPlayerState<ADisplayPlayerState>()->GetPlayerName()));
        return false;
    }
}

bool UDisplayGameInstance::UnregisterClientFromSim(AController* Player)
{
    if (!Player || !Player->GetPlayerState<ADisplayPlayerState>())
    {
        UE_LOG(LogSimSystem, Warning, TEXT("Unregister Client From Sim Failed! Player Is Illegal."));
        return false;
    }

    for (size_t i = 0; i < clientConfigArry.Num(); i++)
    {
        if (Player->GetPlayerState<ADisplayPlayerState>()->GetUniqueId() == clientConfigArry[i].uniqueNetId)
        {
            clientConfigArry.RemoveAt(i);

            // clientResetOutArry.RemoveAt(i);
            // clientUpdateOutArry.RemoveAt(i);
            //  There is a client offline, shutdown simulator.
            bAllClientsLogin = false;
            ShutdownSimModuleThread();
            return true;
        }
    }

    UE_LOG(LogSimSystem, Warning, TEXT("Unregister Client From Sim Failed! Player Is Not Exist."));
    return false;
}

void UDisplayGameInstance::OnAllClientLoadedWorld()
{
    UE_LOG(LogSimSystem, Log, TEXT("All Clients Loaded World!"));
    bIsAllClientsLoadedWorld = true;

    if (currentSimInData->name == TEXT("RESET"))
    {
        Sim_ResetAfterLoadedWorld();
    }

    if (currentSimInData->name == TEXT("INIT"))
    {
        Sim_InitAfterLoadedWorld();
    }
}

void UDisplayGameInstance::SimInput(const FSimData& _Data)
{
    GetWorld()->GetAuthGameMode<ADisplayGameModeBase>()->SimInput(_Data);
    // UE_LOG(LogSimSystem, Log, TEXT("SimTrigger!"));
    // bIsSimActionSuccessed = false;
    // bIsSimTrigger = true;
}

// void UDisplayGameInstance::SimOutput(const FLocalData& _Output)
//{
//     const FLocalData* OutputPtr = &_Output;
//
//     switch (simState)
//     {
//         case ESimState::SA_DONE:
//         {
//             break;
//         }
//         case ESimState::SA_INIT:
//         {
//             if (displayNetworkManager)
//             {
//                 displayNetworkManager->resumeThread();
//             }
//             UE_LOG(LogSimSystem, Log, TEXT("Init succussed"));
//             break;
//         }
//         case ESimState::SA_RESET:
//         {
//             //UE_LOG(LogSimSystem, Log, TEXT("Reset message: %s"), *localRestOut->message);
//             if (displayNetworkManager)
//             {
//                 displayNetworkManager->resumeThread();
//             }
//
//             UE_LOG(LogSimSystem, Log, TEXT("Reset succussed"));
//             break;
//         }
//         case ESimState::SA_UPDATE:
//         {
//             // TODO: Set SimOutData
//             // ..
//             //const FLocalUpdateOut* UpdateOut = static_cast<const FLocalUpdateOut*>(OutputPtr);
//             //if (UpdateOut)
//             //{
//             //    *localUpdateOut = *UpdateOut;
//             //}
//             //UE_LOG(LogSimSystem, Log, TEXT("Update timeStamp: %f"), localUpdateOut->timeStamp);
//             if (displayNetworkManager)
//             {
//                 displayNetworkManager->resumeThread();
//             }
//
//             UE_LOG(LogSimSystem, Log, TEXT("Update succussed"));
//             break;
//         }
//
//     }
//
//     bIsSimActionSuccessed = true;
//     simState = ESimState::SA_DONE;
//
//     GetTimerManager().SetTimer(onTriggerSimTimer, this, &UDisplayGameInstance::CheckSimTriggerHighFrequency, 0.001f,
//     true);
// }

void UDisplayGameInstance::CheckSimTriggerHighFrequency()
{
    // UE_LOG(LogSimSystem, Log, TEXT("CheckSimTriggerHighFrequency"));

    if (bIsSimTrigger)
    {
        bIsSimTrigger = false;
        GetTimerManager().ClearTimer(onTriggerSimTimer);
        OnTriggerSim();
    }
}

void UDisplayGameInstance::OnTriggerSim()
{
    // UE_LOG(LogSimSystem, Log, TEXT("OnTriggerSim, time is: %s"), *FString::Printf(TEXT("%.9f"),
    // FPlatformTime::Seconds()));
    switch (simState)
    {
        case ESimState::SA_DONE:
            break;
        case ESimState::SA_INIT:
            // TFunction<void> Task = Sim_Init();
            // AsyncTask(ENamedThreads::GameThread, Task);
            Sim_InitBeginLoadWorld();
            break;
        case ESimState::SA_RESET:
            Sim_ResetBeginLoadWorld();
            break;
        case ESimState::SA_UPDATE:
            Sim_Update();
            break;
        default:
            break;
    }
}

FLocalResetIn UDisplayGameInstance::TransformResetData(const FSimResetIn& _SimResetIn)
{
    FLocalResetIn NewLocalResetIn;

    /*~ Ego ~*/
    FVehicleConfig EgoConfig;
    EgoConfig.id = 0;
    EgoConfig.timeStamp = _SimResetIn.timeStamp;
    EgoConfig.trafficType = ETrafficType::ST_Ego;
    if (!GConfig->GetString(TEXT("TypeDef"), TEXT("transport/Ego"), egoNames, GGameIni))
    {
        UE_LOG(LogSimSystem, Error, TEXT("Can not get ego define"));
    }
    EgoConfig.typeName = egoNames;
    // TODO: wheels transform
    // Location
    double x = startLon;
    double y = startLat;
    double z = startAlt;
    hadmapHandle->LonLatToLocal(x, y, z, EgoConfig.startLocation);
    // Rotation
    FRotator egoVehicleRotation(ForceInit);
    EgoConfig.startRotation = FRotator(0, -theta * 180 / PI - 90, 0);
    // Velocity
    FVector Velocity = EgoConfig.startRotation.Vector() * startSpeed;
    EgoConfig.initVelocity = Velocity;
    // Snap
    if (!GConfig->GetBool(TEXT("Mode"), TEXT("SnapGround"), EgoConfig.isEgoSnap, GGameIni))
    {
        UE_LOG(LogSimSystem, Error, TEXT("Can not get ego snamp mode"));
    }

    NewLocalResetIn.transportManager.vehicleManagerConfig.egoConfigArry.Add(EgoConfig);

    if (_SimResetIn.sceneBuffer.empty())
    {
        UE_LOG(LogSimSystem, Error, TEXT("Can not get scene Buffer"));
    }
    else
    {
        int64 EgoID = ModuleGroupName.IsEmpty() ? -1 : GetEgoIDByGroupName();
        NewLocalResetIn.sensorManager = ASensorManager::ParseSensorString(_SimResetIn.sceneBuffer, EgoID);
    }

    // traffic
    trafficNames.Empty();

    // Environment
    /*
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*envConfigPath))
    {
        envConfigPath = FPaths::ProjectDir() + TEXT("XMLFiles/EnvironmentConfig.xml");
    }
    UE_LOG(LogTemp, Log, TEXT("Environment Config Path: %s"), *envConfigPath);
    NewLocalResetIn.envManager.timeline.initData.Compute();
    AEnvManager::ReadConfigFile(envConfigPath, NewLocalResetIn.envManager.timeline);
    AEnvManager::ParseEnvString(_SimResetIn.sceneBuffer, NewLocalResetIn.envManager.timeline);
    */
    return NewLocalResetIn;
}

FLocalUpdateIn UDisplayGameInstance::TransformUpdateData(const FSimUpdateIn& _SimUpdateIn)
{
    UE_LOG(LogTemp, Log, TEXT("TransformUpdateData"));
    FLocalUpdateIn NewLocalUpdateIn;

    /*~ Ego ~*/
    FVehicleIn EgoInput;
    EgoInput.id = 0;
    EgoInput.timeStamp = _SimUpdateIn.timeStamp;
    EgoInput.trafficType = ETrafficType::ST_Ego;
    EgoInput.typeName = egoNames;
    /*
    // TODO: wheels transform
    // Location
    hadmapHandle->LonLatToLocal(_SimUpdateIn.egoData.position().x(), _SimUpdateIn.egoData.position().y(),
    _SimUpdateIn.egoData.position().z(), EgoInput.location);
    // Rotation
    FRotator egoVehicleRotation(ForceInit);
    EgoInput.rotation.Roll = (float)(_SimUpdateIn.egoData.rpy().x() * 180 / PI);
    EgoInput.rotation.Pitch = (float)(-_SimUpdateIn.egoData.rpy().y() * 180 / PI);
    EgoInput.rotation.Yaw = (float)(-_SimUpdateIn.egoData.rpy().z() * 180 / PI - 90);
    // Velocity
    FVector Velocity = FVector(_SimUpdateIn.egoData.velocity().x(), -_SimUpdateIn.egoData.velocity().y(),
    _SimUpdateIn.egoData.velocity().z()); EgoInput.velocity = Velocity;
    // Control
    EgoInput.steeringAngle = _SimUpdateIn.planStatusData.expect_steering().angle();
    // Add
    NewLocalUpdateIn.transportManager.vehicleManagerIn.egoVehicleInputArry.Add(EgoInput);
    */
    /*~ Traffic ~*/
    for (auto& Elem : _SimUpdateIn.trafficData.cars())
    {
        FVehicleIn TrafficInput;
        TrafficInput.id = Elem.id();
        TrafficInput.timeStamp = _SimUpdateIn.timeStamp;
        TrafficInput.trafficType = ETrafficType::ST_TRAFFIC;
        TrafficInput.typeName = getTrafficName(Elem.id(), Elem.type());
        // Location
        FVector NewTrafficVehicleLocation;
        double x = Elem.x();
        double y = Elem.y();
        double z = Elem.z();
        hadmapHandle->LonLatToLocal(x, y, z, TrafficInput.location);
        // Rotation
        TrafficInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
        // Velocity
        TrafficInput.velocity = TrafficInput.rotation.Vector().GetSafeNormal() * Elem.v();
        // Add
        NewLocalUpdateIn.transportManager.vehicleManagerIn.trafficVehicleInputArry.Add(TrafficInput);
    }
    // Updat global time
    NewLocalUpdateIn.transportManager.vehicleManagerIn.timeStamp = _SimUpdateIn.timeStamp;

    /* Creature */
    for (auto& Elem : _SimUpdateIn.trafficData.dynamicobstacles())
    {
        /* Pedestrian */
        if (Elem.type() >= 0 && Elem.type() <= 2)
        {
            FPedestrianInput PedestrianInput;
            PedestrianInput.id = Elem.id();
            PedestrianInput.timeStamp = Elem.t();
            if (Elem.type() == 0)
            {
                PedestrianInput.typeName = TEXT("creature/pedestrian.man");
            }
            else if (Elem.type() == 1)
            {
                PedestrianInput.typeName = TEXT("creature/pedestrian.boy");
            }
            else if (Elem.type() == 2)
            {
                PedestrianInput.typeName = TEXT("creature/pedestrian.oldwoman");
            }
            double x = Elem.x();
            double y = Elem.y();
            double z = Elem.z();
            hadmapHandle->LonLatToLocal(x, y, z, PedestrianInput.location);
            FRotator Rotation;
            PedestrianInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
            PedestrianInput.velocity = PedestrianInput.rotation.Quaternion().GetForwardVector() * Elem.v();

            NewLocalUpdateIn.creatureManager.pedestrianManager.pedestrianArry.Add(PedestrianInput);
        }
        /* Animal */
        if (Elem.type() >= 100)
        {
            FAnimalInput AnimalInput;
            AnimalInput.id = Elem.id();
            AnimalInput.timeStamp = Elem.t();
            if (Elem.type() == 100)
            {
                AnimalInput.typeName = TEXT("creature/Animal.cat");
            }
            else if (Elem.type() == 101)
            {
                AnimalInput.typeName = TEXT("creature/Animal.dog");
            }
            double x = Elem.x();
            double y = Elem.y();
            double z = Elem.z();
            hadmapHandle->LonLatToLocal(x, y, z, AnimalInput.location);
            FRotator Rotation;
            AnimalInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
            AnimalInput.velocity = AnimalInput.rotation.Quaternion().GetForwardVector() * Elem.v();

            NewLocalUpdateIn.creatureManager.animalArry.Add(AnimalInput);
        }
    }
    NewLocalUpdateIn.creatureManager.pedestrianManager.timeStamp = _SimUpdateIn.timeStamp;
    NewLocalUpdateIn.creatureManager.timeStamp = _SimUpdateIn.timeStamp;

    /* Obstacle */
    for (auto& Elem : _SimUpdateIn.trafficData.staticobstacles())
    {
        FObstacleInput ObstacleInput;
        ObstacleInput.id = Elem.id();
        ObstacleInput.timeStamp = Elem.t();
        FVector Location;
        double x = Elem.x();
        double y = Elem.y();
        double z = Elem.z();
        hadmapHandle->LonLatToLocal(x, y, z, ObstacleInput.location);
        FRotator Rotation;
        ObstacleInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
        if (Elem.type() == 0)
        {
            ObstacleInput.typeName = TEXT("obstacle.Sedan");
        }
        else if (Elem.type() == 1)
        {
            ObstacleInput.typeName = TEXT("obstacle.Box");
        }
        else if (Elem.type() == 2)
        {
            ObstacleInput.typeName = TEXT("obstacle.Person");
        }
        else if (Elem.type() == 3)
        {
            ObstacleInput.typeName = TEXT("obstacle.Cone");
        }
        else if (Elem.type() == 4)
        {
            ObstacleInput.typeName = TEXT("obstacle.Sand");
        }
        else
        {
            ObstacleInput.typeName = TEXT("obstacle.Box");
        }
        NewLocalUpdateIn.obstacleManager.obstacleArry.Add(ObstacleInput);
    }
    NewLocalUpdateIn.obstacleManager.timeStamp = _SimUpdateIn.timeStamp;

    /* TrafficLight */
    for (auto& Elem : _SimUpdateIn.trafficData.trafficlights())
    {
        FTrafficLightIn NewTrafficLightIn;
        NewTrafficLightIn.id = Elem.id();
        NewTrafficLightIn.age = Elem.age();
        if (Elem.color() == 0)
        {
            NewTrafficLightIn.color = FColor::Green;
        }
        else if (Elem.color() == 1)
        {
            NewTrafficLightIn.color = FColor::Yellow;
        }
        else if (Elem.color() == 2)
        {
            NewTrafficLightIn.color = FColor::Red;
        }
        // else if (Elem.color() == 3)
        //{
        //     NewTrafficLightIn.color = FColor::Black;
        // }
        else
        {
            NewTrafficLightIn.color = FColor::Green;
        }
        double x = Elem.x();
        double y = Elem.y();
        double z = Elem.z();
        hadmapHandle->LonLatToLocal(x, y, z, NewTrafficLightIn.location);
        NewTrafficLightIn.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
        NewTrafficLightIn.timeStamp = _SimUpdateIn.timeStamp;
        NewLocalUpdateIn.signallightManager.trafficlightArry.Add(NewTrafficLightIn);
    }

    /* Environment */
    NewLocalUpdateIn.envManager.timeStamp = _SimUpdateIn.timeStamp;

    /* Sensor */
    NewLocalUpdateIn.sensorManager.timeStamp = _SimUpdateIn.timeStamp;

    return NewLocalUpdateIn;
}

bool UDisplayGameInstance::GetMapInfo(
    int32 _MapIndex, const FString& _MapFileName, FMapInfo& _MapInfo, FString& _ErrorMessage)
{
    /* MapIndex invalid */
    // if (_MapIndex == -1)
    //{
    _MapIndex = getMapIndex(_MapFileName);
    //}
    /* MapName */
    if (!GConfig->GetString(TEXT("MapName"), *FString::FromInt(_MapIndex), _MapInfo.mapName, GGameIni))
    {
        // UScriptStruct* Struct = _MapInfo.StaticStruct();
        // FString Output = TEXT("");
        // Struct->ExportText(Output, &_MapInfo, nullptr, this, (PPF_ExportsNotFullyQualified | PPF_Copy | PPF_Delimited
        // | PPF_IncludeTransient), nullptr); FString VariableName = StandardizeCase(Property->GetName());
        // Struct->ImportText(*MapInfoStr, &_MapInfo, this, (PPF_ExportsNotFullyQualified | PPF_Copy | PPF_Delimited |
        // PPF_IncludeTransient), nullptr, Struct->GetName());
        UE_LOG(LogTemp, Error, TEXT("Can't find map!(MapIndex: %d, MapFileName: %s)"), _MapIndex, *_MapFileName);
        _ErrorMessage = TEXT("Can't find map!");
        return false;
    }

    /* MapPath */
    if (!GConfig->GetString(TEXT("MapPath"), *_MapInfo.mapName, _MapInfo.mapPath, GGameIni))
    {
        UE_LOG(LogTemp, Error,
            TEXT(
                "Can't find map path! The path of the map is not defined!(MapIndex: %d, MapFileName: %s, MapName: %s)"),
            _MapIndex, *_MapFileName, *_MapInfo.mapName);
        _ErrorMessage = TEXT("Can't find map path! The path of the map is not defined!");
        return false;
    }

    /* MapOrigin */
    FJsonSerializableArray ValueArry;
    if (!GConfig->GetSingleLineArray(TEXT("MapOrigin"), *_MapInfo.mapPath, ValueArry, GGameIni))
    {
        UE_LOG(LogTemp, Error,
            TEXT("Can't find map origin! The origin of the map is not defined!(MapIndex: %d, MapFileName: %s, MapName: "
                 "%s)"),
            _MapIndex, *_MapFileName, *_MapInfo.mapName);
        _ErrorMessage = TEXT("Can't find map origin! The origin of the map is not defined!");
        return false;
    }
    if (ValueArry.Num() == 3)
    {
        _MapInfo.origin_Lon = FCString::Atod(*ValueArry[0]);
        _MapInfo.origin_Lat = FCString::Atod(*ValueArry[1]);
        _MapInfo.origin_Alt = FCString::Atod(*ValueArry[2]);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Map origin is illegal!(MapIndex: %d, MapFileName: %s, MapName: %s)"), _MapIndex,
            *_MapFileName, *_MapInfo.mapName);
        _ErrorMessage = TEXT("Map origin is illegal!");
        return false;
    }

    /* MapDecrypt */
    if (GConfig->GetString(TEXT("MapDecrypt"), *_MapFileName, _MapInfo.decryptFilePath, GGameIni))
    {
        if (!_MapInfo.decryptFilePath.IsEmpty())
        {
            // Absolute decrypt file path
            _MapInfo.decryptFilePath = FPaths::ProjectDir() + _MapInfo.decryptFilePath;
        }
    }

    return true;
}

void UDisplayGameInstance::StepUpdateImmediately(const FSimUpdateIn& simIn, TArray<FSimUpdateOut>& simOuts)
{
}

FString UDisplayGameInstance::GetGameConfig(const TCHAR* Section, const TCHAR* Key)
{
    FString sConfig;
    GConfig->GetString(Section, Key, sConfig, GGameIni);
    return sConfig;
}

void UDisplayGameInstance::SetAsynchronousMode(bool _Active)
{
    asynchronousMode = _Active;
}

void UDisplayGameInstance::SimOutput(const FSimData& _Data)
{
    if (_Data.name == TEXT("OUTPUT_SENSOR"))
    {
        currentSimSensorOutData = MakeShared<FSimSensorUpdateOut>();
        *StaticCast<FSimSensorUpdateOut*>(currentSimSensorOutData.Get()) =
            *StaticCast<const FSimSensorUpdateOut*>(&_Data);
    }
    else if (_Data.name == TEXT("OUTPUT_INIT"))
    {
        // TODO: Combine output data
    }

    else if (_Data.name == TEXT("RESET"))
    {
    }
    else if (_Data.name == TEXT("UPDATE"))
    {
        currentSimOutData = MakeShared<FSimUpdateOut>();
        *StaticCast<FSimUpdateOut*>(currentSimOutData.Get()) = *StaticCast<const FSimUpdateOut*>(&_Data);
    }
}

void UDisplayGameInstance::OnAllClientLevelLoaded()
{
    ULevel* Level = GetWorld()->GetCurrentLevel();
    FString LevelName = Level->GetPathName();
    if (!currentSimInData)
    {
        // Not create simmodule
        return;
    }

    // TODO: check loaded map is correct

    if (currentSimInData->name == TEXT("INIT"))
    {
        FSimOut OutData;
        OutData.name = TEXT("OUTPUT_INIT");
        SimOutput(OutData);
    }
    else if (currentSimInData->name == TEXT("RESET"))
    {
        UE_LOG(LogSimGameInstance, Log, TEXT("Reset After Load World!"));
        // Update hadmap
        if (hadmapHandle && hadmapHandle->IsMapReady() && hadmapHandle->GetMapMode() == hadmapue4::MapMode::ROUTINGMAP)
        {
            hadmapHandle->UpdateRoutingmap(StaticCast<FSimResetIn*>(currentSimInData.Get())->startLon,
                StaticCast<FSimResetIn*>(currentSimInData.Get())->startLat,
                StaticCast<FSimResetIn*>(currentSimInData.Get())->startAlt);
        }
        SimInput(*currentSimInData);
        currentSimInData->bIsConsumed = CONSUMED_MAXTICK + 1;
        UE_LOG(LogSimGameInstance, Log, TEXT("Execute ResetAfterLoadedWorld Over!"));

        displayNetworkManager->resumeThread();
    }

    // TODO: Whew STOP
}

int32 UDisplayGameInstance::getMapIndex(const FString& mapname)
{
    FConfigSection* Sec = GConfig->GetSectionPrivate(TEXT("MapIndex"), false, false, GGameIni);
    if (!Sec)
    {
        return 0;
    }
    for (FConfigSection::TIterator It(*Sec); It; ++It)
    {
        FRegexPattern pattern(It.Key().ToString());
        FRegexMatcher matcher(pattern, mapname);
        if (matcher.FindNext() && !It.Value().GetValue().IsEmpty())
        {
            UE_LOG(LogSimGameInstance, Log, TEXT("Find mapIndex: %s"), *It.Value().GetValue());
            return FCString::Atoi(*It.Value().GetValue());
        }
    }
    return 0;
}

int64 UDisplayGameInstance::GetEgoIDByGroupName(FString FindGroupName)
{
    if (FindGroupName.IsEmpty())
    {
        int64* FindValue = EgoName_ID_Mapping.Find(ModuleGroupName);
        if (FindValue)
        {
            return *FindValue;
        }
        else if (EgoName_ID_Mapping.Num() > 0)
        {
            return EgoName_ID_Mapping.begin().Value();
        }
        else
        {
            return 0;
        }
    }
    int64* FindValue = EgoName_ID_Mapping.Find(FindGroupName);
    return FindValue ? *FindValue : 0;
}

TArray<FString> UDisplayGameInstance::GetEgoNameList()
{
    TArray<FString> EgoNameList;
    EgoName_ID_Mapping.GenerateKeyArray(EgoNameList);
    return EgoNameList;
}

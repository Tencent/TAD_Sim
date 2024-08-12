// Fill out your copyright notice in the Description page of Project Settings.

#include "DisplayGameModeBase.h"
#include "Engine/World.h"
#include "DisplayGameInstance.h"
#include "DisplayGameStateBase.h"
#include "DisplayHUD.h"
#include "DisplayPlayerController.h"
#include "DisplayPlayerState.h"
#include "DisplayPawn.h"
#include "OnlineEngineInterface.h"
#include "DisplayGameSession.h"
#include "HadMap/Public/HadmapManager.h"
#include "MapGeneratedActor.h"
#include "Modules/OpenDriveFunctionLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Data/CatalogDataSource.h"

DEFINE_LOG_CATEGORY(LogSimGameMode);

// void ADisplayGameModeBase::SimActionStart(const FSimData& _InputData, const ESimState& _State)
//{
//
//     if (_State == ESimState::SA_DONE || _State == ESimState::SA_INIT)
//     {
//         // Default nothing to do.
//     }
//     else if (_State == ESimState::SA_RESET)
//     {
//         const FSimData* InputDataPtr = &_InputData;
//         const FSimResetIn* ResetIn = static_cast<const FSimResetIn*>(InputDataPtr);
//         check(ResetIn);
//         UE_LOG(LogSimGameMode, Log, TEXT("ResetIn->configFilePath: %s"), *ResetIn->configFilePath);
//         FLocalResetIn LocalResetIn;
//         LocalResetIn.patch = ResetIn->configFilePath;
//         // TODO: Cast simulator data to local data.
//         //..
//         GetGameState<ADisplayGameStateBase>()->Multicast_Reset(LocalResetIn);
//         //GetGameState<ADisplayGameStateBase>()->Multicast_Update(LocalResetIn);
//     }
//     else if (_State == ESimState::SA_UPDATE)
//     {
//         //UE_LOG(LogSimGameMode, Log, TEXT("Update level, timeStamp is %f"), _InData->timeStamp);
//     }
//     else
//     {
//         // TODO: ERROR
//     }
//
//     UE_LOG(LogSimGameMode, Log, TEXT("Execute SimActionStart Over!"));
// }
//
// void ADisplayGameModeBase::SimActionReturn(const FLocalData& _OutputData, const ESimState& _State, const
// FUniqueNetIdRepl& _NetId)
//{
//
//     if (_State == ESimState::SA_DONE || _State == ESimState::SA_INIT)
//     {
//         // Default nothing to do.
//     }
//     else if (_State == ESimState::SA_RESET)
//     {
//         check(GetGameInstance<UDisplayGameInstance>());
//
//         const FLocalData* OutputPtr = &_OutputData;
//         const FLocalResetOut* LocalResetOut = static_cast<const FLocalResetOut*>(OutputPtr);
//         check(LocalResetOut);
//         FSimResetOut Out;
//         Out.message = LocalResetOut->message;
//
//         GetGameInstance<UDisplayGameInstance>()->SimOutput(Out, _State);
//         UE_LOG(LogSimGameMode, Log, TEXT("Execute SimActionReturn Over!"));
//     }
//     else if (_State == ESimState::SA_UPDATE)
//     {
//
//     }
//     else
//     {
//         // TODO: ERROR
//     }
//
// }

ADisplayGameModeBase::ADisplayGameModeBase()
{
    // Allow tick every frame
    PrimaryActorTick.bCanEverTick = true;

    // Turn on Seamless Travel
    bUseSeamlessTravel = true;

    // Set default class
    PlayerControllerClass = ADisplayPlayerController::StaticClass();
    /*DefaultPawnClass = ADisplayPawn::StaticClass();*/
    DefaultPawnClass = APawn::StaticClass();
    GameStateClass = ADisplayGameStateBase::StaticClass();
    PlayerStateClass = ADisplayPlayerState::StaticClass();
    HUDClass = ADisplayHUD::StaticClass();
    GameSessionClass = ADisplayGameSession::StaticClass();
}

void ADisplayGameModeBase::SimInput(const FSimData& _Data)
{
    if (_Data.name == TEXT("RESET"))
    {
        FLocalResetIn ResetInData;
        ConvertData_SimToLocal(_Data, ResetInData);
        GetGameState<ADisplayGameStateBase>()->SimInput(ResetInData);
    }
    else if (_Data.name == TEXT("UPDATE"))
    {
        FLocalUpdateIn UpdateInData;
        ConvertData_SimToLocal(_Data, UpdateInData);
        GetGameState<ADisplayGameStateBase>()->SimInput(UpdateInData);
    }
    else if (_Data.name == TEXT("OUTPUT_SENSOR"))
    {
        FLocalIn UpdateInData;
        UpdateInData.timeStamp = _Data.timeStamp;
        UpdateInData.timeStamp_ego = _Data.timeStamp_ego;
        UpdateInData.timeStamp_tail = _Data.timeStamp_tail;
        ConvertData_SimToLocal(_Data, UpdateInData);
        GetGameState<ADisplayGameStateBase>()->SimInput(UpdateInData);
    }
}

void ADisplayGameModeBase::SimOutput(const FLocalData& _Data, const FUniqueNetIdRepl& _ClientId)
{
    auto& Data = clientOutputMap.FindOrAdd(_ClientId);

    if (_Data.name == TEXT("RESET"))
    {
        Data = MakeShared<FSimResetOut>();
        Data->name = TEXT("RESET");
        ConvertData_LocalToSim(_Data, *Data);
    }
    else if (_Data.name == TEXT("UPDATE"))
    {
        Data = MakeShared<FSimUpdateOut>();
        Data->name = TEXT("UPDATE");
        ConvertData_LocalToSim(_Data, *Data);
    }
    else if (_Data.name == TEXT("OUTPUT_SENSOR"))
    {
        Data = MakeShared<FSimSensorUpdateOut>();
        Data->name = TEXT("OUTPUT_SENSOR");
        ConvertData_LocalToSim(_Data, *Data);
    }

    bool IsAllClientOutput = true;
    for (auto& Elem : clientOutputMap)
    {
        if (Elem.Value == NULL || Elem.Value->bIsSent == true)
        {
            IsAllClientOutput = false;
        }
    }

    if (IsAllClientOutput)
    {
        GetGameInstance<UDisplayGameInstance>()->SimOutput(*Data);
    }
}

bool ADisplayGameModeBase::LoadSceneConfig(const FString& _FilePath)
{
    return false;
}

void ADisplayGameModeBase::ConvertData_SimToLocal(const FSimData& _SimData, FLocalData& _LocalData)
{
    if (_SimData.name == TEXT("RESET"))
    {
        const FSimResetIn* ResetInPtr = static_cast<const FSimResetIn*>(&_SimData);
        FLocalResetIn NewLocalResetIn;

        if (ResetInPtr->sceneBuffer.empty())
        {
            UE_LOG(LogSimSystem, Error, TEXT("Can not get scene Buffer"));
        }
        else
        {
            int64 EgoID = GetGameInstance<UDisplayGameInstance>()->ModuleGroupName.IsEmpty()
                              ? -1
                              : GetGameInstance<UDisplayGameInstance>()->GetEgoIDByGroupName();
            NewLocalResetIn.sensorManager = ASensorManager::ParseSensorString(ResetInPtr->sceneBuffer, EgoID);
            GetGameInstance<UDisplayGameInstance>()->GetCatalogDataSource()->LoadSceneBuffer(ResetInPtr->sceneBuffer);
        }

        UClass* MapGeneratedActorClass =
            LoadClass<AMapGeneratedActor>(NULL, TEXT("Blueprint'/AutoRoad/HadmapActor.HadmapActor_C'"));
        AMapGeneratedActor* HadMapActor = nullptr;
        if (MapGeneratedActorClass)
        {
            HadMapActor =
                GetWorld()->SpawnActor<AMapGeneratedActor>(MapGeneratedActorClass, FVector(0, 0, 0), FRotator(0, 0, 0));
        }
        else
        {
            UE_LOG(LogSimSystem, Error, TEXT("AMapGeneratedActor class load fail"));
        }
        if (HadMapActor)
        {
            bool bArtLevel = false;
            if (ResetInPtr->mapIndex == 0)
            {
                bool bOriginInEgo = false;
                GConfig->GetBool(TEXT("AutoRoad"), TEXT("bOriginInEgo"), bOriginInEgo, GGameIni);
                FVector NewOrigin = UOpenDriveFunctionLibrary::GetMapCenterLonlat(ResetInPtr->mapDataBasePath);
                HadMapActor->ShowObjectOnly = false;

                if (bOriginInEgo)
                {
                    UE_LOG(LogSimSystem, Log, TEXT("AutoRoad bOriginInEgo true"));

                    HadMapActor->RefX = ResetInPtr->startLon;
                    HadMapActor->RefY = ResetInPtr->startLat;
                    HadMapActor->RefZ = NewOrigin.Z - 0.5f;
                }
                else
                {
                    UE_LOG(LogSimSystem, Log, TEXT("AutoRoad bOriginInEgo false"));

                    HadMapActor->RefX = NewOrigin.X;
                    HadMapActor->RefY = NewOrigin.Y;
                    HadMapActor->RefZ = NewOrigin.Z - 0.5f;
                }
                UE_LOG(LogSimSystem, Error, TEXT("Auto Road Origin is %f, %f, %f"), HadMapActor->RefX,
                    HadMapActor->RefY, HadMapActor->RefZ);
            }
            else
            {
                bArtLevel = true;
                // 沿用Game.Ini中的经纬度并只渲染放置物
                HadMapActor->ShowObjectOnly = true;
                HadMapActor->RefX = ResetInPtr->mapOriginLon;
                HadMapActor->RefY = ResetInPtr->mapOriginLat;
                HadMapActor->RefZ = ResetInPtr->mapOriginAlt;
            }
            const TMap<FString, TPair<FString, FVector>>& MapModelData =
                GetGameInstance<UDisplayGameInstance>()->GetCatalogDataSource()->GetMapModelData();
            ;
            HadMapActor->DrawMap(ResetInPtr->mapDataBasePath, ResetInPtr->decryptFilePath, bArtLevel, MapModelData,
                ResetInPtr->ModelPath);
        }

        FLocalResetIn* LocalResetInPtr = static_cast<FLocalResetIn*>(&_LocalData);
        NewLocalResetIn.name = _SimData.name;
        /*~ Ego ~*/
        for (const auto& Elem : ResetInPtr->EgoInitInfoArry)
        {
            FVehicleConfig EgoConfig;
            EgoConfig.id = Elem.EgoID;
            EgoConfig.timeStamp = ResetInPtr->timeStamp;
            EgoConfig.trafficType = ETrafficType::ST_Ego;
            // if (!GConfig->GetString(TEXT("TypeDef"), *ResetInPtr->egoType, egoTypeName, GGameIni))
            //{
            //     UE_LOG(LogSimSystem, Error, TEXT("Can not get ego define"));
            // }

            EgoConfig.type = FCString::Atoi(*(Elem.egoType.Replace(TEXT("transport/Type"), TEXT(""))));
            EgoConfig.typeName = GetTypeIdDef(Elem.egoType);
            EgoConfig.Name = Elem.egoName;
            // TODO: wheels transform
            // Location
            double x = Elem.startLon;
            double y = Elem.startLat;
            double z = Elem.startAlt;
            hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, EgoConfig.startLocation);
            // Rotation
            FRotator egoVehicleRotation(ForceInit);
            EgoConfig.startRotation = FRotator(0, -Elem.startTheta * 180 / PI - 90, 0);
            // Velocity
            FVector Velocity = EgoConfig.startRotation.Vector() * Elem.startSpeed;
            EgoConfig.initVelocity = Velocity;

            /* Container */

            // Snap
            if (!GConfig->GetBool(TEXT("Mode"), TEXT("SnapGround"), EgoConfig.isEgoSnap, GGameIni))
            {
                UE_LOG(LogSimSystem, Error, TEXT("Can not get ego snamp mode"));
            }

            NewLocalResetIn.transportManager.vehicleManagerConfig.egoConfigArry.Add(EgoConfig);
        }

        // if (fovFilterManagerHandle)
        //{
        //     fovFilterManagerHandle->Init(sensorConfigPath, hadmapue4::HadmapManager::Get());
        // }

        // Environment

        FString EnvConfigXMLPath = ResetInPtr->envConfigPath;
        if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ResetInPtr->envConfigPath))
        {
            EnvConfigXMLPath = FPaths::ProjectDir() + TEXT("XMLFiles/EnvironmentConfig.xml");
        }
        UE_LOG(LogTemp, Log, TEXT("Environment Config Path: %s"), *EnvConfigXMLPath);
        NewLocalResetIn.envManager.timeline.initData.Compute();
        AEnvManager::ReadConfigFile(EnvConfigXMLPath, NewLocalResetIn.envManager.timeline);

        AEnvManager::ParseEnvString(ResetInPtr->sceneBuffer, NewLocalResetIn.envManager.timeline);

        NewLocalResetIn.signallightManager.SceneTrafficPath = ResetInPtr->SceneTrafficPath;

        *LocalResetInPtr = NewLocalResetIn;
    }

    if (_SimData.name == TEXT("UPDATE"))
    {
        const FSimUpdateIn* UpdateInPtr = static_cast<const FSimUpdateIn*>(&_SimData);
        FLocalUpdateIn* LocalUpdateInPtr = static_cast<FLocalUpdateIn*>(&_LocalData);

        FLocalUpdateIn NewLocalUpdateIn;
        NewLocalUpdateIn.timeStamp = UpdateInPtr->timeStamp;
        NewLocalUpdateIn.name = UpdateInPtr->name;
        /*~ Ego ~*/
        for (const auto& Elem : UpdateInPtr->egoData)
        {
            const FString& GroupName = Elem.Key;
            const sim_msg::Location& Location = Elem.Value;

            FVehicleIn EgoInput;
            EgoInput.id = GetGameInstance<UDisplayGameInstance>()->GetEgoIDByGroupName(GroupName);
            EgoInput.timeStamp = UpdateInPtr->timeStamp;
            EgoInput.timeStamp0 = Elem.Value.t() * 1000;
            EgoInput.trafficType = ETrafficType::ST_Ego;
            EgoInput.typeName = TEXT("");    // GetTypeIdDef(-1, TEXT("transport"));

            // TODO: wheels transform
            // Location
            hadmapue4::HadmapManager::Get()->LonLatToLocal(
                Location.position().x(), Location.position().y(), Location.position().z(), EgoInput.location);
            // Rotation
            FRotator egoVehicleRotation(ForceInit);
            EgoInput.rotation.Roll = (float) (Location.rpy().x() * 180 / PI);
            EgoInput.rotation.Pitch = (float) (-Location.rpy().y() * 180 / PI);
            EgoInput.rotation.Yaw = (float) (-Location.rpy().z() * 180 / PI - 90);
            // Velocity
            FVector Velocity = FVector(Location.velocity().x(), -Location.velocity().y(), Location.velocity().z());
            EgoInput.velocity = Velocity;

            // Container
            if (const sim_msg::Location* pContainerData = UpdateInPtr->egoContainerData.Find(GroupName))
            {
                if (pContainerData->ByteSize() > 0)
                {
                    EgoInput.hasContainer = true;
                    hadmapue4::HadmapManager::Get()->LonLatToLocal(pContainerData->position().x(),
                        pContainerData->position().y(), pContainerData->position().z(), EgoInput.locContainer);
                    EgoInput.rotContainer.Roll = (float) (pContainerData->rpy().x() * 180 / PI);
                    EgoInput.rotContainer.Pitch = (float) (-pContainerData->rpy().y() * 180 / PI);
                    EgoInput.rotContainer.Yaw = (float) (-pContainerData->rpy().z() * 180 / PI - 90);
                    EgoInput.speedContainer = FVector(
                        pContainerData->velocity().x(), -pContainerData->velocity().y(), pContainerData->velocity().z())
                                                  .Size();
                }
            }

            // Control
            EgoInput.steeringAngle = UpdateInPtr->planStatusData.expect_steering().angle();
            // Control
            if (id_controlled == EgoInput.id)
            {
                EgoInput.drivingMode = UpdateInPtr->controlSimData.car2sim().drive_mode();
                EgoInput.steeringAngle = UpdateInPtr->controlSimData.car2sim().steering_angle();
                EgoInput.throttle = UpdateInPtr->controlSimData.car2sim().throttle_value();
                EgoInput.brake = UpdateInPtr->controlSimData.car2sim().brake_pressure();
                EgoInput.gear = UpdateInPtr->controlSimData.car2sim().gear();
            }
            // Trajectory
            for (size_t i = 0; i < UpdateInPtr->trajectoryData.point().size(); i++)
            {
                FTrajectoryPoint Point;
                hadmapue4::HadmapManager::Get()->LonLatToLocal(UpdateInPtr->trajectoryData.point()[i].x(),
                    UpdateInPtr->trajectoryData.point()[i].y(), UpdateInPtr->trajectoryData.point()[i].z(),
                    Point.location);
                Point.rotator = FRotator(0, -UpdateInPtr->trajectoryData.point()[i].theta() * 180 / PI - 90, 0);
                Point.kappa = UpdateInPtr->trajectoryData.point()[i].kappa();
                Point.distance = UpdateInPtr->trajectoryData.point()[i].s();
                Point.speed = UpdateInPtr->trajectoryData.point()[i].v();
                Point.timestamp = UpdateInPtr->trajectoryData.point()[i].t();
                Point.accel = UpdateInPtr->trajectoryData.point()[i].a();

                EgoInput.trajectory.data.Add(Point);
            }

            // Add
            NewLocalUpdateIn.transportManager.vehicleManagerIn.egoVehicleInputArry.Add(EgoInput);
        }

        /*~ Traffic ~*/
        for (auto& Elem : UpdateInPtr->trafficData.cars())
        {
            FVehicleIn TrafficInput;
            TrafficInput.id = Elem.id();
            TrafficInput.timeStamp = UpdateInPtr->timeStamp;
            TrafficInput.timeStamp0 = Elem.t();
            TrafficInput.sizeLWH = FVector(Elem.length(), Elem.width(), Elem.height()) * 100;
            TrafficInput.trafficType = ETrafficType::ST_TRAFFIC;
            TrafficInput.type = Elem.type();
            TrafficInput.typeName = GetTypeIdDef(Elem.type(), TEXT("transport"));
            // Location
            FVector NewTrafficVehicleLocation;
            double x = Elem.x();
            double y = Elem.y();
            double z = Elem.z();
            hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, TrafficInput.location);
            // Rotation
            TrafficInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
            // Velocity
            TrafficInput.velocity = TrafficInput.rotation.Vector().GetSafeNormal() * Elem.v();
            // Control
            if (id_controlled == Elem.id())
            {
                TrafficInput.drivingMode = UpdateInPtr->controlSimData.car2sim().drive_mode();
                TrafficInput.steeringAngle = UpdateInPtr->controlSimData.car2sim().steering_angle();
                TrafficInput.throttle = UpdateInPtr->controlSimData.car2sim().throttle_value();
                TrafficInput.brake = UpdateInPtr->controlSimData.car2sim().brake_pressure();
                TrafficInput.gear = UpdateInPtr->controlSimData.car2sim().gear();
            }
            // Add
            NewLocalUpdateIn.transportManager.vehicleManagerIn.trafficVehicleInputArry.Add(TrafficInput);
        }
        // Updat global time
        NewLocalUpdateIn.transportManager.vehicleManagerIn.timeStamp = UpdateInPtr->timeStamp;

        /* Creature */
        for (auto& Elem : UpdateInPtr->trafficData.dynamicobstacles())
        {
            /* Pedestrian */
            if (Elem.type() >= 0 && Elem.type() < 100)
            {
                FPedestrianInput PedestrianInput;
                PedestrianInput.id = Elem.id();
                PedestrianInput.timeStamp0 = Elem.t();
                PedestrianInput.sizeLWH = FVector(Elem.length(), Elem.width(), Elem.height()) * 100;
                PedestrianInput.timeStamp = UpdateInPtr->timeStamp;
                PedestrianInput.typeName = GetTypeIdDef(Elem.type(), TEXT("creature"));
                PedestrianInput.type = Elem.type();
                // if (Elem.type() == 0)
                //{
                //     PedestrianInput.typeName = TEXT("creature/pedestrian.man");
                // }
                // else if (Elem.type() == 1)
                //{
                //     PedestrianInput.typeName = TEXT("creature/pedestrian.boy");
                // }
                // else if (Elem.type() == 2)
                //{
                //     PedestrianInput.typeName = TEXT("creature/pedestrian.oldwoman");
                // }
                double x = Elem.x();
                double y = Elem.y();
                double z = Elem.z();
                hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, PedestrianInput.location);
                FRotator Rotation;
                PedestrianInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
                PedestrianInput.velocity = PedestrianInput.rotation.Quaternion().GetForwardVector() * Elem.v();

                NewLocalUpdateIn.creatureManager.pedestrianManager.pedestrianArry.Add(PedestrianInput);
            }
            /* Animal */
            if (Elem.type() >= 100 && Elem.type() < 200)
            {
                FAnimalInput AnimalInput;
                AnimalInput.id = Elem.id();
                AnimalInput.timeStamp0 = Elem.t();
                AnimalInput.sizeLWH = FVector(Elem.length(), Elem.width(), Elem.height()) * 100;
                AnimalInput.timeStamp = UpdateInPtr->timeStamp;
                AnimalInput.typeName = GetTypeIdDef(Elem.type(), TEXT("creature"));
                AnimalInput.type = Elem.type();
                // if (Elem.type() == 100)
                //{
                //     AnimalInput.typeName = TEXT("creature/Animal.cat");
                // }
                // else if (Elem.type() == 101)
                //{
                //     AnimalInput.typeName = TEXT("creature/Animal.dog");
                // }
                double x = Elem.x();
                double y = Elem.y();
                double z = Elem.z();
                hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, AnimalInput.location);
                FRotator Rotation;
                AnimalInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
                AnimalInput.velocity = AnimalInput.rotation.Quaternion().GetForwardVector() * Elem.v();

                NewLocalUpdateIn.creatureManager.animalArry.Add(AnimalInput);
            }
            /* Bike */
            if (Elem.type() >= 200 && Elem.type() < 300)
            {
                FPedestrianInput BikeInput;
                BikeInput.id = Elem.id();
                BikeInput.timeStamp0 = Elem.t();
                BikeInput.sizeLWH = FVector(Elem.length(), Elem.width(), Elem.height()) * 100;
                BikeInput.timeStamp = UpdateInPtr->timeStamp;
                BikeInput.typeName = GetTypeIdDef(Elem.type(), TEXT("creature"));
                BikeInput.type = Elem.type();

                double x = Elem.x();
                double y = Elem.y();
                double z = Elem.z();
                hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, BikeInput.location);
                FRotator Rotation;
                BikeInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
                BikeInput.velocity = BikeInput.rotation.Quaternion().GetForwardVector() * Elem.v();

                NewLocalUpdateIn.creatureManager.pedestrianManager.pedestrianArry.Add(BikeInput);
            }
            /* Moto */
            if (Elem.type() >= 300 && Elem.type() < 500)
            {
                FPedestrianInput MotoInput;
                MotoInput.id = Elem.id();
                MotoInput.timeStamp0 = Elem.t();
                MotoInput.sizeLWH = FVector(Elem.length(), Elem.width(), Elem.height()) * 100;
                MotoInput.timeStamp = UpdateInPtr->timeStamp;
                MotoInput.typeName = GetTypeIdDef(Elem.type(), TEXT("creature"));
                MotoInput.type = Elem.type();

                double x = Elem.x();
                double y = Elem.y();
                double z = Elem.z();
                hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, MotoInput.location);
                FRotator Rotation;
                MotoInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
                MotoInput.velocity = MotoInput.rotation.Quaternion().GetForwardVector() * Elem.v();

                NewLocalUpdateIn.creatureManager.pedestrianManager.pedestrianArry.Add(MotoInput);
            }
            /* Crane */
            if (Elem.type() >= 600)
            {
                FPedestrianInput CraneInput;
                CraneInput.id = Elem.id();
                CraneInput.timeStamp0 = Elem.t();
                CraneInput.sizeLWH = FVector(Elem.length(), Elem.width(), Elem.height()) * 100;
                CraneInput.timeStamp = UpdateInPtr->timeStamp;
                CraneInput.typeName = GetTypeIdDef(Elem.type(), TEXT("creature"));
                CraneInput.type = Elem.type();

                double x = Elem.x();
                double y = Elem.y();
                double z = Elem.z();
                hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, CraneInput.location);
                FRotator Rotation;
                CraneInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
                CraneInput.velocity = CraneInput.rotation.Quaternion().GetForwardVector() * Elem.v();

                NewLocalUpdateIn.creatureManager.pedestrianManager.pedestrianArry.Add(CraneInput);
            }
        }
        NewLocalUpdateIn.creatureManager.pedestrianManager.timeStamp = UpdateInPtr->timeStamp;
        NewLocalUpdateIn.creatureManager.timeStamp = UpdateInPtr->timeStamp;

        /* Obstacle */
        for (auto& Elem : UpdateInPtr->trafficData.staticobstacles())
        {
            FObstacleInput ObstacleInput;
            ObstacleInput.id = Elem.id();
            ObstacleInput.timeStamp0 = Elem.t();
            ObstacleInput.sizeLWH = FVector(Elem.length(), Elem.width(), Elem.height()) * 100;
            ObstacleInput.timeStamp = UpdateInPtr->timeStamp;
            FVector Location;
            double x = Elem.x();
            double y = Elem.y();
            double z = Elem.z();
            hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, ObstacleInput.location);
            FRotator Rotation;
            ObstacleInput.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
            ObstacleInput.typeName = GetTypeIdDef(Elem.type(), TEXT("obstacle"));
            ObstacleInput.type = Elem.type();
            // if (Elem.type() == 0)
            //{
            //     ObstacleInput.typeName = TEXT("obstacle.Sedan");
            // }
            // else if (Elem.type() == 1)
            //{
            //     ObstacleInput.typeName = TEXT("obstacle.Box");
            // }
            // else if (Elem.type() == 2)
            //{
            //     ObstacleInput.typeName = TEXT("obstacle.Person");
            // }
            // else if (Elem.type() == 3)
            //{
            //     ObstacleInput.typeName = TEXT("obstacle.Cone");
            // }
            // else if (Elem.type() == 4)
            //{
            //     ObstacleInput.typeName = TEXT("obstacle.Sand");
            // }
            // else
            //{
            //     ObstacleInput.typeName = TEXT("obstacle.Box");
            // }
            NewLocalUpdateIn.obstacleManager.obstacleArry.Add(ObstacleInput);
        }
        NewLocalUpdateIn.obstacleManager.timeStamp = UpdateInPtr->timeStamp;

        /* TrafficLight */
        for (auto& Elem : UpdateInPtr->trafficData.trafficlights())
        {
            FTrafficLightIn NewTrafficLightIn;
            NewTrafficLightIn.id = Elem.id();
            NewTrafficLightIn.age = Elem.age();
            FString SignalHeadStr = UTF8_TO_TCHAR(Elem.signalhead().c_str());
            TArray<FString> PhysicIDList;
            SignalHeadStr.ParseIntoArray(PhysicIDList, TEXT(","), true);
            NewTrafficLightIn.PhysicLightsID = PhysicIDList;
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
            hadmapue4::HadmapManager::Get()->LonLatToLocal(x, y, z, NewTrafficLightIn.location);
            NewTrafficLightIn.rotation = FRotator(0, -Elem.heading() * 180 / PI - 90, 0);
            NewTrafficLightIn.timeStamp = UpdateInPtr->timeStamp;
            NewLocalUpdateIn.signallightManager.trafficlightArry.Add(NewTrafficLightIn);
        }

        /* Environment */
        NewLocalUpdateIn.envManager.timeStamp = UpdateInPtr->timeStamp;

        NewLocalUpdateIn.envManager.unix_timestamp = UpdateInPtr->environmentData.unix_timestamp();
        NewLocalUpdateIn.envManager.visibility = UpdateInPtr->environmentData.fog().visibility();
        NewLocalUpdateIn.envManager.cloudLevel = UpdateInPtr->environmentData.clouds().fractional_cloud_cover();

        NewLocalUpdateIn.envManager.windSpeed = UpdateInPtr->environmentData.wind().speed();
        sim_msg::EnvironmentalConditions_Precipitation_Type Type = UpdateInPtr->environmentData.precipitation().type();

        NewLocalUpdateIn.envManager.rainFall = 0.0;
        NewLocalUpdateIn.envManager.snowFall = 0.0;
        if (Type ==
            sim_msg::EnvironmentalConditions_Precipitation_Type::EnvironmentalConditions_Precipitation_Type_RAIN)
        {
            NewLocalUpdateIn.envManager.rainFall = UpdateInPtr->environmentData.precipitation().intensity();
        }
        else if (Type ==
                 sim_msg::EnvironmentalConditions_Precipitation_Type::EnvironmentalConditions_Precipitation_Type_SNOW)
        {
            NewLocalUpdateIn.envManager.snowFall = UpdateInPtr->environmentData.precipitation().intensity();
        }
        NewLocalUpdateIn.envManager.temperature = UpdateInPtr->environmentData.temperature();
        NewLocalUpdateIn.envManager.environment = UpdateInPtr->environmentData;

        /* Sensor */
        NewLocalUpdateIn.sensorManager.timeStamp = UpdateInPtr->timeStamp;

        *LocalUpdateInPtr = NewLocalUpdateIn;
    }

    if (_SimData.name == TEXT("OUTPUT_SENSOR"))
    {
        _LocalData.name = _SimData.name;
    }
}

void ADisplayGameModeBase::ConvertData_LocalToSim(const FLocalData& _LocalData, FSimData& _SimData)
{
    if (_LocalData.name == TEXT("RESET"))
    {
        _SimData.name = _LocalData.name;
    }
    if (_LocalData.name == TEXT("UPDATE"))
    {
        const FLocalUpdateOut* UpdateInPtr = static_cast<const FLocalUpdateOut*>(&_LocalData);
        FSimUpdateOut* SimUpdateInPtr = static_cast<FSimUpdateOut*>(&_SimData);

        if (UpdateInPtr && SimUpdateInPtr)
        {
            SimUpdateInPtr->name = UpdateInPtr->name;
            SimUpdateInPtr->timeStamp = UpdateInPtr->timeStamp;
            SimUpdateInPtr->frameID = (int32) (UpdateInPtr->timeStamp);
            SimUpdateInPtr->trafficPose.set_timestamp(UpdateInPtr->timeStamp);
            // bool HasGotMannedOutData = false;
            //  Find ego manned data first
            for (size_t i = 0; i < UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry.Num(); i++)
            {
                if (UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].bHasMannedControlData)
                {
                    // HasGotMannedOutData = true;
                    sim_msg::Vec3* Position = SimUpdateInPtr->overrideEgoLocation.mutable_position();
                    sim_msg::Vec3* Rotation = SimUpdateInPtr->overrideEgoLocation.mutable_rpy();
                    sim_msg::Vec3* Velocity = SimUpdateInPtr->overrideEgoLocation.mutable_velocity();
                    // position
                    double Px, Py, Pz = 0.0;
                    hadmapue4::HadmapManager::Get()->LocalToLonLat(
                        UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].locationManned, Px, Py, Pz);
                    Position->set_x(Px);
                    Position->set_y(Py);
                    Position->set_z(Pz);
                    // rpy
                    Rotation->set_x(
                        UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].rotatorManned.Roll * PI / 180.f);
                    Rotation->set_x(-UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].rotatorManned.Pitch *
                                    PI / 180.f);
                    Rotation->set_x(
                        -(UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].rotatorManned.Yaw + 90.f) * PI /
                        180.f);
                    // velocity
                    Velocity->set_x(
                        UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].velocityManned.X / 100.f);
                    Velocity->set_y(
                        -UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].velocityManned.Y / 100.f);
                    Velocity->set_z(
                        UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].velocityManned.Z / 100.f);

                    // Control
                    sim_msg::PlanMode PM = sim_msg::PlanMode::PLAN_MODE_CHANGE_LANE;
                    if (UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].drivingMode == 0)
                    {
                        PM = sim_msg::PlanMode::PLAN_MODE_CHANGE_LANE;
                    }
                    if (UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].drivingMode == 1)
                    {
                        PM = sim_msg::PlanMode::PLAN_MODE_MANUAL;
                    }
                    SimUpdateInPtr->planStatusData.set_mode(PM);
                }
                if (UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i].bHasPose)
                {
                    const auto& out = UpdateInPtr->transportManager.vehicleManagerOut.egoOutArry[i];
                    double Px, Py, Pz = 0.0;
                    hadmapue4::HadmapManager::Get()->LocalToLonLat(out.locPose, Px, Py, Pz);
                    auto* object = SimUpdateInPtr->trafficPose.add_egos();
                    object->set_id(out.id + 1);
                    object->set_timestamp(out.timeStamp0);
                    object->mutable_pose()->set_longitude(Px);
                    object->mutable_pose()->set_latitude(Py);
                    object->mutable_pose()->set_altitude(Pz);
                    object->mutable_pose()->set_roll(out.rotPose.Roll * PI / 180.f);
                    object->mutable_pose()->set_pitch(-out.rotPose.Pitch * PI / 180.f);
                    object->mutable_pose()->set_yaw(-(out.rotPose.Yaw + 90.f) * PI / 180.f);
                    object->set_length(out.sizeLWH.X * 0.01);
                    object->set_width(out.sizeLWH.Y * 0.01);
                    object->set_height(out.sizeLWH.Z * 0.01);
                    object->set_type(std::string(TCHAR_TO_ANSI(*out.typeName)));
                    object->set_raw_type(out.type);
                }
            }

            for (size_t i = 0; i < UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry.Num(); i++)
            {
                if (UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].bHasMannedControlData)
                {
                    // HasGotMannedOutData = true;
                    //  Set traffic msg data
                    // SimUpdateInPtr->manualTrafficData.mutable_vehicles_control_info();
                    sim_msg::ManualVehicleControlInfo* VehicleInfo =
                        SimUpdateInPtr->manualTrafficData.add_vehicles_control_info();
                    sim_msg::Vec3* Position = VehicleInfo->mutable_veh_control_info()->mutable_position();
                    sim_msg::Vec3* Rotation = VehicleInfo->mutable_veh_control_info()->mutable_rpy();
                    sim_msg::Vec3* Velocity = VehicleInfo->mutable_veh_control_info()->mutable_velocity();
                    // id
                    VehicleInfo->set_id(UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].id);
                    // position
                    double Px, Py, Pz = 0.0;
                    hadmapue4::HadmapManager::Get()->LocalToLonLat(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].locationManned, Px, Py, Pz);
                    Position->set_x(Px);
                    Position->set_y(Py);
                    Position->set_z(Pz);
                    // rpy
                    Rotation->set_x(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].rotatorManned.Roll * PI /
                        180.f);
                    Rotation->set_y(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].rotatorManned.Pitch * PI /
                        180.f);
                    Rotation->set_z(
                        -(UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].rotatorManned.Yaw + 90.f) *
                        PI / 180.f);
                    // velocity
                    Velocity->set_x(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].velocityManned.X / 100.f);
                    Velocity->set_y(
                        -UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].velocityManned.Y / 100.f);
                    Velocity->set_z(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].velocityManned.Z / 100.f);
                    // acceleration
                    VehicleInfo->mutable_veh_control_info()->mutable_acceleration()->set_x(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].acceleration.X / 100.f);
                    VehicleInfo->mutable_veh_control_info()->mutable_acceleration()->set_y(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].acceleration.Y / 100.f);
                    VehicleInfo->mutable_veh_control_info()->mutable_acceleration()->set_z(
                        UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].acceleration.Z / 100.f);
                }

                if (UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i].bHasPose)
                {
                    const auto& out = UpdateInPtr->transportManager.vehicleManagerOut.trafficOutArry[i];
                    double Px, Py, Pz = 0.0;
                    hadmapue4::HadmapManager::Get()->LocalToLonLat(out.locPose, Px, Py, Pz);
                    auto* object = SimUpdateInPtr->trafficPose.add_cars();
                    object->set_id(out.id);
                    object->set_timestamp(out.timeStamp0);
                    object->mutable_pose()->set_longitude(Px);
                    object->mutable_pose()->set_latitude(Py);
                    object->mutable_pose()->set_altitude(Pz);
                    object->mutable_pose()->set_roll(out.rotPose.Roll * PI / 180.f);
                    object->mutable_pose()->set_pitch(-out.rotPose.Pitch * PI / 180.f);
                    object->mutable_pose()->set_yaw(-(out.rotPose.Yaw + 90.f) * PI / 180.f);
                    object->set_length(out.sizeLWH.X * 0.01);
                    object->set_width(out.sizeLWH.Y * 0.01);
                    object->set_height(out.sizeLWH.Z * 0.01);
                    object->set_type(std::string(TCHAR_TO_ANSI(*out.typeName)));
                    object->set_raw_type(out.type);
                }
            }
            for (const auto& out : UpdateInPtr->creatureManager.pedestrianManager.pedestrianArry)
            {
                if (out.bHasPose)
                {
                    double Px, Py, Pz = 0.0;
                    hadmapue4::HadmapManager::Get()->LocalToLonLat(out.locPose, Px, Py, Pz);
                    auto* object = SimUpdateInPtr->trafficPose.add_dynamicobstacles();
                    object->set_id(out.id);
                    object->set_timestamp(out.timeStamp0);
                    object->mutable_pose()->set_longitude(Px);
                    object->mutable_pose()->set_latitude(Py);
                    object->mutable_pose()->set_altitude(Pz);
                    object->mutable_pose()->set_roll(out.rotPose.Roll * PI / 180.f);
                    object->mutable_pose()->set_pitch(-out.rotPose.Pitch * PI / 180.f);
                    object->mutable_pose()->set_yaw(-(out.rotPose.Yaw + 90.f) * PI / 180.f);
                    object->set_length(out.sizeLWH.X * 0.01);
                    object->set_width(out.sizeLWH.Y * 0.01);
                    object->set_height(out.sizeLWH.Z * 0.01);
                    object->set_type(std::string(TCHAR_TO_ANSI(*out.typeName)));
                    object->set_raw_type(out.type);
                }
            }
            for (const auto& out : UpdateInPtr->creatureManager.animalArry)
            {
                if (out.bHasPose)
                {
                    double Px, Py, Pz = 0.0;
                    hadmapue4::HadmapManager::Get()->LocalToLonLat(out.locPose, Px, Py, Pz);
                    auto* object = SimUpdateInPtr->trafficPose.add_dynamicobstacles();
                    object->set_id(out.id);
                    object->set_timestamp(out.timeStamp0);
                    object->mutable_pose()->set_longitude(Px);
                    object->mutable_pose()->set_latitude(Py);
                    object->mutable_pose()->set_altitude(Pz);
                    object->mutable_pose()->set_roll(out.rotPose.Roll * PI / 180.f);
                    object->mutable_pose()->set_pitch(-out.rotPose.Pitch * PI / 180.f);
                    object->mutable_pose()->set_yaw(-(out.rotPose.Yaw + 90.f) * PI / 180.f);
                    object->set_length(out.sizeLWH.X * 0.01);
                    object->set_width(out.sizeLWH.Y * 0.01);
                    object->set_height(out.sizeLWH.Z * 0.01);
                    object->set_type(std::string(TCHAR_TO_ANSI(*out.typeName)));
                    object->set_raw_type(out.type);
                }
            }

            for (const auto& out : UpdateInPtr->obstacleManager.obstacleArry)
            {
                if (out.bHasPose)
                {
                    double Px, Py, Pz = 0.0;
                    hadmapue4::HadmapManager::Get()->LocalToLonLat(out.locPose, Px, Py, Pz);
                    auto* object = SimUpdateInPtr->trafficPose.add_staticobstacles();
                    object->set_id(out.id);
                    object->set_timestamp(out.timeStamp0);
                    object->mutable_pose()->set_longitude(Px);
                    object->mutable_pose()->set_latitude(Py);
                    object->mutable_pose()->set_altitude(Pz);
                    object->mutable_pose()->set_roll(out.rotPose.Roll * PI / 180.f);
                    object->mutable_pose()->set_pitch(-out.rotPose.Pitch * PI / 180.f);
                    object->mutable_pose()->set_yaw(-(out.rotPose.Yaw + 90.f) * PI / 180.f);
                    object->set_length(out.sizeLWH.X * 0.01);
                    object->set_width(out.sizeLWH.Y * 0.01);
                    object->set_height(out.sizeLWH.Z * 0.01);
                    object->set_type(std::string(TCHAR_TO_ANSI(*out.typeName)));
                    object->set_raw_type(out.type);
                }
            }
        }
        _SimData.name = _LocalData.name;
    }
    if (_LocalData.name == TEXT("OUTPUT_SENSOR"))
    {
        const FLocalUpdateOut* UpdateInPtr = static_cast<const FLocalUpdateOut*>(&_LocalData);
        FSimSensorUpdateOut* SimUpdateInPtr = static_cast<FSimSensorUpdateOut*>(&_SimData);
        SimUpdateInPtr->sensorData.set_timestamp(UpdateInPtr->timeStamp);
        for (int i = 0; i < UpdateInPtr->sensorManager.outArray.Num(); i++)
        {
            const auto& senbuf = UpdateInPtr->sensorManager.outArray[i];
            if (senbuf.serialize_string.empty())
            {
                continue;
            }
            auto* sensor = SimUpdateInPtr->sensorData.add_sensor();
            sensor->set_id(senbuf.id);
            sensor->set_raw(senbuf.serialize_string);
            if (senbuf.type == "Lidar")
            {
                sensor->set_type(sim_msg::SensorRaw_Type_TYPE_LIDAR);
            }
            else if (senbuf.type == "Camera")
            {
                sensor->set_type(sim_msg::SensorRaw_Type_TYPE_CAMERA);
            }
            else if (senbuf.type == "Depth")
            {
                sensor->set_type(sim_msg::SensorRaw_Type_TYPE_DEPTH);
            }
            else if (senbuf.type == "Fisheye")
            {
                sensor->set_type(sim_msg::SensorRaw_Type_TYPE_FISHEYE);
            }
            else if (senbuf.type == "Semantic")
            {
                sensor->set_type(sim_msg::SensorRaw_Type_TYPE_SEMANTIC);
            }
            else if (senbuf.type == "Ultrasonic")
            {
                sensor->set_type(sim_msg::SensorRaw_Type_TYPE_ULTRASONIC);
            }
        }
        _SimData.name = _LocalData.name;
    }
}

FString ADisplayGameModeBase::GetTrafficTypeDef(int32 _Id, ETrafficType _Type)
{
    FString KeyName;
    FString TypeDef;
    if (_Type == ETrafficType::ST_Ego)
    {
        KeyName = TEXT("transport/Ego");
    }
    if (_Type == ETrafficType::ST_TRAFFIC)
    {
        KeyName = TEXT("transport/Type") + FString::FromInt(_Id);
    }
    if (GConfig->GetString(TEXT("TypeDef"), *KeyName, TypeDef, GGameIni))
    {
        return TypeDef;
    }
    return FString();
}

FString ADisplayGameModeBase::GetTypeIdDef(int32 _Id, FString _Type)
{
    FString KeyName = _Type + TEXT("/Type") + FString::FromInt(_Id);
    FString TypeDef;
    if (GConfig->GetString(TEXT("TypeDef"), *KeyName, TypeDef, GGameIni))
    {
        return TypeDef;
    }
    return FString();
}

FString ADisplayGameModeBase::GetTypeIdDef(const FString& _TypeName)
{
    FString TypeDef;
    if (GConfig->GetString(TEXT("TypeDef"), *_TypeName, TypeDef, GGameIni))
    {
        return TypeDef;
    }
    return FString();
}

// FString ADisplayGameModeBase::GetTypeDef(int32 _Id, FString _Class)
//{
//     FString KeyName;
//     FString TypeDef;
//     KeyName = _Class + TEXT("/Type") + FString::FromInt(_Id);
//     if (GConfig->GetString(TEXT("TypeDef"), *KeyName, TypeDef, GGameIni))
//     {
//         return TypeDef;
//     }
//     return FString();
// }

void ADisplayGameModeBase::PreLogin(
    const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

APlayerController* ADisplayGameModeBase::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
    const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void ADisplayGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UE_LOG(LogSimSystem, Log, TEXT("Player connected! PlayerName: %s"),
        *(NewPlayer->GetPlayerState<ADisplayPlayerState>()->GetPlayerName()));
    UE_LOG(LogSimSystem, Log, TEXT("PlayerNum: %d"), GetGameState<ADisplayGameStateBase>()->PlayerArray.Num());
    id_controlled = Cast<ADisplayPlayerController>(NewPlayer)->id_controlled;

    check(GetGameInstance<UDisplayGameInstance>());

    // Register client info to simulator in GameInstance.
    GetGameInstance<UDisplayGameInstance>()->RegisterClientToSim(NewPlayer);

    // TODO: Prevent illegal connet.
}

void ADisplayGameModeBase::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    // TODO: Stop simulator, send error message to coordinator.

    check(GetGameInstance<UDisplayGameInstance>());

    // Register client info to simulator in GameInstance.
    GetGameInstance<UDisplayGameInstance>()->UnregisterClientFromSim(Exiting);
}

void ADisplayGameModeBase::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    UE_LOG(LogSimSystem, Log, TEXT("HandleSeamlessTravelPlayer, Player Name: %s"),
        *(C->GetPlayerState<ADisplayPlayerState>()->GetPlayerName()));

    //// Check all client loaded world, send event to GameInstance.
    // if (GetGameInstance<UDisplayGameInstance>()->bAllClientsLogin && CheckAllClientLoadedCurrentWorld())
    //{
    //     GetGameInstance<UDisplayGameInstance>()->OnAllClientLevelLoaded();
    // }
}

void ADisplayGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // ~Init game mode
    UE_LOG(LogSimSystem, Log, TEXT("Init Game!"));
    UDisplayGameInstance* GI = GetGameInstance<UDisplayGameInstance>();
    if (MapName == GI->mapPath_Lobby)
    {
        UE_LOG(LogSimSystem, Log, TEXT("Default Map: %s"), *MapName);
    }

    // TODO: Generate Sync system

    // TODO: Load config of scene

    // check(GetGameInstance<UDisplayGameInstance>());
    // UDisplayGameInstance* GI = GetGameInstance<UDisplayGameInstance>();
    //// Is Server
    // if (GetNetMode() < 3)
    //{
    //     // Server create simulator thread to receive data.
    //     GI->CreateSimModuleThread();
    //     check(GI->simRestIn);
    //     GameSession->MaxPlayers = GI->simRestIn->clientNum;
    //     UE_LOG(LogSimSystem, Log, TEXT("PlayerController Num: %d"), GetWorld()->GetNumPlayerControllers());
    // }
    //// Is Client
    // else
    //{
    //     GI->ShutdownSimModuleThread();
    // }
}

void ADisplayGameModeBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

bool ADisplayGameModeBase::CheckAllClientKeepConnect()
{
    check(GetGameInstance<UDisplayGameInstance>());
    check(GetGameState<ADisplayGameStateBase>());
    bool IsAllConnect = true;
    TArray<FClientInfo> ClientConfigArry = GetGameInstance<UDisplayGameInstance>()->GetAllClientConfig();
    for (size_t i = 0; i < ClientConfigArry.Num(); i++)
    {
        bool IsInArray = false;
        for (auto& Elem : GetGameState<ADisplayGameStateBase>()->PlayerArray)
        {
            if (ClientConfigArry[i].uniqueNetId == Elem->GetUniqueId())
            {
                IsInArray = true;
            }
        }
        if (!IsInArray)
        {
            IsAllConnect = false;
            UE_LOG(LogSimSystem, Warning, TEXT("Client %s Is Disconnect!(%d/%d)"), *ClientConfigArry[i].playerName,
                i + 1, ClientConfigArry.Num());
        }
    }
    return IsAllConnect;
}

bool ADisplayGameModeBase::CheckAllClientLoadedCurrentWorld()
{
    check(GetGameInstance<UDisplayGameInstance>());
    bool IsAllLoaded = true;
    TArray<FClientInfo> ClientConfigArry = GetGameInstance<UDisplayGameInstance>()->GetAllClientConfig();
    for (size_t i = 0; i < ClientConfigArry.Num(); i++)
    {
        APlayerController* PC =
            GetPlayerControllerFromNetId(GetWorld(), *ClientConfigArry[i].uniqueNetId.GetUniqueNetId().Get());
        if (PC)
        {
            if (!PC->HasClientLoadedCurrentWorld())
            {
                IsAllLoaded = false;
                UE_LOG(LogSimSystem, Warning, TEXT("Client %s Has Not Loaded!(%d/%d)"), *ClientConfigArry[i].playerName,
                    i + 1, ClientConfigArry.Num());
            }
        }
        else
        {
            IsAllLoaded = false;
            UE_LOG(LogSimSystem, Warning,
                TEXT("Client %s Has Not Loaded!(%d/%d) Can Not Get PlayerController By It`s UniqueNetId."),
                *ClientConfigArry[i].playerName, i + 1, ClientConfigArry.Num());
        }
    }
    return IsAllLoaded;
}

void ADisplayGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    // Check all client loaded world, send event to GameInstance.
    if (GetGameInstance<UDisplayGameInstance>()->bAllClientsLogin && CheckAllClientLoadedCurrentWorld())
    {
        GetGameInstance<UDisplayGameInstance>()->OnAllClientLevelLoaded();
    }
}

bool ADisplayGameModeBase::SimDataToLocalData(const FSimData& _SimData, FLocalData& _LocalData)
{
    return false;
}

bool ADisplayGameModeBase::LocalDataToSimData(const FLocalData& _LocalData, FSimData& _SimData)
{
    return false;
}

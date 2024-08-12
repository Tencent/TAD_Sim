// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Objects/Sensors/CameraSensors/CameraSensor.h"
#include "Objects/Sensors/CameraSensors/DepthSensor.h"
#include "Objects/Sensors/CameraSensors/FisheyeSensor.h"
#include "Objects/Sensors/CameraSensors/SemanticCamera.h"
#include "Objects/Sensors/SensorFactory.h"
#include "Framework/DisplayGameInstance.h"
#include "Framework/DisplayGameStateBase.h"
#include "Framework/DisplayGameModeBase.h"
#include "Engine/World.h"
#include "Engine.h"

#if WITH_DEV_AUTOMATION_TESTS

DEFINE_LOG_CATEGORY_STATIC(LogSensorCameraTests, Log, All);

// IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSensorCamera_pinhole, "Display.Sensor.camera.pinhole",
// EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
//
// bool FSensorCamera_pinhole::RunTest(const FString& Parameters)
//{
//     if (!GEngine)
//     {
//         UE_LOG(LogSensorCameraTests, Error, TEXT("Failed to get engine"));
//         return false;
//     }
//     UDisplayGameInstance* game = NewObject< UDisplayGameInstance>(GEngine);
//     TestTrue(TEXT("game should success"), game != nullptr);
//     game->InitializeStandalone();
//
//     GConfig->SetBool(TEXT("Sensor"), TEXT("PublicMsg"), true, GGameIni);
//
//     FCameraConfig config;
//     config.intrinsic_Matrix.Add(1945);
//     config.intrinsic_Matrix.Add(0);
//     config.intrinsic_Matrix.Add(946);
//     config.intrinsic_Matrix.Add(0);
//     config.intrinsic_Matrix.Add(1938);
//     config.intrinsic_Matrix.Add(619);
//     config.intrinsic_Matrix.Add(0);
//     config.intrinsic_Matrix.Add(0);
//     config.intrinsic_Matrix.Add(1);
//     config.res_Horizontal = 1920;
//     config.res_Vertical = 1208;
//     ACameraSensor* CameraSensor = ASensorFactory::SpawnSensor<ACameraSensor>(game->GetWorld(),
//     ACameraSensor::StaticClass(), config); TestTrue(TEXT("CameraSensor should success"), CameraSensor != nullptr);
//
//     //ISimActorInterface* InstalledSimActor = CameraSensor->Install(config);
//     //TestTrue(TEXT("CameraSensor install should success"), InstalledSimActor != nullptr);
//
//     //FCameraInput input;
//     //FSensorOutput output;
//     //input.timeStamp = 1;
//     //CameraSensor->Update(input, output);
//     //TestTrue(TEXT("CameraSensor should success"), !output.serialize_string.empty());
//
//
//
//     return true;
// }

BEGIN_DEFINE_SPEC(FSensorCameraTest, "Display.Sensor.camera",
    EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ApplicationContextMask)
ASensorActor* CameraSensor = 0;
UDisplayGameInstance* game = 0;
FDoneDelegate UpdateDone;
TSharedPtr<FSensorConfig> sensorConfig;
void InitGame();
template <class SensorType>
void SensorInit()
{
    CameraSensor = ASensorFactory::SpawnSensor<SensorType>(game->GetWorld(), SensorType::StaticClass(), *sensorConfig);
    TestTrue(TEXT("CameraSensor should success"), CameraSensor != nullptr);
}
void SensorUpdate();
void DestroyGame();
END_DEFINE_SPEC(FSensorCameraTest)

void FSensorCameraTest::InitGame()
{
    if (!GEngine)
    {
        UE_LOG(LogSensorCameraTests, Error, TEXT("Failed to get engine"));
        return;
    }
    game = NewObject<UDisplayGameInstance>(GEngine);
    TestTrue(TEXT("game should success"), game != nullptr);
    game->InitializeStandalone();
    game->GetWorld()->ServerTravel(TEXT("/Game/Maps/LobbyMap"), false, false);
    // game->GetWorld()->SpawnActor< ADisplayGameStateBase>(ADisplayGameStateBase::StaticClass());
    game->GetWorld()->SpawnActor<ADisplayGameModeBase>(ADisplayGameModeBase::StaticClass());

    GConfig->SetBool(TEXT("Sensor"), TEXT("PublicMsg"), true, GGameIni);
}
void FSensorCameraTest::DestroyGame()
{
    if (CameraSensor)
    {
        CameraSensor->Destroy();
    }
    if (game)
    {
        game->Shutdown();
    }
    game = 0;
}
void FSensorCameraTest::SensorUpdate()
{
    if (CameraSensor)
    {
        FCameraInput input;
        FSensorOutput output;
        input.timeStamp = 1;
        CameraSensor->Update(input, output);
        TestTrue(TEXT("Camera update should success"), !output.serialize_string.empty());
        CameraSensor->Destroy();
        CameraSensor = 0;
    }
    UpdateDone.Execute();
}

void FSensorCameraTest::Define()
{
    Describe("Pinhole",
        [this]()
        {
            LatentBeforeEach(
                [this](const FDoneDelegate& Done)
                {
                    InitGame();
                    TSharedPtr<FCameraConfig> config(new FCameraConfig());
                    config->id = 0;
                    config->intrinsic_Matrix.Add(1945);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(946);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(1938);
                    config->intrinsic_Matrix.Add(619);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(1);
                    config->res_Horizontal = 1920;
                    config->res_Vertical = 1208;
                    // config->savePath = TEXT("D:/");
                    sensorConfig = config;
                    Done.Execute();
                });
            LatentIt("Init",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<ACameraSensor>();
                    Done.Execute();
                });
            LatentIt("Update",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<ACameraSensor>();
                    FPlatformProcess::Sleep(0.2f);
                    UpdateDone = Done;
                    AsyncTask(ENamedThreads::GameThread, [this]() { SensorUpdate(); });
                });

            LatentAfterEach(
                [this](const FDoneDelegate& Done)
                {
                    DestroyGame();
                    Done.Execute();
                });
        });
    Describe("Semantic",
        [this]()
        {
            LatentBeforeEach(
                [this](const FDoneDelegate& Done)
                {
                    InitGame();
                    TSharedPtr<FCameraConfig> config(new FCameraConfig());
                    config->id = 0;
                    config->intrinsic_Matrix.Add(1945);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(946);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(1938);
                    config->intrinsic_Matrix.Add(619);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(1);
                    config->res_Horizontal = 1920;
                    config->res_Vertical = 1208;
                    sensorConfig = config;
                    Done.Execute();
                });
            LatentIt("Init",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<ASemanticCamera>();
                    Done.Execute();
                });
            LatentIt("Update",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<ASemanticCamera>();
                    FPlatformProcess::Sleep(0.2f);
                    UpdateDone = Done;
                    AsyncTask(ENamedThreads::GameThread, [this]() { SensorUpdate(); });
                });

            LatentAfterEach(
                [this](const FDoneDelegate& Done)
                {
                    DestroyGame();
                    Done.Execute();
                });
        });

    Describe("Depth",
        [this]()
        {
            LatentBeforeEach(
                [this](const FDoneDelegate& Done)
                {
                    InitGame();
                    TSharedPtr<FCameraConfig> config(new FCameraConfig());
                    config->id = 0;
                    config->intrinsic_Matrix.Add(1945);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(946);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(1938);
                    config->intrinsic_Matrix.Add(619);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(1);
                    config->res_Horizontal = 1920;
                    config->res_Vertical = 1208;
                    sensorConfig = config;
                    Done.Execute();
                });
            LatentIt("Init",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<ADepthSensor>();
                    Done.Execute();
                });
            LatentIt("Update",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<ADepthSensor>();
                    FPlatformProcess::Sleep(0.2f);
                    UpdateDone = Done;
                    AsyncTask(ENamedThreads::GameThread, [this]() { SensorUpdate(); });
                });
            LatentAfterEach(
                [this](const FDoneDelegate& Done)
                {
                    DestroyGame();
                    Done.Execute();
                });
        });

    Describe("Fisheye",
        [this]()
        {
            LatentBeforeEach(
                [this](const FDoneDelegate& Done)
                {
                    InitGame();
                    TSharedPtr<FFisheyeConfig> config(new FFisheyeConfig());
                    config->id = 0;
                    config->intrinsic_Matrix.Add(337.14139734);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(641.86019172);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(336.24194116);
                    config->intrinsic_Matrix.Add(543.23107903);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(0);
                    config->intrinsic_Matrix.Add(1);
                    config->res_Horizontal = 1200;
                    config->res_Vertical = 1080;
                    config->distortion_Parameters.Add(0.00298329);
                    config->distortion_Parameters.Add(0.04234368);
                    config->distortion_Parameters.Add(-0.00231007);
                    config->distortion_Parameters.Add(-0.0028992);
                    sensorConfig = config;
                    Done.Execute();
                });
            LatentIt("Init",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<AFisheyeSensor>();
                    Done.Execute();
                });
            LatentIt("Update",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit<AFisheyeSensor>();
                    FPlatformProcess::Sleep(0.2f);
                    UpdateDone = Done;
                    AsyncTask(ENamedThreads::GameThread, [this]() { SensorUpdate(); });
                });
            LatentAfterEach(
                [this](const FDoneDelegate& Done)
                {
                    DestroyGame();
                    Done.Execute();
                });
        });
}

// IMPLEMENT_COMPLEX_AUTOMATION_TEST(FLoadAllMapsInGameTest, "Display.Sensor.camera.t",
// EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
//
// void FLoadAllMapsInGameTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const
//{
//
// }
//
// bool FLoadAllMapsInGameTest::RunTest(const FString& Parameters)
//{
//     FString MapName = Parameters;
//
//     ADD_LATENT_AUTOMATION_COMMAND(FEnqueuePerformanceCaptureCommands());
//
//     return true;
// }

#endif    // WITH_DEV_AUTOMATION_TESTS

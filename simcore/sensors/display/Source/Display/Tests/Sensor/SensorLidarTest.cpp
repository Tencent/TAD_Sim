// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Objects/Sensors/LidarSensors/TLidarSensor.h"
#include "Objects/Sensors/SensorFactory.h"
#include "Framework/DisplayGameInstance.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Engine.h"

#if WITH_DEV_AUTOMATION_TESTS

DEFINE_LOG_CATEGORY_STATIC(LogSensorLidarTests, Log, All);

BEGIN_DEFINE_SPEC(FSensorLidarTest, "Display.Sensor.lidar",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
ATLidarSensor* sensor = 0;
UDisplayGameInstance* game = 0;
FDoneDelegate UpdateDone;
void InitGame();
void SensorInit(const FString& model, int fps);
void SensorUpdate();
void DestroyGame();
END_DEFINE_SPEC(FSensorLidarTest)

void FSensorLidarTest::SensorInit(const FString& model, int fps)
{
    FLidarConfig config;
    config.cfgDir = FPaths::ProjectDir() + TEXT("XMLFiles/LidarConfig");
    config.id = 1;
    config.model = model;
    config.ip = "127.0.0.1";
    config.port = "2000,2001";
    config.frequency = fps;
    config.channels = 128;
    config.range = 200;
    config.horizontalResolution = 0.2;
    config.upperFovLimit = 30;
    config.lowerFovLimit = 30;
    config.FovStart = 60;
    config.FovEnd = 60;
    sensor = ASensorFactory::SpawnSensor<ATLidarSensor>(game->GetWorld(), ATLidarSensor::StaticClass(), config);
    TestTrue(TEXT("CameraSensor should success"), sensor != nullptr);
}
void FSensorLidarTest::InitGame()
{
    if (!GEngine)
    {
        UE_LOG(LogSensorLidarTests, Error, TEXT("Failed to get engine"));
        return;
    }
    game = NewObject<UDisplayGameInstance>(GEngine);
    TestTrue(TEXT("game should success"), game != nullptr);
    game->InitializeStandalone();
    // game->GetWorld()->ServerTravel(TEXT("/Game/Maps/LobbyMap"), false, false);
    // game->GetWorld()->SpawnActor< ADisplayGameStateBase>(ADisplayGameStateBase::StaticClass());
    // game->GetWorld()->SpawnActor< ADisplayGameModeBase>(ADisplayGameModeBase::StaticClass());

    GConfig->SetBool(TEXT("Sensor"), TEXT("PublicMsg"), true, GGameIni);
}
void FSensorLidarTest::DestroyGame()
{
    if (sensor)
    {
        sensor->Destroy();
    }
    if (game)
    {
        game->Shutdown();
    }
    game = 0;
}
void FSensorLidarTest::SensorUpdate()
{
    if (sensor)
    {
        FLidarInput input;
        FSensorOutput output;
        input.timeStamp = 0;
        input.timeStamp_ego = 0;
        sensor->Update(input, output);
        input.timeStamp = 1000;
        input.timeStamp_ego = 1000;
        sensor->Update(input, output);
        TestTrue(TEXT("Camera update should success"), !output.serialize_string.empty());
        sensor->Destroy();
        sensor = 0;
    }
    UpdateDone.Execute();
}

#define DESCRIBE_MODEL(model)                                                           \
    Describe(#model,                                                                    \
        [this]()                                                                        \
        {                                                                               \
            LatentBeforeEach(                                                           \
                [this](const FDoneDelegate& Done)                                       \
                {                                                                       \
                    InitGame();                                                         \
                    Done.Execute();                                                     \
                });                                                                     \
            LatentIt("FPS 5",                                                           \
                [this](const FDoneDelegate& Done)                                       \
                {                                                                       \
                    SensorInit(#model, 5);                                              \
                    FPlatformProcess::Sleep(0.2f);                                      \
                    UpdateDone = Done;                                                  \
                    AsyncTask(ENamedThreads::GameThread, [this]() { SensorUpdate(); }); \
                });                                                                     \
            LatentIt("FPS 10",                                                          \
                [this](const FDoneDelegate& Done)                                       \
                {                                                                       \
                    SensorInit(#model, 10);                                             \
                    FPlatformProcess::Sleep(0.2f);                                      \
                    UpdateDone = Done;                                                  \
                    AsyncTask(ENamedThreads::GameThread, [this]() { SensorUpdate(); }); \
                });                                                                     \
            LatentIt("FPS 20",                                                          \
                [this](const FDoneDelegate& Done)                                       \
                {                                                                       \
                    SensorInit(#model, 20);                                             \
                    FPlatformProcess::Sleep(0.2f);                                      \
                    UpdateDone = Done;                                                  \
                    AsyncTask(ENamedThreads::GameThread, [this]() { SensorUpdate(); }); \
                });                                                                     \
            LatentAfterEach(                                                            \
                [this](const FDoneDelegate& Done)                                       \
                {                                                                       \
                    DestroyGame();                                                      \
                    Done.Execute();                                                     \
                });                                                                     \
        });

void FSensorLidarTest::Define()
{
    DESCRIBE_MODEL(HS40)
    DESCRIBE_MODEL(HS64)
    DESCRIBE_MODEL(HS64QT)
    DESCRIBE_MODEL(HS128)
    DESCRIBE_MODEL(HS128AT)
    DESCRIBE_MODEL(RS16)
    DESCRIBE_MODEL(RS32)
    DESCRIBE_MODEL(RSBpearl)
    DESCRIBE_MODEL(RSHelios)
    DESCRIBE_MODEL(RSM1)
    DESCRIBE_MODEL(RSM1P)
    DESCRIBE_MODEL(RSRuby)
    DESCRIBE_MODEL(Velodyne32)
    DESCRIBE_MODEL(User)
}

#endif    // WITH_DEV_AUTOMATION_TESTS

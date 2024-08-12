// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Objects/Sensors/Ultrasonic/UltrasonicSensor.h"
#include "Objects/Sensors/SensorFactory.h"
#include "Framework/DisplayGameInstance.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

DEFINE_LOG_CATEGORY_STATIC(LogSensorUltrasonicTests, Log, All);

BEGIN_DEFINE_SPEC(FSensorUltrasonicTest, "Display.Sensor.ultrasonic",
    EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ApplicationContextMask)
ASensorActor* sensor = 0;
UDisplayGameInstance* game = 0;
FDoneDelegate UpdateDone;
void InitGame();
void SensorInit();
void SensorUpdate();
void DestroyGame();
END_DEFINE_SPEC(FSensorUltrasonicTest)

void FSensorUltrasonicTest::InitGame()
{
    if (!GEngine)
    {
        UE_LOG(LogSensorUltrasonicTests, Error, TEXT("Failed to get engine"));
        return;
    }
    game = NewObject<UDisplayGameInstance>(GEngine);
    TestTrue(TEXT("game should success"), game != nullptr);
    game->InitializeStandalone();
    game->GetWorld()->ServerTravel(TEXT("/Game/Maps/LobbyMap"), false, false);
    // game->GetWorld()->SpawnActor< ADisplayGameStateBase>(ADisplayGameStateBase::StaticClass());
    // game->GetWorld()->SpawnActor< ADisplayGameModeBase>(ADisplayGameModeBase::StaticClass());

    GConfig->SetBool(TEXT("Sensor"), TEXT("PublicMsg"), true, GGameIni);
}
void FSensorUltrasonicTest::SensorInit()
{
    FUltrasonicConfig config;
    config.id = 1;

    sensor = ASensorFactory::SpawnSensor<AUltrasonicSensor>(game->GetWorld(), AUltrasonicSensor::StaticClass(), config);
    TestTrue(TEXT("CameraSensor should success"), sensor != nullptr);
}
void FSensorUltrasonicTest::DestroyGame()
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
void FSensorUltrasonicTest::SensorUpdate()
{
    if (sensor)
    {
        FSensorInput input;
        FSensorOutput output;
        input.timeStamp = 1;
        sensor->Update(input, output);
        TestTrue(TEXT("Camera update should success"), !output.serialize_string.empty());
        sensor->Destroy();
        sensor = 0;
    }
    UpdateDone.Execute();
}

void FSensorUltrasonicTest::Define()
{
    Describe("Ultrasonic",
        [this]()
        {
            LatentBeforeEach(
                [this](const FDoneDelegate& Done)
                {
                    InitGame();
                    Done.Execute();
                });
            LatentIt("Init",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit();
                    Done.Execute();
                });
            LatentIt("Update",
                [this](const FDoneDelegate& Done)
                {
                    SensorInit();
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

#endif    // WITH_DEV_AUTOMATION_TESTS

// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Managers/SensorManager.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSensorSensorConfig, "Display.Sensor.sensorconfig",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

/**
 *
 * @param Parameters - Unused for this test
 * @return    TRUE if the test was successful, FALSE otherwise
 */
bool FSensorSensorConfig::RunTest(const FString& Parameters)
{
    FString testdata = FPaths::ProjectDir() + TEXT("Source/Display/Tests/Data/sensor.pb");
    TArray<uint8> pbstr;
    if (FFileHelper::LoadFileToArray(pbstr, *testdata))
    {
        std::string ss;
        ss.assign((char*) pbstr.GetData(), (char*) pbstr.GetData() + pbstr.Num());
        FSensorManagerConfig config = ASensorManager::ParseSensorString(ss, 0);

        TestEqual(TEXT("Sensor Config Test: One Camera."), config.cameraArry.Num(), 1);
        TestEqual(TEXT("Sensor Config Test: One Semantic."), config.semanticArry.Num(), 1);
        TestEqual(TEXT("Sensor Config Test: One Lidar."), config.lidarArry.Num(), 1);
        TestEqual(TEXT("Sensor Config Test: One Ultrasonic."), config.ultrasonicArry.Num(), 1);
        TestEqual(TEXT("Sensor Config Test: One Depth."), config.depthArry.Num(), 1);
        TestEqual(TEXT("Sensor Config Test: One Fisheye."), config.fisheyeArry.Num(), 1);
    };
    return true;
}

#endif    // WITH_DEV_AUTOMATION_TESTS

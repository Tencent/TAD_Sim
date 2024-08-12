// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Managers/Manager.h"
#include "Objects/Sensors/CameraSensors/CameraSensor.h"
#include "Objects/Sensors/CameraSensors/SemanticCamera.h"
#include "Objects/Sensors/LidarSensors/TLidarSensor.h"
#include "Objects/Sensors/Ultrasonic/UltrasonicSensor.h"
#include "Objects/Sensors/CameraSensors/FisheyeSensor.h"
#include "SensorManager.generated.h"

class ISensorInterface;
// struct FCameraConfig;
// struct FCameraOutput;

// USTRUCT()
// struct FIdentify
//{
//     GENERATED_BODY();
// public:
//     FString Name;
//     int32 Id;
//     bool operator== (const FIdentify& Other)
//     {
//         return (Id == Other.Id) && (Name == Other.Name);
//     }
//     //friend uint32 GetTypeHash(const FIdentify& Other)
//     //{
//     //    return GetTypeHash(Other.Id);
//     //}
// };

// USTRUCT()
// struct FSensorGroup
//{
//     GENERATED_BODY();
// public:
//     ISimActorInterface* simActorPtr;
//
// };

USTRUCT()
struct FSensorManagerConfig : public FManagerConfig
{
    GENERATED_BODY();

public:
    TArray<FCameraConfig> cameraArry;
    TArray<FCameraConfig> semanticArry;
    TArray<FLidarConfig> lidarArry;
    TArray<FUltrasonicConfig> ultrasonicArry;
    TArray<FCameraConfig> depthArry;
    TArray<FFisheyeConfig> fisheyeArry;
    TArray<FCameraConfig> ringArry;
};

USTRUCT()
struct FSensorManagerIn : public FManagerIn
{
    GENERATED_BODY();
};

USTRUCT()
struct FSensorManagerOut : public FManagerOut
{
    GENERATED_BODY();

public:
    TArray<FSensorOutput> outArray;
};

/**
 *
 */
UCLASS()
class DISPLAY_API ASensorManager : public AManager
{
    GENERATED_BODY()
public:
    ASensorManager();

public:
    virtual void Init(const FManagerConfig& Config);

    virtual void Update(const FManagerIn& Input, FManagerOut& Output);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    TMap<ISimActorInterface*, TMap<FString, ISensorInterface*>> sensorMap;

    class AROS2Node* rosNode = NULL;

public:
    static FSensorManagerConfig ParseSensorString(const std::string& buffer, int64 EgoId);
    static void CoordinateTransform_RightHandToLeftHand(FVector& _Location, FRotator& _Rotation);
};

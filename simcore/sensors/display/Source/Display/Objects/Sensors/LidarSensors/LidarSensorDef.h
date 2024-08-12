// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../SensorInterface.h"
#include "lidar/Lidar.h"
#include <memory>
#include "LidarSensorDef.generated.h"
////
// USTRUCT(BlueprintType)
// struct FTLidarRenderConfigStruct : public FRenderConfigStruct
//{
//     GENERATED_BODY();
//     UPROPERTY(BlueprintReadWrite, Category = "TLidarRenderConfig")
//         bool bDrawPoint = true;
//
//     UPROPERTY(BlueprintReadWrite, Category = "TLidarRenderConfig")
//         bool bDrawRay = false;
// };

USTRUCT(BlueprintType)
struct FLidarConfig : public FSensorConfig
{
    GENERATED_BODY();
    UPROPERTY(BlueprintReadOnly, Category = "TLidarSensorConfig")
    FString model = "User";

    UPROPERTY(BlueprintReadOnly, Category = "TLidarSensorConfig")
    int channels = 32;

    UPROPERTY(BlueprintReadOnly, Category = "TLidarSensorConfig")
    float range = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TLidarSensorConfig")
    float horizontalResolution = 0.1;

    UPROPERTY(BlueprintReadOnly, Category = "TLidarSensorConfig")
    float upperFovLimit = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TLidarSensorConfig")
    float lowerFovLimit = -30.0f;

    bool bDrawPoint = false;
    FString cfgDir = "";

    FString ip;
    FString port;
    float Attenuation = 0.f;
    FString AngleDefinition;
    float ExtinctionCoe = 0;
    int ReflectionType = 0;
    float FovStart = 0;
    float FovEnd = 360;
    bool hil = false;
};

USTRUCT()
struct FLidarInput : public FSensorInput
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FLidarOutput : public FSensorOutput
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FColor> buffer;
};

struct FTLidarMeasurement
{
    uint32 HorizontalPos = 0;
    uint32 HorizontalToScan = 0;
    double TimeStamp0 = -1;    // lasttime stamp
    double TimeStamp = 0;      // curtime stamp
    double TimeSpan = 0;
    FVector LidarBodyLoc0;    // lasttime loc
    FVector LidarBodyLoc;     // curtime loc
    FRotator LidarBodyRot;    // curtime rot
};

struct LidarBuffer
{
    virtual ~LidarBuffer()
    {
    }
};
enum LidarBufferMethod
{
    BUFMETHOD_RAYCAST = 0,
    BUFMETHOD_DEPTH = 1,
    BUFMETHOD_RAYTRACING = 2,
};

class LidarBufferFun
{
public:
    LidarBufferFun()
    {
        rtMatrix.SetIdentity();
    }
    virtual ~LidarBufferFun()
    {
    }
    virtual LidarBufferMethod Method() = 0;

    virtual bool Init(const FLidarConfig& _config, std::shared_ptr<lidar::TraditionalLidar> _LidarSensor,
        AActor* _actor = 0, class LidarModel* lmodel = 0)
    {
        config = _config;
        lidarSensor = _LidarSensor;
        actor = _actor;
        lidarMd = lmodel;
        return true;
    }
    virtual TSharedPtr<LidarBuffer> GetTBuffer(const FTLidarMeasurement& measure) = 0;
    // muti thread
    virtual bool GetPoints(const LidarBuffer* rawbuf, const FTLidarMeasurement& measure,
        lidar::TraditionalLidar::lidar_ptset& lidarBuffer) = 0;
    virtual void setRotationTranslation(const FTransform& mat)
    {
        rtMatrix = mat;
    }

protected:
    FLidarConfig config;
    std::shared_ptr<lidar::TraditionalLidar> lidarSensor;
    AActor* actor;
    class LidarModel* lidarMd = 0;
    FTransform rtMatrix;
};

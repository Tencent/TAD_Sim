// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LidarSensorDef.h"
#include "Objects/Sensors/SensorActor.h"
#include "lidar/Lidar.h"
#include "lidar/LidarModel.h"
#include "Networking.h"

struct RaytracingLidarBuffer : LidarBuffer
{
    TArray<FColor> pts;
};
class ALidarBufferRayTracing : public LidarBufferFun
{
public:
    virtual LidarBufferMethod Method()
    {
        return BUFMETHOD_RAYTRACING;
    }
    virtual bool Init(const FLidarConfig& config, std::shared_ptr<lidar::TraditionalLidar> _LidarSensor,
        AActor* _actor = 0, class LidarModel* lmodel = 0);
    virtual TSharedPtr<LidarBuffer> GetTBuffer(const FTLidarMeasurement& measure);

    virtual bool GetPoints(const LidarBuffer* rawbuf, const FTLidarMeasurement& measure,
        lidar::TraditionalLidar::lidar_ptset& lidarBuffer);

private:
    std::map<int, std::pair<uint32, uint32>> stencilMap;
    bool LoadStencilMap(const FString& dir);
    class ARTXLidarSensor* rtxActor = nullptr;
};

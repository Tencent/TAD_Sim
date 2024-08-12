// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LidarSensorDef.h"
#include "Objects/Sensors/SensorActor.h"
#include "lidar/Lidar.h"
#include "lidar/LidarModel.h"
#include "Networking.h"
// #include "TLidarBufferRaycast.generated.h"

struct RaycastLidarBuffer : LidarBuffer
{
    struct lpoint
    {
        FVector p0;
        FVector p;
        FVector nor;
        float d = -1;
        unsigned int tag_c = 0u;
        unsigned int tag_t = 0u;
        bool valid() const
        {
            return d > 0;
        }
        void set_invalid()
        {
            d = -1;
        }
    };
    TArray<lpoint> pts;
};

class ALidarBufferRaycast : public LidarBufferFun
{
public:
    virtual LidarBufferMethod Method()
    {
        return BUFMETHOD_RAYCAST;
    }
    virtual bool Init(const FLidarConfig& config, std::shared_ptr<lidar::TraditionalLidar> _LidarSensor,
        AActor* _actor = 0, class LidarModel* lmodel = 0);
    virtual TSharedPtr<LidarBuffer> GetTBuffer(const FTLidarMeasurement& measure);

    virtual bool GetPoints(const LidarBuffer* rawbuf, const FTLidarMeasurement& measure,
        lidar::TraditionalLidar::lidar_ptset& lidarBuffer);

    void SetShowPoint(bool b)
    {
        showPt = b;
    }
    // private:

    friend class linetrace;

    bool showPt = false;
    int linetracing_thread = 1;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LidarSensorDef.h"
#include "Objects/Sensors/SensorActor.h"
#include "lidar/Lidar.h"
#include "lidar/LidarModel.h"
#include "Networking.h"
#include <map>
#include <vector>
#include "CudaLidarModel.h"
// #include "TLidarBufferDepth.generated.h"

struct DepthLidarBuffer : public LidarBuffer
{
    struct ImgBuffer
    {
        TArray<FColor> cpuImg;
        uint8_t* gpuImg = 0;
    };
    TArray<ImgBuffer> imgBuffer;
};

class ALidarBufferDepth : public LidarBufferFun
{
public:
    virtual LidarBufferMethod Method()
    {
        return BUFMETHOD_DEPTH;
    }
    virtual bool Init(const FLidarConfig& config, std::shared_ptr<lidar::TraditionalLidar> _LidarSensor,
        AActor* _actor = 0, class LidarModel* lmodel = 0);
    virtual TSharedPtr<LidarBuffer> GetTBuffer(const FTLidarMeasurement& measure);

    virtual bool GetPoints(const LidarBuffer* rawbuf, const FTLidarMeasurement& measure,
        lidar::TraditionalLidar::lidar_ptset& lidarBuffer);

    virtual void setRotationTranslation(const FTransform& mat);

private:
    TArray<class ADepthLidarBuffer*> depthCameraActors;
    int nImage{8};
    float Hfov = 60;
    float offsetX0;
    float offsetX1;
    std::map<int, std::pair<uint32, uint32>> stencilMap;
    TArray<uint8> depthCamIdx;
    TArray<uint32> camuvIdx;

    CudaLidarModel cudalidar;
    TArray<uint8_t*> imgBuffers_gpu;

    bool LoadStencilMap(const FString& dir);
    void getData(
        const TArray<FColor>& BitMap, float w, float h, float x, float y, float& distance, float& norangle, int& tag);
};

// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @file TLidarBufferDepth.cpp
 * @brief This file contains the implementation of TLidarBufferDepth.
 * 深度方式获取lidar的射线数据
 * @author <kekesong>
 * @date 2024-03-10
 * @copyright Copyright (c) 2024 tencent All Rights Reserved.
 */

#include "TLidarBufferRayTracing.h"
#include <thread>
#include <fstream>
#include <sstream>
#include "lidar/RSLidar.h"
#include "lidar/UTLidar.h"
#include "lidar/LidarModel.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/SaveDataThread.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#if PLATFORM_WINDOWS
#include "RTXLidarSensor.h"
#endif
#include "LidarSensorDef.h"

bool ALidarBufferRayTracing::Init(const FLidarConfig& _config, std::shared_ptr<lidar::TraditionalLidar> _LidarSensor,
    AActor* _actor, class LidarModel* lmodel)
{
#if PLATFORM_WINDOWS
    LidarBufferFun::Init(_config, _LidarSensor, _actor, lmodel);
    LoadStencilMap(_config.cfgDir);

    rtxActor = actor->GetWorld()->SpawnActor<ARTXLidarSensor>();
    rtxActor->SetOwner(actor);
    rtxActor->AttachToActor(actor, FAttachmentTransformRules::KeepRelativeTransform);

    // 获取激光雷达内存，用于设置光追模块
    TArray<FVector2D> angles;
    int hor_num = _LidarSensor->getHorizontalScanCount();
    int ray_num = _LidarSensor->getRaysNum();
    angles.Reserve(hor_num * ray_num);
    for (int i = 0; i < hor_num; i++)
    {
        for (int j = 0; j < ray_num; j++)
        {
            FVector2D a;
            auto yawpitch = lidarSensor->getYawPitchAngle(i, j);
            a.X = yawpitch.first;
            a.Y = yawpitch.second;
            angles.Add(a);
        }
    }
    rtxActor->SetYawPitchArray(angles);

    TArray<FVector4> ref;
    for (int i = 0; i < 1024; i++)
    {
        float fmin = 10, fmax = 10, fd = 0.1;
        uint32 c = 0, t = 0;
        if (stencilMap.find(i) != stencilMap.end())
        {
            c = stencilMap.at(i).first;
            t = stencilMap.at(i).second;
            if (!lidarMd->get_refection_param(c, t, fmin, fmax, fd))
            {
                /// todo
                UE_LOG(LogTemp, Log, TEXT("Lidar depth init warnning, %d, %d found faild."), c, t);
            }
        }
        ref.Add(FVector4(fmin, fmax, fd, 0));
    }
    rtxActor->SetRefArray(ref);
    // 设置POSE
    FTransform rt;
    rt.SetLocation(_config.installLocation);
    rt.SetRotation(_config.installRotation.Quaternion());
    rtxActor->SetInstallPose(rt);
    return true;
#else
    UE_LOG(LogTemp, Log, TEXT("rtx only support windows"));
    return false;
#endif
}

TSharedPtr<LidarBuffer> ALidarBufferRayTracing::GetTBuffer(const FTLidarMeasurement& measure)
{
#if PLATFORM_WINDOWS
    // 设置天气
    if (rtxActor && lidarMd)
    {
        rtxActor->SetMirrorThreshold(lidarMd->get_rain() / 50.f);
        rtxActor->SetLidarIntensity(lidarMd->get_intensity());
        rtxActor->SetLidarRain(lidarMd->get_rain());
        rtxActor->SetLidarSnow(FMath::Max(lidarMd->get_snow(), lidarMd->get_rain() * 0.1f));
        rtxActor->SetLidarFog(lidarMd->get_fog());
    }
    // 获取深度数据
    TSharedPtr<RaytracingLidarBuffer> buffer = MakeShared<RaytracingLidarBuffer>();
    if (rtxActor && rtxActor->GetCaptureData(buffer->pts))
    {
        return buffer;
    }
#endif
    return TSharedPtr<LidarBuffer>();
}

// 获取激光点云
bool ALidarBufferRayTracing::GetPoints(
    const LidarBuffer* rawbuf, const FTLidarMeasurement& measure, lidar::TraditionalLidar::lidar_ptset& lidarBuffer)
{
    const RaytracingLidarBuffer* buffer = StaticCast<const RaytracingLidarBuffer*>(rawbuf);

    uint32_t rn = lidarSensor->getRaysNum();
    uint32_t rtn = lidarSensor->getReturnNum();
    uint32_t hwn = lidarSensor->getHorizontalScanCount() * rn;
    lidarBuffer.points.resize(measure.HorizontalToScan * rn * rtn);
    lidarBuffer.channels.resize(measure.HorizontalToScan);
    double umpsec = 1000.0 * 1000. / (lidarSensor->getRotationFrequency() * lidarSensor->getHorizontalScanCount());
    auto utime = measure.TimeStamp0 * 1000;

    bool exinfo = !config.hil || config.savePath.Len() > 0;
    for (uint32_t i = 0; i < measure.HorizontalToScan; i++)
    {
        lidar::TraditionalLidar::channel_data& dd = lidarBuffer.channels[i];
        dd.hor_pos = (measure.HorizontalPos + i) % lidarSensor->getHorizontalScanCount();
        dd.utime = utime + umpsec * i;
        dd.pn = rn * rtn;
        dd.points = &lidarBuffer.points[i * rn * rtn];
        for (uint32 j = 0; j < rn; ++j)
        {
            uint32_t ii = dd.hor_pos * rn + j;
            const auto& color00 = buffer->pts[ii];
            // 计算距离
            float d = ((float) color00.R * 256.f + (float) color00.G) * 0.005f;
            lidar::TraditionalLidar::lidar_point& lp = dd.points[j];
            if (d > 0.01 && d < 327.f)
            {
                lp.distance = d;
                lp.instensity = (float) color00.B;
                if (exinfo)
                {
                    int tag = color00.A;
                    if (stencilMap.find(tag) == stencilMap.end())
                        tag = 0;
                    lp.tag_c = stencilMap[tag].first;
                    lp.tag_t = stencilMap[tag].second;
                    auto yawpitch = lidarSensor->getYawPitchAngle(dd.hor_pos, j);
                    FRotator LaserRot(
                        yawpitch.second, yawpitch.first, 0);    // float InPitch, float InYaw, float InRoll
                    // 计算3d坐标
                    auto pv = rtMatrix.TransformPosition(lp.distance * UKismetMathLibrary::GetForwardVector(LaserRot));
                    lp.x = pv.X;
                    lp.y = pv.Y;
                    lp.z = pv.Z;
                }
            }
            for (uint32 t = 1; t < rtn; ++t)
            {
                memcpy(&dd.points[rn * t], dd.points, sizeof(lidar::TraditionalLidar::lidar_point) * rn);
            }
        }
    }

    return true;
}

// 加载stencil文件
bool ALidarBufferRayTracing::LoadStencilMap(const FString& dir)
{
    TArray<FString> txt;
    if (!FFileHelper::LoadFileToStringArray(txt, *(dir + TEXT("/stencil.dat"))))
    {
        UE_LOG(LogTemp, Log, TEXT("Cannot load stencil.dat"));
        return false;
    }
    for (auto tt : txt)
    {
        TArray<int64> bufs;
        FString LeftStr;
        FString RightStr;
        while (tt.Split(" ", &LeftStr, &RightStr))
        {
            if (!LeftStr.IsEmpty())
                bufs.Add(FCString::Atoi64(*LeftStr));
            tt = RightStr;
        }
        bufs.Add(FCString::Atoi64(*tt));
        int s = bufs[0];
        int c = bufs[1];
        int t = bufs[2];
        stencilMap[s] = std::make_pair(c, t);
    }
    return true;
}

// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @file TLidarBufferRaycast.CPP
 * @brief This file contains the implementation of the raycast lidar buffer.
 * 射线方式获取点云深度
 * @author <kekesong>
 * @date 2017-09-10
 * @COPYRIGHT tencent
 */

#include "TLidarBufferRaycast.h"
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
#include "TLidarBufferRaycast.h"
#include "lidar/LidarModel.h"

bool ALidarBufferRaycast::Init(const FLidarConfig& _config, std::shared_ptr<lidar::TraditionalLidar> _LidarSensor,
    AActor* _actor, class LidarModel* lmodel)
{
    LidarBufferFun::Init(_config, _LidarSensor, _actor, lmodel);

    FString LidarThreads = TEXT("1");
    if (GConfig->GetString(TEXT("Sensor"), TEXT("LidarThreads"), LidarThreads, GGameIni))
    {
        UE_LOG(LogTemp, Log, TEXT("SensorManger: LidarThreads is: %s."), *LidarThreads);
        linetracing_thread = FCString::Atoi(*LidarThreads);
    }

    return true;
}

// 射线方法获取点云深度
void linetrace(UWorld* world, uint32 scanfrom, uint32 scanto, const FTLidarMeasurement& LidarMeasurement,
    const lidar::TraditionalLidar* LidarSensor, RaycastLidarBuffer::lpoint* pdata)
{
    scanto = std::min(scanto, LidarMeasurement.HorizontalToScan);
    if (scanfrom >= scanto)
        return;

    /*FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, lidarActor);*/
    FCollisionQueryParams TraceParams =
        FCollisionQueryParams(FName(TEXT("Laser_Trace")), true /*, lidarActor->InstalledActor*/);
    TraceParams.bTraceComplex = true;
    TraceParams.bReturnPhysicalMaterial = false;
    FVector locStep =
        (LidarMeasurement.LidarBodyLoc - LidarMeasurement.LidarBodyLoc0) / LidarMeasurement.HorizontalToScan;
    FRotator LidarBodyRot = LidarMeasurement.LidarBodyRot;    // FRotator(0, 0, 0);//
    FHitResult HitInfo(ForceInit);

    float lzr = LidarSensor->getLaserRadius();
    float lzh = LidarSensor->getLaserHeight();

    for (auto i = scanfrom; i < scanto; ++i)
    {
        for (auto c = 0u; c < LidarSensor->getRaysNum(); ++c)
        {
            auto yawpitch = LidarSensor->getYawPitchAngle(
                (LidarMeasurement.HorizontalPos + i) % LidarSensor->getHorizontalScanCount(), c);
            FRotator LaserRot(yawpitch.second, yawpitch.first, 0);    // float InPitch, float InYaw, float InRoll
            FRotator ResultRot = UKismetMathLibrary::ComposeRotators(LaserRot, LidarBodyRot);
            // 计算射线起点
            auto loc = LidarMeasurement.LidarBodyLoc0 + locStep * i;

            // laser positon
            loc.X += lzr * cos(FMath::DegreesToRadians(yawpitch.second));
            loc.Y += lzr * sin(FMath::DegreesToRadians(yawpitch.second));
            loc.Z += lzh;
            // 计算射线终点
            FVector EndTrace = LidarSensor->getRange() * 100 * UKismetMathLibrary::GetForwardVector(ResultRot) + loc;
            // 发射射线
            world->LineTraceSingleByChannel(HitInfo, loc, EndTrace, ECollisionChannel::ECC_GameTraceChannel1,
                TraceParams, FCollisionResponseParams::DefaultResponseParam);
            RaycastLidarBuffer::lpoint* p = pdata + i * LidarSensor->getRaysNum() + c;

            if (HitInfo.bBlockingHit)
            {
                /*UStaticMeshComponent* staticMeshComponent = Cast<UStaticMeshComponent>(HitInfo.GetComponent());
                if (staticMeshComponent)
                {
                    UStaticMesh* mesh = staticMeshComponent->GetStaticMesh();
                    if (mesh)
                    {
                        const int32& faceIndex = HitInfo.FaceIndex;
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Get mesh from static mesh component faild"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cast FHitResul.GetComponent to static mesh component faild"));
                }*/
                //*p = HitInfo.ImpactPoint;
                // 命中距离
                float d = HitInfo.Distance / 100.f;
                // 起点
                p->p0 = loc;
                // 命中点
                p->p = HitInfo.ImpactPoint;
                // 命中发现
                p->nor = HitInfo.ImpactNormal;
                p->d = d;
                p->tag_c = 40000000u;
                p->tag_t = 1u;
                auto actor = HitInfo.Actor;
                if (actor.IsValid() && actor->Tags.Num() > 0)
                {
                    uint32 u1 = FCString::Atoi(*actor->Tags[0].ToString());
                    uint32 u2 = 0;
                    if (actor->Tags.Num() > 1)
                    {
                        u2 = FCString::Atoi(*actor->Tags[1].ToString());
                    }
                    if (actor->Tags.Num() > 1 && actor->Tags[0].ToString().Len() < actor->Tags[1].ToString().Len())
                    {
                        std::swap(u1, u2);
                    }
                    p->tag_c = u1;
                    p->tag_t = u2;
                }
            }
            else
            {
                p->set_invalid();
            }
        }
    }
}

TSharedPtr<LidarBuffer> ALidarBufferRaycast::GetTBuffer(const FTLidarMeasurement& measure)
{
    uint32_t rn = lidarSensor->getRaysNum();

    TSharedPtr<RaycastLidarBuffer> buffer = MakeShared<RaycastLidarBuffer>();
    // 统计当前激光射线，多线程执行
    buffer->pts.SetNum(rn * measure.HorizontalToScan);
    {
        int ltt = std::max(1, linetracing_thread);
        std::vector<std::thread> thrs;
        for (uint32 i = 0; i < (uint32) ltt; i++)
        {
            thrs.push_back(std::thread(linetrace, actor->GetWorld(), measure.HorizontalToScan * i / ltt,
                measure.HorizontalToScan * (i + 1) / ltt, measure, lidarSensor.get(), buffer->pts.GetData()));
            // thrs.push_back(std::move(t1));
        }
        for (std::thread& th : thrs)
        {
            // If thread Object is Joinable then Join that thread.
            if (th.joinable())
                th.join();
        }
    }
    if (showPt)
    {
        for (const auto& p : buffer->pts)
        {
            if (p.valid())
                DrawDebugPoint(actor->GetWorld(), p.p, 4, FColor(0, 255, 0), false, -1);
        }
    }

    return buffer;
}

// 将射线数据转换成激光点云
bool ALidarBufferRaycast::GetPoints(
    const LidarBuffer* rawbuf, const FTLidarMeasurement& measure, lidar::TraditionalLidar::lidar_ptset& lidarBuffer)
{
    const RaycastLidarBuffer* buffer = StaticCast<const RaycastLidarBuffer*>(rawbuf);
    uint32_t rn = lidarSensor->getRaysNum();
    uint32_t rtn = lidarSensor->getReturnNum();
    lidarBuffer.channels.resize(measure.HorizontalToScan);
    lidarBuffer.points.resize(measure.HorizontalToScan * rn * rtn);

    double umpsec = 1000.0 * 1000. / (lidarSensor->getRotationFrequency() * lidarSensor->getHorizontalScanCount());
    auto utime = measure.TimeStamp0 * 1000;
    for (uint32 i = 0; i < measure.HorizontalToScan; i++)
    {
        auto& dd = lidarBuffer.channels.at(i);
        dd.hor_pos = (measure.HorizontalPos + i) % lidarSensor->getHorizontalScanCount();
        dd.utime = utime + umpsec * i;
        dd.pn = rn * rtn;
        dd.points = &lidarBuffer.points[i * rn * rtn];
        for (uint32 j = 0; j < rn; ++j)
        {
            const auto& p = buffer->pts[i * rn + j];
            auto& pt = dd.points[j];
            pt.distance = p.d;
            pt.tag_c = p.tag_c;
            pt.tag_t = p.tag_t;
            pt.norinter = abs(FVector::DotProduct(p.p - p.p0, p.nor));
            // LIDAR 模型
            if (lidarMd)
                lidarMd->simulator(pt.norinter, p.tag_c, p.tag_t, pt.distance, pt.instensity);
            if (pt.distance > 0.01 && pt.distance < 327.f)
            {
                auto yawpitch = lidarSensor->getYawPitchAngle(dd.hor_pos, j);
                FRotator LaserRot(yawpitch.second, yawpitch.first, 0);    // float InPitch, float InYaw, float InRoll
                // 计算3d坐标
                auto pv = rtMatrix.TransformPosition(pt.distance * UKismetMathLibrary::GetForwardVector(LaserRot));
                pt.x = pv.X;
                pt.y = pv.Y;
                pt.z = pv.Z;
            }
        }
        for (uint32 t = 1; t < rtn; ++t)
        {
            memcpy(&dd.points[rn * t], dd.points, sizeof(lidar::TraditionalLidar::lidar_point) * rn);
        }
    }

    return true;
}

// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @file TLidarBufferDepth.cpp
 * @brief This file contains the implementation of TLidarBufferDepth.
 * 深度方式获取lidar的射线数据
 * @author <kekesong>
 * @date 2024-03-10
 * @copyright Copyright (c) 2024 TENCENT Inc. All Rights Reserved.
 */

#include "TLidarBufferDepth.h"
#include <thread>
#include <fstream>
#include <sstream>
#include "lidar/RSLidar.h"
#include "lidar/UTLidar.h"
#include "lidar/LidarModel.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DepthCamera.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Framework/SaveDataThread.h"
#include "Framework/DisplayGameInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include <chrono>

#define ALONEFOV 140.f

// #define DEP_ROTATE
bool ALidarBufferDepth::Init(const FLidarConfig& _config, std::shared_ptr<lidar::TraditionalLidar> _LidarSensor,
    AActor* _actor, class LidarModel* lmodel)
{
    LidarBufferFun::Init(_config, _LidarSensor, _actor, lmodel);
    // load stencil map
    LoadStencilMap(config.cfgDir);

    // 一圈需要几个深度相机：周深=四周深度
    FString DepthImageN;
    if (GConfig->GetString(TEXT("Sensor"), TEXT("LidarDN"), DepthImageN, GGameIni))
    {
        UE_LOG(LogTemp, Log, TEXT("SensorManger: DepthImageN is: %s. "), *DepthImageN);
        nImage = FMath::Min(100, FCString::Atoi(*DepthImageN));
    }
    Hfov = 360.f / nImage;
    // 计算当前lidar的水平fov，用于判断周深的开启
    offsetX0 = offsetX1 = 0;
    float minV = 90, maxV = -90;
    for (uint32 i = 0; i < _LidarSensor->getHorizontalScanCount(); i++)
    {
        for (uint32 j = 0; j < _LidarSensor->getRaysNum(); j++)
        {
            auto yawpitch = _LidarSensor->getYawPitchAngle(i, j);
            minV = std::min(minV, yawpitch.second);
            maxV = std::max(maxV, yawpitch.second);
            offsetX0 = std::min(offsetX0, yawpitch.first - _LidarSensor->getHorizontalScanAngle(i));
            offsetX1 = std::max(offsetX1, yawpitch.first - _LidarSensor->getHorizontalScanAngle(i));
        }
    }
    // 计算俯仰角，用于周深的放置
    float baseV0 = (minV + maxV) * 0.5;
    float vfov = (maxV - minV) * 0.5;

    float Vfov = vfov * 2.f;

    float hfov = FMath::RadiansToDegrees(FMath::Atan(
                     FMath::Tan(FMath::DegreesToRadians(Hfov * 0.5f)) / FMath::Cos(FMath::DegreesToRadians(baseV0)))) *
                 2.1f;
    vfov = FMath::RadiansToDegrees(FMath::Atan(
               FMath::Tan(FMath::DegreesToRadians(Vfov * 0.5f)) / FMath::Cos(FMath::DegreesToRadians(hfov * 0.5f)))) *
           2.1f;

    float ref_scale = _LidarSensor->getRaysNum();
    ref_scale /= 40;
    if (ref_scale < 1.f)
        ref_scale = 1.f;

    float ares =
        FMath::Max(0.1f, FMath::Abs(_LidarSensor->getHorizontalScanAngle(1) - _LidarSensor->getHorizontalScanAngle(0)));
    float la = FMath::Tan(FMath::DegreesToRadians(ares));
    float lw = FMath::Tan(FMath::DegreesToRadians(hfov * 0.5f));

    float w = lw * 2.f / la;
    w = std::max(w, hfov / ares);

    // 生成周深相机配置
    CameraSensorViewConfiguration camCfg;
    camCfg.position = FVector(0);
    camCfg.rotator = FRotator(baseV0, 0, 0);
    camCfg.hfov = hfov;
    camCfg.vfov = vfov;
    camCfg.w = w * ref_scale;
    camCfg.h = FMath::Tan(FMath::DegreesToRadians(vfov * 0.5f)) / FMath::Tan(FMath::DegreesToRadians(0.1f)) * ref_scale;

    float cx = camCfg.w * 0.5f;
    float cy = camCfg.h * 0.5f;
    float fx = cx / FMath::Tan(FMath::DegreesToRadians(hfov * 0.5f));
    float fy = cy / FMath::Tan(FMath::DegreesToRadians(vfov * 0.5f));

    UE_LOG(LogTemp, Log, TEXT("Lidar depth image w=%d, h=%d, vfov=%f, hfov=%f, basev=%f"), camCfg.w, camCfg.h, vfov,
        hfov, baseV0);

    uint32_t rn = lidarSensor->getRaysNum();
    uint32_t rtn = lidarSensor->getReturnNum();
    uint32_t hw = _LidarSensor->getHorizontalScanCount();
    depthCamIdx.SetNum(rn * hw * rtn);
    memset(depthCamIdx.GetData(), 255, depthCamIdx.Num());
    camuvIdx.SetNum(rn * hw * rtn);
    memset(camuvIdx.GetData(), 0, camuvIdx.Num());
    // 计算激光线与周深相机的坐标映射：每个光线的相机编号，和uv坐标位置
    depthCameraActors.SetNumZeroed(nImage);
    for (uint32_t i = 0; i < hw; ++i)
    {
        for (uint32_t c = 0; c < rn; ++c)
        {
            auto yawpitch = lidarSensor->getYawPitchAngle(i, c);
            float ha = yawpitch.first + 360.f;
            int pici = FMath::FloorToInt(ha / Hfov);
            pici = pici % nImage;
            ha = FMath::DegreesToRadians(ha);
            float va = FMath::DegreesToRadians(yawpitch.second);
            FVector p = FVector(FMath::Cos(va) * FMath::Cos(ha), FMath::Cos(va) * FMath::Sin(ha), FMath::Sin(va));
            FVector p1 = FRotator(0, -pici * Hfov - Hfov * 0.5, 0).RotateVector(p);
            p = FRotator(-baseV0, 0, 0).RotateVector(p1);
            if (p.X < 0.0001)
            {
                UE_LOG(LogTemp, Log, TEXT("Lidar depth init warnning, please send to developer."));
                continue;
            }
            p.Y /= p.X;
            p.Z /= p.X;
            float u = fx * p.Y + cx;
            float v = -fy * p.Z + cy;
            int x0 = FMath::FloorToInt(u);
            int y0 = FMath::FloorToInt(v);
            if (y0 < 0 || y0 >= camCfg.h || x0 < 0 || x0 >= camCfg.w)
            {
                UE_LOG(LogTemp, Log, TEXT("Lidar depth init warnning, please send to developer."));
                continue;
            }
            camuvIdx[i * rn * rtn + c] = y0 * camCfg.w + x0;
            depthCamIdx[i * rn * rtn + c] = pici;

            if (!depthCameraActors[pici])
            {
                ADepthLidarBuffer* sensor = actor->GetWorld()->SpawnActor<ADepthLidarBuffer>();
                depthCameraActors[pici] = sensor;
                sensor->SetOwner(actor);
                sensor->AttachToActor(actor, FAttachmentTransformRules::KeepRelativeTransform);
                camCfg.rotator.Yaw = pici * Hfov + Hfov * 0.5f;

                depthCameraActors[pici]->SetCamera(camCfg);
            }
        }
        for (uint32 t = 1; t < rtn; ++t)
        {
            memcpy(&camuvIdx[i * rn * rtn + rn * t], &camuvIdx[i * rn * rtn], sizeof(uint32) * rn);
            memcpy(&depthCamIdx[i * rn * rtn + rn * t], &depthCamIdx[i * rn * rtn], sizeof(uint8) * rn);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DisplayGamma=%f"), GEngine->DisplayGamma);

    TArray<float> ref;
    TArray<uint32> tag;
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
        ref.Add(fmin);
        ref.Add(fmax);
        ref.Add(fd);
        tag.Add(c);
        tag.Add(t);
    }
    // 使用cuda
    cudalidar.set_refmap(ref);
    cudalidar.set_tagmap(tag);
    cudalidar.set_camidx(depthCamIdx);
    cudalidar.set_camuv(camuvIdx);
    TArray<float> yp;
    for (uint32 i = 0; i < hw; i++)
    {
        for (uint32 t = 0; t < rtn; ++t)
        {
            for (uint32 j = 0; j < rn; j++)
            {
                auto yawpitch = _LidarSensor->getYawPitchAngle(i, j);
                yp.Add(FMath::DegreesToRadians(yawpitch.first));
                yp.Add(FMath::DegreesToRadians(yawpitch.second));
            }
        }
    }
    cudalidar.set_yawpitch(yp);
    cudalidar.set_rn_hn(rn * rtn, hw);
    check(depthCamIdx.Num() == camuvIdx.Num());
    check(depthCamIdx.Num() * 2 == yp.Num());
    imgBuffers_gpu.SetNumZeroed(nImage);

    return true;
}

TSharedPtr<LidarBuffer> ALidarBufferDepth::GetTBuffer(const FTLidarMeasurement& measure)
{
    // 设置天气
    if (lidarMd)
    {
        cudalidar.set_intensity(lidarMd->get_intensity());
        cudalidar.set_rain(lidarMd->get_rain());
        cudalidar.set_snow(FMath::Max(lidarMd->get_snow(), lidarMd->get_rain() * 0.1f));
        cudalidar.set_fog(lidarMd->get_fog());
    }
    // 获取深度数据
    TSharedPtr<DepthLidarBuffer> buffer = MakeShared<DepthLidarBuffer>();
    buffer->imgBuffer.SetNum(nImage);
#ifdef DEP_ROTATE

    int picn = std::min(FMath::CeilToInt((measure.HorizontalPointsToScan + offsetX1 - offsetX0) / Hfov), nImage);
    for (int i = 0; i < picn; i++)
    {
        if (!depthCameraActors[ii]->GetCaptureImage(imgBuffer[ii].gpuImg))
        {
            depthCameraActors[ii]->GetCaptureImage(imgBuffer[ii].cpuImg);
        }
    }
    for (int i = 0; i < nImage; i++)
    {
        FRotator rot(0);
        rot.Yaw = FMath::Fmod(lidarSensor->getHorizontalScanAngle(measure.HorizontalPos + measure.HorizontalToScan) +
                                  offsetX0 + i * Hfov + Hfov * 0.5f + 360.f,
            360.f);
        depthCameraActors[i]->SetActorRelativeRotation(rot);
    }
#else
    // 按照当前旋转角度，覆盖那些周深，按需获取
    int i0 = FMath::FloorToInt((lidarSensor->getHorizontalScanAngle(measure.HorizontalPos) + offsetX0 + 360.f) / Hfov);
    int i1 = FMath::FloorToInt(
        (lidarSensor->getHorizontalScanAngle(measure.HorizontalPos + measure.HorizontalToScan) + offsetX1 + 360.f) /
        Hfov);
    bool hascpu = false;
    for (int i = i0; i <= i1; i++)
    {
        int ii = i % nImage;
        if (buffer->imgBuffer[ii].cpuImg.Num() > 0 || buffer->imgBuffer[ii].gpuImg)
            continue;
        if (depthCameraActors[ii])
        {
            if (!depthCameraActors[ii]->GetCaptureImage(buffer->imgBuffer[ii].gpuImg))
            {
                depthCameraActors[ii]->GetCaptureImage(buffer->imgBuffer[ii].cpuImg);
                hascpu = true;
            }
        }
        // test
        /*
        IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked< IImageWrapperModule
        >("ImageWrapper"); TSharedPtr<IImageWrapper> ImageWrapper =
        ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG); UGameInstance* GI =
        actor->GetWorld()->GetGameInstance(); if (GI)
        {
            UDisplayGameInstance*    DGI = Cast<UDisplayGameInstance>(GI);

            FString savePath = TEXT("e:/test/display/");
            if (BitMap[ii].Num() && ImageWrapper->SetRaw(BitMap[ii].GetData(), BitMap[ii].Num() * sizeof(FColor),
                depthCameraActors[ii]->renderTarget2D->SizeX, depthCameraActors[ii]->renderTarget2D->SizeY,
        ERGBFormat::BGRA, 8))
            {
                if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*savePath))
                {
                    static int aaa = 0;
                    FString SaveDir = savePath + FString::FromInt(aaa++) + TEXT(".") + FString::FromInt(ii) + TEXT(".")
        + TEXT("jpg");//FString(GETENUMSTRING("EImageFormat", imageFormat)).ToLower();
                    //UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent INFO: SaveDir is  %s"), *SaveDir);

                    DGI->GetSaveDataHandle()->SaveJPG(ImageWrapper->GetCompressed(80), SaveDir);

                }
            }
        }

        //*/
    }
    // cuda失败，使用ue自带函数获取
    if (hascpu)
    {
        for (int i = 0; i < nImage; i++)
        {
            if (buffer->imgBuffer[i].gpuImg)
            {
                if (depthCameraActors[i])
                {
                    depthCameraActors[i]->GetCaptureImage(buffer->imgBuffer[i].cpuImg);
                }
            }
        }
    }
#endif    // DEP_ROTATE

    return buffer;
}

bool ALidarBufferDepth::LoadStencilMap(const FString& dir)
{
    // 导入材质tag定义
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

void ALidarBufferDepth::getData(
    const TArray<FColor>& BitMap, float w, float h, float _x, float _y, float& distance, float& norangle, int& tag)
{
    // 获取当前深度值
    distance = -1;
    norangle = 0;
    tag = 0;

    float x = FMath::Min(w - 1.0001f, FMath::Max(0.0001f, _x));
    float y = FMath::Min(h - 1.0001f, FMath::Max(0.0001f, _y));
    // 线性插值
#ifdef LINEARINTER

    int x0 = FMath::FloorToInt(x);
    int y0 = FMath::FloorToInt(y);
    int x1 = FMath::CeilToInt(x);
    int y1 = FMath::CeilToInt(y);

    const auto& color00 = BitMap[y0 * w + x0];
    const auto& color01 = BitMap[y1 * w + x0];
    const auto& color10 = BitMap[y0 * w + x1];
    const auto& color11 = BitMap[y1 * w + x1];

    float sd = 0;
    float sni = 0;
    float sp = 0;
    std::map<int, int> st;

    float d = ((float) color00.R * 256.f + (float) color00.G) * 0.005f;
    float ni = (float) color00.B * 0.00390625f;
    int t = color00.A;
    if (d > 1e-6f && d < 327.f)
    {
        float p = (x1 - x) * (y1 - y);
        sp += p;
        sd += d * p;
        sni += ni * p;
        st[t]++;
    }
    d = ((float) color01.R * 256.f + (float) color01.G) * 0.005f;
    ni = (float) color01.B * 0.00390625f;
    int t = color01.A;
    if (d > 1e-6f && d < 327.f)
    {
        float p = (x1 - x) * (y - y0);
        sp += p;
        sd += d * p;
        sni += ni * p;
        st[t]++;
    }
    d = ((float) color10.R * 256.f + (float) color10.G) * 0.005f;
    ni = (float) color10.B * 0.00390625f;
    int t = color10.A;
    if (d > 1e-6f && d < 327.f)
    {
        float p = (x - x0) * (y1 - y);
        sp += p;
        sd += d * p;
        sni += ni * p;
        st[t]++;
    }
    d = ((float) color11.R * 256.f + (float) color11.G) * 0.005f;
    ni = (float) color11.B * 0.00390625f;
    int t = color11.A;
    if (d > 1e-6f && d < 327.f)
    {
        float p = (x - x0) * (y - y0);
        sp += p;
        sd += d * p;
        sni += ni * p;
        st[t]++;
    }
    if (sp > 0)
    {
        distance = sd / sp;
        norangle = sni / sp;
        tag = std::max_element(st.begin(), st.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            })->first;
    }

#else
    // 最近邻
    int x0 = FMath::RoundToInt(x);
    int y0 = FMath::RoundToInt(y);
    const auto& color00 = BitMap[y0 * w + x0];
    float d = ((float) color00.R * 256.f + (float) color00.G) * 0.005f;
    if (d > 1e-6f && d < 327.f)
    {
        distance = d;
        norangle = (float) color00.B * 0.00390625f;
        tag = color00.A;
    }

#endif    // 0
}

// 获取激光点云
bool ALidarBufferDepth::GetPoints(
    const LidarBuffer* rawbuf, const FTLidarMeasurement& measure, lidar::TraditionalLidar::lidar_ptset& lidarBuffer)
{
    const DepthLidarBuffer* curBuffer = StaticCast<const DepthLidarBuffer*>(rawbuf);

    uint32_t rn = lidarSensor->getRaysNum();
    uint32_t rtn = lidarSensor->getReturnNum();
    uint32 hw = lidarSensor->getHorizontalScanCount();
    lidarBuffer.channels.resize(measure.HorizontalToScan);
    lidarBuffer.points.resize(hw * rn * rtn);
    uint32 i0 = measure.HorizontalPos;
    double umpsec = 1000.0 * 1000. / (lidarSensor->getRotationFrequency() * lidarSensor->getHorizontalScanCount());
    auto utime = measure.TimeStamp0 * 1000;

    // 获取深度数据
    bool has_cpu = false;
    bool cuda_img_changed = false;
    for (int i = 0; i < nImage; i++)
    {
        if (curBuffer->imgBuffer[i].gpuImg)
        {
            if (imgBuffers_gpu[i] != curBuffer->imgBuffer[i].gpuImg)
            {
                cuda_img_changed = true;
            }
            imgBuffers_gpu[i] = curBuffer->imgBuffer[i].gpuImg;
        }
        if (curBuffer->imgBuffer[i].cpuImg.Num() > 0)
        {
            has_cpu = true;
        }
    }
    if (!has_cpu)
    {
        // 优先使用cuda计算
        if (cuda_img_changed)
        {
            cudalidar.set_img(imgBuffers_gpu);
        }
        if (!cudalidar.simulation((i0 % hw) * rn * rtn, hw * rn * rtn, lidarBuffer.points.data()))
        {
            lidarBuffer.points.clear();
            UE_LOG(LogTemp, Warning, TEXT("lidar sim faild"));
            return false;
        }
        else
        {
            for (uint32 i = 0; i < measure.HorizontalToScan; ++i)
            {
                uint32 ii = (i + i0) % hw;
                auto& dd = lidarBuffer.channels.at(i);
                dd.hor_pos = (i0 + i) % hw;
                dd.utime = utime + umpsec * i;
                dd.pn = rn * rtn;
                dd.points = &lidarBuffer.points[i * rn * rtn];
            }
        }
    }
    else
    {
        // cpu计算
        uint32 ltt = 16;
        // 处理线程
        auto worker = [&](uint32 bg, uint32 ed)
        {
            for (uint32 i = bg; i < ed; ++i)
            {
                if (i >= measure.HorizontalToScan)
                {
                    break;
                }
                uint32 ii = (i + i0) % hw;
                auto& dd = lidarBuffer.channels.at(i);
                dd.hor_pos = ii;
                dd.utime = utime + umpsec * i;
                dd.pn = rn * rtn;
                dd.points = &lidarBuffer.points[i * rn * rtn];
                for (uint32 c = 0; c < rn; c++)
                {
                    uint32 ic = ii * rn * rtn + c;
                    int pici = depthCamIdx[ic];
                    if (curBuffer->imgBuffer[pici].cpuImg.Num() == 0)
                    {
                        continue;
                    }
                    auto xy = camuvIdx[ic];
                    const auto& color00 = curBuffer->imgBuffer[pici].cpuImg[xy];
                    float distance = ((float) color00.R * 256.f + (float) color00.G) * 0.005f;
                    // 计算距离
                    if (distance > 1e-6f && distance < 327.f)
                    {
                        auto& pt = dd.points[c];
                        pt.distance = distance;
                        pt.norinter = (float) color00.B * 0.00390625f;
                        int tag = color00.A;
                        if (stencilMap.find(tag) == stencilMap.end())
                            tag = 0;
                        pt.tag_c = stencilMap[tag].first;
                        pt.tag_t = stencilMap[tag].second;
                        // LIDAR 模型
                        if (lidarMd)
                            lidarMd->simulator(pt.norinter, pt.tag_c, pt.tag_t, pt.distance, pt.instensity);
                        if (pt.distance > 0.01f && pt.distance < 327.f)
                        {
                            auto yawpitch = lidarSensor->getYawPitchAngle(dd.hor_pos, c);
                            FRotator LaserRot(
                                yawpitch.second, yawpitch.first, 0);    // float InPitch, float InYaw, float InRoll
                            // 计算3d坐标
                            auto pv = rtMatrix.TransformPosition(
                                pt.distance * UKismetMathLibrary::GetForwardVector(LaserRot));
                            pt.x = pv.X;
                            pt.y = pv.Y;
                            pt.z = pv.Z;
                        }
                    }
                }

                for (uint32 t = 1; t < rtn; ++t)
                {
                    memcpy(&dd.points[rn * t], dd.points, sizeof(lidar::TraditionalLidar::lidar_point) * rn);
                }
            }
        };

        // 多线程处理
        std::vector<std::thread> thrs;
        {
            for (uint32 i = 0; i < (uint32) ltt; i++)
            {
                thrs.push_back(
                    std::thread(worker, measure.HorizontalToScan * i / ltt, measure.HorizontalToScan * (i + 1) / ltt));
                // thrs.push_back(std::move(t1));
            }
        }
        for (std::thread& th : thrs)
        {
            // If thread Object is Joinable then Join that thread.
            if (th.joinable())
                th.join();
        }
    }

    return true;
}

void ALidarBufferDepth::setRotationTranslation(const FTransform& mat)
{
    cudalidar.set_rtmat(mat);
    LidarBufferFun::setRotationTranslation(mat);
}

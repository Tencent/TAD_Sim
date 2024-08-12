// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @file CameraSensor.cpp
 * @brief This file contains the implementation of the CameraSensor class.
 * 普通相机的仿真
 * @author <kekesong>
 * @date 2022-09-10
 * @copyright Copyright (c) 2017, tencent. All rights reserved.
 */

#include "CameraSensor.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "CineCameraComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetSystemLibrary.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "HighResScreenshot.h"
#include "HAL/PlatformFilemanager.h"
#include "Framework/SaveDataThread.h"
#include "Framework/DisplayGameInstance.h"
#include "Engine/World.h"
#include "Managers/EnvManager.h"
#include "Framework/DisplayPlayerController.h"
#include "Objects/Transports/TransportPawn.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TexJpeg.h"
#include <ctime>
#include "SimMsg/sensor_raw.pb.h"
#include "HadMap/Public/HadmapManager.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

ACameraSensor::ACameraSensor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    str_PostProcess = TEXT("Material'/Game/SensorSim/Camera/Material/Mat_Camera_Capture.Mat_Camera_Capture'");

    RootComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));

    previewComponent = CreateDefaultSubobject<UCineCameraComponent>(FName(TEXT("PreviewCamera")));
    previewComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    previewComponent->SetUseFieldOfViewForLOD(true);
    previewComponent->SetActive(false);
    captureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName(TEXT("CaptureCamera")));
    captureComponent->SetActive(false);
}

ACameraSensor::~ACameraSensor()
{
}

bool ACameraSensor::Init(const FSensorConfig& _Config)
{
    ASensorActor::Init(_Config);
    // FSensorConfig* NewConfigPtr = &_Config;
    // FCameraConfig* NewCameraSensorConfig = static_cast<FCameraConfig*>(NewConfigPtr);
    const FCameraConfig* NewCameraSensorConfig = Cast_Sim<const FCameraConfig>(_Config);

    sensorConfig = *NewCameraSensorConfig;
    if (!NewCameraSensorConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Cant Cast to FCameraConfig!"));
        return false;
    }

    // ID
    id = NewCameraSensorConfig->id;
    // frequency
    frequency = NewCameraSensorConfig->frequency;
    // Save path
    savePath = NewCameraSensorConfig->savePath;
    if (!savePath.IsEmpty() && !FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*savePath))
    {
        // UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Generate savePath."));
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*savePath);
    }
    sensorConfig = *NewCameraSensorConfig;
    imageRes0.X = NewCameraSensorConfig->res_Horizontal;
    imageRes0.Y = NewCameraSensorConfig->res_Vertical;
    imageRes = imageRes0;

    {
        // 加载材质
        UMaterial* NewMat = LoadObject<UMaterial>(NULL, *str_PostProcess);
        if (!NewMat)
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Cant get Material!"));
            return false;
        }
        // 创建材质实例动态
        mid_CameraPostProcess = UMaterialInstanceDynamic::Create(NewMat, this);
        if (!mid_CameraPostProcess)
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Cant get MaterialInstanceDynamic!"));
            return false;
        }
    }

    double NewFov_H = 0;
    double NewFov_V = 0;
    double fx = -1;
    double fy = -1;
    double cx = 0.5;
    double cy = 0.5;
    double skew = 0;
    double k1 = 0, k2 = 0, k3 = 0, p1 = 0, p2 = 0;
    double fov_scale = 1.0f;
    double maxxfov = 0, maxyfov = 0;
    double ofx = 0, ofy = 0;
    // 检查传感器配置是否包含内参矩阵
    if (sensorConfig.paraType == EParamTypeEnum::PT_Int && sensorConfig.intrinsic_Matrix.Num() == 9)
    {
        // 获取内参矩阵的值
        const auto& im = sensorConfig.intrinsic_Matrix;
        fx = im[0];
        fy = im[4];
        cx = im[2];
        cy = im[5];
        skew = im[1];
        if (NewCameraSensorConfig->distortion_Parameters.Num() > 0)
            k1 = NewCameraSensorConfig->distortion_Parameters[0];
        if (NewCameraSensorConfig->distortion_Parameters.Num() > 1)
            k2 = NewCameraSensorConfig->distortion_Parameters[1];
        if (NewCameraSensorConfig->distortion_Parameters.Num() > 2)
            k3 = NewCameraSensorConfig->distortion_Parameters[2];
        if (NewCameraSensorConfig->distortion_Parameters.Num() > 3)
            p1 = NewCameraSensorConfig->distortion_Parameters[3];
        if (NewCameraSensorConfig->distortion_Parameters.Num() > 4)
            p2 = NewCameraSensorConfig->distortion_Parameters[4];
    }
    // 检查传感器配置是否包含有效的水平和垂直FOV值
    else if (sensorConfig.paraType == EParamTypeEnum::PT_Fov && sensorConfig.fov_Horizontal > 0 &&
             sensorConfig.fov_Vertical > 0)
    {
        // 根据fov配置计算内参矩阵
        cx = 0.5 * sensorConfig.res_Horizontal;
        cy = 0.5 * sensorConfig.res_Vertical;
        fx = 0.5 * sensorConfig.res_Horizontal / FMath::Tan(FMath::DegreesToRadians(sensorConfig.fov_Horizontal) * 0.5);
        fy = 0.5 * sensorConfig.res_Vertical / FMath::Tan(FMath::DegreesToRadians(sensorConfig.fov_Vertical) * 0.5);
    }
    // 检查传感器配置是否包含有效的CCD值
    else if (sensorConfig.paraType == EParamTypeEnum::PT_Ccd && sensorConfig.ccd_Height > 0 &&
             sensorConfig.ccd_Width > 0)
    {
        // 根据ccd配置计算内参矩阵
        cx = 0.5 * sensorConfig.res_Horizontal;
        cy = 0.5 * sensorConfig.res_Vertical;
        fx = sensorConfig.res_Horizontal * sensorConfig.focal_Length / sensorConfig.ccd_Width;
        fx = sensorConfig.res_Vertical * sensorConfig.focal_Length / sensorConfig.ccd_Height;
    }
    if (fx <= 0 || fy <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraSensor: fx fy errro!"));
        return false;
    }
    // 计算了相机传感器的最大水平和垂直视野角度
    {
        TArray<FVector2D> bounds;
        for (int i = 0; i < 11; i++)
        {
            double y = sensorConfig.res_Vertical * i * 0.1;
            for (int j = 0; j < 11; j++)
            {
                // Calculate bounding box points and add them to an array
                double x = sensorConfig.res_Horizontal * j * 0.1;
                bounds.Add(FVector2D(x, y));
            }
        }
        for (const auto& bd : bounds)
        {
            double tX = bd.X;
            double tY = bd.Y;
            double y = (tY - cy) / fy;
            double x = (tX - cx - skew * y) / fx;
            double x0 = x;
            double y0 = y;
            for (int j = 0; j < 10; j++)
            {
                double r2 = x * x + y * y;

                double distRadialA = 1.f / (1.f + k1 * r2 + k2 * r2 * r2 + k3 * r2 * r2 * r2);

                double deltaX = 2.f * p1 * x * y + p2 * (r2 + 2.f * x * x);
                double deltaY = p1 * (r2 + 2.f * y * y) + 2.f * p2 * x * y;

                x = (x0 - deltaX) * distRadialA;
                y = (y0 - deltaY) * distRadialA;
            }

            maxxfov = FMath::Max(maxxfov, (double) FMath::Abs(FMath::Atan(x)));
            maxyfov = FMath::Max(maxyfov, (double) FMath::Abs(FMath::Atan(y)));
        }
        // Calculate new camera's field-of-view based on maximum horizontal and vertical angles
        ofx = 0.5 * NewCameraSensorConfig->res_Horizontal / FMath::Tan(maxxfov);
        ofy = 0.5 * NewCameraSensorConfig->res_Vertical / FMath::Tan(maxyfov);
        double of = fmin(ofx, ofy);
        ofx = of;
        ofy = of;
        maxxfov = FMath::Atan(0.5 * NewCameraSensorConfig->res_Horizontal / of);
        maxyfov = FMath::Atan(0.5 * NewCameraSensorConfig->res_Vertical / of);
        // Update the FOV parameters in the Camera Sensor Config object
        // NewCameraSensorConfig->fov_Horizontal = FMath::RadiansToDegrees(maxxfov) * 2;
        // NewCameraSensorConfig->fov_Vertical = FMath::RadiansToDegrees(maxyfov) * 2;
        NewFov_H = FMath::RadiansToDegrees(maxxfov) * 2;
        NewFov_V = FMath::RadiansToDegrees(maxyfov) * 2;
        // Calculate the scale factor used to adjust the FOV when using a smaller image size
        fov_scale = FMath::Max(fx / ofx, fy / ofy);
    }

    {
        /*
        const int max_refnum = 1024;
        texDistortion = UTexture2D::CreateTransient(max_refnum, 1, PF_A32B32G32R32F);
        check(texDistortion);
        auto mipmap = texDistortion->PlatformData->Mips[0];
        void* data = mipmap.BulkData.Lock(LOCK_READ_WRITE);
        TArray<FColor> texRefdata;
        for (int i = 0; i < max_refnum; i++)
        {
            texRefdata[i].R = 0;
            texRefdata[i].G = 0;
            texRefdata[i].B = 0;
            texRefdata[i].A = 0;

        }
        FMemory::Memcpy(data, texRefdata.GetData(), max_refnum * 16);
        */
    }
    // 设置pp处理参数
    UE_LOG(LogTemp, Log,
        TEXT("CameraSensor: cx=%f, cy=%f, fx=%f, fy=%f, skew=%f, k1=%f, k2=%f, k3=%f, p1=%f, p2=%f, ofx=%f, ofy=%f, "
             "w=%f, h=%f"),
        cx, cy, fx, fy, skew, k1, k2, k3, p1, p2, ofx, ofy, NewCameraSensorConfig->res_Horizontal,
        NewCameraSensorConfig->res_Vertical);
    // 内参
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("cx")), cx);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("cy")), cy);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("fx")), fx);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("fy")), fy);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("skew")), skew);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("k1")), k1);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("k2")), k2);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("k3")), k3);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("p1")), p1);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("p2")), p2);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("ofx")), ofx);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("ofy")), ofy);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("w")), NewCameraSensorConfig->res_Horizontal);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("h")), NewCameraSensorConfig->res_Vertical);
    // Blur
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("BlurIntensity")), NewCameraSensorConfig->blur_Intensity);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("Res_X")), NewCameraSensorConfig->res_Horizontal);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("Res_Y")), NewCameraSensorConfig->res_Vertical);

    // Gamma

    float flen = 100.f;
    // Set the current focus length of the camera
    previewComponent->CurrentFocalLength = flen;
    // Update the minimum and maximum focal lengths for this component's lens settings
    previewComponent->LensSettings.MinFocalLength = flen;
    previewComponent->LensSettings.MaxFocalLength = flen;
    // Set the minimum and maximum aperture values for this component's lens settings
    previewComponent->LensSettings.MinFStop = 7.0f;
    previewComponent->LensSettings.MaxFStop = 7.0f;
    // Calculate the sensor dimensions based on the new field-of-view (FOV), focallength, and aspect ratio
    previewComponent->Filmback.SensorWidth = FMath::Tan(FMath::DegreesToRadians(NewFov_H) / 2) * 2 * flen;
    previewComponent->Filmback.SensorHeight = FMath::Tan(FMath::DegreesToRadians(NewFov_V) / 2) * 2 * flen;
    // Set the post process settings to match the newly calculated FOV
    SetPostProcessSettings(
        *NewCameraSensorConfig, previewComponent->PostProcessSettings, FMath::Max(1.0, fov_scale * 0.9));
    // Add the mid-camera postprocess effect with an opacity value of 1.0 to the list of blendables in the post process
    // settings
    previewComponent->PostProcessSettings.AddBlendable(mid_CameraPostProcess, 1);

    if (GetDisplayInstance() && GetDisplayInstance()->nHILpos.X < 0)
    {
        bool memshared = false, gpushared = false, style = false;
        // 获取配置文件中"Sensor"部分的配置项
        GConfig->GetBool(TEXT("Sensor"), TEXT("CameraMemShare"), memshared, GGameIni);
        GConfig->GetBool(TEXT("Sensor"), TEXT("CameraGPUShare"), gpushared, GGameIni);
        GConfig->GetBool(TEXT("Sensor"), TEXT("CameraStyle"), style, GGameIni);
        GConfig->GetBool(TEXT("Sensor"), TEXT("PublicMsg"), public_msg, GGameIni);
        // 从配置文件获取图像质量设置
        GConfig->GetInt(TEXT("Sensor"), TEXT("JpegQuality"), imageQuality, GGameIni);

        FString gpuid = TEXT("0");
        if (!FParse::Value(FCommandLine::Get(), TEXT("-graphicsadapter="), gpuid))
        {
            gpuid = TEXT("0");
        }

        UE_LOG(LogTemp, Log, TEXT("CameraSensor: use gpu id: %s"), *gpuid);
        texJpg = MakeShared<UTexJpeg>(FCString::Atoi(*gpuid));

        FString stylemode;
        if (style && imageName == TEXT("Camera"))
        {
            FString modelpath = FPaths::ConvertRelativePathToFull(FPaths::ProjectUserDir() + TEXT("NeuralStyle/"));
            AEnvManager* envManager = nullptr;
            if (GetWorld()->GetGameState<ADisplayGameStateBase>())
            {
                envManager = GetWorld()->GetGameState<ADisplayGameStateBase>()->syncSystem->envManager;
            }

            FString configpath = modelpath + TEXT("config.json");
            if (FPaths::FileExists(configpath) && envManager)
            {
                FString jbuf;
                if (FFileHelper::LoadFileToString(jbuf, *configpath))
                {
                    // 获取风格迁移模型配置
                    TSharedPtr<FJsonObject> json;
                    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(jbuf);
                    if (FJsonSerializer::Deserialize(Reader, json))
                    {
                        FSimEnvOriginInfo weather;
                        envManager->GetCurrentEnvData().GetEnvOriginInfo(weather);
                        auto date = FDateTime::FromUnixTimestamp(weather.Unix_timestamp / 1000);
                        auto Model = json->GetArrayField(TEXT("Model"));
                        FString modelname;
                        double maxwt = 99999999999.999;
                        int modelw = 0, modelh = 0;
                        int safNum = 0;
                        // 遍历模型参数，计算合适的模型
                        for (const auto& model : Model)
                        {
                            auto jmodel = model->AsObject();
                            FString name;
                            double wt = 0;
                            int mw = 0, mh = 0;
                            bool satisfy = false;
                            int satcc = 0;
                            // 遍历参数，打分统计
                            for (const auto& key : jmodel->Values)
                            {
                                if (key.Key == "name")
                                {
                                    name = key.Value->AsString();
                                }
                                else if (key.Key == "width")
                                {
                                    mw = key.Value->AsNumber();
                                }
                                else if (key.Key == "height")
                                {
                                    mh = key.Value->AsNumber();
                                }
                                else if (key.Key == "Sun" || key.Key == "Rain" || key.Key == "Snow" ||
                                         key.Key == "Visibility" || key.Key == "Cloud")
                                {
                                    // 天气相关参数评分
                                    if (key.Value->AsObject())
                                    {
                                        double r = 0;
                                        /*if (key.Key == "Sun")
                                        {
                                            r = weather.sunStrength / 200000.0;
                                        }
                                        else */
                                        if (key.Key == "Rain")
                                        {
                                            r = weather.RainFall / 255.0;
                                        }
                                        else if (key.Key == "Snow")
                                        {
                                            r = weather.SnowFall / 10.0;
                                        }
                                        else if (key.Key == "Visibility")
                                        {
                                            r = weather.Visibility / 30000.0;
                                        }
                                        else if (key.Key == "Cloud")
                                        {
                                            r = weather.CloudyDensity / 11.1;
                                        }
                                        double rmin = key.Value->AsObject()->GetNumberField("min");
                                        double rmax = key.Value->AsObject()->GetNumberField("max");
                                        double rbt = key.Value->AsObject()->GetNumberField("better");
                                        if (r < rmin || r > rmax)
                                        {
                                            satisfy = false;
                                            UE_LOG(LogTemp, Log, TEXT("CameraStyle: %s not satisfy in %s"), *key.Key,
                                                *name);
                                            break;
                                        }
                                        satisfy = true;
                                        wt += FMath::Abs(r - rbt) * 2;
                                        satcc++;
                                    }
                                }
                                else if (key.Key == "Fov")
                                {
                                    // fov分数
                                    if (key.Value->AsObject())
                                    {
                                        double r = NewFov_H;
                                        double rmin = key.Value->AsObject()->GetNumberField("min");
                                        double rmax = key.Value->AsObject()->GetNumberField("max");
                                        double rbt = key.Value->AsObject()->GetNumberField("better");
                                        if (r < rmin || r > rmax)
                                        {
                                            satisfy = false;
                                            UE_LOG(LogTemp, Log, TEXT("CameraStyle: %s not satisfy in %s"), *key.Key,
                                                *name);
                                            break;
                                        }
                                        satisfy = true;
                                        wt += FMath::Abs(r - rbt) * 2;
                                        satcc++;
                                    }
                                }
                                else if (key.Key == "Month")
                                {
                                    // 月分分数
                                    if (key.Value->AsObject())
                                    {
                                        int rmin = key.Value->AsObject()->GetIntegerField("min");
                                        int rmax = key.Value->AsObject()->GetIntegerField("max");
                                        int rbt = key.Value->AsObject()->GetIntegerField("better");
                                        int r = date.GetMonth();
                                        if (rmax < rmin)
                                        {
                                            rmax += 12;
                                        }
                                        if (r < rmin)
                                        {
                                            r += 12;
                                        }
                                        if (rbt < rmin)
                                        {
                                            rbt += 12;
                                        }
                                        if (r < rmin || r > rmax)
                                        {
                                            satisfy = false;
                                            UE_LOG(LogTemp, Log, TEXT("CameraStyle: %s not satisfy in %s"), *key.Key,
                                                *name);
                                            break;
                                        }
                                        satisfy = true;
                                        wt += (double) FMath::Abs(r - rbt) / 6.0;
                                        satcc++;
                                    }
                                }
                                else if (key.Key == "Hours")
                                {
                                    // 时间分数
                                    if (key.Value->AsObject())
                                    {
                                        int rmin = key.Value->AsObject()->GetIntegerField("min");
                                        int rmax = key.Value->AsObject()->GetIntegerField("max");
                                        int rbt = key.Value->AsObject()->GetIntegerField("better");
                                        int r = date.GetHour();
                                        if (rmax < rmin)
                                        {
                                            rmax += 24;
                                        }
                                        if (r < rmin)
                                        {
                                            r += 24;
                                        }
                                        if (rbt < rmin)
                                        {
                                            rbt += 24;
                                        }
                                        if (r < rmin || r > rmax)
                                        {
                                            satisfy = false;
                                            UE_LOG(LogTemp, Log, TEXT("CameraStyle: %s not satisfy in %s"), *key.Key,
                                                *name);
                                            break;
                                        }
                                        satisfy = true;
                                        wt += (double) FMath::Abs(r - rbt) / 12.0;
                                        satcc++;
                                    }
                                }
                                else if (key.Key == "LOC")
                                {
                                    // 安装位置评分
                                    if (key.Value->AsObject() && key.Value->AsObject()->GetArrayField("loc").Num() == 3)
                                    {
                                        double x = key.Value->AsObject()->GetArrayField("loc")[0]->AsNumber();
                                        double y = -key.Value->AsObject()->GetArrayField("loc")[1]->AsNumber();
                                        double z = key.Value->AsObject()->GetArrayField("loc")[2]->AsNumber();
                                        double dis = key.Value->AsObject()->GetNumberField("dis");
                                        double d = FVector::Distance(sensorConfig.installLocation, FVector(x, y, z));
                                        if (d > dis)
                                        {
                                            satisfy = false;
                                            UE_LOG(LogTemp, Log, TEXT("CameraStyle: %s not satisfy in %s"), *key.Key,
                                                *name);
                                            break;
                                        }
                                        satisfy = true;
                                        wt += d / dis;
                                        satcc++;
                                    }
                                }
                                else if (key.Key == "RPY")
                                {
                                    // 安装姿态评分
                                    if (key.Value->AsObject() && key.Value->AsObject()->GetArrayField("rpy").Num() == 3)
                                    {
                                        double r = key.Value->AsObject()->GetArrayField("rpy")[0]->AsNumber();
                                        double p = -key.Value->AsObject()->GetArrayField("rpy")[1]->AsNumber();
                                        double y = key.Value->AsObject()->GetArrayField("rpy")[2]->AsNumber();
                                        double cmin = key.Value->AsObject()->GetNumberField("dot");
                                        FVector n1 = sensorConfig.installRotation.RotateVector(FVector(1, 0, 0));
                                        FVector n2 = FRotator(-p, -y, r).RotateVector(FVector(1, 0, 0));
                                        double d = FVector::DotProduct(n1, n2);
                                        if (d < cmin)
                                        {
                                            satisfy = false;
                                            UE_LOG(LogTemp, Log, TEXT("CameraStyle: %s not satisfy in %s"), *key.Key,
                                                *name);
                                            break;
                                        }
                                        satisfy = true;
                                        wt += (1.0 - d) * 0.5;
                                        satcc++;
                                    }
                                }
                            }
                            if (!satisfy)
                            {
                                continue;
                            }
                            if (name.Len() == 0)
                            {
                                continue;
                            }
                            // 模型分数pk
                            if (satcc > safNum ||
                                (satcc == safNum &&
                                    (wt < maxwt ||
                                        (wt == maxwt && FMath::Abs(imageRes0.X * imageRes0.Y - mw * mh) <
                                                            FMath::Abs(imageRes0.X * imageRes0.Y - modelw * modelh)))))
                            {
                                safNum = satcc;
                                maxwt = wt;
                                modelname = name;
                                modelw = mw;
                                modelh = mh;
                            }
                        }
                        // 分数最高的模型
                        if (modelname.Len() > 0)
                        {
                            modelpath += modelname;
                            stylemode = modelpath;
                            imageRes.X = modelw;
                            imageRes.Y = modelh;
                            UE_LOG(LogTemp, Log, TEXT("[CameraSensor] loaded style model: %s"), *modelpath);
                            UE_LOG(LogTemp, Log, TEXT("[CameraSensor] model w=%d, h=%d"), modelw, modelh);
                        }
                    }
                    else
                    {
                        UE_LOG(LogSimGameMode, Warning, TEXT("Can`t read json: %s"), *Reader->GetErrorMessage());
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("CameraSensor: %s may not exists"), *configpath);
                if (!envManager)
                    UE_LOG(LogTemp, Log, TEXT("CameraSensor: envpb is not exists"));
            }
        }
        // 创建一个新的纹理目标2D对象
        renderTarget2D = NewObject<UTextureRenderTarget2D>();
        // 设置共享标志为true，以便可以在GPU上共享
        renderTarget2D->bGPUSharedFlag = 1;
        // 设置渲染目标格式为RGBA8
        renderTarget2D->RenderTargetFormat = RTF_RGBA8;
        // 设置目标gamma
        renderTarget2D->TargetGamma = targetGamma;
        // 初始化自定义格式，设置图像大小和像素格式
        renderTarget2D->InitCustomFormat(imageRes.X, imageRes.Y, PF_B8G8R8A8, true);

        texJpg->texRT = renderTarget2D;
        if (!texJpg->InitResources(true, gpushared, imageRes0.X, imageRes0.Y, stylemode))
        {
            texJpg.Reset();
            imageRes = imageRes0;
            renderTarget2D->InitCustomFormat(imageRes.X, imageRes.Y, PF_B8G8R8A8, true);
            UE_LOG(LogTemp, Warning, TEXT("CameraSensor: texJpg init faild."));
        }
        // 将场景捕获组件附加到根组件上
        captureComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        // 设置场景捕获组件是否每帧进行捕获
        captureComponent->bCaptureEveryFrame = true;
        // 设置场景捕获组件自动激活
        captureComponent->bAutoActivate = true;
        // 设置场景捕获组件为活动状态
        captureComponent->SetActive(true);
        // 设置场景捕获组件的源类型为最终颜色（线性深度）
        captureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
        // 设置场景捕获组件的投影模式为透视
        captureComponent->ProjectionType = ECameraProjectionMode::Perspective;
        // 设置场景捕获组件的近裁剪平面和远裁剪平面
        captureComponent->OrthoWidth = 20000.f;
        // 使用自定义投影矩阵
        captureComponent->bUseCustomProjectionMatrix = true;
        captureComponent->ShowFlags.SetMotionBlur(true);
        captureComponent->ShowFlags.SetAmbientOcclusion(false);
        if ((int32) ERHIZBuffer::IsInverted)
        {
            captureComponent->CustomProjectionMatrix =
                FReversedZPerspectiveMatrix(FMath::DegreesToRadians(NewFov_H * 0.5f),
                    FMath::DegreesToRadians(NewFov_V * 0.5f), 1.f, 1.f, GNearClippingPlane, GNearClippingPlane);
        }
        else
        {
            captureComponent->CustomProjectionMatrix = FPerspectiveMatrix(FMath::DegreesToRadians(NewFov_H * 0.5f),
                FMath::DegreesToRadians(NewFov_V * 0.5f), 1.f, 1.f, GNearClippingPlane, GNearClippingPlane);
        }
        // 设置场景捕获组件的视角
        captureComponent->FOVAngle = fmaxf(NewFov_H, NewFov_V);
        // 设置场景捕获组件的目标纹理
        captureComponent->TextureTarget = renderTarget2D;
        // 设置后处理设置
        SetPostProcessSettings(
            *NewCameraSensorConfig, captureComponent->PostProcessSettings, FMath::Max(1.0, fov_scale * 0.9));
        captureComponent->PostProcessSettings.AddBlendable(mid_CameraPostProcess, 1);

        // 内存共享
        if (memshared)
        {
            UE_LOG(LogTemp, Log, TEXT("CameraSensor: memshare is on."));

            UE_LOG(LogTemp, Warning, TEXT("CameraSensor: shared file is: Tadsim_%s_%d"), *imageName, sensorConfig.id);
            sharedWriter = MakeShared<SharedMemoryWriter>();
            if (!sharedWriter->init(
                    std::string("Tadsim_") + TCHAR_TO_ANSI(*imageName) + "_" + std::to_string(sensorConfig.id), 100))
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraSensor: sharedWriter init faild."));
                sharedWriter.Reset();
            }
        }
        // GPU显存共享
        if (gpushared && texJpg)
        {
            UE_LOG(LogTemp, Log, TEXT("CameraSensor: gpu share is on."));
            UE_LOG(
                LogTemp, Warning, TEXT("CameraSensor: gpu shared file is: Tadsim_%s_%d"), *imageName, sensorConfig.id);
            // ipc
            sharedWriterGpu = MakeShared<SharedMemoryWriter>();
            if (!sharedWriterGpu->init(std::string("Tadsim_") + TCHAR_TO_ANSI(*imageName) + "_" +
                                       std::to_string(sensorConfig.id) + "_GPU"))
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraSensor: sharedWriterGpu init faild."));
                sharedWriterGpu.Reset();
            }

            std::vector<uint8> buf(1024);
            if (texJpg->IpcCreate(buf.data() + 8))
            {
                int w = imageRes0.X, h = imageRes0.Y;
                memcpy(buf.data(), &w, sizeof(int));
                memcpy(buf.data() + 4, &h, sizeof(int));
                std::time_t t = std::time(0);
                UE_LOG(LogTemp, Log, TEXT("CameraSensor: shared gpu verification code is %ld."), t);
                if (!sharedWriterGpu->write(buf, t))
                {
                    UE_LOG(LogTemp, Warning, TEXT("CameraSensor: sharedgpu write faild."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraSensor: share ipc faild."));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("CameraSensor: HIL MODEL."));
    }
    if (renderTarget2D)
        UE_LOG(LogTemp, Log, TEXT("CameraSensor: renderTarget2D is ok"));

    if (sharedWriter)
        UE_LOG(LogTemp, Log, TEXT("CameraSensor: sharedWriter is ok"));
    if (texJpg)
        UE_LOG(LogTemp, Log, TEXT("CameraSensor: texJpg is ok"));
    if (sharedWriterGpu)
        UE_LOG(LogTemp, Log, TEXT("CameraSensor: sharedWriterGpu is ok"));

    return true;
}

ISimActorInterface* ACameraSensor::Install(const FSensorConfig& _Config)
{
    ISimActorInterface* SimActor = Super::Install(_Config);

    /* Install previewCamera */
    if (SimActor)
    {
        // TODO: All simActors support install camera
        ATransportPawn* Transport = Cast<ATransportPawn>(SimActor);
        if (Transport)
        {
            if (Transport->InstallCamera(_Config.typeName + FString::FromInt(_Config.id), previewComponent))
            {
                // previewComponent->SetActive(true);
                auto hilpos = GetDisplayInstance()->nHILpos;
                if (hilpos.X >= 0 && hilpos.Y >= 0)
                {
                    Transport->SetDefaultCamera(_Config.typeName + FString::FromInt(_Config.id));

                    if (GEngine && GEngine->GameViewport)
                    {
                        if (GEngine->GameViewport->Viewport->IsFullscreen())
                        {
                            GEngine->GameViewport->HandleToggleFullscreenCommand();
                        }
                        GEngine->GameViewport->GetWindow()->MoveWindowTo(hilpos);
                        GEngine->GameViewport->HandleToggleFullscreenCommand();
                    }
                }
            }
            return Transport;
        }
    }
    return nullptr;
}

void ACameraSensor::Update(const FSensorInput& _Input, FSensorOutput& _Output)
{
    if (!renderTarget2D)    // sil
    {
        return;
    }
    const FCameraInput* CameraInput = Cast_Sim<const FCameraInput>(_Input);
    // 频率限制
    if (frequency > 0 && (CameraInput->timeStamp - timeStamp) < 999.9999999 / frequency)
    {
        // UE_LOG(LogTemp, Warning, TEXT("%s: Camera frequency async, has return, TimeStamp is: %f"), *this->GetName(),
        // CameraInput->timeStamp);
        return;
    }
    timeStamp = CameraInput->timeStamp;
    double timeStamp_ego = CameraInput->timeStamp_ego;
    if (sensorConfig.installSlot == TEXT("C1") && CameraInput->timeStamp_tail > 0.001)
    {
        timeStamp_ego = CameraInput->timeStamp_tail;
    }
    // CUDA拷贝
    bool cuda = false;
    if (texJpg)
    {
        cuda = texJpg->Copy2Cuda();
    }

    std::vector<uint8>& BitData = dataBuf.buffer;
    BitData.clear();

    // 获取图像数据
    auto getRawBuff = [&]()
    {
        if (!BitData.empty())
        {
            return;
        }
        if (cuda)
        {
            if (!texJpg->Raw(BitData))
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraSensor: read raw image buf faild in texjpg"));
            }
        }
        else
        {
            FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
            FTextureRenderTarget2DResource* RTResource = (FTextureRenderTarget2DResource*) renderTarget2D->Resource;
            if (RTResource)
            {
                TArray<FColor> BitMap;
                RTResource->ReadPixels(BitMap, ReadPixelFlags);
                BitData.resize(BitMap.Num() * 4);
                memcpy(BitData.data(), BitMap.GetData(), BitMap.Num() * 4);
            }
        }
        if (BitData.empty())
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensor: read image buf faild."));
        }
    };

    // 内存共享
    if (sharedWriter)
    {
        getRawBuff();

        if (BitData.empty() || !sharedWriter->write(BitData, timeStamp_ego))
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensor: sharedWriter write faild."));
        }
    }
    // 显存共享
    if (sharedWriterGpu && cuda)
    {
        std::vector<uint8> buf(1024);
        if (texJpg->IpcShare(buf.data() + 8))
        {
            int w = imageRes0.X, h = imageRes0.Y;
            memcpy(buf.data(), &w, sizeof(int));
            memcpy(buf.data() + 4, &h, sizeof(int));
            // UE_LOG(LogTemp, Log, TEXT("CameraSensor: shared gpu write %f."), timeStamp);
            if (!sharedWriterGpu->write(buf, timeStamp_ego))
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraSensor: sharedgpu write faild."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensor: share ipc faild."));
        }
    }
    // 落盘或发布
    TArray64<uint8_t> imgbuf;
    if (!savePath.IsEmpty() || public_msg)
    {
        // jpeg图像，优先cuda编码，否则用ue自带CPU编码
        if (imageFormat == EImageFormat::JPEG)
        {
            if (cuda)
            {
                if (!texJpg->JpegEncoding(imgbuf))
                {
                    UE_LOG(LogTemp, Warning, TEXT("CameraSensor: read jpeg image buf faild in texjpg"));
                }
            }

            if (imgbuf.Num() == 0)
            {
                getRawBuff();

                if (BitData.empty())
                {
                    UE_LOG(LogTemp, Warning, TEXT("CameraSensor: read image buf faild."));
                }
                else
                {
                    IImageWrapperModule& ImageWrapperModule =
                        FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
                    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
                    if (ImageWrapper->SetRaw(BitData.data(), sizeof(FColor) * imageRes0.X * imageRes0.Y, imageRes0.X,
                            imageRes0.Y, ERGBFormat::BGRA, 8))
                    {
                        imgbuf = ImageWrapper->GetCompressed(imageQuality);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("CameraSensor: encode jpeg faild."));
                    }
                }
            }
        }
        // png用自带的编码器
        else if (imageFormat == EImageFormat::PNG)
        {
            getRawBuff();
            if (BitData.empty())
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraSensor: read image buf faild."));
            }
            else
            {
                IImageWrapperModule& ImageWrapperModule =
                    FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
                TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(imageFormat);
                TArray<uint8_t> BitMap;
                BitMap.SetNum(dataBuf.buffer.size() / 4);
                for (int i = 0; i < BitMap.Num(); i++)
                {
                    BitMap[i] = dataBuf.buffer[i * 4 + 3];
                }
                if (ImageWrapper->SetRaw(BitMap.GetData(), sizeof(uint8_t) * imageRes.X * imageRes.Y, imageRes.X,
                        imageRes.Y, ERGBFormat::Gray, 8))
                {
                    imgbuf = ImageWrapper->GetCompressed();
                }
            }
        }
        // exr用自带的编码器
        else if (imageFormat == EImageFormat::EXR)
        {
            getRawBuff();
            if (BitData.empty())
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraSensor: read image buf faild."));
            }
            else
            {
                IImageWrapperModule& ImageWrapperModule =
                    FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
                TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(imageFormat);
                TArray<float> BitMap;
                BitMap.SetNum(dataBuf.buffer.size() / 4);
                for (int i = 0; i < BitMap.Num(); i++)
                {
                    BitMap[i] = (dataBuf.buffer[i * 4 + 2] * 65536.0 + dataBuf.buffer[i * 4 + 1] * 256.0 +
                                    dataBuf.buffer[i * 4]) *
                                0.0001;
                }
                if (ImageWrapper->SetRaw(BitMap.GetData(), sizeof(float) * imageRes.X * imageRes.Y, imageRes.X,
                        imageRes.Y, ERGBFormat::Gray, 32))
                {
                    imgbuf = ImageWrapper->GetCompressed();
                }
            }
        }
        else
        {
            getRawBuff();
        }

        // 落盘
        if (!savePath.IsEmpty())
        {
            if (imgbuf.Num() > 0)
            {
                FString SavePath =
                    savePath + FString::Printf(TEXT("%s_%d_%010d"), *imageName, id, (int64) timeStamp_ego);
                if (imageFormat == EImageFormat::JPEG)
                {
                    SavePath += TEXT(".jpg");
                }
                else if (imageFormat == EImageFormat::PNG)
                {
                    SavePath += TEXT(".png");
                }
                else if (imageFormat == EImageFormat::EXR)
                {
                    SavePath += TEXT(".exr");
                }
                GetDisplayInstance()->GetSaveDataHandle()->SaveJPG(imgbuf, SavePath);
            }
            else
            {
                Save();
            }
            // 保存POSE
            {
                FString savePathWithName = savePath +
                                           FString::Printf(TEXT("%s_%d_%010d"), *imageName, id, (int64) timeStamp_ego) +
                                           TEXT(".pose");
                std::stringstream ss;
                ss << std::setprecision(15);
                ss << "pose of sensor(lon lat atl roll pitch yaw), the enu reference coord(wgs84) and mat of "
                      "world->image\n";
                double X = 0, Y = 0, Z = 0;
                hadmapue4::HadmapManager::Get()->LocalToLonLat(GetActorLocation(), X, Y, Z);
                ss << X << " " << Y << " " << Z << " ";
                auto Rot = GetActorRotation();
                ss << Rot.Roll * PI / 180. << " " << -Rot.Pitch * PI / 180. << " " << -(Rot.Yaw + 90.f) * PI / 180.
                   << "\n";
                ss << hadmapue4::HadmapManager::Get()->mapOriginLon << " "
                   << hadmapue4::HadmapManager::Get()->mapOriginLat << " "
                   << hadmapue4::HadmapManager::Get()->mapOriginAlt << "\n";

                auto loc = GetActorLocation() * 0.01f;
                std::swap(loc.X, loc.Y);
                loc.X *= -1.f;
                loc.Y *= -1.f;
                Rot.Roll *= -1.f;
                Rot.Yaw = -(Rot.Yaw + 90.f);
                auto rot = Rot.Quaternion() * FRotator(0, -90, 0).Quaternion() * FRotator(0, 0, 90).Quaternion();
                FTransform tf;
                tf.SetLocation(loc);
                tf.SetRotation(rot);
                auto tfmat = tf.Inverse().ToMatrixNoScale();
                for (int i = 0; i < 4; i++)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        ss << tfmat.M[j][i] << " ";
                    }
                }
                GetDisplayInstance()->GetSaveDataHandle()->SaveString(
                    ANSI_TO_TCHAR(ss.str().c_str()), savePathWithName);
            }
        }
        // public msg
        if (public_msg)
        {
            sim_msg::CameraRaw craw;
            craw.set_id(id);
            craw.set_timestamp(timeStamp_ego);
            if (imgbuf.Num() > 0)
            {
                if (imageFormat == EImageFormat::JPEG)
                {
                    craw.set_type("JPEG");
                }
                else if (imageFormat == EImageFormat::PNG)
                {
                    craw.set_type("PNG");
                }
                else if (imageFormat == EImageFormat::EXR)
                {
                    craw.set_type("EXR");
                }
                craw.set_image_data(imgbuf.GetData(), imgbuf.Num());
            }
            else
            {
                // if (imageFormat == EImageFormat::PNG)
                //     craw.set_type("PNG");
                // else if (imageFormat == EImageFormat::BMP)
                //     craw.set_type("BMP");
                // else if (imageFormat == EImageFormat::ICO)
                //     craw.set_type("ICO");
                // else if (imageFormat == EImageFormat::EXR)
                //     craw.set_type("EXR");
                // else if (imageFormat == EImageFormat::ICNS)
                //     craw.set_type("ICNS");

                craw.set_type("BGRA");
                craw.set_image_data(BitData.data(), BitData.size());
            }

            double X = 0, Y = 0, Z = 0;
            hadmapue4::HadmapManager::Get()->LocalToLonLat(GetActorLocation(), X, Y, Z);
            craw.mutable_pose()->set_longitude(X);
            craw.mutable_pose()->set_latitude(Y);
            craw.mutable_pose()->set_altitude(Z);
            auto Rot = GetActorRotation();
            craw.mutable_pose()->set_roll(Rot.Roll * PI / 180.f);
            craw.mutable_pose()->set_pitch(-Rot.Pitch * PI / 180.f);
            craw.mutable_pose()->set_yaw(-(Rot.Yaw + 90.f) * PI / 180.f);
            craw.set_width(imageRes0.X);
            craw.set_height(imageRes0.Y);
            craw.SerializeToString(&_Output.serialize_string);

            // UE_LOG(LogTemp, Warning, TEXT("CameraSensor debug: t= %lf. pos= %.12lf, %.12lf, %.12lf"), timeStamp_ego,
            // X, Y, Z);
        }
    }
}

// 保存数据
bool ACameraSensor::Save()
{
    if (savePath.IsEmpty())
    {
        return true;
    }
    // UE_LOG(LogTemp, Log, TEXT("CameraSensorComponent INFO: SaveData!"));
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(imageFormat);

    if (ImageWrapper->SetRaw(dataBuf.buffer.data(),
            /*Elem.buffer.Num() * sizeof(FColor)*/ sizeof(FColor) * imageRes0.X * imageRes0.Y, imageRes0.X, imageRes0.Y,
            ERGBFormat::BGRA, 8))
    {
        if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*savePath))
        {
            FString SaveDir = savePath + FString::Printf(TEXT("%s_%d_%010d.jpg"), *imageName, id, (int64) timeStamp);
            GetDisplayInstance()->GetSaveDataHandle()->SaveJPG(ImageWrapper->GetCompressed(imageQuality), SaveDir);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent WARN: Directory dont Exists!"));
        }
    }
    return true;
}

// 后处理设置
void ACameraSensor::SetPostProcessSettings(
    const FCameraConfig& config, FPostProcessSettings& PostProcessSettings, float screen_scale)
{
    // 清晰度
    PostProcessSettings.bOverride_ScreenPercentage = 1;
    PostProcessSettings.ScreenPercentage =
        100.0f * screen_scale * (1 + config.Exquisite * (config.Exquisite > 0 ? 1 : -0.2));

    // Exposure; 曝光设置
    float AutoExposureBias = config.Compensation * config.Transmittance * 0.01;
    GConfig->GetFloat(TEXT("Sensor"), TEXT("CameraExposureBias"), AutoExposureBias, GGameIni);
    PostProcessSettings.bOverride_AutoExposureBias = config.Exposure == 0;
    PostProcessSettings.AutoExposureBias = AutoExposureBias;
    if (config.Exposure == 1)
    {
        PostProcessSettings.bOverride_AutoExposureMethod = 1;
        PostProcessSettings.AutoExposureMethod = AEM_Manual;
        PostProcessSettings.bOverride_CameraISO = 1;
        PostProcessSettings.CameraISO = config.ISO;
        PostProcessSettings.bOverride_CameraShutterSpeed = 1;
        PostProcessSettings.CameraShutterSpeed = config.ShutterSpeed;
        PostProcessSettings.bOverride_DepthOfFieldFstop = 1;
        PostProcessSettings.DepthOfFieldFstop = config.Aperture;
    }

    // bloom
    PostProcessSettings.bOverride_BloomIntensity = 1;
    PostProcessSettings.BloomIntensity = config.Bloom;

    // noise
    if (config.noise_Intensity > 0)
    {
        PostProcessSettings.bOverride_GrainIntensity = 1;
        PostProcessSettings.GrainIntensity = config.noise_Intensity;
    }

    if (config.LensFlares > 0)
    {
        PostProcessSettings.bOverride_LensFlareIntensity = 1;
        PostProcessSettings.LensFlareIntensity = config.LensFlares * 16.0;
    }

    // Motion blur
    PostProcessSettings.bOverride_MotionBlurAmount = 1;
    PostProcessSettings.MotionBlurAmount = config.motionBlur_Amount;

    // vignetting
    PostProcessSettings.bOverride_VignetteIntensity = true;
    PostProcessSettings.VignetteIntensity = config.vignette_Intensity;

    // Color Grading
    PostProcessSettings.bOverride_WhiteTemp = 1;
    PostProcessSettings.bOverride_WhiteTint = 1;
    PostProcessSettings.WhiteTemp = config.ColorTemperature;
    PostProcessSettings.WhiteTint = config.WhiteHint;

    if (config.color_gray)
    {
        PostProcessSettings.bOverride_ColorSaturation = 1;
        PostProcessSettings.ColorSaturation.W = 0;
    }
}

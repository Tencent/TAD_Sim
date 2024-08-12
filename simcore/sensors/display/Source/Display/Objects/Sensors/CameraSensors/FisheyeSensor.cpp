// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @file FisheyeSensor.cpp
 * @brief This file contains the implementation of the class FisheyeSensor.
 * 鱼眼相机的仿真
 * @author <kekesong>
 * @date 2023-09-10
 * @copyright Copyright (c) 2017 <NAME>
 */

#include "FisheyeSensor.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponentCube.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "CineCameraComponent.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "HighResScreenshot.h"
#include "HAL/PlatformFilemanager.h"
#include "Framework/SaveDataThread.h"
#include "Framework/DisplayGameInstance.h"
#include "Engine/World.h"
#include "Framework/DisplayPlayerController.h"
#include "Objects/Transports/TransportPawn.h"
#include "TexJpeg.h"
#include "SimMsg/sensor_raw.pb.h"

AFisheyeSensor::AFisheyeSensor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));

    previewComponent = CreateDefaultSubobject<UCineCameraComponent>(FName(TEXT("PreviewCamera")));
    previewComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    previewComponent->SetUseFieldOfViewForLOD(true);
    previewComponent->SetActive(false);
    captureComponentCube = CreateDefaultSubobject<USceneCaptureComponentCube>(FName(TEXT("CaptureCameraCube")));
    captureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(FName(TEXT("CaptureCamera")));
    captureComponentCube->SetActive(false);
    captureComponent2D->SetActive(false);
}

AFisheyeSensor::~AFisheyeSensor()
{
}

bool AFisheyeSensor::Init(const FSensorConfig& _Config)
{
    ASensorActor::Init(_Config);
    // FSensorConfig* NewConfigPtr = &_Config;
    // FCameraConfig* NewCameraSensorConfig = static_cast<FCameraConfig*>(NewConfigPtr);
    const FFisheyeConfig* NewCameraSensorConfig = Cast_Sim<const FFisheyeConfig>(_Config);

    if (!NewCameraSensorConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("FisheyeCameraSensorComponent: Cant Cast to FCameraConfig!"));
        return false;
    }
    sensorConfig = *NewCameraSensorConfig;

    // 创建保存路径
    if (!sensorConfig.savePath.IsEmpty() &&
        !FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*sensorConfig.savePath))
    {
        // UE_LOG(LogTemp, Log, TEXT("CameraSensorComponent: Generate savePath."));
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*sensorConfig.savePath);
    }
    // 获取内参
    const auto& im = sensorConfig.intrinsic_Matrix;
    const auto& dp = sensorConfig.distortion_Parameters;
    double fx = im[0];
    double fy = im[4];
    double cx = im[2];
    double cy = im[5];
    double skew = im[1];
    double k1 = dp[0];
    double k2 = dp[1];
    double k3 = dp[2];
    double k4 = dp[3];
    double maxxfov = 0, maxyfov = 0;
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
            double theta_d = sqrt(x * x + y * y);
            double theta = theta_d;
            const double EPS = 0.000001;
            double scale = 1.0;
            if (theta_d > EPS)
            {
                for (int j = 0; j < 10; j++)
                {
                    double theta2 = theta * theta, theta4 = theta2 * theta2, theta6 = theta4 * theta2,
                           theta8 = theta6 * theta2;
                    double k0_theta2 = k1 * theta2, k1_theta4 = k2 * theta4, k2_theta6 = k3 * theta6,
                           k3_theta8 = k4 * theta8;
                    double theta_fix = (theta * (1 + k0_theta2 + k1_theta4 + k2_theta6 + k3_theta8) - theta_d) /
                                       (1 + 3 * k0_theta2 + 5 * k1_theta4 + 7 * k2_theta6 + 9 * k3_theta8);
                    theta = theta - theta_fix;
                    if (abs(theta_fix) < EPS)
                        break;
                }
                scale = tan(theta) / theta_d;
                theta_d = theta;
            }
            if (theta_d > -1.5 && theta_d < 1.5)
            {
                x *= scale;
                y *= scale;
                // Calculate new camera's field-of-view based on maximum horizontal and vertical angles
                maxxfov = FMath::Max(maxxfov, (double) FMath::Abs(FMath::Atan(x)));
                maxyfov = FMath::Max(maxyfov, (double) FMath::Abs(FMath::Atan(y)));
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Fisheye: max fov= %f, %f"), maxxfov, maxyfov);
    if (maxxfov <= 0 || maxyfov <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fisheye: Cant get cal max fov!"));
        return false;
    }
    // 使用cube获取图像
    if (maxxfov > 1.3 || maxyfov > 1.3)    // 75du
    {
        UE_LOG(LogTemp, Log, TEXT("Fisheye: use USceneCaptureComponentCube"));
        UMaterial* NewMat = LoadObject<UMaterial>(NULL,
            TEXT("Material'/Game/SensorSim/Camera/Material/Mat_Camera_Capture_fisheye.Mat_Camera_Capture_fisheye'"));
        if (!NewMat)
        {
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor Component: Cant get Material!"));
            return false;
        }
        cameraPostProcess = UMaterialInstanceDynamic::Create(NewMat, this);
        if (!cameraPostProcess)
        {
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor Component: Cant get MaterialInstanceDynamic!"));
            return false;
        }
        UE_LOG(LogTemp, Log,
            TEXT("FisheyeSensor: cx=%f, cy=%f, fx=%f, fy=%f, skew=%f, k1=%f, k2=%f, k3=%f, k4=%f, w=%f, h=%f"), cx, cy,
            fx, fy, skew, k1, k2, k3, k4, NewCameraSensorConfig->res_Horizontal, NewCameraSensorConfig->res_Vertical);
        renderTargetCube = NewObject<UTextureRenderTargetCube>();
        renderTargetCube->Init(
            FMath::Min(sensorConfig.res_Horizontal, sensorConfig.res_Vertical), EPixelFormat::PF_B8G8R8A8);    //* 3 / 2
        captureComponentCube->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        captureComponentCube->bCaptureEveryFrame = true;
        captureComponentCube->bAutoActivate = true;
        captureComponentCube->Activate();
        captureComponentCube->TextureTarget = renderTargetCube;

        // 设置pp处理参数
        // 内参
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("cx")), cx);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("cy")), cy);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("fx")), fx);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("fy")), fy);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("skew")), skew);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k1")), k1);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k2")), k2);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k3")), k3);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k4")), k4);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("w")), sensorConfig.res_Horizontal);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("h")), sensorConfig.res_Vertical);
        cameraPostProcess->SetTextureParameterValue(FName(TEXT("Param_Cube")), renderTargetCube);
        // Blur
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("BlurIntensity")), sensorConfig.blur_Intensity);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("Res_X")), sensorConfig.res_Horizontal);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("Res_Y")), sensorConfig.res_Vertical);

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
        previewComponent->Filmback.SensorWidth = flen;
        previewComponent->Filmback.SensorHeight = flen * sensorConfig.res_Vertical / sensorConfig.res_Horizontal;
        // Set the post process settings to match the newly calculated FOV
        SetPostProcessSettings(previewComponent->PostProcessSettings);
        // Add the mid-camera postprocess effect with an opacity value of 1.0 to the list of blendables in the post
        // process settings
        previewComponent->PostProcessSettings.AddBlendable(cameraPostProcess, 1);
        if (GetDisplayInstance()->nHILpos.X < 0)
        {
            // 创建一个新的纹理目标2D对象
            renderTarget2D = NewObject<UTextureRenderTarget2D>();
            // 设置共享标志为true，以便可以在GPU上共享
            renderTarget2D->bGPUSharedFlag = 1;
            // 设置渲染目标格式为RGBA8
            renderTarget2D->RenderTargetFormat = RTF_RGBA8;
            // 设置目标gamma
            renderTarget2D->TargetGamma = targetGamma;
            // 初始化自定义格式，设置图像大小和像素格式
            renderTarget2D->InitCustomFormat(
                sensorConfig.res_Horizontal, sensorConfig.res_Vertical, EPixelFormat::PF_B8G8R8A8, true);
            // 将场景捕获组件附加到根组件上
            captureComponent2D->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
            // 设置场景捕获组件的源类型为最终颜色（线性深度）
            captureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
            captureComponent2D->TextureTarget = renderTarget2D;
            SetPostProcessSettings(captureComponent2D->PostProcessSettings);
            captureComponent2D->PostProcessSettings.AddBlendable(cameraPostProcess, 1);
            // 设置场景捕获组件是否每帧进行捕获
            captureComponent2D->bCaptureEveryFrame = true;
            // 设置场景捕获组件为活动状态
            captureComponent2D->bAutoActivate = true;
            captureComponent2D->Activate();
            captureComponent2D->ShowFlags.SetMotionBlur(true);
        }
    }
    else
    {
        if (captureComponentCube)
        {
            captureComponentCube->bCaptureEveryFrame = false;
            captureComponentCube->bAutoActivate = false;
            captureComponentCube->Deactivate();
        }

        UE_LOG(LogTemp, Log, TEXT("Fisheye: use UTextureRenderTarget2D"));
        // Calculate new camera's field-of-view based on maximum horizontal and vertical angles
        double ofx = 0.5 * NewCameraSensorConfig->res_Horizontal / FMath::Tan(maxxfov);
        double ofy = 0.5 * NewCameraSensorConfig->res_Vertical / FMath::Tan(maxyfov);
        double of = fmin(ofx, ofy);
        ofx = of;
        ofy = of;
        maxxfov = FMath::Atan(0.5 * NewCameraSensorConfig->res_Horizontal / of);
        maxyfov = FMath::Atan(0.5 * NewCameraSensorConfig->res_Vertical / of);
        // NEW fov
        double NewFov_H = FMath::RadiansToDegrees(maxxfov) * 2;
        double NewFov_V = FMath::RadiansToDegrees(maxyfov) * 2;
        double fov_scale = FMath::Max(fx / ofx, fy / ofy);

        UMaterial* NewMat =
            LoadObject<UMaterial>(NULL, TEXT("Material'/Game/SensorSim/Camera/Material/"
                                             "Mat_Camera_Capture_fisheye_pinhole.Mat_Camera_Capture_fisheye_pinhole'"));
        if (!NewMat)
        {
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor Component: Cant get Material!"));
            return false;
        }
        cameraPostProcess = UMaterialInstanceDynamic::Create(NewMat, this);
        if (!cameraPostProcess)
        {
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor Component: Cant get MaterialInstanceDynamic!"));
            return false;
        }
        // 设置pp处理参数
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("cx")), cx);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("cy")), cy);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("fx")), fx);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("fy")), fy);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("skew")), skew);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k1")), k1);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k2")), k2);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k3")), k3);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("k4")), k4);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("ofx")), ofx);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("ofy")), ofy);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("w")), NewCameraSensorConfig->res_Horizontal);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("h")), NewCameraSensorConfig->res_Vertical);
        // Blur
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("BlurIntensity")), NewCameraSensorConfig->blur_Intensity);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("Res_X")), NewCameraSensorConfig->res_Horizontal);
        cameraPostProcess->SetScalarParameterValue(FName(TEXT("Res_Y")), NewCameraSensorConfig->res_Vertical);

        UE_LOG(LogTemp, Log,
            TEXT("CameraSensor: cx=%f, cy=%f, fx=%f, fy=%f, skew=%f, k1=%f, k2=%f, k3=%f, k4=%f, ofx=%f, ofy=%f, w=%f, "
                 "h=%f, fscale=%f"),
            cx, cy, fx, fy, skew, k1, k2, k3, k4, ofx, ofy, NewCameraSensorConfig->res_Horizontal,
            NewCameraSensorConfig->res_Vertical, fov_scale);

        // 设置预览组件的焦距
        float flen = 100.f;
        previewComponent->CurrentFocalLength = flen;
        previewComponent->LensSettings.MinFocalLength = flen;
        previewComponent->LensSettings.MaxFocalLength = flen;
        previewComponent->LensSettings.MinFStop = 7.0f;
        previewComponent->LensSettings.MaxFStop = 7.0f;
        previewComponent->Filmback.SensorWidth = FMath::Tan(FMath::DegreesToRadians(NewFov_H) / 2) * 2 * flen;
        previewComponent->Filmback.SensorHeight = FMath::Tan(FMath::DegreesToRadians(NewFov_V) / 2) * 2 * flen;

        SetPostProcessSettings(captureComponent2D->PostProcessSettings, FMath::Max(1.0, fov_scale * 0.7));
        previewComponent->PostProcessSettings.AddBlendable(cameraPostProcess, 1);
        if (GetDisplayInstance()->nHILpos.X < 0)
        {
            // 设置传感器图像的渲染
            renderTarget2D = NewObject<UTextureRenderTarget2D>();
            renderTarget2D->bGPUSharedFlag = 1;
            renderTarget2D->RenderTargetFormat = RTF_RGBA8;
            renderTarget2D->TargetGamma = targetGamma;
            renderTarget2D->InitCustomFormat(
                NewCameraSensorConfig->res_Horizontal, NewCameraSensorConfig->res_Vertical, PF_B8G8R8A8, true);
            captureComponent2D = NewObject<USceneCaptureComponent2D>();
            captureComponent2D->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
            captureComponent2D->bCaptureEveryFrame = true;
            captureComponent2D->bAutoActivate = true;
            captureComponent2D->SetActive(true);
            captureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
            captureComponent2D->ProjectionType = ECameraProjectionMode::Perspective;
            captureComponent2D->OrthoWidth = 20000.f;
            captureComponent2D->bUseCustomProjectionMatrix = true;
            if ((int32) ERHIZBuffer::IsInverted)
            {
                captureComponent2D->CustomProjectionMatrix =
                    FReversedZPerspectiveMatrix(FMath::DegreesToRadians(NewFov_H * 0.5),
                        FMath::DegreesToRadians(NewFov_V * 0.5f), 1.f, 1.f, GNearClippingPlane, GNearClippingPlane);
            }
            else
            {
                captureComponent2D->CustomProjectionMatrix =
                    FPerspectiveMatrix(FMath::DegreesToRadians(NewFov_H * 0.5f),
                        FMath::DegreesToRadians(NewFov_V * 0.5f), 1.f, 1.f, GNearClippingPlane, GNearClippingPlane);
            }
            captureComponent2D->FOVAngle = fmaxf(NewFov_H, NewFov_V);
            captureComponent2D->TextureTarget = renderTarget2D;

            captureComponent2D->ShowFlags.SetMotionBlur(true);
            SetPostProcessSettings(captureComponent2D->PostProcessSettings, FMath::Max(1.0, fov_scale * 0.7));
            captureComponent2D->PostProcessSettings.AddBlendable(cameraPostProcess, 1);
        }
    }
    if (GetDisplayInstance()->nHILpos.X < 0)
    {
        bool memshared = false, gpushared = false, style = false;
        // 获取配置文件中"Sensor"部分的配置项
        GConfig->GetBool(TEXT("Sensor"), TEXT("CameraMemShare"), memshared, GGameIni);
        GConfig->GetBool(TEXT("Sensor"), TEXT("CameraGPUShare"), gpushared, GGameIni);
        GConfig->GetBool(TEXT("Sensor"), TEXT("CameraStyle"), style, GGameIni);
        GConfig->GetBool(TEXT("Sensor"), TEXT("PublicMsg"), public_msg, GGameIni);
        GConfig->GetInt(TEXT("Sensor"), TEXT("JpegQuality"), imageQuality, GGameIni);
        FString gpuid = TEXT("0");
        if (!FParse::Value(FCommandLine::Get(), TEXT("-graphicsadapter="), gpuid))
        {
            gpuid = TEXT("0");
        }

        texJpg = MakeShared<UTexJpeg>(FCString::Atoi(*gpuid));
        texJpg->texRT = renderTarget2D;
        if (!texJpg->InitResources(true))
        {
            texJpg.Reset();
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: texJpg init faild."));
        }

        // 内存共享
        if (memshared)
        {
            UE_LOG(LogTemp, Log, TEXT("FisheyeSensor: memshare is on."));
            sharedWriter = MakeShared<SharedMemoryWriter>();
            if (!sharedWriter->init(
                    std::string("Tadsim_") + TCHAR_TO_ANSI(*imageName) + "_" + std::to_string(sensorConfig.id), 100))
            {
                UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: sharedWriter init faild."));
                sharedWriter.Reset();
            }
        }
        // GPU显存共享
        if (gpushared && texJpg)
        {
            UE_LOG(LogTemp, Log, TEXT("FisheyeSensor: gpu share is on."));
            // ipc
            sharedWriterGpu = MakeShared<SharedMemoryWriter>();
            if (!sharedWriterGpu->init(std::string("Tadsim_") + TCHAR_TO_ANSI(*imageName) + "_" +
                                       std::to_string(sensorConfig.id) + "_GPU"))
            {
                UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: sharedWriterGpu init faild."));
                sharedWriterGpu.Reset();
            }

            std::vector<uint8> buf(1024);
            if (texJpg->IpcCreate(buf.data() + 8))
            {
                int w = sensorConfig.res_Horizontal, h = sensorConfig.res_Vertical;
                memcpy(buf.data(), &w, sizeof(int));
                memcpy(buf.data() + 4, &h, sizeof(int));
                std::time_t t = std::time(0);
                UE_LOG(LogTemp, Log, TEXT("FisheyeSensor: shared gpu verification code is %ld."), t);
                if (!sharedWriterGpu->write(buf, t))
                {
                    UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: sharedgpu write faild."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: share ipc faild."));
            }
        }
    }

    return true;
}

ISimActorInterface* AFisheyeSensor::Install(const FSensorConfig& _Config)
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

void AFisheyeSensor::Update(const FSensorInput& _Input, FSensorOutput& _Output)
{
    if (!renderTarget2D)    // sil
    {
        return;
    }
    const FFisheyeInput* CameraInput = Cast_Sim<const FFisheyeInput>(_Input);
    // 频率限制
    if (sensorConfig.frequency > 0 &&
        (float) (CameraInput->timeStamp - timeStamp) / 1000.f < 1.f / sensorConfig.frequency)
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
                UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: read raw image buf faild in texjpg"));
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
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: read image buf faild."));
        }
    };

    // 内存共享
    if (sharedWriter)
    {
        getRawBuff();

        if (BitData.empty() || !sharedWriter->write(BitData, timeStamp_ego))
        {
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: sharedWriter write faild."));
        }
    }
    // 显存共享
    if (sharedWriterGpu && cuda)
    {
        std::vector<uint8> buf(1024);
        if (texJpg->IpcShare(buf.data() + 8))
        {
            int w = sensorConfig.res_Horizontal, h = sensorConfig.res_Vertical;
            memcpy(buf.data(), &w, sizeof(int));
            memcpy(buf.data() + 4, &h, sizeof(int));
            // UE_LOG(LogTemp, Log, TEXT("CameraSensor: shared gpu write %f."), timeStamp_ego);
            if (!sharedWriterGpu->write(buf, timeStamp_ego))
            {
                UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: sharedgpu write faild."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: share ipc faild."));
        }
    }

    // 落盘或发布
    TArray64<uint8_t> jpegbuf;
    if (!sensorConfig.savePath.IsEmpty() || public_msg)
    {
        // jpeg图像，优先cuda编码，否则用ue自带CPU编码
        if (imageFormat == EImageFormat::JPEG)
        {
            if (cuda)
            {
                if (!texJpg->JpegEncoding(jpegbuf))
                {
                    UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: read jpeg image buf faild in texjpg"));
                }
            }

            // cuda失败，用ue自带CPU编码
            if (jpegbuf.Num() == 0)
            {
                getRawBuff();

                if (BitData.empty())
                {
                    UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: read image buf faild."));
                }
                else
                {
                    IImageWrapperModule& ImageWrapperModule =
                        FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
                    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
                    if (ImageWrapper->SetRaw(BitData.data(), BitData.size(), sensorConfig.res_Horizontal,
                            sensorConfig.res_Vertical, ERGBFormat::BGRA, 8))
                    {
                        jpegbuf = ImageWrapper->GetCompressed(imageQuality);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("FisheyeSensor: encode jpeg faild."));
                    }
                }
            }
        }
        else
        {
            getRawBuff();
        }

        if (!sensorConfig.savePath.IsEmpty())
        {
            if (jpegbuf.Num() > 0)
            {
                FString SavePath = sensorConfig.savePath + imageName + TEXT("_") + FString::FromInt(sensorConfig.id) +
                                   TEXT("_") + FString::SanitizeFloat(timeStamp_ego) + TEXT(".jpg");
                GetDisplayInstance()->GetSaveDataHandle()->SaveJPG(jpegbuf, SavePath);
            }
            else
            {
                Save();
            }
        }
        // public msg
        if (public_msg)
        {
            sim_msg::CameraRaw craw;
            craw.set_id(sensorConfig.id);
            craw.set_timestamp(timeStamp_ego);
            if (jpegbuf.Num() > 0)
            {
                craw.set_type("JPEG");
                craw.set_image_data(jpegbuf.GetData(), jpegbuf.Num());
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
            craw.set_width(sensorConfig.res_Horizontal);
            craw.set_height(sensorConfig.res_Vertical);
            craw.SerializeToString(&_Output.serialize_string);
        }
    }
}

bool AFisheyeSensor::Save()
{
    if (sensorConfig.savePath.IsEmpty())
    {
        return true;
    }
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(imageFormat);

    if (ImageWrapper->SetRaw(dataBuf.buffer.data(), dataBuf.buffer.size(), sensorConfig.res_Horizontal,
            sensorConfig.res_Vertical, ERGBFormat::BGRA, 8))
    {
        if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*sensorConfig.savePath))
        {
            FString SaveDir = sensorConfig.savePath + imageName + TEXT("_") + FString::FromInt(sensorConfig.id) +
                              TEXT("_") + FString::SanitizeFloat(timeStamp) + TEXT(".") +
                              TEXT("jpg") /*FString(GETENUMSTRING("EImageFormat",
                                             imageFormat)).ToLower()*/
                ;
            GetDisplayInstance()->GetSaveDataHandle()->SaveJPG(ImageWrapper->GetCompressed(imageQuality), SaveDir);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent WARN: Directory dont Exists!"));
        }
    }
    return true;
}

void AFisheyeSensor::SetPostProcessSettings(FPostProcessSettings& PostProcessSettings, float screen_scale)
{
    // 清晰度
    PostProcessSettings.bOverride_ScreenPercentage = 1;
    PostProcessSettings.ScreenPercentage =
        100.0f * screen_scale * (1 + sensorConfig.Exquisite * (sensorConfig.Exquisite > 0 ? 1 : -0.2));

    // bloom
    PostProcessSettings.bOverride_BloomIntensity = 1;
    PostProcessSettings.BloomIntensity = sensorConfig.Bloom;

    // noise
    if (sensorConfig.noise_Intensity > 0)
    {
        PostProcessSettings.bOverride_GrainIntensity = 1;
        PostProcessSettings.GrainIntensity = sensorConfig.noise_Intensity;
    }

    if (sensorConfig.LensFlares > 0)
    {
        PostProcessSettings.bOverride_LensFlareIntensity = 1;
        PostProcessSettings.LensFlareIntensity = sensorConfig.LensFlares * 16.0;
    }

    // Exposure;
    PostProcessSettings.bOverride_AutoExposureBias = sensorConfig.Exposure == 0;
    PostProcessSettings.AutoExposureBias = sensorConfig.Compensation * sensorConfig.Transmittance * 0.01;
    if (sensorConfig.Exposure == 1)
    {
        PostProcessSettings.bOverride_AutoExposureMethod = 1;
        PostProcessSettings.AutoExposureMethod = AEM_Manual;
        PostProcessSettings.bOverride_CameraISO = 1;
        PostProcessSettings.CameraISO = sensorConfig.ISO;
        PostProcessSettings.bOverride_CameraShutterSpeed = 1;
        PostProcessSettings.CameraShutterSpeed = sensorConfig.ShutterSpeed;
        PostProcessSettings.bOverride_DepthOfFieldFstop = 1;
        PostProcessSettings.DepthOfFieldFstop = sensorConfig.Aperture;
    }

    // Motion blur
    PostProcessSettings.bOverride_MotionBlurAmount = 1;
    PostProcessSettings.MotionBlurAmount = sensorConfig.motionBlur_Amount;

    // vignetting
    PostProcessSettings.bOverride_VignetteIntensity = true;
    PostProcessSettings.VignetteIntensity = sensorConfig.vignette_Intensity;

    // Color Grading
    PostProcessSettings.bOverride_WhiteTemp = 1;
    PostProcessSettings.bOverride_WhiteTint = 1;
    PostProcessSettings.WhiteTemp = sensorConfig.ColorTemperature;
    PostProcessSettings.WhiteTint = sensorConfig.WhiteHint;

    if (sensorConfig.color_gray)
    {
        PostProcessSettings.bOverride_ColorSaturation = 1;
        PostProcessSettings.ColorSaturation.W = 0;
    }
}

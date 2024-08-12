// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @file DepthCamera.cpp
 * @brief This file contains the implementation of the DepthCamera class.
 * 深度相机类，获取深度数据，用于激光雷达点云生成
 * @author <kekesong>
 * @date 2022-09-10
 * @copyright Copyright (c) 2017, tencent. All rights reserved.
 */

#include "DepthCamera.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include <thread>
#include "TexJpeg.h"
#include "CudaUtils.h"

// Sets default values
ADepthLidarBuffer::ADepthLidarBuffer()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
    captureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName(TEXT("CaptureComp")));
    captureComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

    captureComponent->bCaptureEveryFrame = true;
    captureComponent->bAutoActivate = true;
    // renderTarget2D = CreateDefaultSubobject<UTextureRenderTarget2D>(FName(TEXT("CaptureRenderTarget")));

    // renderTarget2D->CompressionSettings = TextureCompressionSettings::TC_Default;
    // renderTarget2D->bAutoGenerateMips = false;
    // renderTarget2D->AddressX = TextureAddress::TA_Clamp;
    // renderTarget2D->AddressY = TextureAddress::TA_Clamp;

    ppMaterialPath = TEXT("Material'/Game/SensorSim/Camera/Material/Mat_DepthLidarMaterial.Mat_DepthLidarMaterial'");

    fMaxDepth = 100000.f;    // cm
    fResScale = 10.f;
}

ADepthLidarBuffer::~ADepthLidarBuffer()
{
}
bool ADepthLidarBuffer::SetCamera(const CameraSensorViewConfiguration& camCfg)
{
    // 根据配置文件设置相机参数
    camConfig = camCfg;
    SetActorRelativeLocation(camCfg.position);
    SetActorRelativeRotation(camCfg.rotator.Quaternion());

    // 设置相机内参
    captureComponent->ProjectionType = ECameraProjectionMode::Perspective;
    captureComponent->OrthoWidth = 20000.f;
    captureComponent->bUseCustomProjectionMatrix = true;
    if ((int32) ERHIZBuffer::IsInverted)
    {
        captureComponent->CustomProjectionMatrix =
            FReversedZPerspectiveMatrix(FMath::DegreesToRadians(camCfg.hfov * 0.5f),
                FMath::DegreesToRadians(camCfg.vfov * 0.5f), 1.f, 1.f, GNearClippingPlane, GNearClippingPlane);
    }
    else
    {
        captureComponent->CustomProjectionMatrix = FPerspectiveMatrix(FMath::DegreesToRadians(camCfg.hfov * 0.5f),
            FMath::DegreesToRadians(camCfg.vfov * 0.5f), 1.f, 1.f, GNearClippingPlane, GNearClippingPlane);
    }
    captureComponent->FOVAngle = fmaxf(camCfg.vfov, camCfg.hfov);

    // 配置图像capture
    renderTarget2D = NewObject<UTextureRenderTarget2D>();
    renderTarget2D->TargetGamma = 1;
    renderTarget2D->bGPUSharedFlag = 1;
    renderTarget2D->RenderTargetFormat = RTF_RGBA8;
    renderTarget2D->InitCustomFormat(camCfg.w, camCfg.h, EPixelFormat::PF_B8G8R8A8, true);
    captureComponent->TextureTarget = renderTarget2D;
    captureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

    // 配置pp材质
    depthMaterial = LoadObject<UMaterial>(NULL, *ppMaterialPath);
    if (!depthMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("DepthCamera: Cant get Material!"));
        return false;
    }
    depthProcess = UMaterialInstanceDynamic::Create(depthMaterial, this);
    if (!depthProcess)
    {
        UE_LOG(LogTemp, Warning, TEXT("DepthCameraSensorActor: Cant get MaterialInstanceDynamic!"));
        return false;
    }

    depthProcess->SetScalarParameterValue(FName(TEXT("hfov")), FMath::DegreesToRadians(camCfg.hfov));
    depthProcess->SetScalarParameterValue(FName(TEXT("vfov")), FMath::DegreesToRadians(camCfg.vfov));

    // 设置后效
    captureComponent->PostProcessSettings.bOverride_AutoExposureBias = false;
    captureComponent->PostProcessSettings.bOverride_ColorGamma = true;
    captureComponent->PostProcessSettings.ColorGamma = FVector4(1.0, 1.0, 1.0, 1.0);
    captureComponent->PostProcessSettings.AddBlendable(depthProcess, 1);

    // 设置cuda
    FString gpuid = TEXT("0");
    if (!FParse::Value(FCommandLine::Get(), TEXT("-graphicsadapter="), gpuid))
    {
        gpuid = TEXT("0");
    }
    texJpg = MakeShared<UTexJpeg>(FCString::Atoi(*gpuid));
    texJpg->texRT = renderTarget2D;
    if (!texJpg->InitResources(false, false))
    {
        texJpg.Reset();
        UE_LOG(LogTemp, Warning, TEXT("DepthLidarBuffer: texJpg init faild."));
    }
    if (texJpg)
    {
        imgGpuTmp = (uint8_t*) cuda_utils::Malloc(camCfg.w * camCfg.h * 4 + 1024);
        UE_LOG(LogTemp, Log, TEXT("DepthLidarBuffer: texJpg init succ."));
    }
    return true;
}

bool ADepthLidarBuffer::GetCaptureImage(TArray<FColor>& BitMap)
{
    // 优先使用cuda
    std::vector<uint8> BitData;
    if (texJpg && texJpg->Copy2Cuda() && texJpg->Raw(BitData))
    {
        BitMap.SetNum(BitData.size() / 4);
        memcpy(BitMap.GetData(), BitData.data(), BitMap.Num() * 4);
        return true;
    }
    // 失败则使用ue自带函数
    FTextureRenderTarget2DResource* RTResource = (FTextureRenderTarget2DResource*) renderTarget2D->Resource;
    if (RTResource)
    {
        FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
        // ReadPixelFlags.SetLinearToGamma(false);
        RTResource->ReadPixels(BitMap, ReadPixelFlags);
        return true;
    }
    return false;
}
bool ADepthLidarBuffer::GetCaptureImage(uint8_t*& imgbuf)
{
    // return false;
    //  优先使用cuda
    if (!texJpg || !texJpg->Copy2Cuda(true))
    {
        return false;
    }
    size_t imglen = 0;
    uint8_t* img = texJpg->GetRawPtr(imglen);
    if (!img || imglen == 0)
        return false;
    if (!cuda_utils::MemCopy(imgGpuTmp, img, imglen))
        return false;
    imgbuf = imgGpuTmp;
    return true;
}

// Called when the game starts or when spawned
void ADepthLidarBuffer::BeginPlay()
{
    Super::BeginPlay();
}

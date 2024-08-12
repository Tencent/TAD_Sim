// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @file FovDepthBuffer.cpp
 * @brief This file contains the implementation of the DepthCamera class.
 * 深度相机类，获取深度数据，用于fov
 * @author <kekesong>
 * @date 2022-09-10
 * @copyright Copyright (c) 2017, tencent. All rights reserved.
 */

#include "FovDepthBuffer.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "TexJpeg.h"

AFovDepthBuffer::AFovDepthBuffer()
{
    // 默认构造captureComponent
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

    ppMaterialPath = TEXT("Material'/Game/SensorSim/Camera/Material/Mat_DepthMaterial.Mat_DepthMaterial'");

    fMaxDepth = 100000.f;    // cm
    fResScale = 100.f;
}

AFovDepthBuffer::~AFovDepthBuffer()
{
}
bool AFovDepthBuffer::SetCamera(const FovViewConfiguration& camCfg)
{
    // 根据配置文件设置相机参数
    fovCfg = camCfg;
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
    fx = camCfg.w * 0.5 / FMath::Tan(FMath::DegreesToRadians(camCfg.hfov * 0.5f));
    fy = camCfg.h * 0.5 / FMath::Tan(FMath::DegreesToRadians(camCfg.vfov * 0.5f));

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
        UE_LOG(LogTemp, Warning, TEXT("DepthBuffer: Cant get Material!"));
        return false;
    }
    depthProcess = UMaterialInstanceDynamic::Create(depthMaterial, this);
    if (!depthProcess)
    {
        UE_LOG(LogTemp, Warning, TEXT("DepthBuffer: Cant get MaterialInstanceDynamic!"));
        return false;
    }

    depthProcess->SetScalarParameterValue(FName(TEXT("MaxDepth")), fMaxDepth);
    depthProcess->SetScalarParameterValue(FName(TEXT("Scale")), fResScale);

    // 设置后效
    captureComponent->PostProcessSettings.bOverride_AutoExposureBias = false;
    captureComponent->PostProcessSettings.bOverride_ColorGamma = true;
    captureComponent->PostProcessSettings.ColorGamma = FVector4(1.0, 1.0, 1.0, 1.0);
    captureComponent->PostProcessSettings.AddBlendable(depthProcess, 1);

    // 设置cuda
    texJpg = MakeShared<UTexJpeg>();
    texJpg->texRT = renderTarget2D;
    if (!texJpg->InitResources(false, false))
    {
        texJpg.Reset();
        UE_LOG(LogTemp, Warning, TEXT("DepthBuffer: texJpg init faild."));
    }
    UE_LOG(LogTemp, Log, TEXT("DepthBuffer: texJpg init succ."));
    return true;
}

bool AFovDepthBuffer::GetCaptureImage(TArray<FColor>& BitMap)
{
    // 优先使用cuda
    std::vector<uint8> BitData;
    if (texJpg && texJpg->Copy2Cuda(true) && texJpg->Raw(BitData))
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

// 根据某个pitch yaw，获取深度
bool AFovDepthBuffer::GetDepth(
    const TArray<FColor>& BitMap, float pitch, float yaw, float& dis, float& normal, int* tag)
{
    // 计算uv
    float u = fx * FMath::Tan(FMath::DegreesToRadians(yaw)) + fovCfg.w * 0.5;
    float v = fy * FMath::Tan(FMath::DegreesToRadians(pitch)) + fovCfg.h * 0.5;
    int x0 = FMath::FloorToInt(u);
    int y0 = FMath::FloorToInt(v);
    if (y0 < 0 || y0 >= fovCfg.h || x0 < 0 || x0 >= fovCfg.w)
    {
        return false;
    }
    // 获取深度
    const auto& color00 = BitMap[y0 * fovCfg.w + x0];
    dis = ((int) color00.R * 256 + (int) color00.G) / fResScale;
    if (dis > 1e-6f && dis < fMaxDepth)
    {
        normal = (float) color00.B / 256.0f;
        if (tag)
        {
            *tag = color00.A;
        }
        return true;
    }
    return false;
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "RTXLidarSensor.h"
#include "RTXLidarComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

ARTXLidarSensor::ARTXLidarSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    RTXComponent = CreateDefaultSubobject<URTXLidarComponent>(TEXT("RTX Lidar"));
    RootComponent = RTXComponent;
    CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NewSceneCaptureComponent2D"));
    CaptureComponent2D->SetupAttachment(RootComponent);
    CaptureComponent2D->bUseRayTracingIfEnabled = true;
    PrimaryActorTick.bCanEverTick = true;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    PrimaryActorTick.bStartWithTickEnabled = true;
#endif
}

void ARTXLidarSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (RTTemp)
    {
        RTTemp->ReleaseResource();
        RTTemp = nullptr;
    }
    if (CaptureComponent2D)
    {
        CaptureComponent2D->TextureTarget = nullptr;
    }
    if (RTXComponent)
    {
        if (RTXComponent->RTResult)
        {
            RTXComponent->RTResult->ReleaseResource();
            RTXComponent->RTResult = nullptr;
        }
    }
    Super::EndPlay(EndPlayReason);
}

void ARTXLidarSensor::BeginPlay()
{
    Super::BeginPlay();
    // 쏭鱗끝쒼숏학痰
    if (CaptureComponent2D != nullptr && CaptureComponent2D->TextureTarget == nullptr)
    {
        RTTemp = NewObject<UTextureRenderTarget2D>(this);
        RTTemp->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
        RTTemp->RenderTargetFormat = RTF_RGBA8;
        uint32 InitSize = 4;
#if WITH_EDITOR
        InitSize = 64;
#endif
        RTTemp->InitAutoFormat(InitSize, InitSize);
        CaptureComponent2D->TextureTarget = RTTemp;
        CaptureComponent2D->bUseRayTracingIfEnabled = true;
        CaptureComponent2D->ShowFlags.SetAmbientCubemap(false);
        CaptureComponent2D->ShowFlags.SetAmbientOcclusion(false);
        CaptureComponent2D->ShowFlags.SetAntiAliasing(false);
        CaptureComponent2D->ShowFlags.SetAtmosphere(false);
        CaptureComponent2D->ShowFlags.SetBloom(false);
        CaptureComponent2D->ShowFlags.SetDepthOfField(false);
        CaptureComponent2D->ShowFlags.SetFog(false);
        CaptureComponent2D->ShowFlags.SetDynamicShadows(false);
        CaptureComponent2D->ShowFlags.SetEyeAdaptation(false);
        CaptureComponent2D->ShowFlags.SetInstancedFoliage(true);
        CaptureComponent2D->ShowFlags.SetVolumetricFog(false);
    }
    if (CaptureComponent2D)
    {
        RTXComponent->InitViewParam(CaptureComponent2D);
    }
#if WITH_EDITOR
    TArray<FVector2D> angles;
    for (int x = 0; x < 64; ++x)
    {
        for (int y = 0; y < 64; ++y)
        {
            FVector2D angle = FVector2D(x / 64.f, y / 64.f);
            angle = 2 * angle - FVector2D(1.f, 1.f);
            angles.Add(45.f * angle);
        }
    }
    SetYawPitchArray(angles);
#endif
}
void ARTXLidarSensor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (RTXComponent)
    {
        RTXComponent->UpdateParam();
    }
}

void ARTXLidarSensor::SetYawPitchArray(const TArray<FVector2D> angles)
{
    if (RTXComponent)
    {
        RTXComponent->rayDegree = angles;
    }
}

void ARTXLidarSensor::SetRefArray(const TArray<FVector4> ref)
{
    if (RTXComponent)
    {
        RTXComponent->refDef = ref;
    }
}

bool ARTXLidarSensor::GetCaptureData(TArray<FColor>& result)
{
    if (RTXComponent)
    {
        return RTXComponent->GetCaptureData(result);
    }
    return false;
}

void ARTXLidarSensor::SetInstallPose(const FTransform& pose)
{
    if (RTXComponent)
    {
        FVector pos = pose.GetLocation();
        FRotator rot = pose.GetRotation().Rotator();
        pos.Z *= -1;
        rot.Roll *= -1;
        rot.Pitch *= -1;
        FTransform rt;
        rt.SetLocation(pos);
        rt.SetRotation(rot.Quaternion());
        RTXComponent->symmetryPose = rt * pose.Inverse();
    }
}

void ARTXLidarSensor::SetMirrorThreshold(float th)
{
    if (RTXComponent)
    {
        RTXComponent->mirrorThreshold = th;
    }
}

void ARTXLidarSensor::SetLidarIntensity(float i)
{
    if (RTXComponent)
    {
        RTXComponent->t_intensity = i;
    }
}

void ARTXLidarSensor::SetLidarRain(float w)
{
#define POSSON_STDDEV 0.03464f
    if (RTXComponent)
    {
        w *= 0.041666667f;
        RTXComponent->f_rainfall = exp(-0.02f * pow(w, 0.6f));
        RTXComponent->f_noise_dev = POSSON_STDDEV + 0.0245f * w;
    }
}

void ARTXLidarSensor::SetLidarSnow(float s)
{
    if (RTXComponent)
    {
        if (s <= 1e-5)
        {
            RTXComponent->f_snowfall_range = 1000.f;
            RTXComponent->f_snowfall_prob = 1.f;
            return;
        }
        RTXComponent->f_snowfall_range = 1000.f / s;
        RTXComponent->f_snowfall_prob = 1.f - s * 0.03f * 2;
    }
}

void ARTXLidarSensor::SetLidarFog(float f)
{
    if (RTXComponent)
    {
        float r = 1060.282f * pow(f, -0.91f);
        RTXComponent->f_fog = exp(-0.02f * pow(r, 0.6f));
    }
}

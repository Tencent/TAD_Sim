// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RTXLidarComponent.generated.h"

#if ENGINE_MAJOR_VERSION < 5
using FGlobalIlluminationPluginResources = class FGlobalIlluminationExperimentalPluginResources;
#else
class FGlobalIlluminationPluginResources;
#endif

struct FLidarTexturePixels
{
    struct
    {
        uint32 Width = 0;
        uint32 Height = 0;
        uint32 Stride = 0;
        uint32 PixelFormat = 0;
    } Desc;
    TArray<uint8> Pixels;
    FTexture2DRHIRef Texture;
};

class RTXLIDAR_API FRTXLidarSceneProxy
{
public:
    /** Initialization constructor. */
    FRTXLidarSceneProxy(FSceneInterface* InOwningScene) : OwningScene(InOwningScene)
    {
    }

    virtual ~FRTXLidarSceneProxy()
    {
        check(IsInRenderingThread() || IsInParallelRenderingThread());
        AllProxiesReadyForRender_RenderThread.Remove(this);
    }
    // static void HandlePreWorldFinishDestroy(UWorld* InWorld);
    static void RenderDiffuseIndirectLight_RenderThread(const FScene& Scene, const FViewInfo& View,
        FRDGBuilder& GraphBuilder, FGlobalIlluminationPluginResources& Resources);
    void ReallocateSurfaces_RenderThread(FRHICommandListImmediate& RHICmdList);
    void ResetTextures_RenderThread(FRDGBuilder& GraphBuilder);

    static FDelegateHandle OnPreWorldFinishDestroyHandle;
    static FDelegateHandle RenderDiffuseIndirectLightHandle;

    // data from the component
    struct FComponentData
    {
        EPixelFormat RTPixelFormat = EPixelFormat::PF_A32B32G32R32F;
        int Width = 0;
        int Height = 0;
        TArray<FVector2D> RayDirs;
        TArray<FVector4> RefDef;
        FTransform symmetryPose;
        float mirrorThreshold = 0;
        float f_noise_dev = 0;
        float f_accuracy = 0;
        float f_rainfall = 0;
        float f_snowfall_range = 0;
        float f_snowfall_prob = 0;
        float f_fog = 0;
        float t_intensity = 0;
    };
    FComponentData ComponentData;
    TRefCountPtr<IPooledRenderTarget> RenderTarget;
    FTexture2DRHIRef RenderTargetRHI;
    uint32 ViewKey = 0;

    static TSet<FRTXLidarSceneProxy*> AllProxiesReadyForRender_RenderThread;
    // Only render volumes in the scenes they are present in
    FSceneInterface* OwningScene;
};

UCLASS(HideCategories = (Tags, AssetUserData, Collision, Cooking, Transform, Rendering, Mobility, LOD))
class RTXLIDAR_API URTXLidarComponent : public USceneComponent
{
    GENERATED_UCLASS_BODY()

protected:
    void InitializeComponent() override final;
    virtual void BeginDestroy() override;

    //~ Begin UActorComponent Interface
    virtual bool ShouldCreateRenderState() const override
    {
        return true;
    }
    virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
    virtual void DestroyRenderState_Concurrent() override;
    virtual void SendRenderDynamicData_Concurrent() override;
    //~ Begin UActorComponent Interface

    void UpdateRenderThreadData();

public:
    void InitViewParam(class USceneCaptureComponent2D* Capture2D);
    void UpdateParam();
    bool GetCaptureData(TArray<FColor>& result);
    static void Startup();
    static void Shutdown();

public:
    UPROPERTY(EditAnywhere, Category = "Lidar")
    TArray<FVector2D> rayDegree = {FVector2D(0, 0)};
    UPROPERTY(EditAnywhere, Category = "Lidar")
    TArray<FVector4> refDef;

    UPROPERTY(EditAnywhere, Category = "Lidar")
    TEnumAsByte<ETextureRenderTargetFormat> RTFormat = RTF_RGBA8;

    UPROPERTY(EditAnywhere, Category = "Lidar")
    FTransform symmetryPose;

    UPROPERTY(EditAnywhere, Category = "Lidar")
    float mirrorThreshold = 0;

    // noise
    UPROPERTY(EditAnywhere, Category = "Lidar")
    float t_intensity = 0;
    UPROPERTY(EditAnywhere, Category = "Lidar")
    float f_noise_dev = 0;
    UPROPERTY(EditAnywhere, Category = "Lidar")
    float f_accuracy = 0;

    // rain members
    UPROPERTY(EditAnywhere, Category = "Lidar")
    float f_rainfall = 0;

    // snow members
    UPROPERTY(EditAnywhere, Category = "Lidar")
    float f_snowfall_range = 0;
    UPROPERTY(EditAnywhere, Category = "Lidar")
    float f_snowfall_prob = 0;

    // fog members
    UPROPERTY(EditAnywhere, Category = "Lidar")
    float f_fog = 0;
    // protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lidar")
    class UTextureRenderTarget2D* RTResult = nullptr;

    TSharedPtr<FRTXLidarSceneProxy> SceneProxy;

private:
    UPROPERTY()
    class USceneCaptureComponent2D* CaptureComponent2D;
    uint32 ViewKey = UINT32_MAX;
    FTexture2DRHIRef RTResultRHI;
};

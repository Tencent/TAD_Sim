// Fill out your copyright notice in the Description page of Project Settings.
#include "RTXLidarComponent.h"
#include "LegacyEngineCompat.h"
// UE Private Interfaces
#include "PostProcess/SceneRenderTargets.h"
#include "SceneRendering.h"
#include "DeferredShadingRenderer.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "RayTracing/RayTracingMaterialHitShaders.h"
#include "ScenePrivate.h"

class FRayTracingLidarRGS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FRayTracingLidarRGS)
    SHADER_USE_ROOT_PARAMETER_STRUCT(FRayTracingLidarRGS, FGlobalShader)

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
    SHADER_PARAMETER_SRV(RaytracingAccelerationStructure, TLAS)
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
    SHADER_PARAMETER(FMatrix, CameraMatrix)
    SHADER_PARAMETER(FMatrix, MirrorMatrix)
    SHADER_PARAMETER(float, MirrorThreshold)
    SHADER_PARAMETER_SRV(StructuredBuffer<float2>, RayDirs)
    SHADER_PARAMETER_SRV(StructuredBuffer<float4>, RefDef)
    SHADER_PARAMETER(float, f_noise_dev)
    SHADER_PARAMETER(float, f_accuracy)
    SHADER_PARAMETER(float, f_rainfall)
    SHADER_PARAMETER(float, f_snowfall_range)
    SHADER_PARAMETER(float, f_snowfall_prob)
    SHADER_PARAMETER(float, f_fog)
    SHADER_PARAMETER(float, t_intensity)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return ShouldCompileRayTracingShadersForProject(Parameters.Platform);
    }
};
IMPLEMENT_GLOBAL_SHADER(
    FRayTracingLidarRGS, "/Plugin/RTXLidar/Private/RayTracingLidar.usf", "RayTracingLidarMainRGS", SF_RayGen);

// Delegate Handles
FDelegateHandle FRTXLidarSceneProxy::RenderDiffuseIndirectLightHandle;
TSet<FRTXLidarSceneProxy*> FRTXLidarSceneProxy::AllProxiesReadyForRender_RenderThread;
void FRTXLidarSceneProxy::RenderDiffuseIndirectLight_RenderThread(const FScene& Scene, const FViewInfo& View,
    FRDGBuilder& GraphBuilder, FGlobalIlluminationPluginResources& Resources)
{
    if (AllProxiesReadyForRender_RenderThread.Num() < 1)
        return;

    FRayTracingPipelineState* Pipeline = View.RayTracingMaterialPipeline;
    FRHIRayTracingScene* RayTracingSceneRHI = View.RayTracingScene.RayTracingSceneRHI;
    if (!RayTracingSceneRHI || !Pipeline)
    {
        return;
    }
    for (FRTXLidarSceneProxy* proxy : AllProxiesReadyForRender_RenderThread)
    {
        if (proxy->ViewKey != View.GetViewKey())
            continue;
        if (!proxy->RenderTarget.IsValid())
            continue;

        const auto FeatureLevel = GMaxRHIFeatureLevel;
        auto ShaderMap = GetGlobalShaderMap(FeatureLevel);
        FIntPoint Extent = proxy->RenderTarget->GetDesc().Extent;

        auto RayGenShader = ShaderMap->GetShader<FRayTracingLidarRGS>();
        FRayTracingLidarRGS::FParameters* RayGenParameters =
            GraphBuilder.AllocParameters<FRayTracingLidarRGS::FParameters>();
        FVector pos = View.ViewMatrices.GetViewOrigin();
        FQuat viewRot = View.ViewMatrices.GetInvViewMatrix().ToQuat() * FRotator(90, -90, 0).Quaternion();
        FRotator rot = viewRot.Rotator();
        RayGenParameters->CameraMatrix = FTransform(rot, pos).ToMatrixWithScale();
        RayGenParameters->TLAS = RayTracingSceneRHI->GetShaderResourceView();
        // test
        // UE_LOG(LogTemp, Warning, TEXT("test dir:%f,%f,%f"), rot.Roll, rot.Pitch, rot.Yaw);
        //
        TResourceArray<FVector2D> RayDirs;
        RayDirs.Reserve(Extent.X * Extent.Y);
        for (int32 dataIdx = 0; dataIdx < proxy->ComponentData.RayDirs.Num(); dataIdx++)
        {
            RayDirs.Push(proxy->ComponentData.RayDirs[dataIdx]);
        }
        // push null vector to prevent failure in RHICreateStructuredBuffer due to requesting a zero sized allocation
        if (RayDirs.Num() == 0)
        {
            RayDirs.Push(FVector2D::ZeroVector);
        }
        FRHIResourceCreateInfo CreateInfo;
        CreateInfo.ResourceArray = &RayDirs;
        FStructuredBufferRHIRef RayTracingDirs = RHICreateStructuredBuffer(
            sizeof(RayDirs[0]), RayDirs.GetResourceDataSize(), BUF_Static | BUF_ShaderResource, CreateInfo);
        FShaderResourceViewRHIRef RayDirsSRV = RHICreateShaderResourceView(RayTracingDirs);
        TResourceArray<FVector4> RefDef;
        for (const auto& ref : proxy->ComponentData.RefDef)
        {
            RefDef.Add(ref);
        }
        CreateInfo.ResourceArray = &RefDef;
        FStructuredBufferRHIRef RayTracingRefDef = RHICreateStructuredBuffer(
            sizeof(RefDef[0]), RefDef.GetResourceDataSize(), BUF_Static | BUF_ShaderResource, CreateInfo);
        FShaderResourceViewRHIRef RefDefSRV = RHICreateShaderResourceView(RayTracingRefDef);
        RayGenParameters->RayDirs = RayDirsSRV;
        RayGenParameters->RefDef = RefDefSRV;
        RayGenParameters->MirrorMatrix =
            proxy->ComponentData.symmetryPose.ToMatrixWithScale() * RayGenParameters->CameraMatrix;
        RayGenParameters->MirrorThreshold = proxy->ComponentData.mirrorThreshold;
        RayGenParameters->f_noise_dev = proxy->ComponentData.f_noise_dev;
        RayGenParameters->f_accuracy = proxy->ComponentData.f_accuracy;
        RayGenParameters->f_rainfall = proxy->ComponentData.f_rainfall;
        RayGenParameters->f_snowfall_range = proxy->ComponentData.f_snowfall_range;
        RayGenParameters->f_snowfall_prob = proxy->ComponentData.f_snowfall_prob;
        RayGenParameters->f_fog = proxy->ComponentData.f_fog;
        RayGenParameters->t_intensity = proxy->ComponentData.t_intensity;
        FRDGTextureRef RTTexture = GraphBuilder.RegisterExternalTexture(proxy->RenderTarget);
        RayGenParameters->Output = GraphBuilder.CreateUAV(RTTexture);

        GraphBuilder.AddPass(RDG_EVENT_NAME("Lidar"), RayGenParameters, ERDGPassFlags::Compute,
            [RayGenParameters, RayGenShader, RayTracingSceneRHI, Pipeline, Extent](FRHICommandList& RHICmdList)
            {
                FRayTracingShaderBindingsWriter GlobalResources;
                SetShaderParameters(GlobalResources, RayGenShader, *RayGenParameters);

                // Dispatch rays using default shader binding table
                RHICmdList.RayTraceDispatch(Pipeline, RayGenShader.GetRayTracingShader(), RayTracingSceneRHI,
                    GlobalResources, Extent.X, Extent.Y);
            });
    }
    for (FRTXLidarSceneProxy* proxy : AllProxiesReadyForRender_RenderThread)
    {
        // Schedule a copy of the GPU texture to the CPU accessible GPU texture
        GraphBuilder.RHICmdList.CopyTexture(
            proxy->RenderTarget->GetShaderResourceRHI(), proxy->RenderTargetRHI, FRHICopyTextureInfo{});
    }
}
void FRTXLidarSceneProxy::ResetTextures_RenderThread(FRDGBuilder& GraphBuilder)
{
    float ClearColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
    AddClearUAVPass(
        GraphBuilder, GraphBuilder.CreateUAV(GraphBuilder.RegisterExternalTexture(RenderTarget)), ClearColor);
}

void FRTXLidarSceneProxy::ReallocateSurfaces_RenderThread(FRHICommandListImmediate& RHICmdList)
{
    // Render Result
    {
        FIntPoint ProxyDims = FIntPoint(ComponentData.Width, ComponentData.Height);
        EPixelFormat Format = ComponentData.RTPixelFormat;

        FPooledRenderTargetDesc Desc(FPooledRenderTargetDesc::Create2DDesc(ProxyDims, Format,
            FClearValueBinding::Transparent, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
#if ENGINE_MAJOR_VERSION < 5
        GRenderTargetPool.FindFreeElement(
            RHICmdList, Desc, RenderTarget, TEXT("LidarResult"), ERenderTargetTransience::NonTransient);
#else
        GRenderTargetPool.FindFreeElement(RHICmdList, Desc, RenderTarget, TEXT("LidarResult"));
#endif
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
URTXLidarComponent::URTXLidarComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bWantsInitializeComponent = true;
}

void URTXLidarComponent::InitializeComponent()
{
    Super::InitializeComponent();
    if (RTResult == nullptr)
    {
        RTResult = NewObject<UTextureRenderTarget2D>(this);
        RTResult->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
        RTResult->RenderTargetFormat = RTFormat;
        RTResult->bGPUSharedFlag = true;
        RTResult->InitAutoFormat(16, 16);
    }

    MarkRenderDynamicDataDirty();

    TransformUpdated.AddLambda(
        [this](USceneComponent* /*UpdatedComponent*/, EUpdateTransformFlags /*UpdateTransformFlags*/,
            ETeleportType /*Teleport*/) { MarkRenderDynamicDataDirty(); });
}
void URTXLidarComponent::BeginDestroy()
{
    Super::BeginDestroy();
    /*if (RTResult) {
        RTResult->ReleaseResource();
        RTResult = nullptr;
    }*/
}

bool URTXLidarComponent::GetCaptureData(TArray<FColor>& result)
{
    if (RTResult)
    {
        FTextureRenderTargetResource* Res = RTResult->GameThread_GetRenderTargetResource();
        if (Res)
        {
            FReadSurfaceDataFlags ReadPixelFlags(RCM_MinMax);
            return Res->ReadPixels(result, ReadPixelFlags);
        }
    }
    return false;
}

void URTXLidarComponent::InitViewParam(USceneCaptureComponent2D* Capture2D)
{
    if (Capture2D == nullptr || Capture2D->TextureTarget == nullptr)
        return;
    CaptureComponent2D = Capture2D;
    UpdateParam();
}

void URTXLidarComponent::UpdateParam()
{
    bool bDirty = false;
    if (CaptureComponent2D && RTResult)
    {
        FSceneViewStateInterface* ViewState = CaptureComponent2D->GetViewState(0);
        if (ViewState)
        {
            if (ViewKey != ViewState->GetViewKey())
            {
                ViewKey = ViewState->GetViewKey();
                bDirty = true;
            }
        }
        int RetCount = rayDegree.Num();
        if (RetCount > 1)
        {
            int SizeX = FMath::CeilToInt(FMath::Sqrt(RetCount));
            int SizeY = RetCount / SizeX;
            if (SizeX * SizeY < RetCount)
            {
                SizeY += 1;
            }
            if (RTResult->SizeX != SizeX || RTResult->SizeY != SizeY)
            {
                RTResult->ResizeTarget(SizeX, SizeY);
                bDirty = true;
            }
        }
        FTextureRenderTargetResource* FRTR = RTResult->GameThread_GetRenderTargetResource();
        if (FRTR && RTResultRHI != FRTR->GetRenderTargetTexture())
        {
            RTResultRHI = FRTR->GetRenderTargetTexture();
            bDirty = true;
        }
        if (bDirty && RTResultRHI.IsValid())
        {
            MarkRenderDynamicDataDirty();
        }
    }
}

void URTXLidarComponent::UpdateRenderThreadData()
{
    // Send command to the rendering thread to update the transform and other parameters
    if (SceneProxy && RTResultRHI.IsValid())
    {
        // Update the volume component's data
        FRTXLidarSceneProxy::FComponentData ComponentData;
        ComponentData.RayDirs = rayDegree;
        ComponentData.RefDef = refDef;
        ComponentData.Width = RTResult->SizeX;
        ComponentData.Height = RTResult->SizeY;
        ComponentData.RTPixelFormat = RTResult->GetFormat();
        ComponentData.symmetryPose = symmetryPose;
        ComponentData.mirrorThreshold = mirrorThreshold;
        ComponentData.f_noise_dev = f_noise_dev;
        ComponentData.f_accuracy = f_accuracy;
        ComponentData.f_rainfall = f_rainfall;
        ComponentData.f_snowfall_range = f_snowfall_range;
        ComponentData.f_snowfall_prob = f_snowfall_prob;
        ComponentData.f_fog = f_fog;
        ComponentData.t_intensity = t_intensity;
        FRTXLidarSceneProxy* proxy = SceneProxy.Get();
        if (!proxy)
        {
            return;
        }
        FTexture2DRHIRef RenderTargetRHI = RTResultRHI;
        uint32 CaptureViewKey = ViewKey;

        ENQUEUE_RENDER_COMMAND(UpdateRTXLidarCommand)
        (
            [proxy, ComponentData, RenderTargetRHI, CaptureViewKey](FRHICommandListImmediate& RHICmdList)
            {
                FRDGBuilder GraphBuilder(RHICmdList);

                bool needReallocate = proxy->ViewKey != CaptureViewKey ||
                                      proxy->ComponentData.Width != ComponentData.Width ||
                                      proxy->ComponentData.Height != ComponentData.Height ||
                                      proxy->ComponentData.RTPixelFormat != ComponentData.RTPixelFormat;
                // set the data
                proxy->ComponentData = ComponentData;
                proxy->RenderTargetRHI = RenderTargetRHI;
                proxy->ViewKey = CaptureViewKey;

                if (needReallocate)
                {
                    proxy->ReallocateSurfaces_RenderThread(RHICmdList);
                    proxy->ResetTextures_RenderThread(GraphBuilder);
                    FRTXLidarSceneProxy::AllProxiesReadyForRender_RenderThread.Add(proxy);
                }
                GraphBuilder.Execute();
            });
    }
}

void URTXLidarComponent::SendRenderDynamicData_Concurrent()
{
    Super::SendRenderDynamicData_Concurrent();
    UpdateRenderThreadData();
}

void URTXLidarComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
    Super::CreateRenderState_Concurrent(Context);

#if WITH_EDITOR
    if (!GetOwner()->IsTemporarilyHiddenInEditor())
#endif
    {
        SceneProxy = MakeShared<FRTXLidarSceneProxy>(GetScene());
        UpdateRenderThreadData();
    }
}

void URTXLidarComponent::DestroyRenderState_Concurrent()
{
    Super::DestroyRenderState_Concurrent();

    FRTXLidarSceneProxy* LidarProxy = SceneProxy.Get();
    ENQUEUE_RENDER_COMMAND(DeleteProxy)([this](FRHICommandListImmediate& RHICmdList) { SceneProxy.Reset(); });

    // wait for the above command to finish, so we know we got the load context if present
    FlushRenderingCommands();
}
#if RHI_RAYTRACING
FDelegateHandle PrepareRayTracingHandle;
void PrepareRayTracingShaders(const FViewInfo& View, TArray<FRHIRayTracingShader*>& OutRayGenShaders)
{
    const auto FeatureLevel = GMaxRHIFeatureLevel;
    auto ShaderMap = GetGlobalShaderMap(FeatureLevel);

    TShaderRef<FRayTracingLidarRGS> RayGenerationShader = ShaderMap->GetShader<FRayTracingLidarRGS>();

    OutRayGenShaders.Add(RayGenerationShader.GetRayTracingShader());
}
#endif    // RHI_RAYTRACING
void URTXLidarComponent::Startup()
{
#if RHI_RAYTRACING
    FGlobalIlluminationPluginDelegates::FPrepareRayTracing& PRTDelegate =
        FGlobalIlluminationPluginDelegates::PrepareRayTracing();
    PrepareRayTracingHandle = PRTDelegate.AddStatic(PrepareRayTracingShaders);

    FGlobalIlluminationPluginDelegates::FRenderDiffuseIndirectLight& RDILDelegate =
        FGlobalIlluminationPluginDelegates::RenderDiffuseIndirectLight();
    FRTXLidarSceneProxy::RenderDiffuseIndirectLightHandle =
        RDILDelegate.AddStatic(FRTXLidarSceneProxy::RenderDiffuseIndirectLight_RenderThread);
#endif    // RHI_RAYTRACING
}

void URTXLidarComponent::Shutdown()
{
#if RHI_RAYTRACING
    FGlobalIlluminationPluginDelegates::FPrepareRayTracing& PRTDelegate =
        FGlobalIlluminationPluginDelegates::PrepareRayTracing();
    check(PrepareRayTracingHandle.IsValid());
    PRTDelegate.Remove(PrepareRayTracingHandle);
    FGlobalIlluminationPluginDelegates::FRenderDiffuseIndirectLight& RDILDelegate =
        FGlobalIlluminationPluginDelegates::RenderDiffuseIndirectLight();
    check(FRTXLidarSceneProxy::RenderDiffuseIndirectLightHandle.IsValid());
    RDILDelegate.Remove(FRTXLidarSceneProxy::RenderDiffuseIndirectLightHandle);
#endif    // RHI_RAYTRACING
}

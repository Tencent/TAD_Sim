// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FovDepthBuffer.generated.h"

struct FovViewConfiguration
{
    FVector position;
    FRotator rotator;
    float hfov;
    float vfov;
    // float tvfov;
    int w, h;
};

UCLASS()
class DISPLAY_API AFovDepthBuffer : public AActor
{
    GENERATED_BODY()
public:
    // Sets default values for this actor's properties
    AFovDepthBuffer();
    ~AFovDepthBuffer();

    virtual bool SetCamera(const FovViewConfiguration& fov);
    bool GetCaptureImage(TArray<FColor>& BitMap);

    bool GetDepth(const TArray<FColor>& BitMap, float pitch, float yaw, float& dis, float& normal, int* tag = 0);

public:
    FString ppMaterialPath;
    float fMaxDepth = 2000;
    float fResScale = 32;
    class UMaterial* depthMaterial;
    class UMaterialInstanceDynamic* depthProcess;
    FovViewConfiguration fovCfg;
    class USceneCaptureComponent2D* captureComponent;
    class UTextureRenderTarget2D* renderTarget2D;
    TSharedPtr<class UTexJpeg> texJpg;
    float fx = 0, fy = 0;
};

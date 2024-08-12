// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DepthCamera.generated.h"

struct CameraSensorViewConfiguration
{
    FVector position;
    FRotator rotator;
    float hfov;
    float vfov;
    // float tvfov;
    int w, h;
};

UCLASS()
class DISPLAY_API ADepthLidarBuffer : public AActor
{
    GENERATED_BODY()
public:
    // Sets default values for this actor's properties
    ADepthLidarBuffer();
    ~ADepthLidarBuffer();

    virtual bool SetCamera(const CameraSensorViewConfiguration& camCfg);
    bool GetCaptureImage(TArray<FColor>& BitMap);
    bool GetCaptureImage(uint8_t*& imgbuf);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    CameraSensorViewConfiguration camConfig;
    FString ppMaterialPath;
    float fMaxDepth;
    float fResScale;
    class UMaterial* depthMaterial;
    class UMaterialInstanceDynamic* depthProcess;

    class USceneCaptureComponent2D* captureComponent;
    class UTextureRenderTarget2D* renderTarget2D;
    TSharedPtr<class UTexJpeg> texJpg;
    uint8_t* imgGpuTmp = 0;
};

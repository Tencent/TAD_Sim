// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvDepthTrace.generated.h"

UCLASS()
class DISPLAY_API AEnvDepthTrace : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AEnvDepthTrace();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere)
    class USceneCaptureComponent2D* CaptureComponent = NULL;

    UPROPERTY(EditAnywhere)
    class UTextureRenderTarget2D* DepthTraceRT = NULL;
};

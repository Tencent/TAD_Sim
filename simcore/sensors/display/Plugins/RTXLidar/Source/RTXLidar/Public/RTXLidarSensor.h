// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTXLidarSensor.generated.h"

class URTXLidarComponent;

UCLASS(HideCategories = (Navigation, Physics, Collision, Rendering, Tags, Cooking, Replication, Input, Actor, HLOD,
           Mobile, LOD),
    meta = (DisplayName = "RTX Lidar Sensor"))
class RTXLIDAR_API ARTXLidarSensor : public AActor
{
    GENERATED_UCLASS_BODY()

public:
    void SetYawPitchArray(const TArray<FVector2D> angles);
    void SetRefArray(const TArray<FVector4> ref);
    bool GetCaptureData(TArray<FColor>& result);
    void SetInstallPose(const FTransform& pose);
    void SetMirrorThreshold(float th);

    void SetLidarIntensity(float i);
    void SetLidarRain(float w);
    void SetLidarSnow(float s);
    void SetLidarFog(float f);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering", meta = (AllowPrivateAccess = "true"));
    class URTXLidarComponent* RTXComponent;
    /** Scene capture component. */
    UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class USceneCaptureComponent2D* CaptureComponent2D;

private:
    UPROPERTY(transient)
    UTextureRenderTarget2D* RTTemp = nullptr;
};

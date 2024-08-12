// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Sensors/SensorActor.h"
#include "Camera/CameraTypes.h"
#include "Materials/Material.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "SharedMemoryWriter.h"
#ifdef _MSC_VER
#else
// #include "rdmacopy/client.h"
#endif
#include "FisheyeSensor.generated.h"

USTRUCT()
struct FFisheyeConfig : public FSensorConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<double> intrinsic_Matrix;
    UPROPERTY()
    TArray<double> distortion_Parameters;
    UPROPERTY()
    double res_Horizontal = 0;
    UPROPERTY()
    double res_Vertical = 0;
    UPROPERTY()
    double vignette_Intensity = 0.4;
    UPROPERTY()
    double noise_Intensity = 0;
    UPROPERTY()
    double motionBlur_Amount = 0;
    UPROPERTY()
    double blur_Intensity = 0;
    UPROPERTY()
    int color_gray = 0;
    UPROPERTY()
    double LensFlares = 0;

    UPROPERTY()
    double Exquisite = 0;
    UPROPERTY()
    double Bloom = 0.1;
    UPROPERTY()
    int Exposure = 0;
    UPROPERTY()
    double Compensation = 1;
    UPROPERTY()
    double ShutterSpeed = 60;
    UPROPERTY()
    double ISO = 100;
    UPROPERTY()
    double Aperture = 4;
    UPROPERTY()
    double ColorTemperature = 6500;
    UPROPERTY()
    double WhiteHint = 0;
    UPROPERTY()
    double Transmittance = 98;
};

USTRUCT()
struct FFisheyeInput : public FSensorInput
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FFisheyeOutput : public FSensorOutput
{
    GENERATED_BODY()
public:
    std::vector<uint8> buffer;
};

/**
 *
 */
UCLASS()
class DISPLAY_API AFisheyeSensor : public ASensorActor
{
    GENERATED_BODY()
public:
    AFisheyeSensor();
    ~AFisheyeSensor();

    virtual bool Init(const FSensorConfig& _Config);
    virtual ISimActorInterface* Install(const FSensorConfig& _Config);
    virtual void Update(const FSensorInput& _Input, FSensorOutput& _Output);
    virtual bool Save();

protected:
    TArray<class ADrawBatch*> labelTypeArry;
    TArray<FName> onlyShowTypeArry;
    TArray<FName> ignoreTypeArry;

    bool bHasFilterListSet = false;

public:
    UMaterialInstanceDynamic* cameraPostProcess;
    UPROPERTY(/*BlueprintReadOnly*/)
    FFisheyeConfig sensorConfig;

    class USceneCaptureComponent2D* captureComponent2D = NULL;
    class UTextureRenderTarget2D* renderTarget2D = NULL;

    class USceneCaptureComponentCube* captureComponentCube = NULL;
    class UTextureRenderTargetCube* renderTargetCube = NULL;

    class UCineCameraComponent* previewComponent = NULL;

    // CaptureParameter
    FString imageName = TEXT("Fisheye");
    EImageFormat imageFormat = EImageFormat::JPEG;
    int32 imageQuality = 85;
    FFisheyeOutput dataBuf;
    //
    double timeStamp = -10000;
    double lastTimeStamp = 0;
    double targetGamma = 2.2;
    TSharedPtr<class SharedMemoryWriter> sharedWriter;
    TSharedPtr<class SharedMemoryWriter> sharedWriterGpu;
    bool public_msg = false;

    // cuda jpg
    TSharedPtr<class UTexJpeg> texJpg;

    void SetPostProcessSettings(FPostProcessSettings& PostProcessSettings, float screen_scale = 1);
};

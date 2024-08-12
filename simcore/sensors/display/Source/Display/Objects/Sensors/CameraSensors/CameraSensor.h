// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Sensors/SensorActor.h"
#include "Camera/CameraTypes.h"
#include "Materials/Material.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "SharedMemoryWriter.h"
#include "CameraSensor.generated.h"

UENUM(BlueprintType)
enum EParamTypeEnum
{
    PT_Int = 0 UMETA(DisplayName = "Int"),
    PT_Fov = 1 UMETA(DisplayName = "Fov"),
    PT_Ccd = 2 UMETA(DisplayName = "Ccd")
};

// UENUM()
// enum class EParamTypeEnum : uint8
//{
//     PT_Fov,
//     PT_Ccd,
//     PT_Int
// };

USTRUCT()
struct FCameraConfig : public FSensorConfig
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int paraType = 0;
    UPROPERTY()
    double ccd_Width = 0;
    UPROPERTY()
    double ccd_Height = 0;
    UPROPERTY()
    double focal_Length = 0;
    UPROPERTY()
    TArray<double> intrinsic_Matrix;
    UPROPERTY()
    TArray<double> distortion_Parameters;
    UPROPERTY()
    double fov_Horizontal = 60;
    UPROPERTY()
    double fov_Vertical = 60;
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
    double Bloom = 0.675;
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
struct FCameraInput : public FSensorInput
{
    GENERATED_BODY()
public:
};

USTRUCT()
struct FCameraOutput : public FSensorOutput
{
    GENERATED_BODY()
public:
    std::vector<uint8> buffer;
};

/**
 *
 */
UCLASS(config = game)
class DISPLAY_API ACameraSensor : public ASensorActor
{
    GENERATED_BODY()
public:
    ACameraSensor();
    ~ACameraSensor();

    virtual bool Init(const FSensorConfig& _Config);
    virtual ISimActorInterface* Install(const FSensorConfig& _Config);
    virtual void Update(const FSensorInput& _Input, FSensorOutput& _Output);
    virtual bool Save();

protected:
    FString str_PostProcess;

    // class ASimPlayerController* GetPlayerController();

    TArray<class ADrawBatch*> labelTypeArry;
    TArray<FName> onlyShowTypeArry;
    TArray<FName> ignoreTypeArry;

    bool bHasFilterListSet = false;
    // void InitCaptureOnlyShowList();
    // void InitCaptureIgnoreList();

public:
    UMaterialInstanceDynamic* mid_CameraPostProcess;

    int id;
    UPROPERTY(/*BlueprintReadOnly*/)
    FCameraConfig sensorConfig;

    UPROPERTY(EditAnywhere)
    class USceneCaptureComponent2D* captureComponent = NULL;

    UPROPERTY(EditAnywhere)
    class UTextureRenderTarget2D* renderTarget2D = NULL;

    // CaptureParameter
    FString savePath;
    FString imageName = TEXT("Camera");
    FVector2D imageRes0 = FVector2D(500, 500);
    FVector2D imageRes = FVector2D(500, 500);
    EImageFormat imageFormat = EImageFormat::JPEG;
    int32 imageQuality = 85;
    FCameraOutput dataBuf;
    double targetGamma = 2.2;
    //
    double timeStamp = -10000;
    double lastTimeStamp = 0;
    double frequency = 10;
    bool public_msg = false;

    UPROPERTY(BlueprintReadOnly)
    class UCineCameraComponent* previewComponent = NULL;

    class UTexture2D* texDistortion = NULL;

    void SetPostProcessSettings(const FCameraConfig& NewCameraSensorConfig, FPostProcessSettings& PostProcessSettings,
        float screen_scale = 1.0f);

    TSharedPtr<class SharedMemoryWriter> sharedWriter;
    TSharedPtr<class SharedMemoryWriter> sharedWriterGpu;

    // cuda jpg
    TSharedPtr<class UTexJpeg> texJpg;
};

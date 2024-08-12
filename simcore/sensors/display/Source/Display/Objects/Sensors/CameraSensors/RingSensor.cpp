#include "RingSensor.h"
#include "HAL/PlatformFilemanager.h"
#include "Framework/SaveDataThread.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "CineCameraComponent.h"
#include "TexJpeg.h"
#include "Framework/DisplayGameInstance.h"

ARingSensor::ARingSensor()
{
    str_PostProcess = TEXT("Material'/Game/SensorSim/Camera/Material/Mat_Camera_Capture_ring.Mat_Camera_Capture_ring'");
}

bool ARingSensor::Init(const FSensorConfig& _Config)
{
    ASensorActor::Init(_Config);
    // FSensorConfig* NewConfigPtr = &_Config;
    // FCameraConfig* NewCameraSensorConfig = static_cast<FCameraConfig*>(NewConfigPtr);
    const FCameraConfig* NewCameraSensorConfig = Cast_Sim<const FCameraConfig>(_Config);

    sensorConfig = *NewCameraSensorConfig;
    if (!NewCameraSensorConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Cant Cast to FCameraConfig!"));
        return false;
    }

    // ID
    id = NewCameraSensorConfig->id;
    // frequency
    frequency = NewCameraSensorConfig->frequency;
    // Save path
    savePath = NewCameraSensorConfig->savePath;
    if (!savePath.IsEmpty() && !FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*savePath))
    {
        // UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Generate savePath."));
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*savePath);
    }
    sensorConfig = *NewCameraSensorConfig;
    imageRes.X = NewCameraSensorConfig->res_Horizontal;
    imageRes.Y = NewCameraSensorConfig->res_Vertical;

    {
        UMaterial* NewMat = LoadObject<UMaterial>(NULL, *str_PostProcess);
        if (!NewMat)
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Cant get Material!"));
            return false;
        }
        mid_CameraPostProcess = UMaterialInstanceDynamic::Create(NewMat, this);
        if (!mid_CameraPostProcess)
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent: Cant get MaterialInstanceDynamic!"));
            return false;
        }
    }

    double hfov = FMath::DegreesToRadians(sensorConfig.fov_Horizontal);
    double vfov = FMath::DegreesToRadians(sensorConfig.fov_Vertical);
    if (sensorConfig.paraType != EParamTypeEnum::PT_Fov || hfov <= 0 || vfov <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraSensor: fx fy errro!"));
        return false;
    }

    double fx = 0.5 * sensorConfig.res_Horizontal / FMath::Tan(hfov * 0.5);
    double fy = 0.5 * sensorConfig.res_Vertical / FMath::Tan(vfov * 0.5);
    double ovfov = FMath::Atan(FMath::Tan(vfov * 0.5f) / FMath::Cos(hfov * 0.5f)) * 2.0f;
    double ofy = 0.5 * NewCameraSensorConfig->res_Vertical / FMath::Tan(ovfov * 0.5);
    double of = fmin(fx, ofy);
    double NewFov_H = FMath::Atan(0.5 * NewCameraSensorConfig->res_Horizontal / of) * 2;
    double NewFov_V = FMath::Atan(0.5 * NewCameraSensorConfig->res_Vertical / of) * 2;
    double fov_scale = FMath::Max(fx / of, fy / of);

    UE_LOG(LogTemp, Log, TEXT("RingSensor: hfov=%f, vfov=%f, of=%f, nhfov=%f, nvhov=%f, w=%f, h=%f"), hfov, vfov, of,
        NewFov_H, NewFov_V, NewCameraSensorConfig->res_Horizontal, NewCameraSensorConfig->res_Vertical);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("hfov")), hfov);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("vfov")), vfov);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("nhfov")), NewFov_H);
    mid_CameraPostProcess->SetScalarParameterValue(FName(TEXT("nvfov")), NewFov_V);

    float flen = 100.f;
    previewComponent->CurrentFocalLength = flen;
    previewComponent->LensSettings.MinFocalLength = flen;
    previewComponent->LensSettings.MaxFocalLength = flen;
    previewComponent->LensSettings.MinFStop = 7.0f;
    previewComponent->LensSettings.MaxFStop = 7.0f;
    previewComponent->Filmback.SensorWidth = FMath::Tan(NewFov_H * 0.5) * 2 * flen;
    previewComponent->Filmback.SensorHeight = FMath::Tan(NewFov_V * 0.5) * 2 * flen;
    SetPostProcessSettings(
        *NewCameraSensorConfig, previewComponent->PostProcessSettings, FMath::Max(1.0, fov_scale * 1.05));
    previewComponent->PostProcessSettings.AddBlendable(mid_CameraPostProcess, 1);

    if (GetDisplayInstance()->nHILpos.X < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RingSensor: SIL MODEL. TODO"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RingSensor: HIL MODEL."));
    }

    return true;
}

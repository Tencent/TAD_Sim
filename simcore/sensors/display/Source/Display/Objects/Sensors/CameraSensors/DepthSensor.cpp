#include "DepthSensor.h"
#include "HAL/PlatformFilemanager.h"
#include "Framework/SaveDataThread.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "CineCameraComponent.h"
#include "Misc/FileHelper.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/DisplayGameInstance.h"

ADepthSensor::ADepthSensor()
{
    str_PostProcess =
        TEXT("Material'/Game/SensorSim/Camera/Material/Mat_Camera_Capture_depth.Mat_Camera_Capture_depth'");

    targetGamma = 1;

    imageName = TEXT("Depth");
    imageFormat = EImageFormat::EXR;
}
ADepthSensor::~ADepthSensor()
{
}

bool ADepthSensor::Init(const FSensorConfig& _Config)
{
    if (!ACameraSensor::Init(_Config))
        return false;

    captureComponent->PostProcessSettings.bOverride_AutoExposureBias = false;
    captureComponent->PostProcessSettings.bOverride_ColorGamma = true;
    captureComponent->PostProcessSettings.ColorGamma = FVector4(1.0, 1.0, 1.0, 1.0);
    previewComponent->PostProcessSettings.bOverride_AutoExposureBias = false;
    previewComponent->PostProcessSettings.bOverride_ColorGamma = true;
    previewComponent->PostProcessSettings.ColorGamma = FVector4(1.0, 1.0, 1.0, 1.0);
    return true;
}
bool ADepthSensor::Save()
{
    if (savePath.IsEmpty())
    {
        return true;
    }
    // UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent INFO: SaveData!"));
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(imageFormat);

    TArray<float> BitMap;
    BitMap.SetNum(dataBuf.buffer.size() / 4);
    for (int i = 0; i < BitMap.Num(); i++)
    {
        BitMap[i] =
            (dataBuf.buffer[i * 4 + 2] * 65536.0 + dataBuf.buffer[i * 4 + 1] * 256.0 + dataBuf.buffer[i * 4]) * 0.0001;
    }

    if (ImageWrapper->SetRaw(
            BitMap.GetData(), sizeof(float) * imageRes.X * imageRes.Y, imageRes.X, imageRes.Y, ERGBFormat::Gray, 32))
    {
        if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*savePath))
        {
            FString SaveDir = savePath + imageName + TEXT("_") + FString::FromInt(id) + TEXT("_") +
                              FString::FromInt(timeStamp) + TEXT(".") +
                              TEXT("exr") /*FString(GETENUMSTRING("EImageFormat", imageFormat)).ToLower()*/;
            // UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent INFO: SaveDir is  %s"), *SaveDir);

            GetDisplayInstance()->GetSaveDataHandle()->SaveJPG(ImageWrapper->GetCompressed(), SaveDir);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent WARN: Directory dont Exists!"));
        }
    }
    return true;
}

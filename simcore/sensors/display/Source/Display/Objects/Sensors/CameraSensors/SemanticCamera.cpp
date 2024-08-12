#include "SemanticCamera.h"
#include "HAL/PlatformFilemanager.h"
#include "Framework/SaveDataThread.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Framework/DisplayGameInstance.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"

ASemanticCamera::ASemanticCamera()
{
    str_PostProcess = TEXT("Material'/Game/SensorSim/Camera/Material/Mat_Camera_Stencil.Mat_Camera_Stencil'");

    imageName = TEXT("Semantic");
    imageFormat = EImageFormat::PNG;
}
ASemanticCamera::~ASemanticCamera()
{
}
bool ASemanticCamera::Save()
{
    if (savePath.IsEmpty())
    {
        return true;
    }
    // UE_LOG(LogTemp, Warning, TEXT("CameraSensorComponent INFO: SaveData!"));
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(imageFormat);

    if (ImageWrapper->SetRaw(dataBuf.buffer.data(), sizeof(FColor) * imageRes.X * imageRes.Y, imageRes.X, imageRes.Y,
            ERGBFormat::BGRA, 8))
    {
        if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*savePath))
        {
            FString SaveDir = savePath + imageName + TEXT("_") + FString::FromInt(id) + TEXT("_") +
                              FString::FromInt(timeStamp) + TEXT(".") +
                              TEXT("png") /*FString(GETENUMSTRING("EImageFormat", imageFormat)).ToLower()*/;
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

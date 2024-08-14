// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include <vector>
#include <string>

#ifdef FIND_CUDA

#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"
#include <driver_types.h>
#include <nvjpeg.h>
#include <nppdefs.h>
#include "Windows/HideWindowsPlatformTypes.h"

#else

#include <driver_types.h>
#include <nvjpeg.h>
#include <nppdefs.h>

#endif
#endif
// #include "TexJpeg.generated.h"

// UCLASS(Category = "CudaResource|TexJpeg")
class CUDARESOURCE_API UTexJpeg    // : public UObject
{
    // GENERATED_BODY()
public:
    // Sets default values for this component's properties
    UTexJpeg(int gpuid = 0);
    ~UTexJpeg();

    bool InitResources(bool jpeg = false, bool share = false, int w = 0, int h = 0, const FString& stylemode = "");
    void FreeResource();

    bool Copy2Cuda(bool sync = false);

    bool JpegEncoding(TArray64<uint8_t>& jpegbuf);

    bool Raw(std::vector<uint8>& rawbuf);
    uint8_t* GetRawPtr(size_t& len);

    /// <summary>
    ///
    /// </summary>
    /// <param name="handle">ptr to save ipc handle(mem Handle and event Handle)</param>
    /// <returns></returns>
    bool IpcCreate(uint8_t* handle);
    bool IpcShare(uint8_t* handle);

    // UPROPERTY(EditAnywhere)
    class UTextureRenderTarget2D* texRT = NULL;

#ifdef FIND_CUDA

protected:
    // virtual void BeginDestroy() override;

    bool InitRTResource();

private:
    int gpu_id = 0;
    FDelegateHandle postCopyRTHandle;
    /** Cuda resource used for copying from SDK to render API */
    cudaGraphicsResource_t cudaResource = NULL;
    cudaArray_t TransitionArray = NULL;
    cudaMipmappedArray_t mipmap = NULL;
    cudaExternalMemory_t extMem = NULL;
    int imgWidth = 0;
    int imgHeight = 0;
    int imgWidth0 = 0;
    int imgHeight0 = 0;
    uint8_t* imgBufferBGRA = 0;
    uint8_t* imgBufferBGRA0 = 0;
    cudaStream_t stream = NULL;

    // jpeg
    NppStreamContext nppStreamContext = {};
    nvjpegImage_t imageJpg = {};
    nvjpegHandle_t handleJpg = NULL;
    nvjpegEncoderState_t enStateJpg = NULL;
    nvjpegEncoderParams_t enParamsJpg = NULL;
    // ipc
    cudaIpcMemHandle_t memIpcHandle = {};
    cudaIpcEventHandle_t eventIpcHandle = {};
    cudaEvent_t eventIpc = NULL;

    // style
    TSharedPtr<class TADSimTRT> trtStyle;

#endif
};

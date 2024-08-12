// Fill out your copyright notice in the Description page of Project Settings.

#include "TexJpeg.h"

#include "CoreMinimal.h"

#ifdef FIND_CUDA

#include <memory.h>
#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "BoundShaderStateCache.h"
#include "Misc/ScopeRWLock.h"
#include "DynamicRHI.h"
#include "RenderResource.h"
#include "RHICommandList.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineGlobals.h"
#include "Modules/ModuleManager.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "tadsim_trt_engine.h"
#include "CudaCheck.h"

#if PLATFORM_WINDOWS

#include "D3D11RHI.h"
#include "D3D11RHIBasePrivate.h"
#include "D3D11StateCachePrivate.h"
#include "D3D11Util.h"
#include "D3D11State.h"
#include "D3D11Resources.h"
#include "d3d12.h"
#include "D3D12RHI.h"
#include "D3D12RHICommon.h"
#include "D3D12State.h"
#include "D3D12RHIPrivate.h"
#include "D3D12Resources.h"
#include "D3D12Query.h"
#include "D3D12Device.h"
#include "D3D12Texture.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <cuda.h>
#include <cuda_d3d11_interop.h>
#include <cuda_runtime.h>
#include <nvjpeg.h>
#include <npp.h>
#include "Windows/HideWindowsPlatformTypes.h"

#else

#include "VulkanLinuxPlatform.h"
#include "vulkan.h"
#include "vulkan_core.h"
#include "VulkanResources.h"
#include "VulkanCommon.h"
#include "VulkanRHIPrivate.h"
#include "VulkanDevice.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <npp.h>
#include <nvjpeg.h>

#endif

// CUDA Runtime error messages

#undef checkCudaErrors
#undef checkCudaErrors_TF
#define checkCudaErrors(val) cuda_check((val), "TexJpeg", __LINE__)
#define checkCudaErrors_TF(val)                  \
    if (!cuda_check((val), "TexJpeg", __LINE__)) \
        return false;
// Sets default values for this component's properties
UTexJpeg::UTexJpeg(int gpuid) : gpu_id(gpuid)
{
#if PLATFORM_WINDOWS
    FString RHIName = GDynamicRHI->GetName();
    if (RHIName == TEXT("D3D12"))
    {
        int num_cuda_devices = 0;
        checkCudaErrors(cudaGetDeviceCount(&num_cuda_devices));

        if (!num_cuda_devices)
        {
            UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] no cuda device"));
        }
        else
        {
            ID3D12Device* D3DDevice = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
            LUID m_dx12deviceluid = D3DDevice->GetAdapterLuid();

            for (int devId = 0; devId < num_cuda_devices; devId++)
            {
                cudaDeviceProp devProp{};
                checkCudaErrors(cudaGetDeviceProperties(&devProp, devId));
                const auto cmp1 =
                    memcmp(&m_dx12deviceluid.LowPart, devProp.luid, sizeof(m_dx12deviceluid.LowPart)) == 0;
                const auto cmp2 = memcmp(&m_dx12deviceluid.HighPart, devProp.luid + sizeof(m_dx12deviceluid.LowPart),
                                      sizeof(m_dx12deviceluid.HighPart)) == 0;
                if (cmp1 && cmp2)
                {
                    gpu_id = devId;
                    UE_LOG(
                        LogTemp, Log, TEXT("[UTexJpeg] CUDA Device Used [%d] %s"), devId, ANSI_TO_TCHAR(devProp.name));
                    break;
                }
            }
        }
    }
#endif
}

UTexJpeg::~UTexJpeg()
{
    FreeResource();
    UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] destroy"));
}

void UTexJpeg::FreeResource()
{
    if (cudaResource)
        checkCudaErrors(cudaGraphicsUnregisterResource(cudaResource));
    cudaResource = nullptr;
    if (mipmap)
    {
        checkCudaErrors(cudaFreeMipmappedArray(mipmap));
    }
    mipmap = NULL;

    if (enParamsJpg)
        checkCudaErrors(nvjpegEncoderParamsDestroy(enParamsJpg));
    enParamsJpg = NULL;

    if (enStateJpg)
        checkCudaErrors(nvjpegEncoderStateDestroy(enStateJpg));
    enStateJpg = NULL;

    if (handleJpg)
        checkCudaErrors(nvjpegDestroy(handleJpg));
    handleJpg = NULL;

    if (stream)
        checkCudaErrors(cudaStreamDestroy(stream));
    stream = NULL;

    if (eventIpc)
    {
        if (imgBufferBGRA0)
        {
            checkCudaErrors(cudaIpcCloseMemHandle(imgBufferBGRA0));
        }
        checkCudaErrors(cudaEventDestroy(eventIpc));
        eventIpc = nullptr;
    }

    if (imgBufferBGRA0 != imgBufferBGRA)
        checkCudaErrors(cudaFree(imgBufferBGRA0));
    imgBufferBGRA0 = NULL;

    if (imgBufferBGRA)
        checkCudaErrors(cudaFree(imgBufferBGRA));
    imgBufferBGRA = NULL;

    for (int i = 0; i < 4; i++)
    {
        if (imageJpg.channel[i])
            nppiFree(imageJpg.channel[i]);
        imageJpg.channel[i] = NULL;
    }
    TransitionArray = NULL;

    UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] free"));
}

bool UTexJpeg::InitResources(bool jpeg, bool share, int w, int h, const FString& stylemode)
{
    FreeResource();
    if (!texRT)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] rt is null"));
        return false;
    }
    imgWidth = texRT->SizeX;
    imgHeight = texRT->SizeY;
    imgWidth0 = w > 0 ? w : imgWidth;
    imgHeight0 = h > 0 ? h : imgHeight;

    checkCudaErrors_TF(cudaSetDevice(gpu_id));
    checkCudaErrors_TF(cudaStreamCreate(&stream));
    checkCudaErrors_TF(cudaMalloc(&imgBufferBGRA, imgWidth * imgHeight * 4 + 1024));
    if (imgWidth0 != imgWidth || imgHeight0 != imgHeight)
    {
        checkCudaErrors_TF(cudaMalloc(&imgBufferBGRA0, imgWidth0 * imgHeight0 * 4 + 1024));
    }
    else
    {
        imgBufferBGRA0 = imgBufferBGRA;
    }

    if (jpeg)
    {
        int32 imageQuality = 85;
        GConfig->GetInt(TEXT("Sensor"), TEXT("JpegQuality"), imageQuality, GGameIni);
        UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] JpegQuality is %d"), imageQuality);
        checkCudaErrors_TF(nvjpegCreateSimple(&handleJpg));
        checkCudaErrors_TF(nvjpegEncoderStateCreate(handleJpg, &enStateJpg, stream));
        checkCudaErrors_TF(nvjpegEncoderParamsCreate(handleJpg, &enParamsJpg, stream));
        checkCudaErrors_TF(nvjpegEncoderParamsSetQuality(enParamsJpg, imageQuality, stream));
        // nvjpegEncoderParamsSetOptimizedHuffman(enParamsJpg, 1, NULL);
        checkCudaErrors_TF(nvjpegEncoderParamsSetSamplingFactors(enParamsJpg, NVJPEG_CSS_444, stream));

        for (int i = 0; i < 4; i++)
        {
            int pitch;
            imageJpg.channel[i] = nppiMalloc_8u_C1(imgWidth0, imgHeight0, &pitch);
            imageJpg.pitch[i] = pitch;
        }

        checkCudaErrors_TF(nppGetStreamContext(&nppStreamContext));
        nppStreamContext.hStream = stream;
    }
    if (share)
    {
        checkCudaErrors_TF(cudaEventCreate(&eventIpc, cudaEventDisableTiming | cudaEventInterprocess));
    }
    if (!stylemode.IsEmpty())
    {
        FString device = TEXT("0");
        ;
        // if (!FParse::Value(FCommandLine::Get(), TEXT("-device="), device))
        //{
        //     device = TEXT("0");
        // }

        TADOnnxParams para;
        para.width = imgWidth;
        para.height = imgHeight;
        strcpy(para.onnxmodel, TCHAR_TO_ANSI(*stylemode));
        strcpy(para.cachetrt, TCHAR_TO_ANSI(*(stylemode + TEXT(".cachetrt.") + device)));
        trtStyle = MakeShared<TADSimTRT>(para, stream, gpu_id);
        UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] loaded style mode over"));
    }

    UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] succeed to init resource"));
    return InitRTResource();
}
bool UTexJpeg::InitRTResource()
{
    UTexJpeg* This = this;
    ENQUEUE_RENDER_COMMAND(RegisterResource)
    (
        [This](FRHICommandListImmediate& RHICmdList)

        {
            FString RHIName = GDynamicRHI->GetName();

#if PLATFORM_WINDOWS
            if (RHIName == TEXT("D3D11"))
            {
                FD3D11TextureBase* D3D11Texture =
                    GetD3D11TextureFromRHITexture(This->texRT->GetRenderTargetResource()->TextureRHI);
                if (D3D11Texture)
                {
                    if (checkCudaErrors(cudaGraphicsD3D11RegisterResource(
                            &(This->cudaResource), D3D11Texture->GetResource(), cudaGraphicsRegisterFlagsNone)))
                    {
                        if (checkCudaErrors(cudaGraphicsMapResources(1, &(This->cudaResource), This->stream)))
                        {
                            if (checkCudaErrors(cudaGraphicsSubResourceGetMappedArray(
                                    &(This->TransitionArray), This->cudaResource, 0, 0)))
                            {
                                UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] succeed to init cuda resource"));
                                return;
                            }
                        }
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] Failed to GetD3D11TextureFromRHITexture"));
                }
            }
            // DX12
            else if (RHIName == TEXT("D3D12"))
            {
                cudaSetDevice(This->gpu_id);
                HANDLE sharedHandle = 0;
                FD3D12TextureBase* D3D12Texture = static_cast<FD3D12TextureBase*>(
                    This->texRT->GetRenderTargetResource()->TextureRHI->GetTextureBaseRHI());
                if (D3D12Texture)
                {
                    ID3D12Device* D3DDevice = static_cast<ID3D12Device*>(GDynamicRHI->RHIGetNativeDevice());
                    FD3D12Resource* D3D12Resource = D3D12Texture->GetResource();
                    FGuid OutSharedHandleGuid = FGuid::NewGuid();
                    FString UniqueHandleId = FString::Printf(
                        TEXT("%s"), *OutSharedHandleGuid.ToString(EGuidFormats::DigitsWithHyphensInBraces));

                    HRESULT Res = D3DDevice->CreateSharedHandle(
                        D3D12Resource->GetResource(), nullptr, GENERIC_ALL, *UniqueHandleId, &sharedHandle);
                    if (FAILED(Res))
                    {
                        UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] Failed to get handle : %d"), Res);
                    }
                }
                if (sharedHandle)
                {
                    cudaExternalMemory_t extMem = NULL;
                    cudaExternalMemoryHandleDesc exdesc = {};
                    memset(&exdesc, 0, sizeof(exdesc));
                    exdesc.type = cudaExternalMemoryHandleTypeD3D12Resource;
                    exdesc.handle.win32.handle = sharedHandle;
                    exdesc.size = D3D12Texture->GetMemorySize();
                    exdesc.flags = cudaExternalMemoryDedicated;
                    if (checkCudaErrors(cudaImportExternalMemory(&extMem, &exdesc)))
                    {
                        cudaExternalMemoryMipmappedArrayDesc desc = {};

                        memset(&desc, 0, sizeof(desc));
                        desc.offset = D3D12Texture->GetOffset();
                        desc.formatDesc = cudaCreateChannelDesc<uchar4>();
                        desc.extent.width = This->imgWidth;
                        desc.extent.height = This->imgHeight;
                        desc.extent.depth = 0;
                        desc.flags = cudaArrayDefault;
                        desc.numLevels = 1;

                        if (checkCudaErrors(cudaExternalMemoryGetMappedMipmappedArray(&This->mipmap, extMem, &desc)))
                        {
                            if (checkCudaErrors(cudaGetMipmappedArrayLevel(&(This->TransitionArray), This->mipmap, 0)))
                            {
                                UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] succeed to init cuda resource"));
                                return;
                            }
                        }
                    }
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] Does not support this RHI - %s"), *RHIName);
            }

#else

            FVulkanTextureBase* VulkanTexture = static_cast<FVulkanTextureBase*>(
                This->texRT->GetRenderTargetResource()->TextureRHI->GetTextureBaseRHI());
            if (RHIName == TEXT("Vulkan") && VulkanTexture)
            {
                int fd = -1;
                {
                    VkDeviceMemory memory = VulkanTexture->Surface.GetAllocationHandle();
                    VkDevice device = VulkanTexture->Surface.Device->GetInstanceHandle();

                    VkMemoryGetFdInfoKHR fdInfo = {};
                    fdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
                    fdInfo.memory = memory;
                    fdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

                    PFN_vkGetMemoryFdKHR func =
                        (PFN_vkGetMemoryFdKHR) (void*) VulkanRHI::vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");
                    if (!func)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] Failed to vkGetMemoryFdKHR func"));
                    }
                    else
                    {
                        VkResult r = func(device, &fdInfo, &fd);
                        if (r != VK_SUCCESS)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] Failed to vkGetMemoryFdKHR: %d"), r);
                        }
                    }
                }
                if (fd >= 0)
                {
                    cudaExternalMemory_t extMem = NULL;
                    cudaExternalMemoryHandleDesc exdesc = {};
                    memset(&exdesc, 0, sizeof(exdesc));
                    exdesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
                    exdesc.handle.fd = fd;
                    exdesc.size = VulkanTexture->Surface.GetMemorySize();
                    if (checkCudaErrors(cudaImportExternalMemory(&extMem, &exdesc)))
                    {
                        auto getCudaChannelFormatDescForVulkanFormat = [](VkFormat format) -> cudaChannelFormatDesc
                        {
                            cudaChannelFormatDesc d;

                            memset(&d, 0, sizeof(d));

                            switch (format)
                            {
                                case VK_FORMAT_R8_UINT:
                                    d.x = 8;
                                    d.y = 0;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R8_SINT:
                                    d.x = 8;
                                    d.y = 0;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R8G8_UINT:
                                    d.x = 8;
                                    d.y = 8;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R8G8_SINT:
                                    d.x = 8;
                                    d.y = 8;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R8G8B8A8_UINT:
                                    d.x = 8;
                                    d.y = 8;
                                    d.z = 8;
                                    d.w = 8;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R8G8B8A8_SINT:
                                    d.x = 8;
                                    d.y = 8;
                                    d.z = 8;
                                    d.w = 8;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_B8G8R8A8_UNORM:
                                    d.x = 8;
                                    d.y = 8;
                                    d.z = 8;
                                    d.w = 8;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R16_UINT:
                                    d.x = 16;
                                    d.y = 0;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R16_SINT:
                                    d.x = 16;
                                    d.y = 0;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R16G16_UINT:
                                    d.x = 16;
                                    d.y = 16;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R16G16_SINT:
                                    d.x = 16;
                                    d.y = 16;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R16G16B16A16_UINT:
                                    d.x = 16;
                                    d.y = 16;
                                    d.z = 16;
                                    d.w = 16;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R16G16B16A16_SINT:
                                    d.x = 16;
                                    d.y = 16;
                                    d.z = 16;
                                    d.w = 16;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R32_UINT:
                                    d.x = 32;
                                    d.y = 0;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R32_SINT:
                                    d.x = 32;
                                    d.y = 0;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R32_SFLOAT:
                                    d.x = 32;
                                    d.y = 0;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindFloat;
                                    break;
                                case VK_FORMAT_R32G32_UINT:
                                    d.x = 32;
                                    d.y = 32;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R32G32_SINT:
                                    d.x = 32;
                                    d.y = 32;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R32G32_SFLOAT:
                                    d.x = 32;
                                    d.y = 32;
                                    d.z = 0;
                                    d.w = 0;
                                    d.f = cudaChannelFormatKindFloat;
                                    break;
                                case VK_FORMAT_R32G32B32A32_UINT:
                                    d.x = 32;
                                    d.y = 32;
                                    d.z = 32;
                                    d.w = 32;
                                    d.f = cudaChannelFormatKindUnsigned;
                                    break;
                                case VK_FORMAT_R32G32B32A32_SINT:
                                    d.x = 32;
                                    d.y = 32;
                                    d.z = 32;
                                    d.w = 32;
                                    d.f = cudaChannelFormatKindSigned;
                                    break;
                                case VK_FORMAT_R32G32B32A32_SFLOAT:
                                    d.x = 32;
                                    d.y = 32;
                                    d.z = 32;
                                    d.w = 32;
                                    d.f = cudaChannelFormatKindFloat;
                                    break;
                            }
                            return d;
                        };

                        cudaExternalMemoryMipmappedArrayDesc desc = {};

                        memset(&desc, 0, sizeof(desc));
                        desc.offset = VulkanTexture->Surface.GetAllocationOffset();
                        desc.formatDesc = getCudaChannelFormatDescForVulkanFormat(VulkanTexture->Surface.StorageFormat);
                        desc.extent.width = VulkanTexture->Surface.Width;
                        desc.extent.height = VulkanTexture->Surface.Height;
                        desc.extent.depth = 0;    // VulkanTexture->Surface.Depth;
                        desc.flags = cudaArrayColorAttachment;
                        desc.numLevels = VulkanTexture->Surface.GetNumMips();

                        if (checkCudaErrors(cudaExternalMemoryGetMappedMipmappedArray(&This->mipmap, extMem, &desc)))
                        {
                            if (checkCudaErrors(cudaGetMipmappedArrayLevel(&(This->TransitionArray), This->mipmap, 0)))
                            {
                                UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] succeed to init cuda resource"));
                                return;
                            }
                        }
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] Failed to GetVulkan memory"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] Failed to GetTextureFromRHITexture"));
            }

#endif
        });

    return true;    // TransitionArray != nullptr;
}

bool UTexJpeg::Copy2Cuda(bool sync)
{
    if (!TransitionArray)
    {
        return false;
    }
    ENQUEUE_RENDER_COMMAND(Flush)
    ([](FRHICommandListImmediate& RHICmdList) { RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread); });
    FlushRenderingCommands(true);
    checkCudaErrors(cudaMemcpy2DFromArrayAsync(
        imgBufferBGRA, imgWidth * 4, TransitionArray, 0, 0, imgWidth * 4, imgHeight, cudaMemcpyDeviceToDevice, stream));
    if (trtStyle)
    {
        // cudaEvent_t start, stop;
        // cudaEventCreate(&start);
        // cudaEventCreate(&stop);
        // cudaEventRecord(start);
        if (!trtStyle->infer(imgBufferBGRA))
        {
            UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] infer falid"));
            return false;
        }
        // cudaEventRecord(stop);
        // cudaEventSynchronize(stop);
        // float milliseconds = 0;
        // cudaEventElapsedTime(&milliseconds, start, stop);
        // UE_LOG(LogTemp, Log, TEXT("[UTexJpeg] cast time is %f"), milliseconds);
    }
    if (imgBufferBGRA != imgBufferBGRA0)
    {
        checkCudaErrors(nppiResize_8u_C4R_Ctx(imgBufferBGRA, imgWidth * 4, NppiSize{imgWidth, imgHeight},
            NppiRect{0, 0, imgWidth, imgHeight}, imgBufferBGRA0, imgWidth0 * 4, NppiSize{imgWidth0, imgHeight0},
            NppiRect{0, 0, imgWidth0, imgHeight0}, NPPI_INTER_NN, nppStreamContext));
    }
    if (sync)
    {
        checkCudaErrors_TF(cudaStreamSynchronize(stream));
    }
    return true;
}

bool UTexJpeg::Raw(std::vector<uint8>& rawbuf)
{
    if (!TransitionArray)
    {
        return false;
    }
    rawbuf.resize(imgHeight0 * imgWidth0 * 4);
    checkCudaErrors_TF(
        cudaMemcpyAsync(rawbuf.data(), imgBufferBGRA0, imgWidth0 * imgHeight0 * 4, cudaMemcpyDeviceToHost, stream));
    checkCudaErrors_TF(cudaStreamSynchronize(stream));
    return true;
}

uint8_t* UTexJpeg::GetRawPtr(size_t& len)
{
    len = imgWidth0 * imgHeight0 * 4;
    return imgBufferBGRA0;
}

bool UTexJpeg::IpcCreate(uint8_t* handle)
{
#if PLATFORM_WINDOWS
    UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] ipc in only support linux"));
    return false;
#endif

    checkCudaErrors_TF(cudaIpcGetMemHandle(&memIpcHandle, imgBufferBGRA0));
    checkCudaErrors_TF(cudaIpcGetEventHandle(&eventIpcHandle, eventIpc));
    uint8_t* b = handle;
    memcpy(b, &memIpcHandle, sizeof(memIpcHandle));
    b += sizeof(memIpcHandle);
    memcpy(b, &eventIpcHandle, sizeof(eventIpcHandle));

    return true;
}

bool UTexJpeg::IpcShare(uint8_t* handle)
{
#if PLATFORM_WINDOWS
    UE_LOG(LogTemp, Warning, TEXT("[UTexJpeg] ipc in only support linux"));
    return false;
#endif
    // checkCudaErrors(cudaEventSynchronize(eventIpc));
    // checkCudaErrors(cudaEventRecord(eventIpc, stream));

    /*checkCudaErrors_TF(cudaIpcGetMemHandle(&memIpcHandle, ptrIpcBuff));
    checkCudaErrors_TF(cudaIpcGetEventHandle(&eventIpcHandle, eventIpc));
    uint8_t* b = handle;
    memcpy(b, &memIpcHandle, sizeof(memIpcHandle));
    b += sizeof(memIpcHandle);
    memcpy(b, &eventIpcHandle,sizeof(eventIpcHandle));*/

    return true;
}

bool UTexJpeg::JpegEncoding(TArray64<uint8_t>& jpegbuf)
{
    if (!TransitionArray)
    {
        return false;
    }
    Npp8u* aDst[] = {imageJpg.channel[0], imageJpg.channel[1], imageJpg.channel[2], imageJpg.channel[3]};
    NppiSize oSizeRoi = {0};
    oSizeRoi.height = imgHeight0;
    oSizeRoi.width = imgWidth0;
    checkCudaErrors_TF(cudaStreamSynchronize(stream));
    checkCudaErrors_TF(
        nppiCopy_8u_C4P4R_Ctx(imgBufferBGRA0, imgWidth0 * 4, aDst, imageJpg.pitch[0], oSizeRoi, nppStreamContext));
    // encode using encode parameters
    checkCudaErrors_TF(nvjpegEncodeImage(
        handleJpg, enStateJpg, enParamsJpg, &imageJpg, NVJPEG_INPUT_BGR, imgWidth0, imgHeight0, stream));
    // get compressed stream size
    size_t length = 0;
    checkCudaErrors_TF(nvjpegEncodeRetrieveBitstream(handleJpg, enStateJpg, 0, &length, stream));
    jpegbuf.SetNum(length);
    checkCudaErrors_TF(nvjpegEncodeRetrieveBitstream(handleJpg, enStateJpg, jpegbuf.GetData(), &length, stream));
    checkCudaErrors_TF(cudaStreamSynchronize(stream));
    return true;
}

#else
UTexJpeg::UTexJpeg(int gpuid)
{
}

UTexJpeg::~UTexJpeg()
{
}
void UTexJpeg::FreeResource()
{
}
bool UTexJpeg::InitResources(bool jpeg, bool share, int w, int h, const FString& stylemode)
{
    return false;
}
bool UTexJpeg::JpegEncoding(TArray64<uint8_t>& jpegbuf)
{
    return false;
}
bool UTexJpeg::Copy2Cuda(bool sync)
{
    return false;
}
bool UTexJpeg::Raw(std::vector<uint8>& rawbuf)
{
    return false;
}
uint8_t* UTexJpeg::GetRawPtr(size_t& len)
{
    len = 0;
    return 0;
}
bool UTexJpeg::IpcCreate(uint8_t* handle)
{
    return false;
}
bool UTexJpeg::IpcShare(uint8_t* handle)
{
    return false;
}

#endif

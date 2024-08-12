// Fill out your copyright notice in the Description page of Project Settings.

#include "CudaUtils.h"
#include "CoreMinimal.h"

#ifdef FIND_CUDA

#include "CudaCheck.h"
#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include "Windows/HideWindowsPlatformTypes.h"

#else

#include <cuda.h>
#include <cuda_runtime.h>

#endif

namespace cuda_utils
{

#undef checkCudaErrors
#undef checkCudaErrors_TF
#undef checkCudaErrors_TF0
#define checkCudaErrors(val) cuda_check((val), "CudaUtils", __LINE__)
#define checkCudaErrors_TF(val)                    \
    if (!cuda_check((val), "CudaUtils", __LINE__)) \
        return false;
#define checkCudaErrors_TF0(val)                   \
    if (!cuda_check((val), "CudaUtils", __LINE__)) \
        return 0;

void* Malloc(size_t len)
{
    void* ptr = 0;
    checkCudaErrors_TF0(cudaMalloc(&ptr, len));
    return ptr;
}
void* Duplicate(void* src, size_t len)
{
    void* ptr = 0;
    checkCudaErrors_TF0(cudaMalloc(&ptr, len));
    if (!MemCopy(ptr, src, len))
    {
        Free(ptr);
        return 0;
    }
    return ptr;
}
bool MemCopy(void* dst, const void* src, size_t len)
{
    checkCudaErrors_TF(cudaMemcpy(dst, src, len, cudaMemcpyDeviceToDevice));
    return true;
}
void Free(void* ptr)
{
    checkCudaErrors(cudaFree(ptr));
}
}    // namespace cuda_utils

#else

namespace cuda_utils
{

void* Malloc(size_t len)
{
    return 0;
}
void* Duplicate(void* src, size_t len)
{
    return 0;
}
bool MemCopy(void* src, const void* dst, size_t len)
{
    return false;
}
void Free(void* ptr)
{
}

}    // namespace cuda_utils

#endif

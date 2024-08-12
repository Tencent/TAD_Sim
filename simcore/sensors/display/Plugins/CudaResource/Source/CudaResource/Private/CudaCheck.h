// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

#ifdef FIND_CUDA

#include <cuda.h>
#include <nvjpeg.h>
#include <npp.h>

const char* _cudaGetErrorEnum(cudaError_t error);
const char* _cudaGetErrorEnum(CUresult error);

// nvJPEG API errors
const char* _cudaGetErrorEnum(nvjpegStatus_t error);
const char* _cudaGetErrorEnum(NppStatus error);

template <typename T>
bool cuda_check(T result, const char* const file, int const line)
{
    if (result)
    {
        UE_LOG(LogTemp, Warning, TEXT("CUDA error at %s:%d code=%d(%s)"), *FString(file), line,
            static_cast<unsigned int>(result), *FString(_cudaGetErrorEnum(result)));
        return false;
    }
    return true;
}

#endif

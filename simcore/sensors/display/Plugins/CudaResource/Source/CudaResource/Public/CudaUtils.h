// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"

#ifdef FIND_CUDA

#endif

namespace cuda_utils
{

CUDARESOURCE_API void* Malloc(size_t len);
CUDARESOURCE_API void* Duplicate(void* src, size_t len);
CUDARESOURCE_API bool MemCopy(void* dst, const void* src, size_t len);
CUDARESOURCE_API void Free(void* ptr);

}    // namespace cuda_utils

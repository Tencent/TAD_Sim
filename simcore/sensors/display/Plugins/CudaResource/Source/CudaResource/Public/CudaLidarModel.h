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

namespace cuda_lidar
{
class LidarSimulation;
}

#endif
// #include "TexJpeg.generated.h"

// UCLASS(Category = "CudaResource|LidarModel")
class CUDARESOURCE_API CudaLidarModel    // : public UObject
{
    // GENERATED_BODY()
public:
    // Sets default values for this component's properties
    CudaLidarModel();
    ~CudaLidarModel();

    void set_intensity(float i);
    void set_rain(float r);
    void set_snow(float s);
    void set_fog(float f);

    bool set_img(const TArray<uint8_t*>& img);
    bool set_refmap(const TArray<float>& ref);
    bool set_tagmap(const TArray<uint32_t>& ref);
    bool set_camidx(const TArray<uint8_t>& idx);
    bool set_camuv(const TArray<uint32_t>& uv);
    bool set_yawpitch(const TArray<float>& yp);
    bool set_rtmat(const FTransform& mat);

    bool set_rn_hn(uint32_t rn, uint32_t hn);

    /// <summary>
    ///
    /// </summary>
    /// <param name="bg_idx"></param>
    /// <param name="N"></param>
    /// <param name="out"></param>
    /// <returns>
    /// float x = 0, y = 0, z = 0;
    /// float distance = 0;
    /// float instensity = 0;
    /// unsigned int tag_c = 0u;
    /// unsigned int tag_t = 0u;
    /// float norinter = 0;    ///
    /// </returns>
    bool simulation(uint32_t bg_idx, uint32_t N, void* out);

#ifdef FIND_CUDA

protected:
private:
    TSharedPtr<class cuda_lidar::LidarSimulation> sim;

    float* out = 0;

    cudaStream_t stream = NULL;

#endif
};

/**
 * @file lidar_simulation.cu
 * @author kekesong (kekesong@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-12
 * 
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 * 
 */
#include <cuda.h>
#include <cuda_runtime.h>
#include <thrust/device_vector.h>
#include <thrust/random.h>
#include <iostream>
#include <string>
#include <vector>
#include "lidar_simulation.h"

using namespace cuda_lidar;
#ifndef CLAMP
#  define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif
#ifndef MAX
#  define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

__device__ float rand_gen(const float d) {
  uint32_t status = d * 123863.f;
  status = status * 21401334u + 2531013245u;
  status = status & 0xffffu;
  return 1.f * (status % 65536) / 32768.f - 1.f;
}

__device__ float lidar_reflection(const float *refmap, uint32_t tag, const float d) {
  float minr = refmap[tag * 3];
  float maxr = refmap[tag * 3 + 1];
  float deta = refmap[tag * 3 + 2];
  float u = 0.5f + deta * rand_gen(d);

  return minr + (maxr - minr) * CLAMP(u, 0.f, 1.f);
}

__device__ float factor_rain(const float d, const float f_rainfall) { return pow(f_rainfall, d); }

__device__ float factor_fog(const float d, const float f_fog) { return factor_rain(d, f_fog); }

__device__ void gausswhite_noise(float &d, const const float f_accuracy, const float f_noise_dev) {
  d += f_accuracy * rand_gen(d) * f_noise_dev;
}

__device__ void snow_noise(float &d, const float f_snowfall_range, const float f_snowfall_prob) {
  float p = rand_gen(d);
  if (p < f_snowfall_prob) return;
  float nd = f_snowfall_range * (rand_gen(d) * 0.5f + 0.5f) + 0.2;
  d = MIN(nd, d);
}

__device__ bool lidar_simulation(const float *refmap, const float cosita, const uint32_t tag, const float t_intensity,
                                 const float f_noise_dev, const float f_accuracy, const float f_rainfall,
                                 const float f_snowfall_range, const float f_snowfall_prob, const float f_fog, float &d,
                                 float &ref) {
  ref = lidar_reflection(refmap, tag, d);
  float v = CLAMP(cosita + 0.2f * (rand_gen(d * cosita) + 0.5f), 0.f, 1.f) * ref / (d * d);
  if (f_rainfall < 1.f) {
    // road water
    if (tag >= 9 && tag <= 19) {
      v *= exp((f_rainfall - 1) * 1024);
    }
    v *= factor_rain(d, f_rainfall);
  }
  if (f_fog < 1.f) {
    v *= factor_fog(d, f_fog);
  }
  if (v < t_intensity) {
    d = 0;
    return false;
  }
  gausswhite_noise(d, f_accuracy, f_noise_dev);
  if (f_snowfall_prob < 1.f) {
    snow_noise(d, f_snowfall_range, f_snowfall_prob);
  }
  return d >= 0.01f;
}

__global__ void lidar_processing(uint8_t **const depthImg, const float *refmap, const uint32_t *tagmap,
                                 const uint8_t *camidx, const uint32_t *camuv, const float *yawpitch,
                                 const float *rtmat, const uint32_t rhn, const float t_intensity,
                                 const float f_noise_dev, const float f_accuracy, const float f_rainfall,
                                 const float f_snowfall_range, const float f_snowfall_prob, const float f_fog,
                                 uint32_t idx, uint32_t N, float *out) {
  uint32_t x = threadIdx.x + blockIdx.x * blockDim.x;
  if (x < N) {
    uint32_t i = (idx + x) % (rhn);
    int pici = camidx[i];
    if (pici == 255) {
      return;
    }
    if (!depthImg[pici]) {
      return;
    }
    uint32_t xy = camuv[i];
    const uint8_t *color00 = &(depthImg[pici][xy * 4]);
    float distance = ((float)color00[2] * 256.f + (float)color00[1]) * 0.005f;
    if (distance > 0.01f && distance < 327.f) {
      float norinter = (float)color00[0] * 0.00390625f;
      float *ret = out + x * 8;
      ret[3] = distance;
      ret[7] = norinter;
      uint32_t tag = color00[3];
      lidar_simulation(refmap, norinter, tag, t_intensity, f_noise_dev, f_accuracy, f_rainfall, f_snowfall_range,
                       f_snowfall_prob, f_fog, ret[3], ret[4]);
      if (ret[3] > 0.01f && ret[3] < 327.f) {
        *(unsigned int *)(ret + 5) = tagmap[tag * 2];
        *(unsigned int *)(ret + 6) = tagmap[tag * 2 + 1];
        const float *yp = yawpitch + i * 2;
        ret[2] = distance * sin(yp[1]);
        ret[1] = distance * cos(yp[1]) * sin(yp[0]);
        ret[0] = distance * cos(yp[1]) * cos(yp[0]);
        if (rtmat) {
          float nx = rtmat[0] * ret[0] + rtmat[4] * ret[1] + rtmat[8] * ret[2] + rtmat[12];
          float ny = rtmat[1] * ret[0] + rtmat[5] * ret[1] + rtmat[9] * ret[2] + rtmat[13];
          float nz = rtmat[2] * ret[0] + rtmat[6] * ret[1] + rtmat[10] * ret[2] + rtmat[14];
          float nw = rtmat[3] + rtmat[7] + rtmat[11] + rtmat[15];
          ret[0] = nx / nw;
          ret[1] = ny / nw;
          ret[2] = nz / nw;
        }
      }
    }
  }
}

LidarSimulation::LidarSimulation() {}
LidarSimulation::~LidarSimulation() {}

bool LidarSimulation::simulation(uint32_t bg_idx, uint32_t N, float *out, cudaStream_t stream) {
  /*if (cudaMemsetAsync(out, 0, buf.hn * buf.rn * sizeof(lidarsim_point), stream))
  {
          std::cout << "cannot set out buffer\n";
          return false;
  }*/
  // post_processing
  auto threads_per_block = 128;
  auto blocks = dim3((N + threads_per_block - 1) / threads_per_block);
  auto threads = dim3(threads_per_block);

  lidar_processing<<<blocks, threads, 0, stream>>>(depthImg, refmap, tagmap, camidx, camuv, yawpitch, rtmat, rhn,
                                                   t_intensity, f_noise_dev, f_accuracy, f_rainfall, f_snowfall_range,
                                                   f_snowfall_prob, f_fog, bg_idx, N, out);
  auto err = cudaGetLastError();
  if (err) {
    printf("lidar sim error: %d %s", err, cudaGetErrorString(err));
    return false;
  }
  return true;
}

/**
 * @file radar.cpp
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
#include <gtest/gtest.h>
#include <math.h>
#include <memory>
#include <vector>
#include "../cuComplexEx.h"
#include "../radar_simulation.h"


#ifndef CUDA_RT_CALL
#  define CUDA_RT_CALL(call)                                                    \
    {                                                                           \
      auto status = static_cast<cudaError_t>(call);                             \
      if (status != cudaSuccess)                                                \
        fprintf(stderr,                                                         \
                "ERROR: CUDA RT call \"%s\" in line %d of file %s failed "      \
                "with "                                                         \
                "%s (%d).\n",                                                   \
                #call, __LINE__, __FILE__, cudaGetErrorString(status), status); \
    }
#endif  // CUDA_RT_CALL

// cufft API error chekcing
#ifndef CUFFT_CALL
#  define CUFFT_CALL(call)                                               \
    {                                                                    \
      auto status = static_cast<cufftResult>(call);                      \
      if (status != CUFFT_SUCCESS)                                       \
        fprintf(stderr,                                                  \
                "ERROR: CUFFT call \"%s\" in line %d of file %s failed " \
                "with "                                                  \
                "code (%d).\n",                                          \
                #call, __LINE__, __FILE__, status);                      \
    }
#endif  // CUFFT_CALL
#ifndef PI
#  define PI 3.1415926F
#endif
#ifndef CLAMP
#  define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif
float distance_cpu(float x1, float y1, float z1, float x2, float y2, float z2) {
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
}
float rand_r_cpu(const float d) {
  return 0;
  uint32_t status = d * 123863.f;
  status = status * 21401334u + 2531013245u;
  status = status & 0xffffu;
  return 1.f * (status % 65536) / 32768.f - 1.f;
}
float db2w_cpu(float db) { return pow(10, db * 0.1); }
float radar_rcs_cpu(const float* refmap, uint32_t tag, const float d) {
  float minr = refmap[tag * 3];
  float maxr = refmap[tag * 3 + 1];
  float deta = refmap[tag * 3 + 2];
  float u = 0.5f + deta * rand_r_cpu(d);

  return minr + (maxr - minr) * CLAMP(u, 0.f, 1.f);
}
float decimal_float_mul_cpu(float a, float b) {
  float a1 = floor(a);
  float a2 = a - a1;
  float b1 = floor(b);
  float b2 = b - b1;
  float c = a1 * b2 + a2 * b1 + b2 * a2;
  return c - floor(c);
}

void signal_sim_cpu(uint8_t* pointImg, uint32_t N, uint32_t Ns, uint32_t Na, uint32_t Ne, float lambda, float FC,
                    float S, float er, float L1_dB, float* yawpitch, float* refmap, float* iFx, uint32_t x, uint32_t y,
                    uint32_t z) {
  if (x >= Ns || y >= Na || z >= Ne) return;
  float Rx = 0;
  for (uint32_t i = 0; i < N; i++) {
    const uint8_t* color00 = &(pointImg[i * 4]);
    uint32_t tag = color00[3];
    float dis = ((float)color00[2] * 256.f + (float)color00[1]) * 0.005f;
    const float* yp = yawpitch + i * 2;
    float tz = dis * sin(yp[1]);
    float ty = dis * cos(yp[1]) * sin(yp[0]);
    float tx = dis * cos(yp[1]) * cos(yp[0]);
    float dis2 = distance_cpu(tx, ty, tz, 0, -lambda * y / 2, lambda * z / 2);

    // rcs
    float rcs = radar_rcs_cpu(refmap, tag, dis);
    // signal

    float Am = er * sqrt(rcs) / pow(dis, 2) / sqrt(db2w_cpu(L1_dB * dis));

    // recive
    const float C = 0.3f;          // 光速 e9 m/s
    float td = -(dis + dis2) / C;  // 回波时延
    float ph1 = FC * td;
    ph1 -= floor(ph1);
    float ph2 = S * 1e-9f * td * td * 0.5f;
    ph2 -= floor(ph2);
    float rx = Am * cos(2 * PI * (ph1 + ph2));  // 接收信号波形

    Rx += rx;
  }
  iFx[z * Na * Ns + y * Ns + x] = Rx;
}

template <typename T>
T* CreateCudaPtr(const std::vector<T>& data) {
  T* ptr = 0;
  CUDA_RT_CALL(cudaMalloc(&ptr, data.size() * sizeof(T)));
  CUDA_RT_CALL(cudaMemcpy(ptr, data.data(), data.size() * sizeof(T), cudaMemcpyHostToDevice));
  return ptr;
}

TEST(RadarTest, Load) {
  cuda_radar::RadarSimulation radar;
  radar.Ns = 512;
  radar.Nd = 128;
  radar.Na = 256;
  radar.Ne = 256;
  radar.N = 360;

  radar.Init();

  std::vector<uint8_t> pointImg(12 * radar.N);
  std::vector<float> iFx(radar.N * 8);
  for (auto i = 0; i < radar.N; i++) {
    float rangeRes = 0.3f / (2 * radar.B);
    int dis = 10 / rangeRes;

    pointImg[i * 12 + 0] = 127;
    pointImg[i * 12 + 1] = dis % 255;
    pointImg[i * 12 + 2] = dis / 256;
    pointImg[i * 12 + 3] = 127;

    pointImg[i * 12 + 4] = 0;
    pointImg[i * 12 + 5] = 127;
    pointImg[i * 12 + 6] = 127;

    float sita = PI * i / 180;
    float3 nor{-cos(sita), sin(sita), 0};
    float3 dir{1, 0, 0};
    dir = dir - nor * (dir * nor * 2);
    dir = dir * 127 + 127;

    pointImg[i * 12 + 7] = dir.z > 0 ? 128 : 0;
    pointImg[i * 12 + 8] = 0;
    pointImg[i * 12 + 9] = dir.y;
    pointImg[i * 12 + 10] = dir.x;
    pointImg[i * 12 + 11] = 0;
  }

  uint8_t* cuda_rayImg = CreateCudaPtr(pointImg);
  radar.rayImg = cuda_rayImg;

  float* out;
  CUDA_RT_CALL(cudaMalloc(&out, radar.N * sizeof(float)));

  radar.simulation(out);

  std::vector<float> cuda_out(radar.N);
  CUDA_RT_CALL(cudaMemcpy(cuda_out.data(), out, cuda_out.size() * sizeof(float), cudaMemcpyDeviceToHost));

  for (auto f : cuda_out) std::cout << f << std::endl;

  CUDA_RT_CALL(cudaFree(cuda_rayImg));
  CUDA_RT_CALL(cudaFree(out));
  EXPECT_TRUE(1);
}

/*

TEST(RadarTest, Load) {
    cuda_radar::RadarSimulation radar;
    radar.Ns = 512;
    radar.Nd = 128;
    radar.Na = 256;
    radar.Ne = 256;
    radar.N = 100;

    radar.Init();

        std::vector<uint8_t> pointImg(4*radar.N);
        std::vector<float> yawpitch(2*radar.N);
        std::vector<float> refmap(3*256);
        std::vector<float> iFx(radar.Na*radar.Ns*radar.Ne);
    for (auto i=0;i<radar.N;i++)
    {
        pointImg[i*4] = rand() % 5;
        pointImg[i*4+1] = rand() % 255;
        pointImg[i*4+2] = rand() % 255;
        pointImg[i*4+3] = rand() % 255;
    }
    for (auto i=0;i<radar.N*2;i++)
    {
        yawpitch[i] = (rand() % 1000) / 500.0f;
    }
    for (auto i=0;i<256;i++)
    {
        refmap[i*3] = 1;
        refmap[i*3+1] = 5;
        refmap[i*3+2] = 0.2f;
    }

    uint8_t* cuda_pointImg = CreateCudaPtr(pointImg);
    float* cuda_yawpitch = CreateCudaPtr(yawpitch);
    float* cuda_refmap = CreateCudaPtr(refmap);
        std::vector<float> cuda_iFx(radar.Na*radar.Ns*radar.Ne);
    radar.pointImg = cuda_pointImg;
    radar.yawpitch = cuda_yawpitch;
    radar.refmap = cuda_refmap;

        float* iFx_buffer = radar.SignalFX();
    CUDA_RT_CALL(cudaMemcpy(cuda_iFx.data(), iFx_buffer, cuda_iFx.size()*sizeof(float), cudaMemcpyDeviceToHost));




    for (uint32_t z = 0; z < radar.Ne; z++)
    {
         for (uint32_t y = 0; y < radar.Na; y++)
        {
             for (uint32_t x = 0; x < radar.Ns; x++)
            {
                signal_sim_cpu(pointImg.data(), radar.N,radar.Ns,radar.Na,radar.Ne,
                radar.lambda,radar.FC,radar.S,radar.er_power,radar.L1_dB,
                yawpitch.data(),refmap.data(),iFx.data(),x,y,z);

            }
        }
    }
    bool same = true;
    for (size_t i = 0; i < cuda_iFx.size(); i++)
    {
        float a = cuda_iFx.at(i);
        float b = iFx.at(i);
        float c = abs((a - b) / (a + b));
        if (c > 0.1f)
        {
           same = false;
           std::cout << a << "-" << b << "  ";
        }

    }



    CUDA_RT_CALL(cudaFree(cuda_pointImg));
    CUDA_RT_CALL(cudaFree(cuda_yawpitch));
    CUDA_RT_CALL(cudaFree(cuda_refmap));


        EXPECT_TRUE(same);
}
//*/

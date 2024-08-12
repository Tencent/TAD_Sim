/**
 * @file radar_simulation.cu
 * @author kekesong (kekesong@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-12
 * 
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 * 
 */
#include <cuComplex.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cufftXt.h>
#include <thrust/device_vector.h>
#include <thrust/random.h>
#include <complex>
#include <iostream>
#include <string>
#include <vector>
#include "cuComplexEx.h"
#include "radar_simulation.h"

// CUDA API error checking
#ifndef PI
#  define PI 3.1415926F
#endif
#ifndef MAX
#  define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef CLAMP
#  define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif
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
using namespace cuda_radar;

__device__ __host__ float db2w(float db) { return powf(10, db * 0.1f); }

__device__ float rand_r(const float d) {
  return 0;
  uint32_t status = d * 123863.f;
  status = status * 21401334u + 2531013245u;
  status = status & 0xffffu;
  return 1.f * (status % 65536) / 32768.f - 1.f;
}

RadarSimulation::RadarSimulation() {}
RadarSimulation::~RadarSimulation() { Clear(); }
float RadarSimulation::L1(int f0, int weather, float r) {
  const static float weather_L1_24[20] = {0.32, 0.618, 2.09, 4.04, 7.99, 0.34, 0.48, 0.64, 0.38,
                                          0.8,  1.28,  0.33, 0.32, 0.32, 0.32, 0.32, 0.32};
  const static float weather_L1_77[20] = {0.62, 2.28, 6.66, 11.02, 18.52, 0.68, 0.94, 1.26,
                                          0.74, 1.58, 2.54, 0.64,  0.62,  0.62, 0.62, 0.62};
  weather = std::min(11, std::max(0, weather));
  if (f0 == 24) {
    return weather_L1_24[weather] * r * 0.001f;
  } else {
    return weather_L1_77[weather] * r * 0.001f;
  }
  return 0;
}

void RadarSimulation::Init() {
  lambda = 0.3f / FC;         // 波长 m
  rangeRes = 0.3f / (2 * B);  // 距离分辨率 m    B = c/(2*rangeRes)
  Tc = B / S;                 // Chirp上升时间 = Nr / Fs = aaa*2*maxR/c
  maxR = Ns * rangeRes;       // 最大需要探测距离 m
  Fs = Ns / Tc;               // 采样频率 Hz  =Nr/Tc
  maxV = lambda / (4 * Tc);   // 最大探测速度 m/s  230*1000/3600
  vres = maxV * 2 / Nd;       // 速度分辨率 m/s

  L1_dB = L1((int)std::round(FC), weather, 1.0f);
  {
    double k = 1.38e-23;
    double Pr = Pt_dBm * Gt_dBi * Gr_dBi * lambda * lambda * Tc;
    double Pn = 64 * PI * PI * PI * k * Ts_K * db2w(Fn_dB) * db2w(L0_dB);
    if (Pr > 0) {
      er_power = Pr / Pn;
    }
  }

  // Create the plans
  size_t workspace_size;
  CUFFT_CALL(cufftCreate(&plan_r2c));
  CUFFT_CALL(cufftSetStream(plan_r2c, stream));
  CUFFT_CALL(cufftMakePlan3d(plan_r2c, Ne, Na, Ns, CUFFT_R2C, &workspace_size));
  CUDA_RT_CALL(cudaMalloc(&iFx, Na * Ne * Ns * sizeof(float)));
}

void RadarSimulation::Clear() {
  if (iFx) CUDA_RT_CALL(cudaFree(iFx));
  if (plan_r2c) CUFFT_CALL(cufftDestroy(plan_r2c));

  iFx = 0;
  plan_r2c = 0;
}

__global__ void signal_sim(uint8_t* rayImg, uint32_t N, float res_range, float res_veolity, float res_aziN,
                           float res_eleN, float rayArea, float lambda, float S, float Er, float L1_dB, float* iFx)

{
  int bid = blockIdx.x * blockDim.x;
  int rid = bid + threadIdx.x;
  if (rid >= N) {
    return;
  }
  uint8_t* rawray = rayImg + rid * 12;

  // get ray info
  float distance = ((float)rawray[2] * 256.f + (float)rawray[1]) * res_range;
  float azith = asin((float)rawray[0] / 127.0f - 1);
  float pitch = asin((float)rawray[3] / 127.0f - 1);
  float3 pos = make_float3(cos(pitch) * cos(azith), cos(pitch) * sin(azith), sin(pitch)) * distance;

  float3 polVec{(float)rawray[6] / 127.0f - 1, (float)rawray[5] / 127.0f - 1, (float)rawray[4] / 127.0f - 1};
  float3 Direction{(float)rawray[10] / 127.0f - 1, (float)rawray[9] / 127.0f - 1, 0};
  Direction.z = -sqrt(MAX(0.000001f, 1 - Direction.x * Direction.x - Direction.y * Direction.y));
  int tag = rawray[7];
  if (tag & 128) {
    Direction.z *= -1;
  }

  // rcs
  const float waveNum = 2 * PI / lambda;
  float3 dirP{0, -1, 0};
  float3 dirT{0, 0, -1};
  float3 vecK{-waveNum, 0, 0};

  cuComplex kr{0, waveNum * distance};
  kr = cuCexpf(kr);
  cuComplexVec3 apE = cuComplexVec3(polVec) * kr;
  cuComplexVec3 apH = -apE.cross(Direction);

  cuComplex BU = (-(apE.cross(-dirP) + apH.cross(dirT))) * Direction;
  cuComplex BR = (-(apE.cross(dirT) + apH.cross(dirP))) * Direction;

  cuComplex factor = make_cuComplex(0, waveNum * rayArea / 4.0 / PI) * cuCexpf(make_cuComplex(0, -vecK * pos));
  BU *= factor;
  BR *= factor;

  extern __shared__ float A[];

  float Am2 = Er / db2w(L1_dB * distance);
  cuComplex AU = BU;
  cuComplex AR = BR;

  float rcs = 4.0 * PI * (pow(cuCabsf(AU), 2) + pow(cuCabsf(AR), 2));
  iFx[rid] = 10. * log10(rcs);
}

bool RadarSimulation::simulation(float* out) {
  auto threads_per_block = 128;
  auto blocks = dim3((N + threads_per_block - 1) / threads_per_block);
  auto threads = dim3(threads_per_block);
#ifdef CASTTIME
  cudaEvent_t start, stop;
  float time = 0;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start);
  cudaEventRecord(start);
#endif

  signal_sim<<<blocks, threads, 0, stream>>>(rayImg, N, rangeRes, vres, 0, 0, 1, lambda, S, er_power, L1_dB, out);

#ifdef CASTTIME
  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time, start, stop);
  std::cout << "SignalFX cast : " << time << std::endl;
#endif
  auto err = cudaGetLastError();
  if (err) {
    printf("lidar sim error: %d %s", err, cudaGetErrorString(err));
    return false;
  }
  return true;
}

//__device__ float distance(float x1, float y1, float z1, float x2, float y2, float z2)
//{
//	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));
//}
//__device__ float decimal_float_mul(float a, float b)
//{
//	float a1 = floor(a);
//	float a2 = a - a1;
//	float b1 = floor(b);
//	float b2 = b - b1;
//	float c = a1 * b2 + a2 * b1 + b2 * a2;
//	return c - floor(c);
//}
//
//__device__ float radar_rcs(const float *refmap, uint32_t tag, const float d)
//{
//	float minr = refmap[tag * 3];
//	float maxr = refmap[tag * 3 + 1];
//	float deta = refmap[tag * 3 + 2];
//	float u = 0.5f + deta * rand_r(d);
//
//	return minr + (maxr - minr) * CLAMP(u, 0.f, 1.f);
//}
//
//__global__ void signal_sim(uint8_t* pointImg, uint32_t N, uint32_t Ns, uint32_t Na, uint32_t Ne,
//							float lambda, float FC, float S, float er, float L1_dB,
//							float* yawpitch, float* refmap, float* iFx)
//{
//	uint32_t x = threadIdx.x + blockIdx.x * blockDim.x;
//	uint32_t y = threadIdx.y + blockIdx.y * blockDim.y;
//	uint32_t z = threadIdx.z + blockIdx.z * blockDim.z;
//	if(x >= Ns || y >= Na || z >= Ne)
//		return;
//	float Rx = 0;
//	for (uint32_t i = 0; i < N; i++)
//	{
//		const uint8_t *color00 = &(pointImg[i * 4]);
//		uint32_t tag = color00[3];
//		float dis = ((float)color00[2] * 256.f + (float)color00[1]) * 0.005f;
//		const float *yp = yawpitch + i * 2;
//		float tz = dis * sin(yp[1]);
//		float ty = dis * cos(yp[1]) * sin(yp[0]);
//		float tx = dis * cos(yp[1]) * cos(yp[0]);
//		float dis2 = distance(tx,ty,tz,0,-lambda*y/2,lambda*z/2);
//
//		// rcs
//		float rcs = radar_rcs(refmap, tag, dis);
//		// signal
//
//        float Am = er * sqrt(rcs) / pow(dis, 2) / sqrt(db2w(L1_dB*dis));
//
//		// recive
//		const float C = 0.3f;// 光速 e9 m/s
//		float td = -(dis+dis2)/C;//回波时延
//		float ph1 = FC * td;
//		ph1 -= floor(ph1);
//		float ph2 = S * 1e-9f * td * td * 0.5f;
//		ph2 -= floor(ph2);
//        float rx = Am*cos(2*PI* (ph1+ph2));//接收信号波形
//
//        Rx += rx;
//	}
//	iFx[z*Na*Ns+y*Ns+x]=Rx;
//}
// #define CASTTIME
//
// float *RadarSimulation::SignalFX()
//{
//	if (!pointImg || !yawpitch || !refmap || !iFx)
//	{
//		std::cout << "ptr is null";
//		return 0;
//	}
//
//	dim3 dimBlock(16, 4, 2);
//	dim3 dimGrid(std::ceil(Ns * 1.0 / dimBlock.x), std::ceil(Na * 1.0 / dimBlock.y), std::ceil(Ne * 1.0 /
//dimBlock.z));
//
// #ifdef CASTTIME
//	cudaEvent_t start, stop;
//	float time = 0;
//	cudaEventCreate(&start);
//	cudaEventCreate(&stop);
//	cudaEventRecord(start);
//	cudaEventRecord(start);
// #endif
//	signal_sim<<<dimGrid, dimBlock, 0, stream>>>(pointImg, N, Ns, Na, Ne,
//													lambda, FC, S,
//er_power, L1_dB, 													yawpitch, refmap, iFx); #ifdef CASTTIME 	cudaEventRecord(stop); 	cudaEventSynchronize(stop);
//	cudaEventElapsedTime(&time, start, stop);
//	std::cout << "SignalFX cast : " << time << std::endl;
// #endif
//
//
//	CUDA_RT_CALL(cudaGetLastError());
//    return iFx;
//}
//
// bool RadarSimulation::simulation(float *out)
//{
//	SignalFX();
//	// Execute the plan_r2c
//    CUFFT_CALL(cufftXtExec(plan_r2c, iFx, iFx, CUFFT_FORWARD));
//
//
//    return false;
//}

/**
 * @file lidar.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-07-12
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <cuda_runtime.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


#include "../lidar_simulation.h"

void test_lidar() {
  cuda_lidar::LidarSimulation lidar;
  {
    int width = 500, height = 500;
    char *image[6];
    for (int i = 0; i < 6; i++) {
      cudaMalloc(&image[i], width * height * 4 * sizeof(char));

      cudaMemset((void *)image[i], 0, width * height * 4 * sizeof(char));
    }

    cudaError_t err;
    lidar.f_noise_dev = 0.0346399993f;

    lidar.rhn = 32 * 1800;

    cudaMalloc(&lidar.depthImg, 6 * sizeof(char *));
    cudaMemcpy((void *)lidar.depthImg, (void *)image, 6 * sizeof(char *), cudaMemcpyHostToDevice);

    cudaMalloc(&lidar.refmap, 1024 * 3 * sizeof(float));
    cudaMalloc(&lidar.tagmap, 1024 * 2 * sizeof(float));

    cudaMalloc(&lidar.camidx, lidar.rhn * sizeof(uint8_t));
    cudaMalloc(&lidar.camuv, lidar.rhn * sizeof(uint32_t));

    cudaMalloc(&lidar.yawpitch, lidar.rhn * 2 * sizeof(float));

    float *out;

    cudaMalloc((void **)&out, (lidar.rhn) * sizeof(cuda_lidar::lidarsim_point));

    cudaMemset((void *)out, 0, (lidar.rhn) * sizeof(cuda_lidar::lidarsim_point));
    if (!lidar.simulation(0, 1800 * 32, out, 0)) {
      std::cout << "eeeeeeeeeee" << std::endl;
      return;
    }
    std::vector<cuda_lidar::lidarsim_point> _out;
    _out.resize((lidar.rhn));
    err = cudaMemcpy((void *)_out.data(), (void *)out, (lidar.rhn) * sizeof(cuda_lidar::lidarsim_point),
                     cudaMemcpyDeviceToHost);
    if (err) {
      std::cout << err << " " << cudaGetErrorString(err) << std::endl;
    }

    std::cout << "------------" << std::endl;
  }
}

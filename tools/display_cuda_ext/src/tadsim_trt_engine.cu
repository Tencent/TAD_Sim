/**
 * @file tadsim_trt_engine.cu
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-07-12
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include <NvInfer.h>
#include <NvInferPlugin.h>
#include <NvInferRuntime.h>
#include <NvOnnxParser.h>
#include <cuda_runtime_api.h>
#include <npp.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "buffers.h"
#include "common.h"
#include "logger.h"
#include "tadsim_trt_engine.h"
// #include "parserOnnxConfig.h"

using samplesCommon::SampleUniquePtr;

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
__global__ void pre_processing(float* plane, uchar* const bgra, const int N) {
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  if (x < N) {
    plane[x] = 0.0078431372549f * ((float)bgra[x * 4 + 2] - 127.5f);
    plane[x + N] = 0.0078431372549f * ((float)bgra[x * 4 + 1] - 127.5f);
    plane[x + N + N] = 0.0078431372549f * ((float)bgra[x * 4 + 0] - 127.5f);
  }
}
__global__ void post_processing(uchar* bgra, float* const plane, const int N) {
  int x = threadIdx.x + blockIdx.x * blockDim.x;
  if (x < N) {
    bgra[x * 4] = 127.5f * (CLAMP(plane[x + N + N], -1.0f, 1.0f) + 1.0f);
    bgra[x * 4 + 1] = 127.5f * (CLAMP(plane[x + N], -1.0f, 1.0f) + 1.0f);
    bgra[x * 4 + 2] = 127.5f * (CLAMP(plane[x], -1.0f, 1.0f) + 1.0f);
  }
}

TADSimTRT::TADSimTRT(const TADOnnxParams& params, cudaStream_t cudastream, int gpu_id) : mParams(params) {
  cudaSetDevice(gpu_id);
  syncstream = cudastream;

  std::cout << mParams.width << std::endl;
  std::cout << mParams.height << std::endl;
  std::cout << mParams.onnxmodel << std::endl;
  std::cout << mParams.cachetrt << std::endl;
  printf("start to build engine\n");
  build();
  printf("findish building engine\n");
}
TADSimTRT::~TADSimTRT() {
  if (mBuffers) delete mBuffers;
  if (mContext) mContext->destroy();
  if (mEngine) mEngine->destroy();
}

bool TADSimTRT::build() {
  std::stringstream gieModelStdStream;
  gieModelStdStream.seekg(0, gieModelStdStream.beg);

  std::ifstream trt_cache(mParams.cachetrt, std::ios::binary);
  if (!trt_cache) {
    if (!ONNX2Trt()) {
      printf("faild to init engine.\n");
      return false;
    } else {
      printf("succ to init engine\n");
    }
    trt_cache.close();
    nvinfer1::IHostMemory* gieModelStream = mEngine->serialize();
    if (!gieModelStream) {
      std::cout << "failed to serialize CUDA engine" << std::endl;
      return false;
    }
    std::ofstream gieModelStdStream(mParams.cachetrt, std::ios::binary);
    std::cout << "model size = " << gieModelStream->size() << std::endl;
    gieModelStdStream.write((const char*)gieModelStream->data(), gieModelStream->size());
    gieModelStream->destroy();
  } else {
    printf("load trt cache\n");
    IRuntime* runtime = createInferRuntime(common::tensorrt::myLogger);
    if (!runtime) {
      return false;
    }
    gieModelStdStream << trt_cache.rdbuf();
    trt_cache.close();
    gieModelStdStream.seekg(0, std::ios::end);
    std::streamsize modelSize = gieModelStdStream.tellg();
    gieModelStdStream.seekg(0, std::ios::beg);
    void* modelMem = malloc(modelSize + 1024);
    gieModelStdStream.read((char*)modelMem, modelSize);

    // This is very important otherwise rt is not running
    initLibNvInferPlugins(&common::tensorrt::myLogger, "");
    mEngine = runtime->deserializeCudaEngine(modelMem, modelSize), samplesCommon::InferDeleter();
    runtime->destroy();
    free(modelMem);

    for (int b = 0; b < mEngine->getNbBindings(); b++) {
      auto dims = mEngine->getBindingDimensions(b);
      std::cout << dims.d[0] << " " << dims.d[1] << " " << dims.d[2] << " " << dims.d[3] << std::endl;

      if (mEngine->bindingIsInput(b)) {
        mInfo.height_in = dims.d[2];
        mInfo.width_in = dims.d[3];
      } else {
        mInfo.height_out = dims.d[2];
        mInfo.width_out = dims.d[3];
      }
    }
  }

  // Create RAII buffer manager object
  auto now = std::chrono::steady_clock::now();
  mBuffers = new samplesCommon::BufferManager(mEngine);
  mContext = mEngine->createExecutionContext();
  if (!mContext) {
    printf("cannot create mContext\n");
    return false;
  }
  inputdev = static_cast<float*>(mBuffers->getDeviceBuffer("input"));
  outputdev = static_cast<float*>(mBuffers->getDeviceBuffer("output"));
  auto n2 = std::chrono::steady_clock::now();
  auto tspan = std::chrono::duration_cast<std::chrono::milliseconds>(n2 - now);
  std::cout << "cast " << tspan.count() << std::endl;

  return true;
}

bool TADSimTRT::ONNX2Trt() {
  auto builder = SampleUniquePtr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(common::tensorrt::myLogger));
  if (!builder) {
    printf("createInferBuilder falid\n");
    return false;
  }
  const auto explicitBatch = 1U << static_cast<uint32_t>(NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
  auto network = SampleUniquePtr<nvinfer1::INetworkDefinition>(builder->createNetworkV2(explicitBatch));
  if (!network) {
    printf("createNetworkV2 falid\n");
    return false;
  }
  auto config = SampleUniquePtr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
  if (!config) {
    printf("createBuilderConfig falid\n");
    return false;
  }
  auto parser =
      SampleUniquePtr<nvonnxparser::IParser>(nvonnxparser::createParser(*network, common::tensorrt::myLogger));
  if (!parser) {
    printf("createParser falid\n");
    return false;
  }
  auto profileCalib = builder->createOptimizationProfile();
  const auto inputName = "input";  // mParams.inputTensorNames[0];
  const int calibBatchSize{1};
  // We do not need to check the return of setDimension and setCalibrationProfile here as all dims are explicitly set
  profileCalib->setDimensions(inputName, OptProfileSelector::kMIN,
                              Dims4{calibBatchSize, 3, mParams.height, mParams.width});
  profileCalib->setDimensions(inputName, OptProfileSelector::kOPT,
                              Dims4{calibBatchSize, 3, mParams.height, mParams.width});
  profileCalib->setDimensions(inputName, OptProfileSelector::kMAX,
                              Dims4{calibBatchSize, 3, mParams.height, mParams.width});
  config->addOptimizationProfile(profileCalib);

  // constructNetwork
  std::cout << mParams.onnxmodel << std::endl;
  // auto parsed = parser->parseFromFile(mParams.onnxFileName.c_str(), 0);
  auto parsed = parser->parseFromFile(mParams.onnxmodel, 0);
  if (!parsed) {
    printf("constructNetwork falid\n");
    return false;
  }
  config->setMaxWorkspaceSize(16_MiB);
  if (mParams.fp16) {
    config->setFlag(BuilderFlag::kFP16);
  }
  if (mParams.int8) {
    config->setFlag(BuilderFlag::kINT8);
    samplesCommon::setAllDynamicRanges(network.get(), 127.0f, 127.0f);
  }
  samplesCommon::enableDLA(builder.get(), config.get(), mParams.dlaCore);

  // CUDA stream used for profiling by the builder.
  // if(cudastream == NULL)
  //{
  //   cudastream = new cudaStream_t;
  //}
  auto profileStream = samplesCommon::makeCudaStream();
  if (!profileStream) {
    printf("makeCudaStream falid");
    return false;
  }
  config->setProfileStream(*profileStream);

  SampleUniquePtr<IHostMemory> plan{builder->buildSerializedNetwork(*network, *config)};
  if (!plan) {
    printf("buildSerializedNetwork falid");
    return false;
  }

  SampleUniquePtr<IRuntime> runtime{createInferRuntime(common::tensorrt::myLogger)};
  if (!runtime) {
    printf("createInferRuntime falid");
    return false;
  }

  mEngine = runtime->deserializeCudaEngine(plan->data(), plan->size()), samplesCommon::InferDeleter();
  if (!mEngine) {
    printf("deserializeCudaEngine falid");
    return false;
  }

  // ASSERT(network->getNbInputs() == 1);
  auto mInputDims = network->getInput(0)->getDimensions();
  mInfo.height_in = mInputDims.d[2];
  mInfo.width_in = mInputDims.d[3];
  // ASSERT(mInputDims.nbDims == 4);

  // ASSERT(network->getNbOutputs() == 1);
  auto mOutputDims = network->getOutput(0)->getDimensions();
  mInfo.height_out = mOutputDims.d[2];
  mInfo.width_out = mOutputDims.d[3];
  //// Caching tensorrt engine file
  // network->destroy();
  // parser->destroy();
  return true;
}

bool TADSimTRT::infer(uchar* input) {
  // post_processing
  auto threads_per_block = 128;
  auto blocks = dim3((mParams.width * mParams.height + threads_per_block - 1) / threads_per_block);
  auto threads = dim3(threads_per_block);

// #define CASTTIME
#ifdef CASTTIME
  cudaEvent_t start, stop;
  float time = 0;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start);
#endif
  pre_processing<<<blocks, threads, 0, syncstream>>>(inputdev, input, mParams.width * mParams.height);
#ifdef CASTTIME
  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time, start, stop);
  std::cout << "cast 1: " << time << std::endl;
#endif

  // rt

  float* bindings[] = {inputdev, outputdev};
#ifdef CASTTIME
  cudaEventRecord(start);
#endif
  if (!mContext->enqueueV2((void**)bindings, syncstream, nullptr)) {
    printf("faild enqueueV2\n");
    return false;
  }
#ifdef CASTTIME
  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time, start, stop);
  std::cout << "cast 2: " << time << std::endl;
#endif

#ifdef CASTTIME
  cudaEventRecord(start);
#endif
  post_processing<<<blocks, threads, 0, syncstream>>>(input, outputdev, mParams.width * mParams.height);

#ifdef CASTTIME
  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&time, start, stop);
  std::cout << "cast 3: " << time << std::endl;
#endif

  cudaStreamSynchronize(syncstream);
  return true;
}

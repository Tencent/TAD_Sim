#ifndef TADSIM_TRT_ENGINE_H
#define TADSIM_TRT_ENGINE_H


#include <string>
#include <vector>
#include <cuda_runtime.h>
#include "def.h"
typedef unsigned char uchar;

namespace nvinfer1 {
	class ICudaEngine;
	class IExecutionContext;
}
namespace samplesCommon {
	class BufferManager;
}

struct TADOnnxParams
{
	int dlaCore{ -1 };               //!< Specify the DLA core to run network on.
	bool int8{ false };                  //!< Allow runnning the network in Int8 mode.
	bool fp16{ true };                  //!< Allow running the network in FP16 mode.
       

	int width = 960; //must be width of training
	int height = 512; //must be height of training
	
	char onnxmodel[128]{ 0 };
	char cachetrt[128]{ 0 };
};

struct ModelInfo
{
	int width_in = 0;
	int height_in = 0;
	int width_out = 0;
	int height_out = 0;
};

class TXSIM_API TADSimTRT
{
public:
	TADSimTRT(const TADOnnxParams& params, cudaStream_t cudastream = 0, int gpu_id = 0);	
    ~TADSimTRT();

	/// <summary>
	/// trt one frame
	/// </summary>
	/// <param name="input">BGRA buffer in gpu memory</param>
	/// <returns>BGRA Image, don`t free it</returns>
	bool infer(uchar* input); 


	ModelInfo mInfo;
private:
	TADOnnxParams mParams;
	cudaStream_t syncstream = 0;


	bool build();
	bool ONNX2Trt();

	class nvinfer1::ICudaEngine* mEngine = nullptr;
	class nvinfer1::IExecutionContext* mContext = nullptr;
	class samplesCommon::BufferManager* mBuffers = nullptr;
	float* inputdev = 0, *outputdev = 0;
};


#endif

#ifndef LIDAR_SIMULATION_H
#define LIDAR_SIMULATION_H


#include "def.h"
#include <map>

namespace cuda_lidar
{

	//* the out result data 
	struct  CUDA_ALIGN(4) lidarsim_point {
		float x = 0, y = 0, z = 0;
		float distance = 0;
		float instensity = 0;
		unsigned int tag_c = 0u;
		unsigned int tag_t = 0u;
		float norinter = 0;
	};



	class TXSIM_API LidarSimulation
	{
	public:
		LidarSimulation();
		~LidarSimulation();

		float t_intensity = 0;
		float f_noise_dev = 0;
		float f_accuracy = 0;

		//rain members
		float f_rainfall = 0;

		//snow members
		float f_snowfall_range = 0;
		float f_snowfall_prob = 0;

		//fog members
		float f_fog = 0;

		uint8_t** depthImg = 0;
		float* refmap = 0;
		uint32_t* tagmap = 0;
		uint8_t* camidx = 0;
		uint32_t* camuv = 0;
		float* yawpitch = 0;
		uint32_t rhn =0;
		float* rtmat = 0;

		bool simulation(uint32_t bg_idx, uint32_t N, float* out, cudaStream_t stream = NULL);

	};
}

#endif

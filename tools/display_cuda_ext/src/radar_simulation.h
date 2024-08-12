#ifndef LIDAR_SIMULATION_H
#define LIDAR_SIMULATION_H


#include "def.h"
#include <map>

namespace cuda_radar
{

	class TXSIM_API RadarSimulation
	{
	public:
		RadarSimulation();
		~RadarSimulation();

		struct CUstream_st* stream = NULL;
		// remove e9
		float FC = 77;     				// Carrier frequency e9Hz
		float B = 4;                    // 雷达带宽 e9 Hz
		float S = 200e3; 				// Chirp斜率 e9 Hz/s    MHz/µs

		uint32_t Ns = 2048 * 2;         // 一个Chirp内的采样点数
		uint32_t Nd = 1024;             // 一帧内Chirp数 
		uint32_t Na = 192;				// 水平虚拟通道
		uint32_t Ne = 192;				// 垂直虚拟通道

		// signal parameter
		float	Pt_dBm = 10;			//power
		float	Gt_dBi = 15;			//Transmit antenna gain
		float	Gr_dBi = 15;			//Receive antenna gain
		float	Ts_K = 300;			//temperature
		float	Fn_dB = 12;			//Receiver noise figure
		float	L0_dB = 10;			//System attenuation
		float	L1_dB = 0;			//Atmospheric attenuation, base by weather
		int     weather = 0;

		uint32_t N = 0;
		uint8_t* rayImg = 0;

		//
		//
		float lambda = 0.3f / FC;          // 波长 m
		float rangeRes = 0.3f / (2 * B);       // 距离分辨率 m    B = c/(2*rangeRes)
		float Tc = B / S;                // Chirp上升时间 = Nr / Fs = aaa*2*maxR/c  
		float maxR = Ns * rangeRes;      // 最大需要探测距离 m
		float Fs = Ns / Tc;              // 采样频率 Hz  =Nr/Tc
		float maxV = lambda / (4 * Tc);  // 最大探测速度 m/s  230*1000/3600  
		float vres = maxV * 2 / Nd;      // 速度分辨率 m/s
		float er_power = 0;				// 综合功率（系数）

		void Init();
		void Clear();

		bool simulation(float* out);


		// for test


	private:
		float* iFx = 0;
		int plan_r2c = 0;

		float L1(int f0, int weather, float r);
	};

	/*
	
	class TXSIM_API RadarSimulation
	{
	public:
		RadarSimulation();
		~RadarSimulation();

		struct CUstream_st * stream = NULL;
		// remove e9
		float FC = 77;     				// Carrier frequency e9Hz
		float B = 4;                    // 雷达带宽 e9 Hz
		float S = 200e3; 				// Chirp斜率 e9 Hz/s    MHz/µs
		
		uint32_t Ns = 2048 * 2;         // 一个Chirp内的采样点数
		uint32_t Nd = 1024;             // 一帧内Chirp数 
		uint32_t Na = 192;				// 水平虚拟通道
		uint32_t Ne = 192;				// 垂直虚拟通道

		// signal parameter
		float	Pt_dBm=10;			//power
		float	Gt_dBi=15;			//Transmit antenna gain
		float	Gr_dBi=15;			//Receive antenna gain
		float	Ts_K=300;			//temperature
		float	Fn_dB=12;			//Receiver noise figure
		float	L0_dB=10;			//System attenuation
		float	L1_dB=0;			//Atmospheric attenuation, base by weather
		int     weather=0;

		uint32_t N = 0;
		uint8_t* pointImg = 0;
		float* yawpitch = 0;
		float* refmap = 0;

		//
		//
		float lambda = 0.3f/FC;          // 波长 m
		float rangeRes = 0.3f/(2*B);       // 距离分辨率 m    B = c/(2*rangeRes)
		float Tc = B / S;                // Chirp上升时间 = Nr / Fs = aaa*2*maxR/c  
		float maxR = Ns * rangeRes;      // 最大需要探测距离 m
		float Fs = Ns / Tc;              // 采样频率 Hz  =Nr/Tc
		float maxV = lambda / (4*Tc);  // 最大探测速度 m/s  230*1000/3600  
		float vres = maxV * 2 / Nd;      // 速度分辨率 m/s
		float er_power = 0;				// 综合功率（系数）

		void Init();
		void Clear();

		bool simulation(float* out);


		// for test


		float* SignalFX();
	private:
		float* iFx = 0;
		int plan_r2c = 0;

		float L1(int f0, int weather, float r);
	};
	//*/
}

#endif

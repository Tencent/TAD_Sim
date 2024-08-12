// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//

#pragma once
namespace radar_bit {
#define MAX_RAD_NUM (20)  // Max radar num
#define MAX_TAR_NUM (64)  // Max target num

struct radar_parameter {
  float F0_GHz = 77;      // Carrier frequency
  float Pt_dBm = 10;      // power
  float Gt_dBi = 15;      // Transmit antenna gain
  float Gr_dBi = 15;      // Receive antenna gain
  float Ts_K = 300;       // temperature
  float Fn_dB = 12;       // Receiver noise figure
  float L0_dB = 10;       // System attenuation
  float L1_dB = 0;        // Atmospheric attenuation
  float SNR_min_dB = 18;  // Minimum detectable signal to noise ratio
  float accumtime = 0;    // Accumulation time
  float radar_angle = 0;  // Radar installation angle
  float R_m = 150;        // Maximum range of target
  float rcs = 10;         // Radar cross-section
  int weather = 0;
  int tag = 0;       // waveform, only 0
  int anne_tag = 1;  // Antenna tag，1 is fov
  float hwidth = 3;  // Horizontal angle resolution
  float vwidth = 3;  // Vertical angle resolution
  float vfov = 10;   // Vertical fov
  float hfov = 20;   // Horizontal fov
};

struct tar_info_in {
  float tar_range;  // Target distance relative to radar, unit: m
  float tar_speed;  // Radial velocity of target relative to radar, unit: m/s
  float tar_angle;  // The azimuth angle of the target relative to the radar,
                    // unit: degree
  float tar_rcs;    // Radar cross-section of the target, unit: m^2
  float tar_ID;     // ID of the target
};

struct tar_info_out {
  int tar_num;                   // return num of the target
  float tar_range[MAX_TAR_NUM];  // Radial velocity of target relative to radar,
                                 // unit: m/s
  float tar_speed[MAX_TAR_NUM];  // Radial velocity of target relative to radar,
                                 // unit: m/s
  float tar_angle[MAX_TAR_NUM];  // The azimuth angle of the target relative to
                                 // the radar, unit: degree
  float tar_rcs[MAX_TAR_NUM];
  float tar_Ar[MAX_TAR_NUM];  // Amplitude of the target
  float tar_ID[MAX_TAR_NUM];  // ID of the target
};

struct output {
  double delta_R[MAX_RAD_NUM], delta_V[MAX_RAD_NUM], hfov[MAX_RAD_NUM], delta_A[MAX_RAD_NUM], R_max[MAX_RAD_NUM],
      sigma_R[MAX_RAD_NUM], sigma_V[MAX_RAD_NUM], sigma_A[MAX_RAD_NUM];
};

// -------------------Calculate radar indicator function--------------------
output Radar_Initialization(radar_parameter radar_figure, int radar_idx);

// -------------------Radar signal processing function--------------------
tar_info_out Rar_signal_pro(float radar_angle, tar_info_in tar_info[], int num, const output &initialization_out,
                            int radar_ID, int weather);
}  // namespace radar_bit

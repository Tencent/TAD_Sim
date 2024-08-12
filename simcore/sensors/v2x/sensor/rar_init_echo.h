// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <cstdint>
#include <vector>

namespace radar_bit {
#define MAX_RAD_NUM (20)
#define MAX_TAR_NUM (64)

struct radar_parameter {
  float F0_GHz;
  float Pt_dBm;
  float Gt_dBi;
  float Gr_dBi;
  float Ts_K;
  float Fn_dB;
  float L0_dB;
  // float L1_dBkm;
  float SNR_min_dB;
  // float   accumtime;
  float radar_angle;
  float R_m;
  float rcs;
  int weather;
  int tag;
  std::vector<double> ANTENNA1;
  std::vector<double> ANGLE1;
  std::vector<double> ANTENNA2;
  std::vector<double> ANGLE2;
  int anne_tag;
  float hwidth;
  float vwidth;
  float vfov;
  float hfov;
};
struct RMAX {
  double Pt_dBm;
  double Gt_dBi;
  double Gr_dBi;
  double Ts_K;
  double Fn_dB;
  double L0_dB;
  double SNR_min_dB;
};

struct tar_info_in {
  float tar_range;
  float tar_speed;
  float tar_angle;
  float tar_rcs;
  float tar_ID;
};

#pragma pack()
struct tar_info_out {
  std::uint16_t tar_num;
  float tar_range[MAX_TAR_NUM];
  float tar_speed[MAX_TAR_NUM];
  float tar_angle[MAX_TAR_NUM];
  float tar_rcs[MAX_TAR_NUM];
  float tar_Ar[MAX_TAR_NUM];
  float tar_ID[MAX_TAR_NUM];
};

struct output {
  double delta_R[MAX_RAD_NUM], delta_V[MAX_RAD_NUM], hfov[MAX_RAD_NUM], delta_A[MAX_RAD_NUM], R_max[MAX_RAD_NUM],
      sigma_R[MAX_RAD_NUM], sigma_V[MAX_RAD_NUM], sigma_A[MAX_RAD_NUM];
};
output Radar_Initialization(radar_parameter radar_figure, int radar_tag);
void antenna(float width, float width2, int radar_tag);

RMAX iRadar_Init(double Rmax, int weather, double F0_GHz, double radar_angle);

tar_info_out Rar_signal_pro(float radar_angle, tar_info_in tar_info[], int num, output initialization_out, int radar_ID,
                            int weather);
}  // namespace radar_bit

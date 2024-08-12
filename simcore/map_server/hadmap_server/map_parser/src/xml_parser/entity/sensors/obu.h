/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "base.h"

class COBU : public CSensorBase {
 public:
  COBU();
  void Reset();
  std::string m_strFrequency;
  std::string m_strDevice;
  std::string m_strDistance;
  std::string m_v2x_loss_type;
  std::string m_v2x_loss_rand_prob;
  std::string m_v2x_loss_burs_prob;
  std::string m_v2x_loss_burs_min;
  std::string m_v2x_loss_burs_max;
  std::string m_v2x_bandwidth;
  std::string m_v2x_freq_channel;
  std::string m_v2x_broad_speed;
  std::string m_v2x_delay_type;
  std::string m_v2x_delay_fixed_time;
  std::string m_v2x_delay_uniform_min;
  std::string m_v2x_delay_uniform_max;
  std::string m_v2x_delay_normal_mean;
  std::string m_v2x_delay_normal_deviation;
};

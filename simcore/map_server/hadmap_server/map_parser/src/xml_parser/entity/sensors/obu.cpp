/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "obu.h"

COBU::COBU() { Reset(); }

void COBU::Reset() {
  CSensorBase::Reset();

  m_strFrequency = "30";
  m_strDevice = ".0";
  m_strDistance = "300";
  m_v2x_loss_type = "0";
  m_v2x_loss_rand_prob = "1.00";
  m_v2x_loss_burs_prob = "1.00";
  m_v2x_loss_burs_min = "1";
  m_v2x_loss_burs_max = "5";
  m_v2x_bandwidth = "10.0";
  m_v2x_freq_channel = "5.9";
  m_v2x_broad_speed = "1";
  m_v2x_delay_type = "0";
  m_v2x_delay_fixed_time = "45.00";
  m_v2x_delay_uniform_min = "40.00";
  m_v2x_delay_uniform_max = "45.00";
  m_v2x_delay_normal_mean = "45.00";
  m_v2x_delay_normal_deviation = "1.00";
}

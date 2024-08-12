/**
 * @file sim_obu_congestion.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "sim_obu_congestion.h"
#include <iostream>

void SimObuCongestion::RunSim() {
  // std::cout << "SimObuCongestion::RunSim" << std::endl;
}

void SimObuCongestion::ObuSimExcute(const SimObus& sim, SimAlgorithmRet& sim_ret) {
  // std::cout << "SimObuCongestion::ObuSimExcute" << std::endl;
  if (sim_ret.frequency == 1)
    sim_ret.frequency = 0.3;
  else
    sim_ret.frequency = 0.782;
}

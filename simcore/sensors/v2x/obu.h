/**
 * @file obu.h
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include "../sim_algorithm/sim_obu_congestion.h"
#include "base.h"
#include "sensor_objects_def.h"

#define EGO_CAR_COVERAGE_DISTANCE 500

class Obu : public Base {
 public:
  void Init(tx_sim::InitHelper& helper);
  void Reset(tx_sim::ResetHelper& helper);
  void Step(tx_sim::StepHelper& helper);

 protected:
 private:
  void SimObuDataInput(const sim_msg::Traffic& traffic, const double time, SimObuSensors& sim);
  void FilterCars(const sim_msg::Traffic& traffic, const sim_msg::Location& egoCar);
  bool CheckObuId(int id);

 private:
  std::map<int, SimObus> m_traffic_obu_data_;
  std::vector<std::shared_ptr<SimObuBase> > m_simObu_;
  double m_step_ms = 20;
  double m_sub_step_ms = 0;
  double last_timestamp = 0;

  std::map<int64_t, double> m_obu_timestamp_last_;
};

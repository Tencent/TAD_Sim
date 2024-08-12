/**
 * @file radar.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <list>
#include <map>
#include <string>
#include "object_size.h"
#include "kdtree_flann.h"
#include "sensor.h"
#include "traffic.pb.h"
#include "txsim_module.h"
#include "union.pb.h"


class SimRadar final : public tx_sim::SimModule {
 public:
  SimRadar();
  virtual ~SimRadar();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  tx_sim::Vector3d map_ori;
  bool ignoreZ{false};
  std::string device;
  std::string debugDir;
  std::string tadsimPath;
  bool LoadMap(const std::string &fname);
  double car_rcs(double a);
  double man_rcs(double a);
  double obj_rcs(double a);
  radar_bit::output initOut{{0}};

  std::list<std::pair<double, std::pair<size_t, std::pair<std::string, std::string>>>> ls_traffic;
  double max_delay = 0;

  struct Object : public ObjectBBox {
    double pos_random_len = 0;
    double pos_random_yaw = 0;
    double angle = 0;
    double rcs = 0;
  };
  std::vector<Object> objects;
  kdtree_flann<double, size_t> bpFlann;

  struct TmpData {
    double vert_angle = 0;
    double hor_sita = 0;
    double vert_sita = 0;
    double size = 0;
  };
  std::map<int, TmpData> tmpdatas;
  std::vector<radar_bit::tar_info_in> handle_traffic(const Sensor &radar, const Eigen::Vector3d &senser_postion,
                                                     const Eigen::Vector3d &egov, const sim_msg::Union &union_location,
                                                     const sim_msg::Traffic &traffic);
};

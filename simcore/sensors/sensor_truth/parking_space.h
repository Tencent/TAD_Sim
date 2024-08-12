/**
 * @file ParkingSpace.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <map>
#include <vector>
#include "fov_filter_sensor.h"
#include "hmap.h"
#include "truth_base.h"
#include "kdtree_flann.h"
#include "parking_space.pb.h"
#include "traffic.pb.h"
#include "union.pb.h"


class ParkingSpace : public TruthBase {
 public:
  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  struct parkingbuf {
    int id = 0;
    int type = 0;
    size_t roadid = 0;
    double depth = 0;
    double length = 0;
    double yaw = 0;
    Eigen::Vector3d pos_84;
    Eigen::Vector3d pos;
    Eigen::Vector3d p_84[4];  // lb,lt,rt,rb  in wgs84
    Eigen::Vector3d p[4];     // lb,lt,rt,rb
  };
  std::vector<parkingbuf> parkings;

  kdtree_flann<double, size_t> bpFlann;
  hadmap::txLaneId egoLaneid;
  hadmap::txRoadPtr roadPtr;

  bool cal_parking(parkingbuf &pkb, double &hdg);
  void obj2pk(const hadmap::txObject *obj);
  bool InitMap();

  // swap
  struct ObsSwap {
    sim_msg::Obstacle world;
    sim_msg::Obstacle st;
    hadmap::roadpkid roadid = ROAD_PKID_INVALID;
  };
  std::map<int, ObsSwap> obs_swap;
  struct ParkingSwap {
    sim_msg::Parking world;
    sim_msg::Parking st;
    hadmap::roadpkid roadid = ROAD_PKID_INVALID;
  };
  std::map<int, ParkingSwap> parking_swap;

  void cal_wd(const Eigen::Vector3d &pt, sim_msg::ParkingPoint *pp);

  bool cal_st(const Eigen::Vector3d &pt, sim_msg::ParkingPoint *pp);
  void handle_ego(const sim_msg::Union &union_location, sim_msg::ParkingSpace &tParkingspace,
                  const FovFilterSensor &fov);
  void handle_traffic(const sim_msg::Traffic &traffic, sim_msg::ParkingSpace &tParkingspace,
                      const FovFilterSensor &fov);
  void handle_uss(const sim_msg::Traffic &traffic, const sim_msg::Union &union_location,
                  sim_msg::ParkingSpace &tParkingspace);
};

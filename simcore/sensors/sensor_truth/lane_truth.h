// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <map>
#include <vector>
#include "age.h"
#include "fov_filter_sensor.h"
#include "hmap.h"
#include "truth_base.h"
#include "detectedlane.pb.h"
#include "structs/hadmap_lanelink.h"
#include "traffic.pb.h"
#include "types/map_defs.h"

class LaneTruth : public TruthBase {
 public:
  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  hadmap::txLaneId lastLid;
  double maxDetectLen = 50;
  double nearestLimit = 8;

  using EPointList = std::vector<Eigen::Vector3d>;
  struct Boundary {
    hadmap::LANE_MARK mark = hadmap::LANE_MARK_None;
    double lenght = 0;
    EPointList pts;
  };
  std::vector<Boundary> mapBounds;

  struct Lane {
    Eigen::Vector3d normBegin, normEnd;
    std::vector<hadmap::txLaneId> prev, next;
    size_t left = BOUN_PKID_INVALID;
    size_t right = BOUN_PKID_INVALID;
    hadmap::txLaneId leftLane;
    hadmap::txLaneId rightLane;
  };
  std::map<hadmap::txLaneId, Lane> mapLanes;
  Age lbdyAge;

  bool selectpts(FovFilterSensor &fov, const EPointList &orip, EPointList &dstp, double &slen, double &len);

  void collectpts(FovFilterSensor &fov, const hadmap::txLaneId &laneid, EPointList &dstp, bool isleft, double &dstart,
                  double &dend, hadmap::laneboundarypkid &fstBid);

  bool cal3c(const EPointList &orip, double coe[4]);

  bool detect(FovFilterSensor &fov, sim_msg::DetectedLaneBoundarySeries &lanebdys, const hadmap::txLaneId &laneid,
              bool isleft, int id);

  void calLen(Boundary &bds);
  void soline(EPointList &pts);
  void calnormal(const hadmap::PointVec &pts, Eigen::Vector3d &begin, Eigen::Vector3d &end);
  size_t addbound(hadmap::txLaneBoundaryPtr bd, std::map<hadmap::laneboundarypkid, size_t> &boudidxmap);
  void calLinkBdy(Boundary &bdy, const EPointList &cenPts, const EPointList &prl, const EPointList &nxl, double arot);
  void sortLanes();
  bool InitMap();
  void InitMap_s1(hadmap::roadpkid &roadlink0, std::map<hadmap::laneboundarypkid, size_t> &boudidxmap);
  struct Roadlk {
    double sita;
    std::vector<double> others;
    std::vector<hadmap::txLaneLinkPtr> rlinks;
  };
  std::map<hadmap::roadpkid, Roadlk> InitMap_s2_rlink(const hadmap::txRoadPtr &road_ptr);
  void InitMap_s2(std::map<hadmap::txLaneLinkPtr, int> &linelinks);
};

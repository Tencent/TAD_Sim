// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "eigen3/Eigen/Dense"

#include "basic.pb.h"
#include "laneMarks.pb.h"
#include "location.pb.h"
#include "traffic.pb.h"

#include "mapengine/hadmap_engine.h"
#include "structs/base_struct.h"
#include "structs/hadmap_predef.h"
#include "txsim_messenger.h"
#include "txsim_module.h"
#include "types/map_defs.h"

class EgoLaneMarker final : public tx_sim::SimModule {
 public:
  EgoLaneMarker();
  virtual ~EgoLaneMarker();

  void Init(tx_sim::InitHelper& helper) override;
  void Reset(tx_sim::ResetHelper& helper) override;
  void Stop(tx_sim::StopHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;

 private:
  bool checkGeometry(const hadmap::txLanePtr& lane);
  double getDistanceAlongLane(const hadmap::txLanePtr& lane, const hadmap::txPoint& loc, hadmap::txPoint& pedal);
  double getRemainLengthToNextJunction(const hadmap::txLanePtr& curLane, double passedDis);
  // lane No. is from right to left, starting from 0.
  size_t getEgoLaneNo(const hadmap::txLanePtr& curLane);
  void countEgoLaneKeepTime(const hadmap::txLanePtr& lane);
  void retriveLineData(sim_msg::LaneMarks& laneMarks, const hadmap::txLanePtr& curLane, double passedDis, bool forward,
                       double totalLength);
  void getLaneMarks(sim_msg::LaneMarks& laneMarks, const hadmap::txLanePtr& baseLane, double startPos, double length,
                    bool left, bool forward);
  void fillSampleBoundaryPoint(sim_msg::LaneMark* laneMark, const hadmap::txLaneBoundaryPtr& boundary, double startPos,
                               double length, bool forward);

  // we check in this API whether this lane is connected to Junction
  /****************************************************************/
  hadmap::txLanePtr getNextRoadLanes(const hadmap::txLanePtr& lane);
  /****************************************************************/

  void getEgoLaneArrow(sim_msg::LaneMarks& laneMarks, hadmap::LANE_ARROW arrow);

  hadmap::txLanePtr chooseNextLane(const hadmap::txLane& currentLane, const hadmap::txLanes& nextLanes);
  void transformCoordinates(sim_msg::LaneMarks& laneMarks, const hadmap::txPoint& locPoint,
                            const hadmap::txLanePtr& cur_lane, const sim_msg::Vec3& rpy, sim_msg::Traffic& traffic,
                            double passed_dis);
  void transformCoordinates(sim_msg::LaneMark& laneMark, const hadmap::txPoint& loc, const Eigen::Matrix3d& m);
  void transformCoordinates(sim_msg::Traffic& traffic, const hadmap::txPoint& loc, const Eigen::Matrix3d& m);

  void adjustTrafficTheta(sim_msg::Traffic& traffic, double egoTheta);  // transform traffic theta angle to Ege frame
  void getInitialParameters(tx_sim::InitHelper& params);
  void checkHandleCode(int optCode);
  hadmap::txLaneBoundaryPtr fetchLaneBoundary(hadmap::laneboundarypkid id);
  hadmap::txLanePtr fetchNextLane(const hadmap::txLanePtr& lane);
  hadmap::txLanePtr fetchPrevLane(const hadmap::txLanePtr& lane);
  hadmap::txLanePtr fetchLeftLane(const hadmap::txLanePtr& lane);
  hadmap::txLanePtr fetchRightLane(const hadmap::txLanePtr& lane);
  void doTransformingWork(size_t tid);
  void resetMapHandle();

  bool isRoadEndLane(std::string laneUniqueId);
  bool isConnectToJunction(std::string laneUniqueId);
  void resetLaneState();

  void setLaneColor(int32_t& LM, sim_msg::LaneMark* lane);  // set the color of lanemarks
  void getTwoSideLaneMarksNumber(const hadmap::txLanePtr& lane, int& leftNum,
                                 int& rightNum);       // get ego two side lane number
  bool isLaneDrivable(const hadmap::txLanePtr& lane);  // check whether the lane is drivable

  void FillMiddlePoints(const hadmap::txLanePtr currentLane, bool forward, double startPos, double remainingLength);

  double forwardLength = 80;          // the length of forward lane mark detection range of the ego car.
  double sampleInterval = 3;          // the interval between 2 consective sample points of the lane mark.
  size_t detectingLaneMarkCount = 2;  // the count of lane mark be detected per side.
  bool isOuputInLonLat = false;       // true for debug use.
  std::uint64_t frameCount = 0;
  std::string payload;
  hadmap::txMapHandle* mapHandle = nullptr;  // hadMap handle
  std::unordered_map<hadmap::laneboundarypkid, hadmap::txLaneBoundaryPtr> cachedBoundaries;
  std::unordered_map<std::string, hadmap::txLanePtr> cachedLeftLanes;
  std::unordered_map<std::string, hadmap::txLanePtr> cachedRightLanes;
  std::unordered_map<std::string, hadmap::txLanePtr> cachedNextLanes;
  std::unordered_map<std::string, hadmap::txLanePtr> cachedPrevLanes;
  struct TransformData {
    sim_msg::LaneMark* laneMark;            // 一个车道数
    const hadmap::txPoint* locPoint;        // 主车位置 GPS
    const Eigen::Matrix3d* rotationMatrix;  // 坐标旋转矩阵 ENU->车身坐标
  };
  std::unordered_map<size_t, TransformData> transformed;
  std::mutex mtx;
  std::condition_variable cv;
  hadmap::txLaneId lastEgoLaneId;       // 记录上一次主车所在车道的 ID 包括 roadpkid roadId sectionpkid
                                        // sectionId; lanepkid laneId;
  hadmap::txSectionPtr lastEgoSection;  // txSection 智能指针
  size_t lastEgoLaneNo = 0;             // 上一帧主车车道号码 从左到右 0-1-2...
  uint32_t laneKeepMs = 0;              // 主车在车当上待的时间
  uint32_t stepIntervalMs = 0;          // 记录step仿真步长

  std::set<std::string> roadEndLaneUniqIdVector{};      // not used
  std::set<std::string> connectToJunctionLaneIdVect{};  // store the lane Id of lane which is connected to a junction

  std::map<std::string, std::vector<bool>> laneStateCheck{};

  int leftValidatIndex = 0;
  int rightValidatIndex = 0;

  int leftLaneNum, rightLaneNum;  // ego left and right lane number
  std::list<std::tuple<double, double, double>> m_listMiddlePoint;
};

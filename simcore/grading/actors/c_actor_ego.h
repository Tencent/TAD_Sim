// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "actors/c_actor_vehicle.h"
#include "stateflow/veh_state_flow.h"

namespace eval {
class CEgoActor;
class CEgoActorBuilder;

/**
 * @brief "主车", ego actor type, which is a vehicle actor.
 */
class CEgoActor final : public CVehicleActor {
 public:
  const std::string _name = "CEgoActor";
  using CLocationPtr = std::shared_ptr<CLocation>;
  CEgoActor();
  virtual ~CEgoActor();
  friend CEgoActorBuilder;

 protected:
  /**
   * @brief update map info. This function calls the UpdateMapInfo method of m_map_agent to update map information based
   * on the provided task.
   * @param task a const reference of type MapQueryTaskCfg, containing the information required to update the map.
   */
  inline void UpateMapInfo(const MapQueryTaskCfg &task) { m_map_agent.UpdateMapInfo(task); }

  /**
   * @brief test if actor on solid/dotted line
   */
  void CheckActorOnSolidOrDottedLine();

  /**
   * @brief test if actor on one specific stop line
   */
  void CheckActorOnStopLine();

  /**
   * @brief test if actor on one crosswalk
   */
  void CheckActorOnCrosswalk();

  /**
   * @brief calculate actor lateral offset from reference line
   */
  void CalRefLineLateralOffset();

  /**
   * @brief calculate state of actor wheels with stop line
   */
  void CalStateWihtStopLine();

  /**
   * @brief calculate actor lateral yaw from reference line
   */
  void CalRefLineLateralYaw();

  /**
   * @brief calculate max distance from ego to the furthest point of reference line
   */
  void CalRefLineMaxDistance();

  /**
   * @brief check if number of lanes in front of ego is increasing or decreasing
   */
  void CheckLaneNumChange();

  /**
   * @brief calculate vehicle speed distance vector
   */
  void CalSpeedDistanceYaw();

  /**
   * @brief get specific obj from map file in range
   * @param obj_type obj type to get
   * @param range radius of the ego to be checked
   * @return a list of map objects
   */
  EvalMapObjectPtr GetSpecificTypesOfObjects(const hadmap::OBJECT_TYPE &obj_type, double radius = 100.0);

  /**
   * @brief get the ptr to the modifiable vehicle body control command object.
   */
  inline VehicleBodyControl *MutableVehicleBodyCmd() { return &m_veh_body_cmd; }
  /**
   * @brief get the ptr to the modifiable vehicle state object.
   */
  inline VehicleState *MutableVehicleState() { return &m_veh_state; }
  /**
   * @brief get the ptr to the modifiable vehicle wheels position object.
   */
  inline RectCorners *MutableWheelPosENU() { return &m_wheel_pos; }

 public:
  /**
   * @brief retrieve a constant reference the vehicle body command.
   */
  inline const VehicleBodyControl &GetVehicleBodyCmd() { return m_veh_body_cmd; }
  /**
   * @brief retrieve a constant reference the vehicle state.
   */
  inline const VehicleState &GetVehicleState() { return m_veh_state; }

  /**
   * @brief get map info.
   */
  inline ActorMapInfoPtr GetMapInfo() { return m_map_agent.GetMapInfo(); }

  /**
   * @brief Check if ego is on the solid boundary. If so, copy the boundary information into the param.
   * @param boundry_on Reference of type LaneBoundry, used to store info about the current solid boundary (if any).
   * @return bool Returns a value indicating whether ego is on the solid boundary.
   */
  bool IsOnSolidBoundry(LaneBoundry &boundry_on);

  /**
   * @brief Check if ego is on the stricter solid boundary. If so, copy the boundary information into the param.
   * @param boundry_on Reference of type LaneBoundry, used to store info about the current stricter solid boundary (if
   * any).
   * @return bool Returns a value indicating whether ego is on the stricter solid boundary.
   */
  bool IsOnSolidSolidBoundry(LaneBoundry &boundry_on);

  /**
   * @brief Check if ego is on the dotted boundary. If so, copy the boundary information into the param.
   * @param boundry_on Reference of type LaneBoundry, used to store info about the current dotted boundary (if any).
   * @return bool Returns a value indicating whether ego is on the dotted boundary.
   */
  bool IsOnDottedBoundry(LaneBoundry &boundry_on);

  /**
   * @brief Check if ego is on the stop line. If so, copy the boundary information into the param.
   * @param boundry_on Reference of type LaneBoundry, used to store info about the current stop line (if any).
   * @return bool Returns a value indicating whether ego is on the stop line.
   */
  bool IsOnStopLine(StopLine &stop_line_on);

  /**
   * @brief Check if ego is on the crosswalk. If so, copy the boundary information into the param.
   * @param boundry_on Reference of type LaneBoundry, used to store info about the current crosswalk (if any).
   * @return bool Returns a value indicating whether ego is on the crosswalk.
   */
  bool IsOnCrosswalk(Crosswalk &crosswalk_on);

  /**
   * @brief Check if the number of lanes of the road where the ego is on will increase.
   * @param radius radius of the ego to be checked when radius > 0. please focus on the provided radius in
   * CalRefLine() which also has impacts.
   */
  bool IsLaneNumIncreasing(double radius = -1.0);

  /**
   * @brief Check if the number of lanes of the road where the ego is on will decrease.
   * @param radius radius of the ego to be checked when radius > 0. please focus on the provided radius in
   * CalRefLine() which also has impacts.
   */
  bool IsLaneNumDecreasing(double radius = -1.0);

  /**
   * @brief Get the lateral offset of the ego from the lane reference line.
   */
  double GetRefLineLateralOffset() { return m_ref_line_lateral_offset; }

  /**
   * @brief Get the lateral yaw of the ego from the lane reference line.
   */
  double GetRefLineLateralYaw() { return m_ref_line_lateral_yaw; }

  /**
   * @brief Get the max distance from ego to the furthest point of reference line
   */
  double GetRefLineMaxDistance() { return m_ref_line_max_distance; }

  /**
   * @brief Get the distance of the ego from the stop line.
   */
  double GetStopLineRoadDistance() {
    // CheckDistanceFromStopLine();
    return m_stop_line_s_dist;
  }

  /**
   * @brief Get the wheels position of the ego. (fl fr rr rl)
   * @return RectCorners 主车四轮位置.
   */
  const RectCorners &GetWheelPosENU() const { return m_wheel_pos; }

  /**
   * @brief Get the positional relationship state between the ego (its wheel position) and the stop line.
   */
  StateWhithStopLine GetStateWithStopLine() { return m_state_with_stopline; }

  /**
   * @brief Checks for signs of a specific type within the given radius, returns true if found, and stores the result in
   * the obj param.
   * @param obj The pointer where the result is stored.
   * @param sub_type_set The desired object subtype list, which must belong to the type of obj_type below.
   * @param obj_type represents the object type we care about.
   * @param radius represents the distance to search for objects of a specific type.
   * @param front_or_rear is used to specify whether the logo should be in front or behind the actor. 0 --> error; 1 -->
   * front; 2 ---> rear
   * @return bool Returns a Boolean value indicating whether is successful to get.
   */
  bool GetSign(EvalMapObjectPtr obj, const std::set<hadmap::OBJECT_SUB_TYPE> &sub_type_set,
               const hadmap::OBJECT_TYPE &obj_type = hadmap::OBJECT_TYPE_TrafficSign, double radius = 100.0,
               uint8_t front_or_rear = 1);

  /**
   * @brief Obtains its position info in the ENU coordinate system based on the provided object. This function first
   * obtains the position of the object on the map, then convert it from WGS84 coordinate system to ENU coordinate
   * system. At the same time, obtain the attitude, including roll angle (roll), pitch angle (pitch) and yaw angle
   * (yaw).
   * @param obj A constant reference of type hadmap::txObjectPtr, representing the target object on the map.
   * @return eval::CLocation returns the position and attitude information of the target object in the ENU coordinate
   * system.
   */
  eval::CLocation GetObjectCLocation(const hadmap::txObjectPtr &obj);

 public:
  /**
   * @brief check given position is front or rear of ego. 0 --> error 1 --> front 2 ---> rear
   */
  uint8_t CheckFrontOrRear(const eval::CLocation &loc);

  /**
   * @brief check four wheels of the main vehicle within the roadside
   */
  bool CheckWheelWithinTheLaneBoundry();

  /**
   * @brief check four corners of the main vehicle within the road boundary
   */
  bool CheckCornerWithinTheLaneBoundry();

  /**
   * @brief Obtain a speed limit sign in front of the main vehicle (100m)
   */
  bool GetSpeedLimitSign(EvalMapObjectPtr obj, double radius = 100);

  /**
   * @brief Obtain indicator signs (bus stop signs, keep right, turn left, roundabout) in front of the main vehicle
   * (100m)
   */
  bool GetIndicateSign(EvalMapObjectPtr obj, double radius = 100);

  /**
   * @brief check the current lane of the main vehicle is in the rightmost lane
   */
  bool CurLaneIsRightmostLane();

  /**
   * @brief check the current lane of the main vehicle is in the leftmost lane
   */
  bool CurLaneIsLeftmostLane();

  /**
   * @brief Obtain various warning signs 100 meters ahead of the main vehicle
   */
  bool GetWarningSign(EvalMapObjectPtr obj, double radius = 100);

  /**
   * @brief Obtain various prohibited signs 100 meters ahead of the main vehicle
   */
  bool GetProhibitionSign(EvalMapObjectPtr obj, double radius = 100);

  /**
   * @brief get stop line and crosswalk
   */
  bool GetStopLineAndCrossWalk(EvalMapObjectPtr obj, double radius = 100);

  /**
   * @brief distance between the front of vehicle and the stop line (update m_stop_line_s_dist)
   */
  void CheckDistanceFromStopLine();

  /**
   * @brief check is motorway
   */
  bool IsMotorway();

  /**
   * @brief check is reversing
   */
  bool IsReverse();

  /**
   * @brief is the obstacle located behind the vehicle
   */
  bool TheObstacleIsBehindVehicle(hadmap::objectpkid obj_id, double radius = 100);

  /**
   * @brief Determine the position of obstacles based on CLocation
   */
  bool TheObstacleIsBehindVehicle(const CLocation &loc);

  /**
   * @brief Is the obstacle behind the vehicle
   */
  bool TheObstacleIsBehindVehicle(hadmap::txObjectPtr obj);

  /**
   * @brief Obtain road markings ahead of the main vehicle(stopline  arrow crosswalk)
   */
  bool GetFrontRoadMarkings(std::vector<EvalMapObjectPtr> &obj, double radius = 20);

  /**
   * @brief Check Distance With Object
   */
  double CheckDistanceWithObject(CPosition pos);

  /**
   * @brief get the closest parking space to the vehicle
   */
  hadmap::txObjectPtr getClosestParkingSpace(double radius = 100);

 protected:
  VehicleBodyControl m_veh_body_cmd;         // vehicle body command
  VehicleState m_veh_state;                  // VehicleState
  LaneBoundry m_solid_boundry;               // solid boundry info actor on
  LaneBoundry m_solid_solid_boundry;         // solidsolid boundry info actor on
  LaneBoundry m_dotted_boundry;              // dotted boundry info actor on
  StopLine m_stopline;                       // stop line info actor on
  Crosswalk m_crosswalk;                     // crosswalk info actor on
  StateWhithStopLine m_state_with_stopline;  // state of actor wheels with stop line
  CLocationPtr m_previous_frame_location;    // previous frame
  bool m_on_solid_boundry;                   // indicators
  bool m_on_solid_solid_boundry;             // indicators

  bool m_on_dotted_boundry;  // if ego is on dotted boundry
  bool m_on_stop_line;       // if ego is on stop line
  bool m_on_crosswalk;       //  if ego is on crosswalk
  bool m_laneNum_Dec;        // if lane number is decreasing
  bool m_laneNum_Inc;        // if lane number is increasing

  double m_ref_line_lateral_offset;  // lateral offset from reference line
  double m_ref_line_lateral_yaw;     // lateral yaw from reference line
  double m_ref_line_max_distance;    // lateral yaw from reference line
  double m_stop_line_s_dist;         // distance from stop line in sl coord
  double m_speed_yaw;                // speed yaw

  // dynamic actor map info
  MapQueryAgent m_map_agent;
  RectCorners m_wheel_pos;
};

/**
 * @brief "主车集合", assembly ego type. Include all the actors vehicle, ego and trailers. It is used to process the ego
 * vehicle state.
 */
class CEgoAssemble {
 public:
  CEgoAssemble();
  virtual ~CEgoAssemble();

 public:
  /**
   * @brief get front of ego
   */
  inline CEgoActorPtr GetFront() { return m_front; }

  /**
   * @brief if has trailers
   */
  bool HasTrailer();

  /**
   * @brief get all trailers
   */
  inline const std::vector<CEgoActorPtr> GetTrailers() { return m_trailers; }

  /**
   * @brief get first trailer
   */
  inline CEgoActorPtr GetFirstTrailer() { return m_trailers.size() > 0 ? m_trailers.at(0) : nullptr; }

 public:
  /**
   * @brief get vehicle body command
   */
  VehicleBodyControl GetVehicleBodyCmd();

  /**
   * @brief get map info
   */
  ActorMapInfoPtr GetMapInfo();

  /**
   * @brief get if ego on solid boundry
   */
  bool IsOnSolidBoundry(LaneBoundry &boundry_on);

  /**
   * @brief get if ego on solidsolid boundry
   */
  bool IsOnSolidSolidBoundry(LaneBoundry &boundry_on);

  /**
   * @brief get if ego on dotted boundry
   */
  bool IsOnDottedBoundry(LaneBoundry &boundry_on);

  /**
   * @brief get if ego on stop line
   */
  bool IsOnStopLine(StopLine &stop_line);

  /**
   * @brief get ego lateral offset from reference line
   */
  double GetRefLineLateralOffset();

  /**
   * @brief get actor lateral yaw from reference line
   */
  double GetRefLineLateralYaw();

  /**
   * @brief if has ref line
   */
  bool HasRefLine();

 public:
  void SetFront(CEgoActorPtr front);
  void AddTrailer(CEgoActorPtr trailer);

 private:
  CEgoActorPtr m_front;
  std::vector<CEgoActorPtr> m_trailers;
};

/**
 * @brief "主车建造者", ego actor builder, which build the actors from the map and the msg.
 */
class CEgoActorBuilder : public CActorBuilder {
 public:
  virtual void Build(const EvalMsg &msg, ActorReposity &actor_repo) override;

 protected:
  static void BuildFrom(const EvalMsg &msg, ActorReposity &actor_map);
  static CJerk sPreAcc;
};
}  // namespace eval

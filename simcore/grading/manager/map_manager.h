// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

/*
 * @autor dongyuanhu
 * Contact dongyuanhu@tencent.com
 *
 * @brief mapsdk wrapper for performance reason, singleton used
 */

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "map_data_type.h"
#include "utils/eval_fwd.h"
#include "utils/eval_utils.h"

namespace eval {
/**
 * map info of one actor, including lane/lanelink info with its
 * sampled points, pre/next info, reference line, left/right boundry
 */
struct ActorMapInfo {
 public:
  LaneID m_lane_id;  // if m_on_road is false, lane id info will be cleared. if m_in_junction is false, lanelink
                     // info will be cleared

  // actor on road
  bool m_on_road;  // if m_on_road is false, m_active_lane is the lastest valid lane info

  // actor in junction
  bool m_in_junction;  // if m_in_junction is false, m_active_lanelink is the lastest valid lanelink info

  EvalLane m_active_lane;  // current actor lane, if m_on_road is false, this variable will NOT be cleared
  EvalRoad m_active_road;  // current road
  EvalLaneLink
      m_active_lanelink;  // current actor lane link, if m_in_junction is false, this variable will NOT be cleared

  ActorMapInfo() {
    m_lane_id.Clear();
    m_on_road = false;
    m_in_junction = false;
  }
};

/**
 * one task configuration to get map info
 * param[in] m_pivot_enu one point in enu coordinate, which will be used as a pivot point to query map
 * param[in] m_query_lane_id one lane id which will be queried, used only when m_use_query_laneid is true
 * param[in] m_use_query_laneid switch if use m_query_lane_id, default is false
 * param[in] m_use_sample switch if sample boundry/ref line info, default is true
 */
struct MapQueryTaskCfg {
  CPosition m_pivot_enu;
  LaneID m_query_lane_id;
  bool m_use_query_laneid;
  bool m_use_sample;

  MapQueryTaskCfg(const CPosition &pivot_enu, const LaneID &query_lane_id = LaneID(), bool use_query_laneid = false,
                  bool use_sample = true)
      : m_pivot_enu(pivot_enu),
        m_query_lane_id(query_lane_id),
        m_use_query_laneid(use_query_laneid),
        m_use_sample(use_sample) {}
  MapQueryTaskCfg() : MapQueryTaskCfg(CPosition()) {}
};

/*
 * @brief actor map info update agent, start a thread to update map info.
 * @brief the update thread is started once this agent object is created.
 * @brief each of this agent has a thread, remember this. when agent object is deleted, background task will stop.
 * @brief there is no need to concern about the start/stop of thread
 */
struct MapQueryAgent : public NoneCopyable {
 public:
  explicit MapQueryAgent(double update_thresh = 3.0) {
    m_update_thresh = update_thresh;
    m_query = std::make_shared<ActorMapInfo>();
    m_last_enu.SetValues(INT32_MIN, INT32_MIN, INT32_MIN);
    m_update_task.m_pivot_enu = m_last_enu;
    StartTask();
  }
  virtual ~MapQueryAgent() {
    StopTask();
    m_query.reset();
  }

 public:
  /*
   *	@brief get queried map info, if one new map info is ready
   */
  ActorMapInfoPtr GetMapInfo();
  /*
   *	@brief update map info
   *	@param pivot_enu, point in ENU, if use_id_from_task is false, agent will get map info by this pivot
   *	@param query_lane_id�� query lane id
   *	@param use_query_laneid, if true, agent will use query_lane_id
   */
  void UpdateMapInfo(const MapQueryTaskCfg &task);

 private:
  void MapUpdataThreadFunc();
  void StartTask() {
    m_stop_task = false;
    if (!m_task_thread.joinable()) m_task_thread = std::thread(&MapQueryAgent::MapUpdataThreadFunc, this);
  }
  void StopTask() {
    m_stop_task = true;
    if (m_task_thread.joinable()) m_task_thread.join();
  }

 private:
  ActorMapInfoPtr m_query;  // queried map info
  std::mutex m_query_lock;

  MapQueryTaskCfg m_update_task;  // map update task
  std::mutex m_update_lock;       // lock

  CPosition m_last_enu;    // latest enu
  double m_update_thresh;  // thresh to update map, unit m

  std::thread m_task_thread;      // map update thread
  std::atomic<bool> m_stop_task;  // if stop update task
};

/**
 * @brief "地图管理器", map manager, manages all map behavior, including map query, map update, etc.
 */
class MapManager : public NoneCopyable {
 private:
  explicit MapManager(double radius = 20.0);

 public:
  friend CMapManager;
  virtual ~MapManager() { Release(); }

  // connect and release map
  bool Connect(const std::string &map_file, const std::string &start_loc_payload);
  void Release();

 public:
  // query map info sync
  bool QueryMapInfo(ActorMapInfoPtr &m_query, const MapQueryTaskCfg &task);

  // coordinate convert
  void WGS84ToENU(CPosition &pt);
  void ENUToWGS84(CPosition &pt);

  // query map objects from hadmap, the reason that this function is not in MapQueryAgent is objects that user
  // want is not fixed.
  EvalMapObjectPtr GetMapObjects(const hadmap::OBJECT_TYPE &obj_type, const CPosition &pivot_enu, double radius = 20.0);

  EvalMapObjectPtr GetMapObjects(const hadmap::OBJECT_TYPE &obj_type,
                                 const std::vector<hadmap::OBJECT_SUB_TYPE> &obj_sub_types, const CPosition &pivot_enu,
                                 double radius = 20.0);

  EvalMapStatActorPtr GetMapStatActors(const hadmap::OBJECT_TYPE &obj_type, const CPosition &pivot_enu,
                                       double radius = 20.0);
  /*
   * convert hadmap points to eval points, use pivot_enu to filter point which is too far, determined by radius
   * this operation is heavy, consider invoke this function within a thread
   */
  void ConvertToEvalPoints(const hadmap::txCurve *curve_wgs84, EvalPoints &sample_points,
                           const CPosition &pivot_enu = {0.0, 0.0, 0.0}, double provided_radius = -1.0,
                           bool enable_pivot_filter = true, bool sample_yaw = true);

  // get lanelink by id
  hadmap::txLaneLinkPtr GetLaneLink(const hadmap::lanelinkpkid &lanelink_pkid);

  // get lanes by lane_id
  hadmap::txLanes GetLanesUnderSection(const hadmap::txLaneId &lane_id);

  // private:
  // get the lestmost lane boundary of actor driving road
  hadmap::txLaneBoundaryPtr GetLeftmostLaneBoundary(const hadmap::txLaneId &lane_id);

  // calculate the number of driving ways in the direction of the actor
  int CalDrivingLaneNum(const hadmap::txLaneId &lane_id);

  // calculate lane width
  double CalLaneWidth(const hadmap::txLaneId &lane_id);

  // whether it an opposite direction vehicle
  bool IsOppositeDirection(const CLocation &loc_1, const CLocation &loc_2);

  // whether two points in range
  bool IsInRange(const CLocation &loc_1, const CLocation &loc_2, const double radius);

  double GetSpeedLimitFromMap(const hadmap::txLanePtr &lane_ptr);

  uint8_t GetSlopeGrade(const hadmap::txRoadPtr &road_ptr);
  bool GetCondOnSlopeGrade(const hadmap::txRoadPtr &road_ptr);

  double GetSharpCurve(const hadmap::txRoadPtr &road_ptr);
  bool GetCondOnSharpCurve(const hadmap::txRoadPtr &road_ptr);

  bool GetCondUnbidirectional(const hadmap::txLaneId &lane_id);

  bool GetCondOnNarrowRoad(const hadmap::txLanePtr &lane_ptr);

  bool GetCondHasParkingSign(const CPosition &pivot_enu, double radius = 20.0);

  bool GetCondHasParkingMarking(const CPosition &pivot_enu, double radius = 20.0);

  bool GetCondHasTunnel(const CPosition &pivot_enu, double radius = 3.0);

  bool GetCondHasPedestrianBridge(const CPosition &pivot_enu, double radius = 3.0);

 public:
  // get lane by id
  hadmap::txLanePtr GetLane(const eval::LaneID &lane_id);
  hadmap::txLanePtr GetLane(const hadmap::txLaneId &lane_id);

  // get road by lane-id
  hadmap::txRoadPtr GetRoad(const eval::LaneID &lane_id);
  hadmap::txRoadPtr GetRoad(const hadmap::txLaneId &lane_id);

  // get section by lane-id
  hadmap::txSectionPtr GetSection(const eval::LaneID &lane_id);
  hadmap::txSectionPtr GetSection(const hadmap::txLaneId &lane_id);

 private:
  // calculate map lines
  hadmap::txLanePtr CalLaneByPosition(double longti, double lati, double alti);
  hadmap::txLaneLinkPtr CalLaneLinkByPosition(double longti, double lati, double alti);
  bool CalLaneBoundries(hadmap::txLanePtr &lane_ptr, ActorMapInfoPtr &map_query, const MapQueryTaskCfg &task);
  bool CalLaneSingleBoundry(hadmap::txLanePtr &lane_ptr, LaneBoundry &lane_boundary, const MapQueryTaskCfg &task,
                            const bool is_left_bdr);
  void CalRefLine(hadmap::txLanePtr &lane_ptr, ActorMapInfoPtr &map_query, const MapQueryTaskCfg &task);
  void TransAllObjects();

 private:
  hadmap::txPoint _map_origin;
  MAPState _state;
  hadmap::txMapHandle *_map_handle;

  std::string _map_path;
  hadmap::MAP_DATA_TYPE _map_type;

  double _sample_step;  // unit m
  double _radius;       // unit m

  std::map<int32_t, EvalMapObjectPtr> m_map_objects;
  std::map<int32_t, EvalMapStatActorPtr> m_map_stat_actors;
};
}  // namespace eval

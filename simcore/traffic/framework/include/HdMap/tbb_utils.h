// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tbb/tbb.h>
#if TBB_NEW_API
#include <tbb/task_arena.h>
#endif
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_header.h"
#include "tx_spatial_query.h"
#ifdef max
#  undef max
#  undef min
#endif  // max

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_ShowMapCacheInfo)
#define LogWarn LOG(WARNING)

namespace TBB {
class parallel_create_lane_cache {
 public:
  /**
   * @brief operator() 重载函数
   * 该重载函数用于并行处理指定范围的任务。
   *
   * @param r 包含子任务范围的参数，该参数类型为 tbb::blocked_range<size_t>
   */
  void operator()(const tbb::blocked_range<size_t>& r) const {
    LogInfo << "parallel_create_lane_cache " << TX_VARS_NAME(task_size, r.size())
#if TBB_NEW_API
            << TX_VARS_NAME(thread_id, tbb::this_task_arena::current_thread_index());
#else
            << TX_VARS_NAME(thread_id, tbb::task_arena::current_thread_index());
#endif
    for (size_t i = r.begin(); i != r.end(); ++i) {
      HdMap::HadmapCacheConCurrent::RegisterLaneSyncInit(_lanePtrVec[i]);
      Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().RegisterLane(_lanePtrVec[i]);
    }
  }

  explicit parallel_create_lane_cache(const hadmap::txLanes& lanePtrVec) : _lanePtrVec(lanePtrVec) {}

 protected:
  const hadmap::txLanes& _lanePtrVec;
};

class parallel_create_lanelink_cache {
 public:
  /**
   * @brief operator() 并行处理指定范围的任务
   * 这个操作符重载函数用于并行处理指定范围的任务。
   *
   * @param r 包含子任务范围的参数
   */
  void operator()(const tbb::blocked_range<size_t>& r) const {
    LogInfo << "parallel_create_lanelink_cache " << TX_VARS_NAME(task_size, r.size())
#if TBB_NEW_API
            << TX_VARS_NAME(thread_id, tbb::this_task_arena::current_thread_index());
#else
            << TX_VARS_NAME(thread_id, tbb::task_arena::current_thread_index());
#endif
    for (size_t i = r.begin(); i != r.end(); ++i) {
      HdMap::HadmapCacheConCurrent::RegisterLaneLinkSyncInit(_lanelinkPtrVec[i]);
      HdMap::HadmapCacheConCurrent::RegisterLaneLinkPrePostTopoInfoInit(_lanelinkPtrVec[i]);
      Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().RegisterLaneLink(_lanelinkPtrVec[i]);
      HdMap::HadmapCacheConCurrent::RegisterRoadInfoSyncRunTime(_lanelinkPtrVec[i]);
    }
  }

  explicit parallel_create_lanelink_cache(const hadmap::txLaneLinks& lanelinkPtrVec)
      : _lanelinkPtrVec(lanelinkPtrVec) {}

 protected:
  const hadmap::txLaneLinks& _lanelinkPtrVec;
};

class parallel_create_road_cache {
 public:
  /**
   * @brief operator() 重载函数
   * 该重载函数用于并行处理指定范围的任务。
   *
   * @param r 包含子任务范围的参数，该参数类型为 tbb::blocked_range<size_t>
   */
  void operator()(const tbb::blocked_range<size_t>& r) const {
    LogInfo << "parallel_create_road_cache " << TX_VARS_NAME(task_size, r.size())
#if TBB_NEW_API
            << TX_VARS_NAME(thread_id, tbb::this_task_arena::current_thread_index());
#else
            << TX_VARS_NAME(thread_id, tbb::task_arena::current_thread_index());
#endif
    for (size_t i = r.begin(); i != r.end(); ++i) {
      LogInfo << TX_VARS_NAME(road_index, i) << TX_PTR_FUNC(_roadPtrVec[i], getId(), 999);
      HdMap::HadmapCacheConCurrent::RegisterRoadSectionSyncInit(_roadPtrVec[i]);
      HdMap::HadmapCacheConCurrent::RegisterRoadInfoSyncRunTime(_roadPtrVec[i]);
    }
  }

  explicit parallel_create_road_cache(const hadmap::txRoads& roadPtrVec) : _roadPtrVec(roadPtrVec) {}

 protected:
  const hadmap::txRoads& _roadPtrVec;
};

class parallel_Make_Lane_Pre_Next_Laneset {
 public:
  /**
   * @brief operator() 重载函数
   * 该重载函数用于并行处理指定范围的任务。
   *
   * @param r 包含子任务范围的参数，该参数类型为 tbb::blocked_range<size_t>
   */
  void operator()(const tbb::blocked_range<size_t>& r) const {
    for (size_t i = r.begin(); i != r.end(); ++i) {
      HdMap::HadmapCacheConCurrent::RegisterLanePrePostTopoInfoInit(_lanePtrVec[i]);
    }
  }

  explicit parallel_Make_Lane_Pre_Next_Laneset(const hadmap::txLanes& lanePtrVec) : _lanePtrVec(lanePtrVec) {}

 protected:
  const hadmap::txLanes& _lanePtrVec;
};

class parallel_Make_LaneLink_Pre_Next_Info {
 public:
  /**
   * @brief operator() 重载函数
   * 该重载函数用于并行处理指定范围的任务。
   *
   * @param r 包含子任务范围的参数，该参数类型为 tbb::blocked_range<size_t>
   */
  void operator()(const tbb::blocked_range<size_t>& r) const {
    for (size_t i = r.begin(); i != r.end(); ++i) {
      HdMap::HadmapCacheConCurrent::RegisterLaneLinkPrePostTopoInfoInit(_lanelinkPtrVec[i]);
    }
  }

  explicit parallel_Make_LaneLink_Pre_Next_Info(const hadmap::txLaneLinks& lanelinkPtrVec)
      : _lanelinkPtrVec(lanelinkPtrVec) {}

 protected:
  const hadmap::txLaneLinks& _lanelinkPtrVec;
};

class parallel_make_dead_end_laneset {
 public:
  /**
   * @brief operator() 重载函数
   * 该重载函数用于并行处理指定范围的任务。
   *
   * @param r 包含子任务范围的参数，该参数类型为 tbb::blocked_range<size_t>
   */
  void operator()(const tbb::blocked_range<size_t>& r) const {
    for (size_t i = r.begin(); i != r.end(); ++i) {
      HdMap::HadmapCacheConCurrent::Make_dead_end_laneSet_by_laneUid(_lanePtrVec[i]);
    }
  }

  explicit parallel_make_dead_end_laneset(const hadmap::txLanes& lanePtrVec) : _lanePtrVec(lanePtrVec) {}

 protected:
  const hadmap::txLanes& _lanePtrVec;
};
} /*namespace TBB*/

#undef LogInfo
#undef LogWarn

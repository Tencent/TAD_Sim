// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/hashed_lane_info_orthogonal_list.h"
#include "tad_component.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_header.h"
#include "tx_serialization.h"
#include "vehicle_interaction.pb.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_PlanningModule {
 public:
  using Planning_Activate = Base::Enums::Planning_Activate;
  virtual ~TAD_PlanningModule() TX_DEFAULT;

 protected:
  /**
   * @brief CheckReachable 检查车辆是否能到达给定的道路信息
   * @param _laneInfo 给定的道路信息
   * @return 返回真值表示车辆是否能够到达给定的道路
   */
  virtual Base::txBool CheckReachable(const Base::Info_Lane_t& _laneInfo) const TX_NOEXCEPT = 0;

  /**
   * @brief SearchOrthogonalListFrontByLevel 搜索具有给定级别的车辆列表的最前方坐标
   * @param check_reachable 指定检查车辆是否可达
   * @param center_node_ptr 指向用于搜索的中心节点信息
   * @param front_search_distance 指定搜索车辆的前方距离
   * @param level_idx 指定要搜索的车辆级别索引
   * @param results_by_level 按级别存储搜索到的车辆结果
   * @return 返回真值表示成功搜索到车辆
   */
  Base::txBool SearchOrthogonalListFrontByLevel(
      const Base::txBool check_reachable,
      const Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr& center_node_ptr,
      const Base::txFloat front_search_distance, const Base::txInt level_idx,
      std::list<Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr>& results_by_level) TX_NOEXCEPT;

  /**
   * @brief 判断当前是否需要激活纵向控制模块
   * @return 返回真值，表示需要激活纵向控制模块
   */
  virtual Base::txBool Need_Longitudinal_Activate() const TX_NOEXCEPT {
    return (_plus_(Planning_Activate::eAutopilot) == mAutoMode) ||
           (_plus_(Planning_Activate::eLongitudinal) == mAutoMode);
  }

  /**
   * @brief 判断当前是否需要激活横向控制模块
   * @return 返回真值，表示需要激活横向控制模块
   */
  virtual Base::txBool Need_Lateral_Activate() const TX_NOEXCEPT {
    return (_plus_(Planning_Activate::eAutopilot) == mAutoMode) || (_plus_(Planning_Activate::eLateral) == mAutoMode);
  }

  /**
   * @brief 判断是否需要激活自动驾驶功能
   * @return 返回真值，表示需要激活自动驾驶功能
   */
  virtual Base::txBool Need_Autopilot_Activate() const TX_NOEXCEPT {
    return _plus_(Planning_Activate::eOff) != (mAutoMode);
  }

  /**
   * @brief 更新自动驾驶模式
   * @param _new 新的自动驾驶模式
   * @return 返回当前自动驾驶模式
   */
  virtual Planning_Activate UpdateActivateMode(const Planning_Activate _new) TX_NOEXCEPT {
    mAutoMode = _new;
    return mAutoMode;
  }

  /**
   * @brief 获取自动驾驶模式
   * @return 返回当前自动驾驶模式
   */
  virtual Planning_Activate AutoMode() const TX_NOEXCEPT { return mAutoMode; }

  /**
   * @brief pb转枚举
   *
   * @param pb sim_msg::Activate pb对象
   * @return Planning_Activate
   */
  virtual Planning_Activate pb2enum(const sim_msg::Activate& pb) const TX_NOEXCEPT;

 protected:
  TrafficFlow::Component::RouteAI mRouteAI;
  Planning_Activate mAutoMode = _plus_(Planning_Activate::eAutopilot);
  TrafficFlow::CentripetalCatMullPtr mPlanningTrajectoryPtr = nullptr;
  TrafficFlow::CentripetalCatMullPtr mPlanningCenterLineTrajectoryPtr = nullptr;

 public:
  /**
   * @brief 对象序列化模板
   *
   * @tparam Archive
   * @param archive
   */
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("TAD_PlanningModule");
    archive(_MAKE_NVP_("RouteAI", mRouteAI));
    archive(_MAKE_NVP_("AutoMode", mAutoMode));
  }
};
TX_NAMESPACE_CLOSE(TrafficFlow)

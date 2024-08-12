// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_obb.h"
#include "tx_serialization.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(TrafficFlow)

#if USE_VehicleKinectInfo

class IKineticsHandler {
 public:
  struct KineticsUtilInfo_t {
    Base::txInt _ElemId = -1;
    Base::txSysId _SysId = -1;
    Coord::txENU _ElementGeomCenter;
    Coord::txENU _LastElementGeomCenter;
    Base::txVec3 vElementDir;
    Base::txFloat _Velocity = 0.0;
    Base::txFloat _Acc = 0.0;
    Geometry::OBB2D::PolygonArray _vecPolygon;
    Base::txBool m_isValid = false;
    std::array<Base::txVec3, 2> _LocalCoords;
    Base::txMat2 _TransMat;
    Base::txMat2 _TransMatInv;
  };

  enum class RelationShip : Base::txInt {
    syntropy_egohead_front_of_selfhead_base_on_self,
    syntropy_egohead_behind_of_selfhead_base_on_self,
    reverse_egohead_front_of_selfhead_base_on_self, /*not collision*/
    reverse_egohead_behind_of_selfhead_base_on_self /*has collision*/
  };
  friend std::ostream& operator<<(std::ostream& os, const RelationShip v) TX_NOEXCEPT;

 public:
  virtual ~IKineticsHandler() TX_DEFAULT;

  /**
   * @brief 获取车辆动力学描述
   *
   * 本函数用于获取车辆的动力学描述。
   *
   * @return 返回车辆动力学描述的字符串
   */
  virtual Base::txString KineticsDesc() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取车辆动力学信息
   *
   * 本函数用于获取车辆的动力学信息。
   *
   * @return KineticsUtilInfo_t 车辆动力学信息，包含对应的动力学参数
   */
  virtual KineticsUtilInfo_t GetKineticsUtilInfo() const TX_NOEXCEPT = 0;

  /**
   * @brief 计算车辆动力学信息
   *
   * 本函数用于计算车辆的动力学信息，包括对应的速度、转向等。
   *
   * @param timeMgr  时间管理器，用于获取当前时间和时间步长
   * @param _objInfo 车辆动力学信息，包括车辆的各个部分的动力学参数
   * @return true    成功计算动力学信息
   * @return false   失败计算动力学信息
   */
  virtual Base::txBool ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                       const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT = 0;

 public:
  /**
   * @brief 获取车辆的绝对速度
   *
   * 此函数用于获取车辆的绝对速度，以千米为单位。绝对速度是指车辆在仅由其可控制的力量作用下的实际速度。
   *
   * @return 返回车辆的绝对速度，单位为千米/小时
   */
  virtual Base::txFloat show_abs_velocity() const TX_NOEXCEPT { return m_show_abs_velocity; }

  /**
   * @brief 获取绝对加速度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_abs_acc() const TX_NOEXCEPT { return m_show_abs_acc; }

  /**
   * @brief 获取相对加速度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_relative_acc() const TX_NOEXCEPT { return m_show_relative_acc; }

  /**
   * @brief 获取相对横向加速度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_relative_acc_horizontal() const TX_NOEXCEPT { return m_show_relative_acc_horizontal; }

  /**
   * @brief 获取相对纵向速度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_relative_velocity_vertical() const TX_NOEXCEPT {
    return m_show_relative_velocity_vertical;
  }

  /**
   * @brief 获取相对横向速度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_relative_velocity_horizontal() const TX_NOEXCEPT {
    return m_show_relative_velocity_horizontal;
  }

  /**
   * @brief 获取纵向相对距离
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_relative_dist_vertical() const TX_NOEXCEPT { return m_show_relative_dist_vertical; }

  /**
   * @brief 获取横向相对距离
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_relative_dist_horizontal() const TX_NOEXCEPT { return m_show_relative_dist_horizontal; }

  /**
   * @brief 获取车辆相对高度
   *
   * 此函数用于获取车辆的相对高度，即车辆离路面高度与路面平均高度之差，以米为单位。
   *
   * @return 返回车辆相对高度，单位为米
   */
  virtual Base::txFloat show_relative_dist_elevation() const TX_NOEXCEPT { return m_show_relative_dist_elevation; }

  /**
   * @brief 获取车辆绝对高度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat show_abs_dist_elevation() const TX_NOEXCEPT { return m_show_abs_dist_elevation; }

 protected:
  Base::txFloat m_show_abs_velocity = 0.0;
  Base::txFloat m_show_abs_acc = 0.0;
  Base::txFloat m_show_relative_acc = 0.0;
  Base::txFloat m_show_relative_acc_horizontal = 0.0;
  Base::txFloat m_show_relative_velocity_vertical = 0.0;
  Base::txFloat m_show_relative_velocity_horizontal = 0.0;
  Base::txFloat m_show_relative_dist_vertical = 0.0;
  Base::txFloat m_show_relative_dist_horizontal = 0.0;
  Base::txFloat m_show_relative_dist_elevation = 0.0;
  Base::txFloat m_show_abs_dist_elevation = 0.0;

  Base::txVec2 m_local_last_obs_center = Base::txVec2(0.0, 0.0);
  Base::txVec2 m_local_last_obs_velocity = Base::txVec2(0.0, 0.0);
  KineticsUtilInfo_t m_self_KineticsUtilInfo;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("IKineticsHandler");
  }
};

using IKineticsHandlerPtr = std::shared_ptr<IKineticsHandler>;

#endif /*USE_VehicleKinectInfo*/

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef SerializeInfo

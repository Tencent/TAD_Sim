// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <vector>
#include "traffic.pb.h"
#include "tx_header.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
#include "tx_tadsim_flags.h"

TX_NAMESPACE_OPEN(TrafficFlow)

struct txTrajManagerBase;
using txTrajManagerBasePtr = std::shared_ptr<txTrajManagerBase>;

struct txTrajManagerBase {
 public:
  struct history_pos_info {
    Coord::txWGS84 bus_pos;
    Base::txFloat rsv_abs_time = 0.0;
    Base::txVec3 bus_dir = Base::txVec3(0.0, 0.0, 0.0);
    sim_msg::Car raw_pb_data;

    history_pos_info() {}
    /**
     * @brief Construct a new history_pos_info object with a reference to another one.
     *
     * @param ref The reference object to copy the information from.
     */
    history_pos_info(const history_pos_info& ref) {
      bus_pos.FromWGS84(ref.bus_pos.WGS84());
      rsv_abs_time = ref.rsv_abs_time;
      bus_dir = ref.bus_dir;
      raw_pb_data.CopyFrom(ref.raw_pb_data);
    }

    history_pos_info& operator=(const history_pos_info& ref) noexcept {
      bus_pos.FromWGS84(ref.bus_pos.WGS84());
      rsv_abs_time = ref.rsv_abs_time;
      bus_dir = ref.bus_dir;
      raw_pb_data.CopyFrom(ref.raw_pb_data);
      return *this;
    }
  };
  using history_pos_info_array = std::vector<history_pos_info>;

  /* space-time type */
  struct txSpaceTimeCar {
    txSpaceTimeCar() TX_DEFAULT;

    /**
     * @brief txSpaceTimeCar 构造函数
     *
     * 根据给定的时间和车辆信息，初始化一个 txSpaceTimeCar 对象
     * @param _absTime 时间戳
     * @param refCar 车辆信息
     */
    txSpaceTimeCar(const Base::txFloat _absTime, const sim_msg::Car& refCar) : rsv_abs_time(_absTime) {
      raw_car_pb.CopyFrom(refCar);
      // m_car_loc.FromWGS84(raw_car_pb.x(), raw_car_pb.y());
    }

    txSpaceTimeCar(const txSpaceTimeCar& refObj) : rsv_abs_time(refObj.rsv_abs_time) {
      raw_car_pb.CopyFrom(refObj.raw_car_pb);
      // m_car_loc.FromWGS84(refObj.raw_car_pb.x(), refObj.raw_car_pb.y() /*refObj.m_car_loc.WGS84()*/);
    }

    Base::txFloat rsv_abs_time;
    sim_msg::Car raw_car_pb;
    // Coord::txWGS84 m_car_loc;
  };
  using txSpaceTimeCarList = std::vector<txSpaceTimeCar>;

  struct txSpaceTimeLocation {
    txSpaceTimeLocation() TX_DEFAULT;
    txSpaceTimeLocation(const Base::txFloat _absTime, const sim_msg::Location& refEgo) : rsv_abs_time(_absTime) {
      raw_location_pb.CopyFrom(refEgo);
      m_car_loc.FromWGS84(raw_location_pb.position().x(), raw_location_pb.position().y());
    }
    txSpaceTimeLocation(const txSpaceTimeLocation& refObj)
        : rsv_abs_time(refObj.rsv_abs_time), m_car_loc(refObj.m_car_loc) {
      raw_location_pb.CopyFrom(refObj.raw_location_pb);
    }
    Base::txFloat rsv_abs_time;
    sim_msg::Location raw_location_pb;
    Coord::txWGS84 m_car_loc;
  };
  using txSpaceTimeLocationList = std::vector<txSpaceTimeLocation>;

  struct txKinectInfo {
    Base::txFloat mAverSpeed = 0.0;
    Base::txFloat mAverAcc = 0.0;
    Base::txFloat mAverTimeStamp = 0.0;
    void Clear() TX_NOEXCEPT {
      mAverSpeed = 0.0;
      mAverAcc = 0.0;
      mAverTimeStamp = 0.0;
    }
  };

 public:
  history_pos_info_array mVecPosInfo;
  Base::txSysId mId = 0;
  Base::txBool mValid = true;
  txKinectInfo mKinectInfo;

 public:
  virtual Base::txLpsz className() const TX_NOEXCEPT = 0;

  /**
   * @brief 是否启用填充功能
   * @param timeMgr 当前时间参数管理器
   * @return Base::txBool 是否启用填充功能
   */
  virtual Base::txBool EnableFilling(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT = 0;

  /**
   * @brief 使用给定的时间参数计算运动轨迹
   *
   * 使用给定的时间参数计算运动轨迹，并在返回期间修改bus_interpolation_pos。
   *
   * @param timeMgr 当前时间参数管理器
   * @param bus_interpolation_pos 当前车辆的轨迹点
   * @param refCarInfo 当前车辆信息
   * @return Base::txBool 若计算成功则返回 true，否则返回 false
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr, Coord::txWGS84& bus_interpolation_pos,
                                   sim_msg::Car& refCarInfo) TX_NOEXCEPT = 0;

  /**
   * @brief 向当前对象中添加新的位置信息节点.
   *
   * 在添加新的位置信息节点时，需要提供以下信息：
   * @param infoNode 要添加的位置信息节点信息
   * @param last_vLaneDir 上一个车辆方向
   * @return Base::txBool 添加成功返回 true，否则返回 false
   */
  virtual Base::txBool add_new_pos(history_pos_info infoNode, const Base::txVec3 last_vLaneDir) TX_NOEXCEPT = 0;

  /**
   * @brief 添加新的车辆位置信息列表到当前对象中
   *
   * 在添加新的位置信息列表时，需要提供以下信息：
   * @param l2w_car_list 待添加的车辆位置信息列表
   * @return Base::txBool 添加成功返回 true，否则返回 false
   */
  virtual Base::txBool add_new_pos(const txSpaceTimeCarList& l2w_car_list) TX_NOEXCEPT { return false; }

  /**
   * @brief 使用给定的时间参数计算车辆的位置、速度和加速度
   *
   * 当使用给定的时间参数计算车辆的位置、速度和加速度时，需要提供以下信息：
   * @param timeMgr 用于获取时间参数的管理器
   * @param refCarInfo 待计算的车辆信息
   * @param refVelocity 待计算的车辆速度
   * @param refAcc 待计算的车辆加速度
   * @return Base::txBool 计算成功返回 true，否则返回 false
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr, sim_msg::Car& refCarInfo,
                                   Base::txFloat& refVelocity, Base::txFloat& refAcc) TX_NOEXCEPT {
    return false;
  }

  /**
   * @brief 添加新的位置信息到车辆路径列表
   *
   * 如果成功添加，则返回 true，否则返回 false。使用此功能，必须确保车辆之间的坐标系是一致的。
   *
   * @param l2w_ego_list 包含新车辆位置信息的列表
   * @return true 成功添加; false 添加失败
   */
  virtual Base::txBool add_new_pos(const txSpaceTimeLocationList& l2w_ego_list) TX_NOEXCEPT { return false; };

  /**
   * @brief 计算车辆的位置信息
   *
   * 根据指定的时间参数，计算车辆的位置信息，并将结果存储在参数'refEgoInfo'中。
   *
   * @param timeMgr 时间参数管理器，包含用于计算的时间参数
   * @param refEgoInfo 用于存储计算结果的车辆位置信息
   * @return true 计算成功; false 计算失败
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr, sim_msg::Location& refEgoInfo) TX_NOEXCEPT {
    return false;
  }

  /**
   * @brief 复制当前的交通状态和行驶方式（包括颜色、速度等）
   *
   * 该函数将创建一个副本对象，用于复制当前的交通状态和行驶方式。
   *
   * @param[in] forShadowElement 指示是否为副本对象（默认为 false）
   * @return 返回一个新的 txTrajManager 对象，包含当前状态和行驶方式的副本
   */
  virtual txTrajManagerBasePtr clone_traj(/*for shadow element*/) TX_NOEXCEPT = 0;

  virtual void Clear() TX_NOEXCEPT {
    mVecPosInfo.clear();
    mValid = true;
    mId = 0;
    mKinectInfo.Clear();
  }

  /**
   * @brief 判断当前对象是否有效
   *
   * @return Base::txBool 如果当前对象有效，则返回 true；否则返回 false
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief 获取当前对象中的元素数量
   * @return Size 当前对象中的元素数量
   */
  Base::txSize Size() const TX_NOEXCEPT { return mVecPosInfo.size(); }

  /**
   * @brief 判断当前对象是否为空
   * @return txBool 如果当前对象为空，则返回 true，否则返回 false
   */
  Base::txBool Empty() const TX_NOEXCEPT { return mVecPosInfo.empty(); }

  /**
   * @brief 返回最后一个已经记录的位置信息
   * @return history_pos_info 最后一个已经记录的位置信息
   */
  history_pos_info back_pos() const TX_NOEXCEPT { return mVecPosInfo.back(); }

  /**
   * @brief 返回L2W驱动距离
   * @return Base::txFloat 返回L2W驱动距离
   */
  Base::txFloat L2W_Motion_Dist() const TX_NOEXCEPT { return FLAGS_L2W_Motion_Dist; }

  /**
   * @brief 检查是否需要结束当前任务
   *
   * 检查当前任务是否需要结束，返回 true 表示需要完成，false 表示不需要结束。
   *
   * @param timeMgr 当前时间参数管理器
   * @return Base::txBool 是否需要结束当前任务
   */
  virtual Base::txBool NeedFinish(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT { return false; }
};

TX_NAMESPACE_CLOSE(TrafficFlow)

// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "road_realtimeinfo.pb.h"
#include "tx_header.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
#include "tx_tc_gflags.h"
TX_NAMESPACE_OPEN(TrafficFlow)
TX_NAMESPACE_OPEN(Cloud)

struct txCloudTrajManager {
 public:
  struct history_pos_info {
    Coord::txWGS84 bus_pos;
    Base::txFloat rsv_abs_time = 0.0;
    Base::txVec3 bus_dir = Base::txVec3(0.0, 0.0, 0.0);
    ObjectInfo raw_pb_data;

    history_pos_info() {}

    /**
     * @brief Copy constructor
     * @param ref 引用的历史轨迹信息对象
     */
    history_pos_info(const history_pos_info& ref) {
      bus_pos.FromWGS84(ref.bus_pos.WGS84());
      rsv_abs_time = ref.rsv_abs_time;
      bus_dir = ref.bus_dir;
      raw_pb_data.CopyFrom(ref.raw_pb_data);
    }

    /**
     * @brief Copy operator for history_pos_info class
     * @param ref The reference history_pos_info object
     * @return A reference to the updated history_pos_info object
     */
    history_pos_info& operator=(const history_pos_info& ref) noexcept {
      bus_pos.FromWGS84(ref.bus_pos.WGS84());
      rsv_abs_time = ref.rsv_abs_time;
      bus_dir = ref.bus_dir;
      raw_pb_data.CopyFrom(ref.raw_pb_data);
      return *this;
    }
  };
  using history_pos_info_array = std::vector<history_pos_info>;

 public:
  history_pos_info_array mVecPosInfo;
  Base::txSysId mId = 0;
  Base::txBool mValid = true;

 public:
  /**
   * @brief 判断是否使用线性插值进行DITW路径计算
   * @return true 表示使用线性插值，false 表示不使用线性插值
   */
  Base::txBool UseLinearInterpolation() const TX_NOEXCEPT { return false && FLAGS_cloud_ditw_use_linear_interpolation; }

  /**
   * @brief 返回当前对象的类名
   *
   * @return Base::txLpsz
   */
  virtual Base::txLpsz className() const TX_NOEXCEPT { return "txCloudTrajManager"; }

  /**
   * @brief 清除DITW路径管理器中的数据
   *
   * 清除DITW路径管理器中的所有数据，并将mValid设置为true。
   */
  void Clear() TX_NOEXCEPT {
    mVecPosInfo.clear();
    mValid = true;
    mId = 0;
  }

  /**
   * @brief 清除头部数据
   *
   * 清除DITW管理器中的头部数据，并将mValid设置为true。
   */
  virtual void ClearHead() TX_NOEXCEPT {
    if (mVecPosInfo.size() > 1) {
      mVecPosInfo.erase(mVecPosInfo.begin(), mVecPosInfo.end() - 1);
      if (1 != mVecPosInfo.size()) {
        LOG(FATAL) << "1 != mVecPosInfo.size()";
      }
    }
  }

  /**
   * @brief 获取是否有效的状态
   *
   * 当该接口被调用时，返回一个布尔值表示 DITW 管理器中是否存在有效的数据。
   * @return true 存在有效的数据, false 没有有效的数据
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief 获取数据大小
   *
   * 返回DITW数据管理器中数据的大小。
   *
   * @return size_t 数据大小
   */
  Base::txSize Size() const TX_NOEXCEPT { return mVecPosInfo.size(); }

  /**
   * @brief 判断是否为空
   *
   * 检查 DITW 数据管理器中是否存在有效的数据。
   *
   * @return true 存在有效数据, false 没有有效数据
   */
  Base::txBool Empty() const TX_NOEXCEPT { return mVecPosInfo.empty(); }

  /**
   * @brief 判断是否启用填充模式
   *
   * 检查 DITW 数据管理器是否启用了填充模式。
   *
   * @return true 启用了填充模式, false 未启用填充模式
   */
  Base::txBool EnableFilling() TX_NOEXCEPT {
    if (CallSucc(UseLinearInterpolation())) {
      return (mVecPosInfo.size() > 1);
    } else {
      return (mVecPosInfo.size() > 0);
    }
  }

  /**
   * @brief 根据给定的参数计算沿车辆的当前位置
   *
   * 使用沿车辆的插值轨迹和当前时间计算沿车辆的当前位置。
   *
   * @param timeMgr 时间参数管理器
   * @param bus_interpolation_pos 结果参数，沿车辆的轨迹插值
   * @param refCarInfo 结果参数，存储关于车辆信息
   * @return Base::txBool 如果启用填充状态，则返回 true，否则返回 false
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr, Coord::txWGS84& bus_interpolation_pos,
                                   ObjectInfo& refCarInfo) TX_NOEXCEPT {
    {
      if (IsValid() && EnableFilling()) {
        bus_interpolation_pos = mVecPosInfo.back().bus_pos;
        refCarInfo.CopyFrom(mVecPosInfo.back().raw_pb_data);
        return true;
      } else {
        return false;
      }
    }

    return IsValid();
  }

  /**
   * @brief 添加新位置
   *
   * 向当前历史位置列表中添加一个新的位置。
   *
   * @param infoNode 新位置的信息
   * @param last_vLaneDir 上一个车辆车道的方向
   * @return Base::txBool 如果添加成功则返回 true，否则返回 false
   */
  virtual Base::txBool add_new_pos(history_pos_info infoNode, const Base::txVec3 last_vLaneDir) TX_NOEXCEPT {
    if (CallFail(mVecPosInfo.empty())) {
      Base::txVec2 bus_dir = (infoNode.bus_pos.ToENU().ENU2D() - mVecPosInfo.back().bus_pos.ToENU().ENU2D());
      bus_dir.normalize();
      infoNode.bus_dir = Base::txVec3(bus_dir.x(), bus_dir.y(), 0.0);
    }
    // LOG(WARNING) << TXST_TRACE_VARIABLES(mId) << TXST_TRACE_VARIABLES(rsv_abs_time) <<
    // TXST_TRACE_VARIABLES(infoNode.bus_pos);
    mVecPosInfo.push_back(infoNode);
    return true;
  }

  /**
   * @brief 获取回退位置信息
   *
   * 获取当前历史位置列表中的回退位置信息。
   *
   * @return history_pos_info 返回回退位置信息
   */
  history_pos_info back_pos() const TX_NOEXCEPT { return mVecPosInfo.back(); }
};

using txCloudTrajManagerPtr = std::shared_ptr<txCloudTrajManager>;

TX_NAMESPACE_CLOSE(Cloud)
TX_NAMESPACE_CLOSE(TrafficFlow)

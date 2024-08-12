// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_sim_point.h"
TX_NAMESPACE_OPEN(Base)
#if 1
class DeadLineAreaManager {
 public:
  virtual Base::txFloat DeadLineLength() const TX_NOEXCEPT { return FLAGS_exit_region_length; }

  /**
   * @brief deadline manager的初始化函数
   *
   * @param endPt 终点坐标
   * @return Base::txBool 初始化成功返回true
   */
  virtual Base::txBool Initialize(Coord::txWGS84 endPt) TX_NOEXCEPT;

  /**
   * @brief 当前元素是否在dead line内
   *
   * 当当前元素（由其 lane 和坐标点）到达dead line内时，返回 true；否则返回 false。
   *
   * @param elemLaneUid 当前元素的 lane 的 UID
   * @param elementPt 当前元素的坐标点
   * @return true 当前元素在dead line内
   * @return false 当前元素不在dead line内
   */
  virtual Base::txBool ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid,
                                            const Coord::txENU& elementPt) const TX_NOEXCEPT;

  /**
   * @brief 检测当前元素是否到达dead line内
   *
   * 如果当前元素到达de a d内，返回 true；否则返回 false。
   *
   * @param elemLaneUid 当前元素所在的车道 UID
   * @param elemDir 当前元素的方向（以当前车道为参考方向）
   * @param elementPt 当前元素的坐标点
   * @return 当前元素是否到达dead line内
   */
  virtual Base::txBool ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid, const Base::txVec3& elemDir,
                                            const Coord::txENU& elementPt) const TX_NOEXCEPT;

  /**
   * @brief 获取当前车道的闭合时车道线的顺序点
   *
   * 此函数返回一个表示闭合时车道线的顺序点的 vector。每个点代表着当前元素所在的车道的一个点的坐标。
   *
   * @return 一个表示闭合时车道线的顺序点的 vector
   */
  virtual std::vector<Base::txVec2> DeadlineAreaClockWiseClose() const TX_NOEXCEPT {
    return m_vecPolygon_clockwise_close;
  }

  /**
   * @brief 返回当前对象的格式化输出
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT;

  /**
   * @brief Set the In Valid object
   *
   */
  void SetInValid() TX_NOEXCEPT { _valid = false; }

  /**
   * @brief 获取当前是否为有效对象
   *
   * 对象是否被初始化并正确创建。无效的对象可能无法正常工作。
   *
   * @return 是否有效
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return _valid; }

  /**
   * @brief 获取当前对象所在的LaneUId
   *
   * 获取当前对象所在的LaneUId，并返回其引用。
   *
   * @return const Base::txLaneUId& 当前对象所在的LaneUId
   */
  const Base::txLaneUId& GetLocLaneUid() const TX_NOEXCEPT { return _deadline_LaneUid; }

  /**
   * @brief 获取当前对象所在的区域的结束位置的ENU坐标
   *
   * 获取当前对象所在的区域的结束位置的ENU坐标，并返回其引用。
   *
   * @return const Coord::txENU& 当前对象所在的区域的结束位置的ENU坐标
   */
  const Coord::txENU& GetEndPos() const TX_NOEXCEPT { return _EndPos; }

  /**
   * @brief 获取当前对象所在的区域的DEAD线方向
   *
   * 获取当前对象所在的区域的DEAD线方向，即DEAD线在道路上的方向。
   *
   * @return const Base::txVec3& 当前对象所在的区域的DEAD线方向
   */
  const Base::txVec3& GetDeadLineDir() const TX_NOEXCEPT { return m_vLaneDir_Deadline; }
  friend std::ostream& operator<<(std::ostream& os, const DeadLineAreaManager& v) TX_NOEXCEPT {
    os << "{" << v.Str() << "}";
    return os;
  }

 protected:
  Base::txBool _valid = false;
  Base::txLaneUId _deadline_LaneUid;
  Coord::txENU _EndPos;
  std::vector<Base::txVec2> m_vecPolygon_clockwise_close;
  Base::txVec3 m_vLaneDir_Deadline;
  txString _str_polygon;

 public:
  /**
   * @brief 对象的序列化模板
   *
   * @tparam Archive 类型
   * @param archive 实例对象
   */
  template <class Archive>
  void serialize(Archive& archive) {
    archive(_MAKE_NVP_("valid", _valid));
    archive(_MAKE_NVP_("deadline_LaneUid", _deadline_LaneUid));
    archive(_MAKE_NVP_("EndPos", _EndPos));
    archive(_MAKE_NVP_("vecPolygon_clockwise_close", m_vecPolygon_clockwise_close));
    archive(_MAKE_NVP_("vLaneDir_Deadline", m_vLaneDir_Deadline));
    archive(_MAKE_NVP_("str_polygon", _str_polygon));
  }
};
#endif
TX_NAMESPACE_CLOSE(Base)

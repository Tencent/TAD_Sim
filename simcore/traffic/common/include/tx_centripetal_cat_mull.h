// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <array>
#include "HdMap/tx_lane_geom_info.h"
#include "tx_class_counter.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class CentripetalCatMull : public HdMap::txLaneInfoInterface, public HdMap::txLaneProjectionUtils {
 public:
  using controlPointVec = HdMap::txLaneInfoInterface::controlPointVec;
  using control_path_node_vec = Base::ISceneLoader::IRouteViewer::control_path_node_vec;

 public:
  explicit CentripetalCatMull(const control_path_node_vec& samplepoints);
  virtual ~CentripetalCatMull() TX_DEFAULT;

  /**
   * @brief 获取指定位置的车道方向
   *
   * 该函数用于获取指定位置的车道方向。当车道方向已经确定且未改变时，函数将返回一个确定的方向，否则将返回一个偏移量与该位置之间的方向差异最小的方向。
   *
   * @param[in] pos 指定位置，范围在[0, 1]之间
   * @return 返回指定位置的车道方向
   */
  virtual Base::txVec3 GetLaneDir(const Base::txFloat) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定位置的局部坐标
   *
   * 该函数用于获取指定位置的局部坐标。输入参数为相对于三维空间中贝塞尔曲线起点的距离，范围在[0, 1]之间。
   * 输出为三维空间中的局部坐标，即曲线上该点的空间投影。
   *
   * @param[in] pos 相对于曲线起点的距离，范围在[0, 1]之间
   * @return 返回曲线上该点的局部坐标
   */
  virtual Base::txVec3 GetLocalPos(const Base::txFloat) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车道的形状
   *
   * 这个函数返回当前车道的形状，Line表示直线，Spline表示贝塞尔曲线，其他表示未知形状。
   *
   * @return LaneShape 当前车道的形状
   */
  virtual LaneShape GetShape() const TX_NOEXCEPT TX_OVERRIDE { return LaneShape::sLine; }

  /**
   * @brief 获取当前车道的起始点
   *
   * 这个函数返回当前车道的起始点的3D坐标。
   *
   * @return Base::txVec3 当前车道的起始点3D坐标
   */
  virtual Base::txVec3 GetStartPt() const TX_NOEXCEPT TX_OVERRIDE { return Utils::Vec2_Vec3(m_sample_points.front()); }

  /**
   * @brief 获取当前车道的终点点坐标
   *
   * 这个函数返回当前车道的终点点的3D坐标，即路径描述的结尾。
   *
   * @return Base::txVec3 当前车道的终点点3D坐标
   */
  virtual Base::txVec3 GetEndPt() const TX_NOEXCEPT TX_OVERRIDE { return Utils::Vec2_Vec3(m_sample_points.back()); }

  /**
   * @brief 获取当前折线的实际长度
   *
   * 此函数返回当前折线的实际长度，单位为实际单位
   *
   * @return Base::txFloat 当前折线的实际长度
   */
  virtual Base::txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE { return m_real_lenth; }

  /**
   * @brief 获取指定点处的参数值
   *
   * 该函数返回指定索引处的参数值，可以视为曲线的长度参数。
   * 如果需要使用这个参数值进行插值计算，需要先获取到对应的控制点坐标，
   * 使用这个参数值作为 t 的值进行计算。
   *
   * @param idx 指定点的索引值
   * @return Base::txFloat 指定点处的参数值
   */
  virtual Base::txFloat ParameterAtPoint(const size_t idx) const TX_NOEXCEPT;

  /**
   * @brief 根据给定的距离值，定位对应的索引值
   *
   * 该函数接受一个浮点数距离值，该值代表当前曲线中某个点与控制点集中点的距离。
   * 该函数返回对应的索引值，以便可以获取到对应的控制点坐标，进行插值计算。
   *
   * @param dist 给定的距离值
   * @return Base::txInt 对应的索引值
   */
  virtual Base::txInt LocateIndexByDistance(const Base::txFloat dist) const TX_NOEXCEPT;

  /**
   * @brief xy2sl将ENU空间的二维坐标转换为等距球面线性插值法中的s和l值
   *
   * @param enu2d 待转换的ENU空间二维坐标(x, y)
   * @param s 转换后的s值
   * @param l 转换后的l值
   * @return true 转换成功
   * @return false 转换失败（坐标不在地图范围内）
   */
  virtual Base::txBool xy2sl(const Base::txVec2& enu2d, Base::txFloat& s,
                             Base::txFloat& l) const TX_NOEXCEPT TX_OVERRIDE {
    return HdMap::txLaneProjectionUtils::xy2sl_func(enu2d, s, l);
  }

  /**
   * @brief GetLocInfo 根据指定的索引值获取路网节点信息
   *
   * @param idx 路网节点的索引值
   * @param locInfo 返回指定索引位置的路网节点信息
   * @param s 返回指定索引位置的路网节点的s值
   * @return true 索引值存在
   * @return false 索引值不存在
   */
  virtual Base::txBool GetLocInfo(const Base::txInt idx, Base::Info_Lane_t& locInfo,
                                  Base::txFloat& s) const TX_NOEXCEPT;

 protected:
  /**
   * @brief 创建控制点，并返回一个四元组。
   *
   * @param p1_idx 第一个路网节点的索引值
   * @param p2_idx 第二个路网节点的索引值
   * @return
   * 一个四元组，第一个元素是第一个路网节点的控制点；第二个元素是第二个路网节点的控制点；第三个元素是第一个路网节点与第一个控制点之间的矢量；第四个元素是第二个路网节点与第二个控制点之间的矢量。
   * @return 当索引值无效时，返回一个空的四元组。
   */
  virtual std::tuple<Base::txVec2, Base::txVec2, Base::txVec2, Base::txVec2> MakeControlPoint(
      const Base::txInt p1_idx, const Base::txInt p2_idx) const TX_NOEXCEPT;

 public:
  /**
   * @brief 将samplepoints中的坐标点转换为controlPointVec。
   *
   * @param samplepoints 包含原始点坐标的数组。
   * @return 返回将samplepoints转换为controlPointVec的结果。
   */
  static controlPointVec to_controlPointVec(const control_path_node_vec& samplepoints) TX_NOEXCEPT;

  /**
   * @brief GenerateParallelSpline 生成并返回一个用于控制车辆沿着当前lane进行平行转向的贝塞尔曲线
   * @param _dir 平行转向的方向, 从VehicleLaneChangeType枚举中选取
   * @param _offset 平行转向的距离
   * @return 返回一个HdMap::txLaneInfoInterfacePtr的智能指针，表示贝塞尔曲线的控制点集合
   */
  virtual HdMap::txLaneInfoInterfacePtr GenerateParallelSpline(const Base::Enums::VehicleLaneChangeType _dir,
                                                               const Base::txFloat _offset) const TX_NOEXCEPT;

 protected:
  std::vector<Base::txVec2> m_sample_points;
  std::vector<std::tuple<Base::Info_Lane_t, Base::txFloat> > m_sample_locInfos;
  Base::txFloat m_real_lenth = 0.0;
  std::vector<std::tuple<Base::txFloat, Base::txFloat> > m_vec_segment_lookup;
};
using CentripetalCatMullPtr = std::shared_ptr<CentripetalCatMull>;

TX_NAMESPACE_CLOSE(TrafficFlow)

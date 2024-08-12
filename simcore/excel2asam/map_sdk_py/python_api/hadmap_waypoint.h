// Copyright 2024 Tencent Inc. All rights reserved.
//
// This file defines the class Waypoint.

#pragma once

#include <memory>
#include <string>
#include "hadmap_position.h"

namespace hadmap {
class Map;
class Waypoint {
 public:
  std::shared_ptr<Map> map;
  Position position;
  bool is_junction;

 public:
  /*! \~Chinese
   * @brief 默认构造函数.
   *
   */
  Waypoint();

  /*! \~Chinese
   * \brief 参数化构造函数.
   *
   * @param[in] k_map 共享指针, 指向一个 Map 类的实例.
   * @param[in] k_position 位置点信息, 可以为 world 和 lane.
   * @param[in] k_is_junction(Optional): 是否为路口.
   */
  explicit Waypoint(Position k_position);
  Waypoint(std::shared_ptr<Map> k_map, Position k_position);
  Waypoint(std::shared_ptr<Map> k_map, Position k_position, bool k_is_junction);

  /*! \~Chinese
   * \brief 拷贝构造函数.
   *
   * @param[in] other another Waypoint.
   */
  Waypoint(const Waypoint& other);

  /*! \~Chinese
   * @brief 赋值运算符重载, 允许将一个 Waypoint 对象的值赋给另一个同类型对象的实例.
   *
   * @param[in] k another Waypoint.
   * @return Waypoint&
   */
  Waypoint& operator=(const Waypoint& k);

  /*! \~Chinese
   * @brief 等号运算符重载, 用于比较两个 Waypoint 对象是否相等.
   *
   * @param[in] other another Waypoint.
   * @return bool
   */
  bool operator==(const Waypoint& other) const;

  /*! \~Chinese
   * @brief 逻辑非运算符重载, 用于判断一个 Waypoint 对象是否为空.
   *
   * @return bool
   */
  bool operator!() const;

  /*! \~Chinese
   * @brief 加运算符重载, 用于将一个 Waypoint 对象的值加上另一个同类型对象的实例.
   *
   * @param[in] waypoint
   * @return Waypoint&
   */
  const Waypoint operator+(const Waypoint& waypoint) const;

  /*! \~Chinese
   * @brief 减运算符重载, 用于将一个 Waypoint 对象的值减去另一个同类型对象的实例.
   *
   * @param[in] waypoint
   * @return Waypoint&
   */
  const Waypoint operator-(const Waypoint& waypoint) const;

  /*! \~Chinese
   * @brief 乘运算符重载, 用于将一个 Waypoint 对象的值乘以一个实数.
   *
   * @param[in] d 实数
   * @return Waypoint&
   */
  const Waypoint operator*(const double d) const;

  /*! \~Chinese
   * @brief 除运算符重载, 用于将一个 Waypoint 对象的值除以一个实数.
   *
   * @param[in] d 实数
   * @return Waypoint&
   */
  const Waypoint operator/(const double d) const;

 public:
  /*! \~Chinese
   * @brief 更新 Waypoint 对象的属性.
   *
   * @param[in] road_id 位置信息中的 road_id. (Default: 0)
   * @param[in] lane_id 位置信息中的 lane_id. (Default: 0)
   * @param[in] offset 与车道中心线的偏移量. (Default: 0.0)
   * @param[in] s 从道路的起点沿道路参考线获取的 s 坐标. (Default: 0.0)
   * @param[in] x 坐标值 X. (Default: 0.0)
   * @param[in] y 坐标值 Y. (Default: 0.0)
   * @param[in] z 坐标值 Z. (Default: 0.0)
   * @param[in] h 航向角 heading angle. (Unit: rad, Default: 0.0)
   * @param[in] p 俯仰角 pitch angle. (Unit: rad, Default: 0.0)
   * @param[in] r 旋转角 roll angle. (Unit: rad, Default: 0.0)
   * @return Waypoint&
   */
  Waypoint& Update(uint64_t road_id = 0, int64_t lane_id = 0, double offset = 0.0, double s = 0.0, double x = 0.0,
                   double y = 0.0, double z = 0.0, double h = 0.0, double p = 0.0, double r = 0.0);

  /*! \~Chinese
   * @brief 计算并返回下一个 Waypoint.
   *
   * @param[in] distance 下一个点与当前点的沿车道中心线的距离（Unit: m, Default: 0.0）.
   * @param[in] direction 下一个点的朝向（Default: "T", 表示直行）.
   * @return Waypoint
   */
  Waypoint Next(double distance = 0.0, std::string direction = "T") const;
};

}  // namespace hadmap

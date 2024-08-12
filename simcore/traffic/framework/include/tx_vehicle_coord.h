// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_catalog_cache.h"
#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_sim_point.h"
#include "vehicle_geometry.pb.h"

TX_NAMESPACE_OPEN(Base)
class VehicleCoord {
 public:
  virtual ~VehicleCoord() TX_DEFAULT;

  /**
   * @brief 获取车辆几何信息对象的引用
   *
   * 返回一个指向当前车辆几何信息的对象的引用。这个对象包含了车辆的各种几何属性，如位置、大小等。
   *
   * @return 车辆几何信息对象的引用
   */
  virtual const sim_msg::VehicleGeometory& VehicleGeometory() const TX_NOEXCEPT { return m_pb_veh_geom; }
  virtual sim_msg::VehicleGeometory& VehicleGeometory() TX_NOEXCEPT { return m_pb_veh_geom; }

  /**
   * @brief 同步车辆坐标位置信息
   *
   * 此函数用于同步车辆坐标的位置信息。返回一个布尔值，指示是否成功同步了车辆坐标的位置信息。
   * 在同步过程中，函数可能会受到传递的参数影响，例如 _passTime。
   *
   * @param _passTime 用于控制同步的时间参数
   * @return txBool 返回一个布尔值，指示是否成功同步了车辆坐标的位置信息
   */
  virtual txBool SyncPosition(const txFloat _passTime) TX_NOEXCEPT = 0;

  /**
   * @brief 获取车辆坐标的中心点，以WGS84地理坐标系的形式
   *
   * 返回车辆当前坐标的中心点，使用txWGS84格式的数据。
   *
   * @return Coord::txWGS84& 车辆坐标的中心点，以WGS84地理坐标系的形式
   */
  virtual Coord::txWGS84& GeomCenter() TX_NOEXCEPT = 0;

  /**
   * @brief 获取车辆坐标的中心点
   *
   * 返回车辆当前坐标的中心点，使用txWGS84地理坐标系的形式。
   *
   * @return const Coord::txWGS84& 车辆坐标的中心点
   */
  virtual const Coord::txWGS84& GeomCenter() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取车辆后车轮垂直轴线中心坐标
   *
   * 获取车辆后车轮垂直轴线中心坐标（经纬度），使用txWGS84地理坐标系。
   *
   * @return const Coord::txWGS84& 后车轮垂直轴线中心坐标
   */
  virtual Coord::txWGS84& RearAxleCenter() TX_NOEXCEPT = 0;
  virtual const Coord::txWGS84& RearAxleCenter() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取原始车辆坐标(经纬度)
   *
   * 获取车辆在WGS-84地理坐标系中的原始坐标。
   *
   * @return const Coord::txWGS84& 原始车辆坐标
   */
  virtual const Coord::txWGS84& RawVehicleCoord() const TX_NOEXCEPT { return m_vehicle_coord; }
  virtual Coord::txWGS84& RawVehicleCoord() TX_NOEXCEPT { return m_vehicle_coord; }

  /**
   * @brief 检查原始车辆坐标是否为几何中心
   *
   * 通过本函数，可以检查车辆的原始坐标是否为几何中心，也就是大致位于车辆边框内。
   *
   * @return 如果原始坐标是几何中心，则返回 true，否则返回 false.
   */
  virtual const Base::txBool RawVehicleCoord_Is_GeomCenter() const TX_NOEXCEPT;

  /**
   * @brief 判断原始车辆坐标是否是后车辆中心
   *
   * 该函数用于判断车辆的原始坐标是否位于后车辆中心，主要用于判断车辆是否准确地位于汽车行驶区域内。
   *
   * @return 如果原始坐标为后车辆中心，则返回 true，否则返回 false.
   */
  virtual const Base::txBool RawVehicleCoord_Is_RearAxleCenter() const TX_NOEXCEPT;

  /**
   * @brief 获取车辆坐标字符串
   *
   * 此函数用于获取车辆当前坐标的字符串表示形式。此字符串形式通常为 "X,Y" 形式的文本格式，其中 X 和 Y
   * 分别为车辆的经度和纬度。
   *
   * @return 车辆坐标字符串
   */
  virtual txString VehicleCoordStr() const TX_NOEXCEPT;

  /**
   * @brief 初始化vehicle的catalog
   *
   * @param _vehType 交通车类型
   * @return txBool
   */
  virtual txBool InitCatalog(const Base::Enums::VEHICLE_TYPE _vehType) TX_NOEXCEPT;

  /**
   * @brief 初始化行人的catalog
   *
   * @param _pedType 行人类型
   * @return txBool
   */
  virtual txBool InitCatalog(const Base::Enums::PEDESTRIAN_TYPE _pedType) TX_NOEXCEPT;

  /**
   * @brief 初始化障碍物的catalog
   *
   * @param _obsType 障碍物类型
   * @return txBool
   */
  virtual txBool InitCatalog(const Base::Enums::STATIC_ELEMENT_TYPE _obsType) TX_NOEXCEPT;
#if USE_CustomModelImport
  virtual txBool InitVehicleCatalog(const Base::txString& _vehType) TX_NOEXCEPT;
  virtual txBool InitPedestrianCatalog(const Base::txString& _pedType) TX_NOEXCEPT;
  virtual txBool InitObstacleCatalog(const Base::txString& _obsType) TX_NOEXCEPT;
#endif /*USE_CustomModelImport*/

  /**
   * @brief 获取catalog的长
   *
   * @return txFloat
   */
  virtual txFloat Catalog_Length() const TX_NOEXCEPT { return m_catalog_length; }

  /**
   * @brief 获取catalog的宽
   *
   * @return txFloat
   */
  virtual txFloat Catalog_Width() const TX_NOEXCEPT { return m_catalog_width; }

  /**
   * @brief 获取catalog的高
   *
   * @return txFloat
   */
  virtual txFloat Catalog_Height() const TX_NOEXCEPT { return m_catalog_height; }

 protected:
  /**
   * @brief 获取车辆坐标元素类型
   *
   * 此函数用于获取车辆坐标元素的类型。此函数返回的类型定义了车辆坐标的特性以及与其他元素的交互方式。
   *
   * @return 返回车辆坐标元素的类型
   */
  virtual Base::Enums::ElementType VehicleCoordElementType() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取车辆坐标元素行为类型
   *
   * 此函数用于获取车辆坐标元素的行为类型。通过此行为类型，我们可以了解车辆行为的特性以及与其他元素的交互方式。
   *
   * @return 返回车辆坐标元素的行为类型
   */
  virtual Base::Enums::VEHICLE_BEHAVIOR VehicleCoordElementBehavior() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取车辆后方中心点在 y 方向上的坐标
   *
   * 该函数返回车辆后方中心点在 y 方向上的坐标值。该值表示车辆对应的后方中心点在图像坐标系中的垂直位置。
   *
   * @return 返回车辆后方中心点在 y 方向上的坐标值
   */
  virtual Base::txFloat Catalog_Rear2Center_East() const TX_NOEXCEPT { return m_catalog_bbx_center.y(); }

  /**
   * @brief 获取车辆后方中心点在 Y 轴方向上的坐标
   *
   * 这个函数返回车辆后方中心点在图像坐标系中的垂直坐标值。
   *
   * @return 返回车辆后方中心点在 Y 轴方向上的坐标值
   */
  virtual Base::txFloat Catalog_Rear2Center_North() const TX_NOEXCEPT { return m_catalog_bbx_center.x(); }

  /**
   * @brief 获取车辆后方中心点在 Z 轴方向上的坐标
   *
   * 此函数返回车辆后方中心点在图像坐标系中的垂直坐标值。
   *
   * @return 返回车辆后方中心点在 Z 轴方向上的坐标值
   */
  virtual Base::txFloat Catalog_Rear2Center_Up() const TX_NOEXCEPT { return m_catalog_bbx_center.z(); }

  /**
   * @brief 获取车辆后方中心点在零坐标系的欧拉坐标
   *
   * 此函数返回车辆后方中心点在零坐标系的欧拉坐标，其中 X 轴表示东北方向，Y 轴表示北方向，Z 轴为零。
   *
   * @return 返回车辆后方中心点在零坐标系的欧拉坐标
   */
  virtual Base::txVec3 Catalog_Rear2Center_ENU() const TX_NOEXCEPT {
    return Base::txVec3(Catalog_Rear2Center_East(), Catalog_Rear2Center_North(), 0.0);
  }

  /**
   * @brief 计算车辆后方中心点在基坐标系中的笛卡尔坐标
   *
   * 此函数返回车辆后方中心点在基坐标系中的笛卡尔坐标，其中 X 轴表示东北方向，Y 轴表示北方向，Z 轴为零。
   *
   * @return 返回车辆后方中心点在基坐标系中的笛卡尔坐标
   */
  virtual Base::txVec3 Catalog_Center2Rear_ENU() const TX_NOEXCEPT {
    return Base::txVec3(Catalog_Rear2Center_East() * -1.0, Catalog_Rear2Center_North() * -1.0, 0.0);
  }

  /**
   * @brief 初始化顶点目录
   *
   * 本函数用于初始化目录，其中包括设置目录的中心点坐标。
   *
   * @param x 目录中心点的x坐标值
   * @param y 目录中心点的y坐标值
   * @param z 目录中心点的z坐标值
   */
  virtual void InitCatalog(const txFloat x, const txFloat y, const txFloat z) TX_NOEXCEPT {
    m_catalog_bbx_center = Base::txVec3(x, y, z);
  }

 private:
  sim_msg::VehicleGeometory m_pb_veh_geom;
  Coord::txWGS84 m_vehicle_coord;
  txVec3 m_catalog_bbx_center;
  txFloat m_catalog_length = 0.0, m_catalog_width = 0.0, m_catalog_height = 0.0;
};
using VehicleCoordPtr = std::shared_ptr<VehicleCoord>;
TX_NAMESPACE_CLOSE(Base)

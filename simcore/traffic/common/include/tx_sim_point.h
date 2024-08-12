// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/optional.hpp>
#include "structs/base_struct.h"
#include "tx_frame_utils.h"
#include "tx_marco.h"
#include "tx_math.h"
#include "tx_serialization.h"
#include "tx_type_def.h"
#if __UseProj__
#  include "txCoordTrans.h"
#endif /*__UseProj__*/

TX_NAMESPACE_OPEN(Coord)
class txWGS84;
class txENU;

class txReferencePoint {
 protected:
  using txFloat = Base::txFloat;
  using txEnuOptional = boost::optional<Base::txVec3>;
  using txWGS84Optional = boost::optional<hadmap::txPoint>;

 public:
  txReferencePoint() TX_DEFAULT;
  ~txReferencePoint() TX_DEFAULT;
  static void SetReferencePoint(const Base::txFloat _lon, const Base::txFloat _lat,
                                const Base::txFloat _alt) TX_NOEXCEPT {
    SetReferencePoint(hadmap::txPoint(_lon, _lat, _alt));
  }
  static void SetReferencePoint(const hadmap::txPoint& _wgs84) TX_NOEXCEPT;

  static hadmap::txPoint ConverGPS4HDMap(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
    return hadmap::txPoint(__Lon__(_wgs84), __Lat__(_wgs84), FLAGS_HdMap_3D_Alt);
  }
  static hadmap::txPoint ConverGPS4RTree(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
    return hadmap::txPoint(__Lon__(_wgs84), __Lat__(_wgs84), FLAGS_default_altitude);
  }

  /**
   * @brief 获取经度
   *
   * @return Base::txFloat
   */
  static Base::txFloat Ref_Lon() TX_NOEXCEPT { return __Lon__(s_reference_wgs84); }

  /**
   * @brief 获取纬度
   *
   * @return Base::txFloat
   */
  static Base::txFloat Ref_Lat() TX_NOEXCEPT { return __Lat__(s_reference_wgs84); }

  /**
   * @brief 获取高程
   *
   * @return Base::txFloat
   */
  static Base::txFloat Ref_Alt() TX_NOEXCEPT { return __Alt__(s_reference_wgs84); }

 protected:
  /**
   * @brief enu坐标系->wgs84
   *
   * @param _enu3d
   * @return hadmap::txPoint
   */
  static hadmap::txPoint enu2wgs84(const Base::txVec3& _enu3d) TX_NOEXCEPT;

  /**
   * @brief wgs84 to enu
   *
   * @param _wgs84
   * @return Base::txVec3
   */
  static Base::txVec3 wgs842enu(const hadmap::txPoint& _wgs84) TX_NOEXCEPT;

 protected:
  static hadmap::txPoint s_reference_wgs84;
#if __UseProj__
  static Coord::UTM2WGS84Utility m_proj_api;
#endif /*__UseProj__*/
};

class txWGS84 : public txReferencePoint {
 protected:
  friend class txENU;
  txWGS84(const hadmap::txPoint& _wgs84, const Base::txVec3& _enu) : m_wgs84(_wgs84), m_opEnu(_enu) {}

 public:
  txWGS84() : m_wgs84(0.0, 0.0, 0.0) {}
  explicit txWGS84(const hadmap::txPoint& _wgs84) : m_wgs84(_wgs84) {}
  txWGS84(txWGS84&& _other) noexcept;
  txWGS84(const txWGS84& _other) noexcept : m_wgs84(_other.m_wgs84), m_opEnu(_other.m_opEnu) {}
  ~txWGS84() TX_DEFAULT;

  txWGS84& operator=(txWGS84&& _other) noexcept;
  txWGS84& operator=(const txWGS84& _other) noexcept;

  /**
   * @brief 获取经度
   *
   * @return Base::txFloat
   */
  inline Base::txFloat Lon() const TX_NOEXCEPT { return __Lon__(m_wgs84); }
  inline Base::txFloat& Lon() TX_NOEXCEPT { return __Lon__(m_wgs84); }

  /**
   * @brief 获取纬度
   *
   * @return Base::txFloat
   */
  inline Base::txFloat Lat() const TX_NOEXCEPT { return __Lat__(m_wgs84); }
  inline Base::txFloat& Lat() TX_NOEXCEPT { return __Lat__(m_wgs84); }

  /**
   * @brief 获取高度
   *
   * @return Base::txFloat
   */
  inline Base::txFloat Alt() const TX_NOEXCEPT { return __Alt__(m_wgs84); }
  inline Base::txFloat& Alt() TX_NOEXCEPT { return __Alt__(m_wgs84); }

  /**
   * @brief 获取txPoint点
   *
   * @return const hadmap::txPoint&
   */
  inline const hadmap::txPoint& WGS84() const TX_NOEXCEPT { return m_wgs84; }
  inline const hadmap::txPoint WGS84_Adsorb() const TX_NOEXCEPT {
    return hadmap::txPoint(Lon(), Lat(), FLAGS_HdMap_3D_Alt);
  }
  inline hadmap::txPoint& WGS84() TX_NOEXCEPT { return m_wgs84; }

  /**
   * @brief txSimPoint 设置使用WGS84坐标系中的点
   * @param _pos WGS84坐标系的点
   * @return 返回当前坐标系下的相应点
   */
  inline const txWGS84& FromWGS84(const hadmap::txPoint& _pos) TX_NOEXCEPT {
    m_wgs84 = _pos;
    ClearOpENU();
    return *this;
  }
  inline const txWGS84& FromWGS84(const Base::txFloat _lon, const Base::txFloat _lat,
                                  const Base::txFloat _alt) TX_NOEXCEPT {
    return FromWGS84(hadmap::txPoint(_lon, _lat, _alt));
  }
  inline const txWGS84& FromWGS84(const Base::txFloat _lon, const Base::txFloat _lat) TX_NOEXCEPT {
    return FromWGS84(hadmap::txPoint(_lon, _lat, FLAGS_default_altitude));
  }

  /**
   * @brief 从enu坐标转换为wgs84
   *
   * @param _enu3d enu向量
   * @return const txWGS84&
   */
  inline const txWGS84& FromENU(const Base::txVec3& _enu3d) TX_NOEXCEPT {
    m_wgs84 = enu2wgs84(_enu3d);
    m_opEnu = _enu3d;
    return *this;
  }

  /**
   * @brief 从enu xyz转换为wgs84
   *
   * @param _x x
   * @param _y y
   * @param _z z
   * @return const txWGS84&
   */
  inline const txWGS84& FromENU(const Base::txFloat& _x, const Base::txFloat& _y, const Base::txFloat& _z) TX_NOEXCEPT {
    return FromENU(Base::txVec3(_x, _y, _z));
  }

  /**
   * @brief 从 xy转换，z默认0
   *
   * @param _x x
   * @param _y y
   * @return const txWGS84&
   */
  inline const txWGS84& FromENU(const Base::txFloat& _x, const Base::txFloat& _y) TX_NOEXCEPT {
    return FromENU(Base::txVec3(_x, _y, 0.0));
  }

  /**
   * @brief 将点矢量坐标沿指定的基准点旋转
   *
   * 使用原点坐标为基准点的三维空间内旋转函数。该函数将模型点从原点坐标系移动到新的坐标系，再进行旋转，最后将结果转回原点坐标系。
   *
   * @param _step 本次操作的步长
   * @return void
   */
  void TranslateLocalPos(const Base::txVec3& _step) TX_NOEXCEPT;

  /**
   * @brief 本函数将当前坐标系的点矢量沿指定的基准点旋转指定的距离。
   *
   * 首先，使用原点坐标系将输入坐标转移到新的坐标系。然后，该函数执行矩阵转换以完成旋转。最后，将结果点矢量转回原点坐标系。
   *
   * @param _x  沿x轴方向旋转的距离。
   * @param _y  沿y轴方向旋转的距离。
   * @param _z  沿z轴方向旋转的距离。
   * @return void
   */
  void TranslateLocalPos(const Base::txFloat _x, const Base::txFloat _y, const Base::txFloat _z) TX_NOEXCEPT {
    TranslateLocalPos(Base::txVec3(_x, _y, _z));
  }

  /**
   * @brief 本函数用于沿着本地坐标系的 x 轴和 y 轴进行平移操作。
   *
   * 这个函数将会获取传入的平移值 _x 和 _y，在不影响原始坐标系的情况下，
   * 沿着本地坐标系的 x 轴和 y 轴进行平移操作，然后将结果转回原始坐标系。
   *
   * @param _x 本地坐标系沿 x 轴平移的距离。
   * @param _y 本地坐标系沿 y 轴平移的距离。
   */
  void TranslateLocalPos(const Base::txFloat _x, const Base::txFloat _y) TX_NOEXCEPT {
    TranslateLocalPos(Base::txVec3(_x, _y, 0.0));
  }

  /**
   * @brief 根据WGS84参考系计算两个点间的欧几里得距离。
   *
   * 这个函数用于计算两个使用WGS84参考系的点在地理坐标中的欧几里得距离。注意，该函数不包括高度信息。
   *
   * @param _pt1 第一个点的坐标。
   * @param _pt2 第二个点的坐标。
   * @return 返回两个点之间的欧几里得距离。
   */
  static Base::txFloat EuclideanDistanceByWGS84WithoutAlt(const hadmap::txPoint& _pt1,
                                                          const hadmap::txPoint& _pt2) TX_NOEXCEPT;

  /**
   * @brief 根据WGS84参考系计算两个点之间的欧几里得距离，不包括高度信息。
   *
   * @param _pt1 第一个点的坐标（使用WGS84参考系）。
   * @param _pt2 第二个点的坐标（使用WGS84参考系）。
   * @return 返回两个点之间的欧几里得距离（单位：米）。
   */
  static Base::txFloat EuclideanDistanceByWGS84WithoutAlt(const txWGS84& _pt1, const txWGS84& _pt2) TX_NOEXCEPT;

  /**
   * @brief 转换为ENU坐标系
   *
   * @return txENU
   */
  txENU ToENU() TX_NOEXCEPT;

  /**
   * @brief 是否是ENU坐标系
   *
   * @return Base::txBool
   */
  Base::txBool hasENU() const TX_NOEXCEPT { return m_opEnu.is_initialized(); }

  /**
   * @brief 获取ENU坐标系下点
   *
   * @return txENU
   */
  txENU GetENU() const TX_NOEXCEPT;

  /**
   * @brief 获取WGS84坐标系点的字符串表示
   *
   * @return Base::txString
   */
  Base::txString StrWGS84() const TX_NOEXCEPT;

  /**
   * @brief 获取ENU坐标系下字符串表示
   *
   * @return Base::txString
   */
  Base::txString StrENU() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const txWGS84& v) TX_NOEXCEPT {
    os << v.StrWGS84() << v.StrENU();
    return os;
  }

 public:
  template <class Archive>
  void save(Archive& archive) const {
    archive(_MAKE_NVP_("wgs84", m_wgs84));
  }

  template <class Archive>
  void load(Archive& archive) {
    ClearOpENU();
    archive(_MAKE_NVP_("wgs84", m_wgs84));
  }

 protected:
  void ClearOpENU() TX_NOEXCEPT { m_opEnu = ::boost::none; }

 protected:
  hadmap::txPoint m_wgs84;
  txEnuOptional m_opEnu = ::boost::none;
};

class txENU : public txReferencePoint {
 protected:
  friend class txWGS84;
  txENU(const Base::txVec3& _enu, const hadmap::txPoint& _wgs84) : m_enu(_enu), m_opWGS84(_wgs84) {}

 public:
  txENU() : m_enu(0.0, 0.0, 0.0) {}
  explicit txENU(const Base::txVec3& _enu) : m_enu(_enu) {}
  txENU(txENU&& _other) noexcept;
  txENU(const txENU& _other) noexcept : m_enu(_other.m_enu), m_opWGS84(_other.m_opWGS84) {}
  ~txENU() TX_DEFAULT;

  txENU& operator=(txENU&& _other) noexcept;
  txENU& operator=(const txENU& _other) noexcept;

  /**
   * @brief 获取enu中x
   *
   * @return Base::txFloat
   */
  inline Base::txFloat X() const TX_NOEXCEPT { return m_enu.x(); }
  inline Base::txFloat& X() TX_NOEXCEPT { return m_enu.x(); }

  /**
   * @brief 获取enu中y
   *
   * @return Base::txFloat
   */
  inline Base::txFloat Y() const TX_NOEXCEPT { return m_enu.y(); }
  inline Base::txFloat& Y() TX_NOEXCEPT { return m_enu.y(); }
  /*   inline Base::txFloat Z() const TX_NOEXCEPT { return m_enu.z(); }
     inline Base::txFloat& Z() TX_NOEXCEPT { return m_enu.z(); }*/

  /**
   * @brief 获取enu的坐标
   *
   * @return const Base::txVec3&
   */
  inline const Base::txVec3& ENU() const TX_NOEXCEPT { return m_enu; }
  inline Base::txVec3& ENU() TX_NOEXCEPT { return m_enu; }

  /**
   * @brief 获取enu的2d坐标
   *
   * @return Base::txVec2
   */
  inline Base::txVec2 ENU2D() const TX_NOEXCEPT { return Utils::Vec3_Vec2(m_enu); }

  /**
   * @brief 从wgs84坐标点设置当前点
   *
   * @param _wgs84
   * @return const txENU&
   */
  inline const txENU& FromWGS84(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
    m_enu = wgs842enu(_wgs84);
    m_opWGS84 = _wgs84;
    return *this;
  }

  /**
   * @brief 从经纬度设置当前点
   *
   * @param _lon 经度
   * @param _lat 纬度
   * @param _alt 高度
   * @return const txENU&
   */
  inline const txENU& FromWGS84(const Base::txFloat _lon, const Base::txFloat _lat,
                                const Base::txFloat _alt) TX_NOEXCEPT {
    return FromWGS84(hadmap::txPoint(_lon, _lat, _alt));
  }
  inline const txENU& FromWGS84(const Base::txFloat _lon, const Base::txFloat _lat) TX_NOEXCEPT {
    return FromWGS84(hadmap::txPoint(_lon, _lat, FLAGS_default_altitude));
  }

  /**
   * @brief 设置enu点
   *
   * @param _enu3d 提供的3d enu坐标
   * @return const txENU&
   */
  inline const txENU& FromENU(const Base::txVec3& _enu3d) TX_NOEXCEPT {
    m_enu = _enu3d;
    ClearOpWGS84();
    return *this;
  }
  inline const txENU& FromENU(const Base::txFloat& _x, const Base::txFloat& _y, const Base::txFloat& _z) TX_NOEXCEPT {
    return FromENU(Base::txVec3(_x, _y, _z));
  }
  inline const txENU& FromENU(const Base::txFloat& _x, const Base::txFloat& _y) TX_NOEXCEPT {
    return FromENU(Base::txVec3(_x, _y, 0.0));
  }

  /**
   * @brief txSimPoint 将相对于当前点的偏移量作为笛卡尔坐标，累加到当前的笛卡尔坐标上。
   * @param _step 笛卡尔坐标的偏移量。
   */
  inline void TranslateLocalPos(const Base::txVec3& _step) TX_NOEXCEPT { FromENU(ENU() + _step); }

  /**
   * @brief TranslateLocalPos 沿着当前坐标的坐标系移动一定距离，将笛卡尔坐标（x,y,z）移动到当前坐标位置加上该移动向量。
   * @param _x 笛卡尔坐标系的x轴坐标。
   * @param _y 笛卡尔坐标系的y轴坐标。
   * @param _z 笛卡尔坐标系的z轴坐标。
   */
  inline void TranslateLocalPos(const Base::txFloat _x, const Base::txFloat _y, const Base::txFloat _z) TX_NOEXCEPT {
    TranslateLocalPos(Base::txVec3(_x, _y, _z));
  }

  /**
   * @brief 翻译相对于当前坐标系中的位置
   *
   * 使用当前坐标系的坐标轴系统，以相对方式将当前坐标向量移动。
   * 根据 x, y 轴的距离偏移进行移动。
   *
   * @param _x 沿 x 轴移动的距离
   * @param _y 沿 y 轴移动的距离
   */
  inline void TranslateLocalPos(const Base::txFloat _x, const Base::txFloat _y) TX_NOEXCEPT {
    TranslateLocalPos(Base::txVec3(_x, _y, 0.0));
  }

  /**
   * @brief 转换为wgs84
   *
   * @return txWGS84
   */
  txWGS84 ToWGS84() TX_NOEXCEPT;
  Base::txBool hasWGS84() const TX_NOEXCEPT { return m_opWGS84.is_initialized(); }
  txWGS84 GetWGS84() const TX_NOEXCEPT;
  Base::txString StrWGS84() const TX_NOEXCEPT;
  Base::txString StrENU() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const txENU& v) TX_NOEXCEPT {
    os << v.StrWGS84() << v.StrENU();
    return os;
  }

 public:
  static Base::txFloat EuclideanDistanceLocal_3D(const txENU& v0, const txENU& v1) TX_NOEXCEPT;
  static Base::txFloat EuclideanDistanceLocal_2D(const txENU& v0, const txENU& v1) TX_NOEXCEPT;

 public:
  template <class Archive>
  void save(Archive& archive) const {
    archive(_MAKE_NVP_("enu", m_enu));
  }

  template <class Archive>
  void load(Archive& archive) {
    ClearOpWGS84();
    archive(_MAKE_NVP_("enu", m_enu));
  }

 protected:
  void ClearOpWGS84() TX_NOEXCEPT { m_opWGS84 = ::boost::none; }

 protected:
  Base::txVec3 m_enu;
  txWGS84Optional m_opWGS84 = boost::none;
};

/**
 * @brief txSimPoint 把输入的 WGS84 经纬度坐标转换为 HDMap 系统使用的坐标
 * @param _wgs84 WGS84 经纬度坐标
 * @return txPoint 返回 HDMap 系统的坐标
 */
inline hadmap::txPoint ConverGPS4HDMap(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
  return txReferencePoint::ConverGPS4HDMap(_wgs84);
}

/**
 * @brief txSimPoint 把输入的 WGS84 经纬度坐标转换为 RTree 索引使用的 UTM 坐标
 * @param _wgs84 WGS84 经纬度坐标
 * @return txPoint 返回 UTM 坐标
 */
inline hadmap::txPoint ConverGPS4RTree(const hadmap::txPoint& _wgs84) TX_NOEXCEPT {
  return txReferencePoint::ConverGPS4RTree(_wgs84);
}

/**
 * @brief 计算车道偏移
 *
 * @param srcPos
 * @param vLaneDir
 * @param offset
 * @return Coord::txWGS84
 */
inline Coord::txWGS84 ComputeLaneOffset(Coord::txWGS84 srcPos, const Base::txVec3& vLaneDir,
                                        const Base::txFloat offset) TX_NOEXCEPT {
  if (Math::isNotZero(offset)) {
    Base::txVec3 vTargetDir;
    vTargetDir = Utils::VetRotVecByDegree(vLaneDir, Unit::txDegree::MakeDegree(LeftOnENU), Utils::Axis_Enu_Up());
    srcPos.TranslateLocalPos(__East__(vTargetDir) * offset, __North__(vTargetDir) * offset);
  }
  return srcPos;
}

TX_NAMESPACE_CLOSE(Coord)

#define __3d_Alt__(var) Coord::ConverGPS4HDMap(var)
#define __2d_Alt__(var) Coord::ConverGPS4RTree(var)

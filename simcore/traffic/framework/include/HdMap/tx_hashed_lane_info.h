// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_class_counter.h"
#include "tx_header.h"
#include "tx_lane_geom_info.h"
#include "tx_locate_info.h"
#include "tx_serialization.h"
#include "tx_sim_point.h"
#if USE_HashedRoadNetwork

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

class HashedLaneInfo;
using HashedLaneInfoPtr = std::shared_ptr<HashedLaneInfo>;

class HashedLaneInfo {
  Utils::Counter<HashedLaneInfo> _c;

 public:
  using ScopeType = std::tuple<Base::txFloat /*section_start_s*/, Base::txFloat /*section_end_s*/>;
  HashedLaneInfo() TX_DEFAULT;

  /**
   * @brief 哈希的LaneInfo构造函数
   * @param[in] _LaneInfo 输入的LaneInfo结构体
   * @param[in] _SubSecIdx 子路径的索引
   * @param[in] _SecScope 子路径的范围
   * @param[in] _CurveLen 路径的长度
   * @param[in] _sectionStartPt 路径起点的WGS84坐标
   * @param[in] _distanceOnCurve 当前子路径的长度
   * @param[in] geom_ptr 指向HdMap::txLaneInfoInterface的指针
   */
  HashedLaneInfo(const Base::Info_Lane_t& _LaneInfo, const Base::txSize _SubSecIdx, const ScopeType& _SecScope,
                 const Base::txFloat _CurveLen, const Coord::txWGS84& _sectionStartPt,
                 const Base::txFloat _distanceOnCurve, HdMap::txLaneInfoInterfacePtr geom_ptr)
      : m_laneinfo(_LaneInfo),
        m_subSectionIdx(_SubSecIdx),
        m_subSectionScope(_SecScope),
        m_curveLength(_CurveLen),
        subSectionStartWGS84(_sectionStartPt),
        m_s_without_hash(_distanceOnCurve),
        m_is_valid_without_hash(true),
        m_geom_ptr_without_hash(geom_ptr) {}
  virtual ~HashedLaneInfo();

  /**
   * @brief 根据给定的当前道路信息、距离、道路位置信息生成哈希后的道路信息
   *
   * @param cur_geom_ptr 当前道路的地图信息指针
   * @param distOnCurce 当前距离道路起点的长度
   * @param laneLocInfo 道路位置信息
   * @return HashedLaneInfo 哈希后的道路信息
   */
  friend HashedLaneInfo GenerateHashedLaneInfo(const HdMap::txLaneInfoInterfacePtr cur_geom_ptr,
                                               const Base::txFloat distOnCurce,
                                               const Base::Info_Lane_t& laneLocInfo) TX_NOEXCEPT;

  /**
   * @brief 生成哈希后的道路信息
   *
   * @param laneLocInfo 道路位置信息
   * @param cur_geom_ptr 当前道路的地图信息指针
   * @param subSectionIdx 子路径索引
   * @param curveLen 当前子路径的长度
   * @return HashedLaneInfo 哈希后的道路信息
   */
  friend HashedLaneInfo GenerateHashedLaneInfo(const Base::Info_Lane_t& laneLocInfo,
                                               const HdMap::txLaneInfoInterfacePtr cur_geom_ptr,
                                               const Base::txSize subSectionIdx, Base::txFloat curveLen) TX_NOEXCEPT;

 public:
  /**
   * @brief 为 HashedLaneInfo 对象分配另一个 HashedLaneInfo 对象的内容
   *
   * 为当前 HashedLaneInfo 对象分配另一个 HashedLaneInfo 对象的内容，并拷贝其值。
   *
   * @param _other 被拷贝的 HashedLaneInfo 对象
   * @return 当前 HashedLaneInfo 对象的引用
   */
  HashedLaneInfo& operator=(const HashedLaneInfo& _other) TX_NOEXCEPT {
    m_laneinfo = _other.m_laneinfo;
    m_subSectionIdx = _other.m_subSectionIdx;
    m_subSectionScope = _other.m_subSectionScope;
    m_curveLength = _other.m_curveLength;
    subSectionStartWGS84 = _other.subSectionStartWGS84;
    m_s_without_hash = _other.m_s_without_hash;
    m_is_valid_without_hash = _other.m_is_valid_without_hash;
    m_geom_ptr_without_hash = _other.m_geom_ptr_without_hash;
    return *this;
  }

  /**
   * @brief 设置当前 HashedLaneInfo 对象的有效性标志
   *
   * 根据给定的布尔值，将当前 HashedLaneInfo 对象的有效性标志设置为相应的值。
   * 当某个 HashedLaneInfo 对象的有效性标志被设置为 false 时，表示该对象的数据无效或已过期。
   *
   * @param _f 布尔值，表示对象是否有效
   */
  void SetValid(const Base::txBool _f) TX_NOEXCEPT { m_is_valid_without_hash = _f; }

  /**
   * @brief 判断当前 HashedLaneInfo 对象是否有效
   *
   * 如果当前 HashedLaneInfo 对象的 IsValid() 函数返回
   * true，表示该对象的数据是有效的，否则表示该对象的数据无效或已过期。
   *
   * @return true 表示对象有效，false 表示对象无效或已过期
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return m_is_valid_without_hash; }

  /**
   * @brief 获取当前子域的开始范围
   *
   * 该函数返回当前子域的开始范围，即子域在数据流中的起始位置。
   * 这里的子域是指哈希后的子域，其大小由当前对象的哈希宽度和子域大小决定。
   *
   * @return float 当前子域的开始范围
   */
  Base::txFloat StartScope() const TX_NOEXCEPT { return std::get<0>(m_subSectionScope); }

  /**
   * @brief 获取当前子域的结束范围
   *
   * 该函数返回当前子域的结束范围，即子域在数据流中的结束位置。
   * 这里的子域是指哈希后的子域，其大小由当前对象的哈希宽度和子域大小决定。
   *
   * @return float 当前子域的结束范围
   */
  Base::txFloat EndScope() const TX_NOEXCEPT { return std::get<1>(m_subSectionScope); }

  /**
   * @brief 获取当前哈希子域的实际长度
   *
   * 该函数返回当前哈希子域的实际长度，即子域在数据流中的实际范围。
   * 这里的子域是指哈希后的子域，其大小由当前对象的哈希宽度和子域大小决定。
   *
   * @return Base::txFloat 当前哈希子域的实际长度
   */
  Base::txFloat RealLength() const TX_NOEXCEPT { return (EndScope() - StartScope()); }

  /**
   * @brief 获取当前哈希子域的实际长度
   *
   * 该函数返回当前哈希子域的实际长度，即子域在数据流中的实际范围。
   * 这里的子域是指哈希后的子域，其大小由当前对象的哈希宽度和子域大小决定。
   *
   * @return Base::txFloat 当前哈希子域的实际长度
   */
  Base::txFloat GeomLength() const TX_NOEXCEPT { return m_curveLength; }

  /**
   * @brief 判断当前约束是否处于结束位置
   *
   * 该函数返回是否在当前约束的起始位置检测到约束的结束，若检测到则返回 true，否则返回 false。
   *
   * @return 是否在当前约束的起始位置检测到约束的结束
   */
  Base::txBool IsLastSection() const TX_NOEXCEPT { return Math::isEqual(EndScope(), GeomLength()); }

  /**
   * @brief 检查当前车道是否处于开始的section
   *
   * 该函数返回是否在当前车道的起始位置，若是，则返回 true，否则返回 false。
   *
   * @return 是否在当前车道的起始位置
   */
  Base::txBool IsFirstSection() const TX_NOEXCEPT { return (0 == m_subSectionIdx); }

  /**
   * @brief 计算曲线上车辆与起点之间的距离
   *
   * 该函数返回一个浮点数，表示车辆在曲线上距离起点的距离，单位为米。
   *
   * @return 曲线上车辆与起点之间的距离，单位为米
   */
  Base::txFloat DistanceOnCurve() const TX_NOEXCEPT { return m_s_without_hash; }

  /**
   * @brief 获取车辆在当前节点的水平线距离
   *
   * 该函数返回一个浮点数，表示车辆在当前节点的水平线与起点的距离。
   *
   * @return 车辆在当前节点的水平线距离，单位为米
   */
  Base::txFloat S_in_Node() const TX_NOEXCEPT { return (DistanceOnCurve() - StartScope()); }

  /**
   * @brief 获取作用域终点与当前curve上距离的差值
   *
   * @return Base::txFloat
   */
  Base::txFloat S_Inv_in_Node() const TX_NOEXCEPT { return (EndScope() - DistanceOnCurve()); }

  /**
   * @brief 获取当前线条对应的Geometry指针
   *
   * 返回一个HdMap::txLaneInfoInterfacePtr指针，表示当前线条对应的地图中的Geometry对象。
   *
   * @return HdMap::txLaneInfoInterfacePtr 当前线条对应的地图中的Geometry对象指针
   */
  HdMap::txLaneInfoInterfacePtr GeomPtr() const TX_NOEXCEPT { return m_geom_ptr_without_hash; }

  /**
   * @brief 获取当前的车道信息
   *
   * @return const Base::Info_Lane_t&
   */
  const Base::Info_Lane_t& LaneInfo() const TX_NOEXCEPT { return m_laneinfo; }

  /**
   * @brief 转换为可读字符串
   *
   * @return const std::string& 转换后的字符串
   */
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << _StreamPrecision_ << TX_COND_NAME(IsValid, m_is_valid_without_hash) << m_laneinfo << TX_VARS(m_subSectionIdx)
        << TX_VARS_NAME(section_start_s, std::get<0>(m_subSectionScope))
        << TX_VARS_NAME(section_end_s, std::get<1>(m_subSectionScope)) << TX_VARS(m_curveLength)
        << TX_VARS(subSectionStartWGS84.StrWGS84()) << TX_VARS_NAME(distanceOnCurve, m_s_without_hash);
    return oss.str();
  }

  /**
   * @brief ==运算符重写
   *
   * @param o 比较的对象
   * @return Base::txBool
   */
  Base::txBool operator==(const HashedLaneInfo& o) const TX_NOEXCEPT { return HashCompare::equal(*this, o); }

  /**
   * @brief 重载输出运算符，用于将 HashedLaneInfo 对象转换为字符串
   *
   * @param os   要输出的流对象
   * @param v    要转换为字符串的 HashedLaneInfo 对象
   * @return os  经过转换后的字符串流
   */
  friend std::ostream& operator<<(std::ostream& os, const HashedLaneInfo& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
  Base::Info_Lane_t m_laneinfo;
  Base::txSize m_subSectionIdx = -1; /*0,1,2,3...*/
  ScopeType m_subSectionScope = std::make_tuple(-1.0, -1.0);
  Base::txFloat m_curveLength = -1.0; /*lane length, link length*/
  Coord::txWGS84 subSectionStartWGS84;
  Base::txFloat m_s_without_hash = -1.0;
  Base::txBool m_is_valid_without_hash = false;
  HdMap::txLaneInfoInterfacePtr m_geom_ptr_without_hash = nullptr;

 public:
  /**
   * @brief 对象保存模板
   *
   * @tparam Archive 对象的类型
   * @param archive 对象
   */
  template <class Archive>
  void save(Archive& archive) const {
    archive(_MAKE_NVP_("laneInfo", (m_laneinfo)), _MAKE_NVP_("subSectionIdx", (m_subSectionIdx)),
            _MAKE_NVP_("subSectionScope", (m_subSectionScope)), _MAKE_NVP_("curveLength", (m_curveLength)),
            _MAKE_NVP_("subSectionStartWGS84", (subSectionStartWGS84)), _MAKE_NVP_("s", (m_s_without_hash)),
            _MAKE_NVP_("bValid", (m_is_valid_without_hash)));
  }

  /**
   * @brief 加载对象到指定的存档文件。
   *
   * @param archive 要加载的存档文件。
   */
  template <class Archive>
  void load(Archive& archive) {
    archive(_MAKE_NVP_("laneInfo", (m_laneinfo)), _MAKE_NVP_("subSectionIdx", (m_subSectionIdx)),
            _MAKE_NVP_("subSectionScope", (m_subSectionScope)), _MAKE_NVP_("curveLength", (m_curveLength)),
            _MAKE_NVP_("subSectionStartWGS84", (subSectionStartWGS84)), _MAKE_NVP_("s", (m_s_without_hash)),
            _MAKE_NVP_("bValid", (m_is_valid_without_hash)));
    m_geom_ptr_without_hash = HdMap::HadmapCacheConCurrent::GetGeomInfoById(m_laneinfo);
  }

 public:
  /**
   * @brief 计算子部分哈希范围长度
   *
   * @return Base::txFloat 子部分哈希范围长度
   */
  static Base::txFloat HashScopeLen() TX_NOEXCEPT { return std::pow(2, FLAGS_SubSectionPower); }

  /**
   * @brief 计算子路径索引
   * @param distOnCurve 当前位置与曲线起点的距离
   * @param curveLength 曲线总长度
   * @return 一个包含子路径起始索引和结束索引的元组
   * @note 该函数计算一个在给定曲线上的投影点所对应的子路径索引，其中子路径是由多个具有相同长度的有序段组成的
   */
  static std::tuple<Base::txSize, ScopeType> ComputeSubSectionIdx(const Base::txFloat distOnCurve,
                                                                  const Base::txFloat curveLength) TX_NOEXCEPT {
    static const Base::txFloat scopeLen = HashScopeLen();
    static const Base::txInt subSectionPower = -1 * FLAGS_SubSectionPower;
    const Base::txSize subSectionIdx = scalbn(distOnCurve, subSectionPower);
    TX_MARK("distOnCurve * 2^subSectionPower");
    if (scopeLen * (subSectionIdx + 1) <= curveLength) {
      return std::make_tuple(subSectionIdx,
                             std::make_tuple(scopeLen * (subSectionIdx), scopeLen * (subSectionIdx + 1)));
    } else {
      return std::make_tuple(subSectionIdx, std::make_tuple(scopeLen * (subSectionIdx), curveLength));
    }
  }

 public:
  struct HashCompare {
    /**
     * @brief 对给定的 HashedLaneInfo 进行哈希操作
     *
     * @param _hash_id 需要进行哈希操作的 HashedLaneInfo
     * @return size_t 返回哈希值
     */
    static size_t hash(const HashedLaneInfo& _hash_id) TX_NOEXCEPT {
      size_t seed = Utils::Info_Lane_t_HashCompare::hash(_hash_id.m_laneinfo);
      /*boost::hash_combine(seed, _hash_id.m_laneinfo.isOnLaneLink);
      boost::hash_combine(seed, _hash_id.m_laneinfo.onLaneUid.roadId);
      boost::hash_combine(seed, _hash_id.m_laneinfo.onLaneUid.sectionId);
      boost::hash_combine(seed, _hash_id.m_laneinfo.onLaneUid.laneId);
      boost::hash_combine(seed, _hash_id.m_laneinfo.onLinkId);*/
      boost::hash_combine(seed, _hash_id.m_subSectionIdx);
      return seed;
    }

    /**
     * @brief 判断两个 HashedLaneInfo 是否相等
     * @param lv 第一个 HashedLaneInfo 对象
     * @param rv 第二个 HashedLaneInfo 对象
     * @return 如果两个 HashedLaneInfo 对象相等，则返回 true，否则返回 false
     * @note 该函数比较两个 HashedLaneInfo 对象的 m_laneinfo 和 m_subSectionIdx 是否相等
     */
    static bool equal(const HashedLaneInfo& lv, const HashedLaneInfo& rv) TX_NOEXCEPT {
      return lv.m_laneinfo == rv.m_laneinfo && (lv.m_subSectionIdx == rv.m_subSectionIdx);
    }

    size_t operator()(const HashedLaneInfo& interval) const TX_NOEXCEPT { return HashCompare::hash(interval); }
  };
};

extern Base::txBool GetPrevHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                          HashedLaneInfo& retHashedLaneInfo) TX_NOEXCEPT;
extern Base::txBool GetPostHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                          HashedLaneInfo& retHashedLaneInfo) TX_NOEXCEPT;

extern Base::txBool GetFrontHashedLaneInfoList(const HashedLaneInfo& srcHashedLaneInfo,
                                               std::list<HashedLaneInfoPtr>& retHashedLaneInfoList,
                                               const Base::txInt nStep = 3 /* 3*16=48m */) TX_NOEXCEPT;
extern Base::txBool GetBackHashedLaneInfoList(const HashedLaneInfo& srcHashedLaneInfo,
                                              std::list<HashedLaneInfoPtr>& retHashedLaneInfoList,
                                              const Base::txInt nStep = 3 /* 3*16=48m */) TX_NOEXCEPT;
extern Base::txBool GetLeftHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                          HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT;
extern Base::txBool GetRightHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo,
                                           HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT;
extern Base::txBool MoveFrontHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo, const Base::txFloat distance,
                                            HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT;
extern Base::txBool MoveBackHashedLaneInfo(const HashedLaneInfo& srcHashedLaneInfo, const Base::txFloat distance,
                                           HashedLaneInfoPtr& retHashedLaneInfoPtr) TX_NOEXCEPT;
extern Base::txBool CheckHashedValid(const Base::txSize _subSectionIdx, Base::txFloat _curveLength) TX_NOEXCEPT;
extern HashedLaneInfo GenerateHashedLaneInfo(const HdMap::txLaneInfoInterfacePtr cur_geom_ptr,
                                             const Base::txFloat distOnCurce,
                                             const Base::Info_Lane_t& laneLocInfo) TX_NOEXCEPT;

extern HashedLaneInfo GenerateHashedLaneInfo(const Base::Info_Lane_t& laneLocInfo,
                                             const HdMap::txLaneInfoInterfacePtr cur_geom_ptr,
                                             const Base::txSize _subSectionIdx, Base::txFloat _curveLength) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)

#endif /*USE_HashedRoadNetwork*/

// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <array>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <functional>
#include "hadmap.h"
#include "mapengine/engine_defs.h" /*hadmap::MAP_DATA_TYPE*/
#include "mapengine/hadmap_engine.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_unordered_set.h"
#include "tbb/parallel_for.h"
#include "tx_hash_utils.h"
#include "tx_header.h"
#include "tx_lane_geom_info.h"
#include "tx_locate_info.h"
#include "tx_map_info.h"
#include "tx_sim_point.h"
#include "tx_units.h"
TX_NAMESPACE_OPEN(HdMap)

class HadmapCacheConCurrent {
  // public:
  //     static HadmapCacheConCurrent& getInstance()
  //     {
  //         static HadmapCacheConCurrent instance;
  //         return instance;
  //     }

 private:
  HadmapCacheConCurrent() TX_DEFAULT;
  ~HadmapCacheConCurrent() TX_DEFAULT;
  HadmapCacheConCurrent(const HadmapCacheConCurrent&) TX_DELETE;
  HadmapCacheConCurrent& operator=(const HadmapCacheConCurrent&) TX_DELETE;

 public:
  using txVec3 = Base::txVec3;
  using txULong = Base::txULong;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txLpsz = Base::txLpsz;
  using txLaneID = Base::txLaneID;
  using txSectionID = Base::txSectionID;
  using txRoadID = Base::txRoadID;
  using txLaneUId = Base::txLaneUId;
  using txLaneLinkID = Base::txLaneLinkID;
  using txSectionUId = Base::txSectionUId;
  using txBoundaryID = Base::txBoundaryID;
  using txString = Base::txString;
  using size_t = std::size_t;
  using txWGS84 = Coord::txWGS84;
  using txENU = Coord::txENU;

  struct InitParams_t {
    Base::txString strHdMapFilePath;
    hadmap::txPoint SceneOriginGPS;
    Base::txString strTrafficFilePath; /*simData.traffic.c_str(),*/
    Base::txString strHadmapFilter;    /*black list*/
    boost::optional<Base::map_range_t> op_map_range;
  };

  using LaneUIdHashCompare = Utils::LaneUIdHashCompare;
  using RoadIdHashCompare = Utils::RoadIdHashCompare;
  using LandLinkIDHashCompare = Utils::LandLinkIDHashCompare;
  using FromToUidHashCompare = Utils::FromToUidHashCompare;
  using SectionUIdHashCompare = Utils::SectionUIdHashCompare;
  using LaneBoundaryIDHashCompare = Utils::LaneBoundaryIDHashCompare;
  using Info_Lane_t_HashCompare = Utils::Info_Lane_t_HashCompare;
  using Info_Road_t_HashCompare = Utils::Info_Road_t_HashCompare;

  struct LandLinkIDSetHasher {
    size_t operator()(const txLaneLinkID& _link_id) const { return std::hash<txLaneLinkID>{}(_link_id); }
  };

  struct LandLinkIDSetEqual {
    bool operator()(const txLaneLinkID& lv, const txLaneLinkID& rv) const { return lv == rv; }
  };

  using RoadPair TX_MARK("GetLaneLinkByFromToRoadId") = std::pair<txRoadID, txRoadID>;

  struct RoadPairHashCompare {
    /**
     * @brief hash 对 RoadPair 结构体进行哈希操作，用于在 ConcurrentMap 中存储交叉感知信息
     * @param _roadId_pair RoadPair 结构体，包含两个交叉点编号
     * @return 返回经过哈希后的结果
     */
    static size_t hash(const RoadPair& _roadId_pair) {
      size_t seed = 0;
      boost::hash_combine(seed, _roadId_pair.first);
      boost::hash_combine(seed, _roadId_pair.second);
      return seed;
    }

    /**
     * @brief equal 比较两个 RoadPair 结构体是否相等
     * @param lv 第一个 RoadPair 结构体
     * @param rv 第二个 RoadPair 结构体
     * @return 如果 lv 和 rv 结构体的 first 和 second 成员相等，则返回 true；否则返回 false
     */
    static bool equal(const RoadPair& lv, const RoadPair& rv) { return lv.first == rv.first && lv.second == rv.second; }
  };

  using controlPoint = std::array<txFloat, 2>;
  enum class CurveType : txInt { cubic_spline, catmull_rom, short_segment };
  enum { LaneResampleCount = 100 };

  struct LaneUtilityInfo {
    txBool bFinalLane = false;
    hadmap::ROAD_TYPE roadType = hadmap::ROAD_TYPE_Normal;
    Base::Info_Lane_t laneInfo;

    /**
     * @brief GetLaneIndex 获取当前车辆所在车道的索引
     * @return 返回当前车辆所在车道的索引
     */
    Base::txLaneID GetLaneIndex() const TX_NOEXCEPT;
  };

  using RoadStartEndInfo = std::array<txWGS84, 2>;

 public:
  /**
   * @brief 初始化
   *
   * 初始化一个txMapHandle的实例，以便后续使用
   * @param[in] hdr 一个指向 txMapHandle 类型实例的指针
   * @return txBool，初始化成功返回 true，否则返回 false
   */
  static txBool Initialize(hadmap::txMapHandle* hdr) TX_NOEXCEPT;

  /**
   * @brief 初始化并发缓存
   *
   * 用于初始化一个并发缓存实例，以便存储传入参数的相关信息，用于后续使用。
   *
   * @param[in] refParam 初始化参数的结构体引用
   * @return txBool 初始化成功返回 true，否则返回 false
   */
  static txBool Initialize(const InitParams_t& refParam) TX_NOEXCEPT;

  /**
   * @brief MapType 获取指定路径的地图类型
   *
   * 该函数根据输入的地图路径，返回地图的类型。
   *
   * @param mapPath 输入的地图路径，可以是一个文件路径或目录路径
   * @return hadmap::MAP_DATA_TYPE 返回地图的类型
   */
  static hadmap::MAP_DATA_TYPE MapType(const Base::txString mapPath) TX_NOEXCEPT;

  /**
   * @brief PrintMapFile 打印指定路径的地图文件内容
   *
   * 该函数读取并打印指定路径的地图文件的内容。
   *
   * @param mapPath 输入地图文件的路径
   */
  static void PrintMapFile(const Base::txString mapPath) TX_NOEXCEPT;

  /**
   * @brief 获取地图缓存的边界矩阵
   *
   * 获取地图缓存的边界矩阵，该矩阵表示地图数据的范围。
   * 返回的矩阵为一个包含两个点的向量，分别表示左下角和右上角的坐标。
   *
   * @return hadmap::PointVec 地图缓存的边界矩阵
   */
  static hadmap::PointVec GetEnvelope() TX_NOEXCEPT { return s_envelope; }

  /**
   * @brief 释放缓存
   *
   * 释放缓存所占用的内存和其他资源，如果没有被其他操作使用则返回true，否则返回false。
   *
   * @return txBool 缓存是否成功释放
   */
  static txBool Release() TX_NOEXCEPT;

  /**
   * @brief 显示缓存信息
   *
   * 显示缓存的信息，包括缓存的读取次数、写入次数、所有缓存项的数量等。
   */
  static void ShowCacheInfo() TX_NOEXCEPT;

  /**
   * @brief 启用调试输出
   *
   * 设置是否启用调试输出，如果启用调试输出，则会在控制台输出指定文件的调试信息。
   *
   * @param _strFile 需要启用调试输出的文件名
   * @return txBool 如果调试输出被成功启用，则返回 true，否则返回 false
   */
  static txBool DebugOutput(const Base::txString _strFile) TX_NOEXCEPT;

  /**
   * @brief 释放道路图缓存
   *
   * 该函数用于释放道路图缓存所占用的内存空间，以节省内存和提高效率。
   */
  static void ReleaseRoadGraphCache() TX_NOEXCEPT;

  /**
   * @brief 获取地图处理器
   *
   * 该函数用于获取地图处理器的指针，通常在地图处理需要进行的任务前使用。
   *
   * @return hadmap::txMapHandle* 返回地图处理器的指针
   */
  static hadmap::txMapHandle* GetMapHandler() TX_NOEXCEPT { return pMapHandle; }

  /**
   * @brief 获取地图处理器是否有效
   *
   * 如果地图处理器有效，则返回 true；否则返回 false。通常在调用地图处理器的功能前使用此函数进行判断。
   *
   * @return txBool 地图处理器是否有效
   */
  static txBool IsValid() TX_NOEXCEPT { return sValid; }

 public:
  /**
   * @brief 获取指定道路的指针
   *
   * 这个函数返回一个指向指定道路ID的道路指针。通常在需要调用其他对道路对象的操作之前，使用这个函数进行判断。
   *
   * @param _roadId 道路ID
   * @return hadmap::txRoadPtr 指向道路的指针
   */
  static hadmap::txRoadPtr GetTxRoadPtr(const txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路的长度
   *
   * 获取给定道路ID的道路长度。
   *
   * @param _roadId 道路ID
   * @return txFloat 道路长度
   */
  static txFloat GetRoadLength(const txRoadID _roadId) TX_NOEXCEPT;
  /*static hadmap::txLaneLinkPtr GetTxLaneLinkPtr(const txLaneLinkID _lanelinkId) TX_NOEXCEPT;*/

  /**
   * @brief 初始化两个通道之间的连接指针
   *
   * 这个函数用于初始化指定从通道和到通道之间的连接指针，并返回这个指针。
   *
   * @param _fromLaneUid 从通道的ID
   * @param _toLaneUid 到通道的ID
   * @param linkPtr 连接指针
   * @return hadmap::txLaneLinkPtr 初始化后的连接指针
   */
  static hadmap::txLaneLinkPtr InitTxLaneLinkPtr(const txLaneUId& _fromLaneUid, const txLaneUId& _toLaneUid,
                                                 hadmap::txLaneLinkPtr linkPtr) TX_NOEXCEPT;

  /**
   * @brief 获取指定两个通道之间的连接指针
   *
   * 此函数用于获取指定两个通道之间的连接指针。若连接已存在，则返回指向该连接的指针，否则将创建一个新的连接并返回指向该连接的指针。
   *
   * @param _fromLaneUid 起始通道的ID
   * @param _toLaneUid 目标通道的ID
   * @return hadmap::txLaneLinkPtr 指向连接的指针
   */
  static hadmap::txLaneLinkPtr GetTxLaneLinkPtr(const txLaneUId& _fromLaneUid, const txLaneUId& _toLaneUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定通道的指针
   *
   * 通过指定通道的ID，获取对应的txLanePtr。如果对应的通道不存在，则返回空指针。
   *
   * @param _laneUid 通道ID
   * @return hadmap::txLanePtr 对应通道的指针，如果不存在则返回空指针
   */
  static hadmap::txLanePtr GetTxLanePtr(const txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定section的指针
   *
   * 通过指定section的ID，获取对应的section指针。如果对应的section不存在，则返回空指针。
   *
   * @param _sectionUid 区块ID
   * @return hadmap::txSectionPtr 对应区块的指针，如果不存在则返回空指针
   */
  static hadmap::txSectionPtr GetTxSectionPtr(const Base::txSectionUId& _sectionUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定边界ID的边界指针
   * @param _boundaryId 边界ID
   * @return 返回指定边界ID的边界指针
   */
  static hadmap::txLaneBoundaryPtr GetTxBoundaryPtr(const txBoundaryID _boundaryId) TX_NOEXCEPT;

  /**
   * @brief 获取指定section下的所有车道列表
   *
   * @param _sectionUid 指定区域的ID
   * @return 返回一个包含所有车道信息的列表
   */
  static const hadmap::txLanes GetLanesUnderSection(const txSectionUId& _sectionUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定section下的车道数量
   *
   * 此函数用于获取指定区域下的车道数量。
   *
   * @param _sectionUid 指定区域的ID
   * @return 返回指定区域下的车道数量
   */
  static const Base::txSize GetLaneCountOnSection(const txSectionUId& _sectionUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路下的section数量
   *
   * @param _roadId 指定的roadid
   * @return const Base::txSize 返回的section数量
   */
  static const Base::txSize GetSectionCountOnRoad(const txRoadID& _roadId) TX_NOEXCEPT;

  /**
   * @brief 并行创建车道缓存
   *
   * 此函数用于并行创建指定车道列表的缓存。
   *
   * @param lanePtrVec 车道指针列表
   * @param lanelinkPtrVec 车道链接指针列表
   * @param roadPtrVec 道路指针列表
   */
  static void ParallelCreateLaneCache(hadmap::txLanes& lanePtrVec, hadmap::txLaneLinks& lanelinkPtrVec,
                                      hadmap::txRoads& roadPtrVec) TX_NOEXCEPT;

  /**
   * @brief 并行创建车道的前后图
   *
   * 此函数用于并行创建指定车道列表的前后图。
   *
   * @param lanePtrVec 车道指针列表
   * @param lanelinkPtrVec 车道链接指针列表
   * @param roadPtrVec 道路指针列表
   */
  static void ParallelCreateLanePrevPostGraph(const hadmap::txLanes& lanePtrVec,
                                              const hadmap::txLaneLinks& lanelinkPtrVec,
                                              const hadmap::txRoads& roadPtrVec) TX_NOEXCEPT;

  /**
   * @brief 对给定的车道、道路列表进行并发检查，保留唯一的车道和道路
   *
   * 此函数用于处理多个线程同时检查车道和道路的场景，确保输入列表中的车道和道路都是唯一的。
   *
   * @param lanePtrVec 要检查的车道指针列表
   * @param lanelinkPtrVec 要检查的车道链接指针列表
   * @param roadPtrVec 要检查的道路指针列表
   * @param uniqueLanePtrVec 输出参数，存储检查后唯一的车道指针列表
   * @param uniqueLanelinkPtrVec 输出参数，存储检查后唯一的车道链接指针列表
   */
  static void MapCheck(const hadmap::txLanes& lanePtrVec, const hadmap::txLaneLinks& lanelinkPtrVec,
                       const hadmap::txRoads& roadPtrVec, hadmap::txLanes& uniqueLanePtrVec,
                       hadmap::txLaneLinks& uniqueLanelinkPtrVec) TX_NOEXCEPT;

  /**
   * @brief 并行处理指定车道列表的AABB空间划分
   *
   * 此函数接收车道指针列表，并采用多线程并行处理。其目的是将每个指定的车道划分为独立的空间，并在给定的 roadPtrVec
   * 中存储相关信息。
   *
   * @param roadPtrVec 包含车道指针的列表，将会被修改以维护所有已分配空间的信息
   */
  static void ParallelMapAABB(hadmap::txRoads& roadPtrVec) TX_NOEXCEPT;

  /**
   * @brief 在并发操作时注册车道信息的初始化函数
   *
   * 此函数在并发操作时，通过传入的车道指针注册车道信息，同时设置该车道的初始化状态。
   *
   * @param _lanePtr 指向车道的指针
   * @return txBool 注册成功返回 true，否则返回 false
   */
  static txBool RegisterLaneSyncInit(const hadmap::txLanePtr _lanePtr) TX_NOEXCEPT;

  /**
   * @brief 注册并发初始化车道链接信息
   *
   * 该函数用于在并发情况下，注册并初始化车道链接信息。
   *
   * @param _lanelinkPtr 指向车道链接的指针
   * @return txBool 注册成功返回 true，否则返回 false
   */
  static txBool RegisterLaneLinkSyncInit(const hadmap::txLaneLinkPtr _lanelinkPtr) TX_NOEXCEPT;

  /**
   * @brief 在并发环境下注册和初始化道路段信息
   *
   * 该函数用于在并发环境下注册和初始化道路段信息，并返回注册是否成功。
   *
   * @param _roadPtr 指向道路段的指针
   * @return txBool 注册成功返回 true，否则返回 false
   */
  static txBool RegisterRoadSectionSyncInit(const hadmap::txRoadPtr _roadPtr) TX_NOEXCEPT;

  /**
   * @brief 初始化和注册道路链接信息，包括前置道路、后置道路
   *
   * 该函数用于在并发环境下初始化和注册道路链接信息，该信息包括前置道路和后置道路。
   *
   * @param _lanelinkPtr 指向道路链接的指针
   * @return txBool 注册成功返回 true，否则返回 false
   */
  static txBool RegisterLaneLinkPrePostTopoInfoInit(const hadmap::txLaneLinkPtr _lanelinkPtr) TX_NOEXCEPT;

  /**
   * @brief 在并发环境下注册道路节点信息，包括前置道路和后置道路
   *
   * 该函数用于在并发环境下注册道路节点信息，包括前置道路和后置道路。
   * 该函数初始化相关道路信息，并在调用完成后返回注册结果。
   *
   * @param[in] _lanePtr 指向道路节点的指针
   * @return txBool 注册成功返回 true，否则返回 false
   */
  static txBool RegisterLanePrePostTopoInfoInit(const hadmap::txLanePtr _lanePtr) TX_NOEXCEPT;

  /**
   * @brief 设置指定道路为死胡同
   * @param lanePtr 指向待设置为死胡同的道路指针
   * @return true 设置成功
   * @return false 设置失败
   */
  static txBool Make_dead_end_laneSet_by_laneUid(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 将指定道路注册到缓存容器，并执行实时运行时注册
   * @param _lanePtr 指向待注册道路的指针
   * @return 返回注册结果对象
   * @retval true 注册成功
   * @retval false 注册失败
   */
  static txLaneInfoInterfacePtr RegisterLaneInfoSyncRunTime(const hadmap::txLanePtr _lanePtr) TX_NOEXCEPT;

  /**
   * @brief 注册指定道路链接信息并实时运行时注册
   * @param _lanelinkPtr 指向待注册道路链接的指针
   * @return 返回注册结果对象
   * @retval true 注册成功
   * @retval false 注册失败
   */
  static txLaneInfoInterfacePtr RegisterLaneLinkInfoSyncRunTime(const hadmap::txLaneLinkPtr _lanelinkPtr) TX_NOEXCEPT;

  /**
   * @brief 在运行时注册指定道路信息
   *
   * 该函数在运行时将指定道路信息注册到缓存容器，确保在注册之后可以被其他地图功能查询到
   *
   * @param _roadPtr 指向待注册道路的指针
   */
  static void RegisterRoadInfoSyncRunTime(const hadmap::txRoadPtr _roadPtr) TX_NOEXCEPT;

  /**
   * @brief 同步注册道路信息
   *
   * 该函数将指定的道路信息同步地注册到缓存容器中，确保它能够被其他地图功能查询到。
   *
   * @param _lanelinkPtr 指向待注册道路链接的指针
   */
  static void RegisterRoadInfoSyncRunTime(const hadmap::txLaneLinkPtr _lanelinkPtr) TX_NOEXCEPT;

  /**
   * @brief 同步注册道路信息
   *
   * 该函数将指定的道路信息同步地注册到缓存容器中，确保它能够被其他地图功能查询到。
   *
   * @param _lane_loc_info 一个包含道路信息的对象
   * @return 返回一个指向 txLaneInfoInterface 的指针
   * @see txLaneInfoInterface
   */
  static txLaneInfoInterfacePtr RegisterGeomtryInfoSyncRunTime(const Base::Info_Lane_t& _lane_loc_info) TX_NOEXCEPT;

  /**
   * @brief SubdivisionCurves 用于将给定的线段曲线分成若干个控制点
   *
   * @param[in] lineCurve 待分割的线段曲线指针
   * @param[out] controlPoints 存放分割后的控制点
   * @param[in] curveType 线段曲线的类型
   * @return txBool 分割成功则返回 true，否则返回 false
   *
   * @see txLineCurve
   * @see controlPoint
   * @see CurveType
   */
  static txBool SubdivisionCurves(const hadmap::txLineCurve*, std::vector<controlPoint>&, const CurveType) TX_NOEXCEPT;

  /**
   * @brief 通过道路运行时更新车道数据
   *
   * 更新给定道路运行时的车道数据，如果更新成功则返回 true，否则返回 false。
   *
   * @param[in] _lanePtr 待更新的车道指针
   * @return txBool 更新成功则返回 true，否则返回 false
   */
  static txBool UpdateLaneDataByRoadRunTime(const hadmap::txLanePtr _lanePtr) TX_NOEXCEPT;

  /**
   * @brief 通过道路运行时更新车道链接数据
   *
   * 更新给定道路运行时的车道链接数据，如果更新成功则返回 true，否则返回 false。
   *
   * @param[in] _lanelinkPtr 待更新的车道链接指针
   * @return txBool 更新成功则返回 true，否则返回 false
   */
  static txBool UpdateLaneLinksDataByRoadRunTime(const hadmap::txLaneLinkPtr _lanelinkPtr) TX_NOEXCEPT;

  /**
   * @brief 从给定车道的下一个车道中得到一个新的车道集合
   *
   * 从给定车道的下一个车道中得到一个新的车道集合。该车道集合包含指定车道的 nSectionCnt 个车道，并将其存储在一个 set
   * 中。
   *
   * @param[in] lanePtr  指定车道的指针
   * @param[in] nSectionCnt 车道集合的大小
   * @return std::set<Base::txLaneUId> 新的车道集合，包含指定车道的下一个 nSectionCnt 个车道的 UID
   */
  static std::set<Base::txLaneUId> Make_Lane_Next_Laneset(const hadmap::txLanePtr lanePtr,
                                                          const Base::txSize nSectionCnt) TX_NOEXCEPT;

  /**
   * @brief 生成一个包含给定车道信息对应的前置车道的集合
   *
   * 该函数根据给定的车道信息，找到其前置车道，并将其车道ID集合存储在一个集合中，然后返回这个集合。
   *
   * @param lanePtr 给定的车道信息
   * @return 包含给定车道信息对应的前置车道的集合
   */
  static std::set<Base::txLaneUId> Make_Lane_Pre_Laneset(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 根据给定的路段信息和距离(_s)，获取相对于该路段起点的方向向量
   *
   * 根据输入的路段信息和距离(_s)，计算出起点与路段方向之间的夹角，并将该方向向量以一个向量返回。
   *
   * @param[in] _lane_loc_info 路段信息
   * @param[in] _s 距离路段起点的距离，单位为米
   * @return txVec3 相对于路段起点的方向向量，单位为米
   */
  static txVec3 GetDir(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 获取给定路段信息和距离后，相对于该路段起点的WGS84坐标
   *
   * 根据输入的路段信息和距离后，计算出相对于该路段起点的WGS84坐标，并返回其坐标信息。
   *
   * @param[in] _lane_loc_info 路段信息
   * @param[in] _s 从路段起点到目标点的距离，单位为米
   * @return txWGS84 相对于路段起点的WGS84坐标
   */
  static txWGS84 GetPos(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道信息和距离，获取相对于车道起点的高程值
   *
   * 该函数根据给定的车道信息和距离，计算并返回相对于车道起点的高程值。
   *
   * @param[in] _lane_loc_info 车道信息
   * @param[in] _s 从车道起点到目标点的距离，单位为米
   * @return txFloat 相对于车道起点的高程值
   */
  static Base::txFloat GetHdMapAlt(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道信息和距离，计算车道的方向向量
   *
   * 该函数根据给定的车道信息和距离，计算车道的方向向量。
   *
   * @param _lane_loc_info 给定的车道信息
   * @param _s 距离，表示从车道起点到目标点的距离
   * @return txVec3 车道的方向向量
   */
  static txVec3 GetLaneDir(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s) TX_NOEXCEPT;

   /**
   * @brief 根据给定的车道信息和距离，计算车道的方向向量
   *
   * 该函数根据给定的车道信息和距离，计算车道的方向向量。
   *
   * @param _lane_loc_info 给定的车道信息
   * @param _s 距离，表示从车道起点到目标点的距离
   * @return txVec3 车道的方向向量
   */
  static txVec3 GetLaneDir(const Base::txLaneUId& _lane_uid, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 获取给定车道的方向向量
   *
   * 该函数根据给定的车道信息和距离，计算车道的方向向量。
   *
   * @param lanePtr 给定的车道信息
   * @param _s 距离，表示从车道起点到目标点的距离
   * @return txVec3 车道的方向向量
   */
  static txVec3 GetLaneDir(const hadmap::txLanePtr lanePtr, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的长度
   *
   * 计算并返回指定车道的长度，单位为米。
   *
   * @param _laneUid 指定车道的唯一ID
   * @return 车道长度，单位为米
   */
  static txFloat GetLaneLength(const txLaneUId& _laneUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的左右边界
   *
   * 根据指定的车道指针，返回其左右边界。
   *
   * @param lanePtr 指定的车道指针
   * @return std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> 车道的左右边界
   */
  static std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> GetLaneMark_LR(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的左右边界
   *
   * 根据指定的车道ID，返回其左右边界。
   *
   * @param _laneUid 指定的车道ID
   * @return std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> 车道的左右边界
   */
  static std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> GetLaneMark_LR(const txLaneUId& _laneUid) TX_NOEXCEPT;

  /**
   * @brief 根据给定的道路位置信息和距离，计算道路在WGS84坐标系下的位置。
   *
   * 该函数根据道路在局部坐标系下的位置信息和给定的距离，计算出道路在WGS84坐标系下的位置。
   *
   * @param _lane_loc_info 道路在局部坐标系下的位置信息。
   * @param _s 距离，单位为米。
   * @return txWGS84 道路在WGS84坐标系下的位置。
   */
  static txWGS84 GetLanePos(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 获取给定道路位置信息和距离下的WGS84坐标
   *
   * 通过提供的道路在局部坐标系下的位置信息和距离，此函数会计算出该道路在WGS84坐标系下的位置。
   *
   * @param _lane_loc_info 道路在局部坐标系下的位置信息。
   * @param _s 距离，单位为米。
   * @param resPos 输出参数，储存计算得到的道路在WGS84坐标系下的位置。
   * @return 如果计算成功，则返回true；否则返回false。
   */
  static txBool GetLanePos(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s, txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 根据给定的道路位置信息和距离，计算道路在WGS84坐标系下的位置。
   *
   * 该函数根据道路在局部坐标系下的位置信息和给定的距离，计算出道路在WGS84坐标系下的位置。
   *
   * @param _laneUid 道路在局部坐标系下的位置信息。
   * @param _s 距离，单位为米。
   * @return txWGS84 道路在WGS84坐标系下的位置。
   */
  static txWGS84 GetLanePos(const Base::txLaneUId& _laneUid, const txFloat _s) TX_NOEXCEPT;

    /**
   * @brief 获取给定道路位置信息和距离下的WGS84坐标
   *
   * 通过提供的道路在局部坐标系下的位置信息和距离，此函数会计算出该道路在WGS84坐标系下的位置。
   *
   * @param _laneUid 道路在局部坐标系下的位置信息。
   * @param _s 距离，单位为米。
   * @param resPos 输出参数，储存计算得到的道路在WGS84坐标系下的位置。
   * @return 如果计算成功，则返回true；否则返回false。
   */
  static txBool GetLanePos(const Base::txLaneUId& _laneUid, const txFloat _s, txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道指针和距离，计算车道在WGS84坐标系下的位置。
   *
   * 该函数通过输入的车道指针和距离，计算出车道在WGS84坐标系下的位置。
   *
   * @param lanePtr 车道指针。
   * @param _s 给定的距离，单位为米。
   * @return 返回车道在WGS84坐标系下的位置。
   */
  static txWGS84 GetLanePos(const hadmap::txLanePtr lanePtr, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 获取给定车道的WGS84坐标
   *
   * 通过输入的车道指针和距离，计算出车道在WGS84坐标系下的位置。
   *
   * @param lanePtr 车道指针
   * @param _s 给定的距离，单位为米
   * @param resPos 返回车道在WGS84坐标系下的位置
   * @return 如果计算成功，返回true，否则返回false
   */
  static txBool GetLanePos(const hadmap::txLanePtr lanePtr, const txFloat _s, txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道链接和距离，计算车道链接在WGS84坐标系下的位置
   *
   * 该函数根据输入的车道链接指针和距离，计算出车道链接在WGS84坐标系下的位置。
   *
   * @param _lanelinkPtr 车道链接指针
   * @param _s 给定的距离，单位为米
   * @return 返回车道链接在WGS84坐标系下的位置
   */
  static txWGS84 GetLaneLinkPos(const hadmap::txLaneLinkPtr& _lanelinkPtr, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 获取给定车道链接在WGS84坐标系下的位置
   *
   * 该函数根据输入的车道链接信息和距离，计算出车道链接在WGS84坐标系下的位置。
   *
   * @param _lane_loc_info 车道链接的地理位置信息
   * @param _s 给定的距离，单位为米
   * @return 返回车道链接在WGS84坐标系下的位置
   */
  static txWGS84 GetLaneLinkPos(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道链接信息和距离，计算车道链接在WGS84坐标系下的位置
   *
   * 该函数根据输入的车道链接指针和距离，计算出车道链接在WGS84坐标系下的位置。
   *
   * @param _lanelinkPtr 车道链接的地理位置信息
   * @param _s 给定的距离，单位为米
   * @param resPos 返回车道链接在WGS84坐标系下的位置
   * @return 如果计算成功，返回true；否则返回false
   */
  static txBool GetLaneLinkPos(const hadmap::txLaneLinkPtr& _lanelinkPtr, const txFloat _s,
                               txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 获取给定车道链接在WGS84坐标系下的位置
   *
   * 该函数根据输入的车道链接信息和距离，计算出车道链接在WGS84坐标系下的位置。
   *
   * @param _lane_loc_info 车道链接的地理位置信息
   * @param _s 给定的距离，单位为米
   * @param resPos 返回车道链接在WGS84坐标系下的位置
   * @return 如果计算成功，返回true；否则返回false
   */
  static txBool GetLaneLinkPos(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s, txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 通过给定的ST坐标获取经纬度坐标
   *
   * 该函数通过给定的道路ID和ST坐标，获取道路在WGS84坐标系下的经纬度坐标。
   *
   * @param _laneUid 输入的道路ID
   * @param _s 输入的道路沿着S轴的距离
   * @param _t 输入的道路沿着T轴的距离
   * @param resPos 输出的经纬度坐标
   * @return 如果获取成功，返回true；否则返回false
   */
  static txBool GetLonLatFromSTonLaneWithOffset(const Base::txLaneUId _laneUid, const txFloat _s, const txFloat _t,
                                                txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 根据给定的道路信息和S轴坐标，获取道路连接的方向向量
   *
   * 该函数根据给定的道路ID和S轴距离，计算出在WGS84坐标系下的经纬度坐标。
   *
   * @param _lane_loc_info 输入的道路信息
   * @param _s 输入的道路沿着S轴的距离
   * @return 返回一个包含方向向量的txVec3对象
   */
  static txVec3 GetLaneLinkDir(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 根据给定的道路连接和S轴距离，计算出方向向量
   *
   * 该函数根据给定的道路连接ID和S轴距离，计算出方向向量。
   * 方向向量是一个三维向量，表示沿着S轴方向从给定道路连接的起点到终点的方向。
   *
   * @param _linkPtr 输入的道路连接指针
   * @param _s 输入的道路沿着S轴的距离
   * @return 返回一个三维向量，表示方向向量
   */
  static txVec3 GetLaneLinkDir(const hadmap::txLaneLinkPtr _linkPtr, const txFloat _s) TX_NOEXCEPT;

  /**
   * @brief 获取给定道路连接的长度
   *
   * 计算给定道路连接的长度，单位为米。
   *
   * @param _link_locInfo 给定道路连接的位置信息
   * @return 返回道路连接的长度，单位为米
   */
  static txFloat GetLaneLinkLength(const Base::Info_Lane_t& _link_locInfo) TX_NOEXCEPT;
  /*static txLaneInfoInterface::LaneShape GetLaneLinkShape(const Base::Info_Lane_t& _lane_loc_info) TX_NOEXCEPT;*/

  /**
   * @brief 判断给定的车道是否包含停止线
   *
   * 该函数用于判断指定车道是否包含停止线。
   *
   * @param _laneUid 车道的ID
   * @return 如果车道包含停止线，返回true；否则返回false
   */
  static txBool IsLaneHasStopLine(const txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 通过给定的 S 和 T 值，获取与指定车道相关的位置信息
   *
   * 该函数通过输入的车道 ID 和给定的 S 和 T 值，获取与指定车道相关的位置信息。
   *
   * @param[in] _lane_loc_info 车道的位置信息
   * @param[in] _s 参数 S
   * @param[in] _t 参数 T
   * @param[out] resPos 结果位置信息
   * @return 如果车道包含停止线，返回true；否则返回false
   */
  static txBool Get_LAL_Lane_By_ST(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s, const txFloat _t,
                                   txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 通过给定的 S 值，获取与指定车道相关的位置信息
   *
   * 该函数通过输入的车道 ID 和给定的 S 值，获取与指定车道相关的位置信息。
   *
   * @param[in] _lane_loc_info 车道的位置信息
   * @param[in] _s 参数 S
   * @param[out] resPos 结果位置信息
   * @return 如果车道包含停止线，返回true；否则返回false
   */
  static txBool Get_LAL_Lane_By_S(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s,
                                  txWGS84& resPos) TX_NOEXCEPT;

   /**
   * @brief 通过给定的 S 值，获取与指定车道相关的位置信息
   *
   * 该函数通过输入的车道 ID 和给定的 S 值，获取与指定车道相关的位置信息。
   *
   * @param[in] _lane_uid 车道的位置信息
   * @param[in] _s 参数 S
   * @param[out] resPos 结果位置信息
   * @return 如果车道包含停止线，返回true；否则返回false
   */
  static txBool Get_LAL_Lane_By_S(const Base::txLaneUId& _lane_uid, const txFloat _s, txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 通过给定的S和T值，获取与指定车道相关的位置信息
   *
   * 该函数通过输入的车道ID和给定的S和T值，获取与指定车道相关的位置信息。
   *
   * @param[in] _lane_loc_info 车道的位置信息
   * @param[in] _s 参数S
   * @param[in] _t 参数T
   * @param[out] resPos 结果位置信息
   * @return 如果车道包含停止线，返回true；否则返回false
   */
  static txBool Get_LAL_LaneLink_By_ST(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s, const txFloat _t,
                                       txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 通过给定的 S 值，获取与指定车道相关的位置信息
   *
   * 该函数通过输入的车道 ID 和给定的 S 值，获取与指定车道相关的位置信息。
   *
   * @param[in] _lane_loc_info 车道的位置信息
   * @param[in] _s 参数 S
   * @param[out] resPos 结果位置信息
   * @return 如果车道包含停止线，返回 true；否则返回 false
   */
  static txBool Get_LAL_LaneLink_By_S(const Base::Info_Lane_t& _lane_loc_info, const txFloat _s,
                                      txWGS84& resPos) TX_NOEXCEPT;

  /**
   * @brief 通过给定的起始道路 ID 和目标道路 ID 获取连接这两条道路的所有车道
   *
   * 该函数根据输入的起始道路 ID 和目标道路 ID，获取连接这两条道路的所有车道。
   *
   * @param[in] _fromRoadId 起始道路 ID
   * @param[in] _toRoadId 目标道路 ID
   * @return Base::LocInfoSet 包含连接这两条道路的所有车道的位置信息集合
   */
  static Base::LocInfoSet GetLaneLinkByFromToRoadId(const Base::txRoadID _fromRoadId,
                                                    const Base::txRoadID _toRoadId) TX_NOEXCEPT;

  /**
   * @brief 通过给定的起始道路 ID 和目标道路 ID 获取连接这两条道路的所有车道
   *
   * 该函数根据输入的起始道路 ID 和目标道路 ID，获取连接这两条道路的所有车道。
   *
   * @param[in] _fromRoadId 起始道路 ID
   * @param[in] _toRoadId 目标道路 ID
   * @return 包含连接这两条道路的所有车道的位置信息集合
   */
  static Base::LocInfoSet GetLaneLinkByFromToRoadId4Route(const Base::txRoadID _fromRoadId,
                                                          const Base::txRoadID _toRoadId) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路ID的起点坐标 (WGS84)
   *
   * 此函数用于获取给定道路ID的起点坐标。输入参数为道路ID，返回值为 WGS84 坐标系的起点坐标。
   *
   * @param[in] _roadId 道路ID
   * @return txWGS84 起点坐标
   */
  static txWGS84 GetRoadStart(const Base::txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路ID的终点坐标（WGS84）
   *
   * 此函数用于获取给定道路ID的终点坐标。输入参数为道路ID，返回值为 WGS84 坐标系的终点坐标。
   *
   * @param[in] _roadId 道路ID
   * @return txWGS84 终点坐标
   */
  static txWGS84 GetRoadEnd(const Base::txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路ID的起点和终点信息
   *
   * 该函数用于获取给定道路ID的起点和终点坐标信息，包括起点经纬度和终点经纬度。
   *
   * @param[in] _roadId 道路ID
   * @return RoadStartEndInfo 结构体，包含起点和终点坐标信息
   */
  static RoadStartEndInfo GetRoadStartEnd(const txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 根据给定的道路ID获取道路的起终点坐标信息
   *
   * 此函数接收一个道路ID作为输入参数，返回包含起点和终点经纬度信息的结构体。
   *
   * @param[in] _roadId 道路ID
   * @return txWGS84 结构体，包含道路的起终点经纬度信息
   */
  static txWGS84 GetRoad2Junction(const txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的下一个车道ID集合
   *
   * 该函数接收一个车道指针作为输入参数，返回包含下一个车道ID的集合，不会为空
   *
   * @param[in] lanePtr 输入车道指针
   * @return std::set<Base::txLaneUId> 包含下一个车道ID的集合
   */
  static std::set<Base::txLaneUId> GetNextLaneSetByLaneUid(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的下一个车道集合
   *
   * 该函数接收一个车道UID作为输入参数，返回一个包含指定车道下一个车道ID的集合，此集合不为空。
   *
   * @param laneUid 输入车道的UID
   * @return std::set<Base::txLaneUId> 包含指定车道下一个车道ID的集合
   */
  static std::set<Base::txLaneUId> GetNextLaneSetByLaneUid(const Base::txLaneUId laneUid) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道UID获取下一个车道集合
   *
   * 该函数接收一个车道UID作为输入参数，并返回包含该车道下一个车道ID的集合，此集合不为空。
   *
   * @param laneUid 输入车道的UID
   * @return std::set<hadmap::txLaneUId> 包含指定车道下一个车道ID的集合
   */
  static hadmap::txLanes GetNextLanesByLaneUid(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的下一个车道列表
   *
   * 根据给定的车道ID，获取该车道的下一个车道列表，列表中的每个车道都将作为当前车道的下一个车道进行连接。
   *
   * @param laneUid 指定的车道ID
   * @return hadmap::txLanes 包含所有下一个车道的车道列表
   */
  static hadmap::txLanes GetNextLanesByLaneUid(const Base::txLaneUId laneUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的前置车道ID集合
   *
   * 根据给定的车道ID，获取该车道的前置车道ID集合，其中包含了所有作为该车道前置车道的车道。
   *
   * @param lanePtr 指定的车道ID指针
   * @return std::set<Base::txLaneUId> 包含所有前置车道ID的集合
   */
  static std::set<Base::txLaneUId> GetPreLaneSetByLaneUid(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道ID，获取该车道的前置车道ID集合
   *
   * @param laneUid 指定的车道ID
   * @return std::set<Base::txLaneUId> 包含所有前置车道ID的集合
   * @throw 无
   */
  static std::set<Base::txLaneUId> GetPreLaneSetByLaneUid(const Base::txLaneUId laneUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的下一个车道列表
   *
   * 此函数用于获取指定车道的下一个车道列表，返回的是一个包含所有下一个车道ID的列表。
   *
   * @param _laneUid 指定的车道ID
   * @return Base::LocInfoSet 包含所有下一个车道ID的列表
   * @throw 无
   */
  static Base::LocInfoSet GetLaneNextLinkSet(const txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的下一个车道连接
   *
   * 此函数用于获取指定车道的下一个车道连接，并返回一个包含下一个车道连接的结构体对象。
   *
   * @param _laneUid 指定车道的ID
   * @return hadmap::txLaneLinks 包含下一个车道的结构体对象
   * @throw 无
   */
  static hadmap::txLaneLinks GetLaneNextLinks(const txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的所有下一个车道的ID
   *
   * 此函数用于获取指定车道的所有下一个车道的ID，并以set的形式返回。
   *
   * @param _link_locInfo 指定车道的位置信息
   * @return std::set<Base::txLaneUId> 包含所有下一个车道的ID的set集合
   * @throw 无
   */
  static std::set<Base::txLaneUId> GetLaneLinkToLaneUidSet(const Base::Info_Lane_t& _link_locInfo) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的上一个车道link列表
   *
   * 此函数用于获取指定车道的上一个车道link列表，并以结构体形式返回。
   *
   * @param _laneUid 要查询的车道ID
   * @return Base::LocInfoSet 包含上一个车道的结构体对象
   * @throw 无
   */
  static Base::LocInfoSet GetLanePrevLinkSet(const txLaneUId _laneUid) TX_NOEXCEPT;

  static std::set<Base::txLaneUId> GetLaneLinkFromLaneUidSet(const Base::Info_Lane_t& _link_locInfo) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道信息，获取到所有与之相邻的车道ID
   *
   * 根据给定的车道信息，返回包含所有与之相邻的车道ID的集合。
   *
   * @param _link_locInfo 要查询的车道信息
   * @return std::set<Base::txLaneUId> 包含所有相邻车道ID的集合
   * @throw 无
   */
  static const std::set<Base::txRoadID> GetPreRoadByRoadId(const Base::txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路ID的下一个道路ID
   * @param _roadId 要查询的道路ID
   * @return 包含指定道路ID下一个道路ID的集合
   * @throw 无
   */
  static const std::set<Base::txRoadID> GetNextRoadByRoadId(const Base::txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 添加断头道路
   *
   * 给定一条道路的唯一标识符和结束点，将该道路标记为断头路，并将其添加到断头道路的集合中。
   *
   * @param laneUid 要查询的道路ID
   * @param endPt 道路的结束点
   * @throw 无
   */
  static void add_dead_end_road(const Base::txLaneUId& laneUid, const txWGS84& endPt) TX_NOEXCEPT;

  /**
   * @brief 检查道路是否为断头路
   * @param laneUid 要查询的道路ID
   * @param endPt 道路的结束点坐标
   * @return true 为断头路, false 非断头路
   * @throw 无
   */
  static Base::txBool IsDeadEndRoad(const Base::txLaneUId& laneUid, txWGS84& endPt) TX_NOEXCEPT;

  /**
   * @brief 初始化黑名单
   *
   * 初始化txHdMapCacheConcurrent类的黑名单成员，该黑名单用于记录不同主机的默认优先级。
   *
   * @throws TxNoexcept 无异常
   */
  static void init_black_list() TX_NOEXCEPT;

  /**
   * @brief 将链路信息添加到黑名单
   *
   * 该函数将链路信息添加到黑名单中，用于标记当前链路为不需要缓存。
   *
   * @param link_locInfo 链路的位置信息
   * @return 无
   * @throws 无
   */
  static void add_link_black_list(const Base::Info_Lane_t& link_locInfo) TX_NOEXCEPT;

  /**
   * @brief 判断链路是否在黑名单中
   *
   * 判断链路的位置信息是否在黑名单中，如果在则表示不需要缓存该链路信息。
   *
   * @param link_locInfo 链路的位置信息
   * @return true 链路在黑名单中
   * @return false 链路不在黑名单中
   * @throws 无
   */
  static txBool IsLinkIdInBlackList(const Base::Info_Lane_t& link_locInfo) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道连接的控制相位
   *
   * 获取指定车道连接的控制相位，用于表示当前的车道连接控制状态。
   *
   * @param _linkptr 车道连接指针
   * @return 返回获取到的控制相位字符串，包含信息如 "S1-GREEN" 等。
   * @throws 无
   */
  static Base::txString GetLaneLinkControlPhase(const hadmap::txLaneLinkPtr _linkptr) TX_NOEXCEPT;

  /**
   * @brief 根据给定的经纬度获取初始相关信息
   *
   * 该函数根据给定的经纬度，计算出最近的车道，然后计算当前车辆与最近车道之间的距离。
   *
   * @param lon 指定的经度值
   * @param lat 指定的纬度值
   * @param distanceCurve 存储离最近车道的曲线距离
   * @param distancePedal 存储离最近车道的脚踏距离
   * @return 返回最近的车道指针
   * @throws 无
   */
  static hadmap::txLanePtr GetLaneForInit(const Base::txFloat lon, const Base::txFloat lat,
                                          Base::txFloat& distanceCurve, Base::txFloat& distancePedal) TX_NOEXCEPT;

  /**
   * @brief 根据给定的经纬度获取初始相关信息
   *
   * 该函数根据给定的经纬度，计算出最近的车道，然后计算当前车辆与最近车道之间的距离。
   *
   * @param lon 指定的经度值
   * @param lat 指定的纬度值
   * @param distanceCurve 存储离最近车道的曲线距离
   * @param distancePedal 存储离最近车道的脚踏距离
   * @param _lOffset 存储离最近车道的起始偏移量
   * @return 返回最近的车道指针
   * @throws 无
   */
  static hadmap::txLanePtr GetLaneForInit(const Base::txFloat lon, const Base::txFloat lat,
                                          Base::txFloat& distanceCurve, Base::txFloat& distancePedal,
                                          Base::txFloat& _lOffset) TX_NOEXCEPT;

  /**
   * @brief 获取给定经纬度对应的车道连接数据
   *
   * 该函数根据给定的经纬度，计算出最近的车道连接，并计算车辆与最近车道连接之间的距离。
   *
   * @param lon 指定的经度值
   * @param lat 指定的纬度值
   * @param distanceCurve 存储离最近车道连接的曲线距离
   * @param distancePedal 存储离最近车道连接的脚踏距离
   * @return 返回最近的车道连接指针
   * @throws 无
   */
  static hadmap::txLaneLinkPtr GetLaneLinkForInit(const Base::txFloat lon, const Base::txFloat lat,
                                                  Base::txFloat& distanceCurve,
                                                  Base::txFloat& distancePedal) TX_NOEXCEPT;

  /**
   * @brief 根据给定的经纬度获取车道连接数据
   *
   * 该函数根据给定的经纬度，计算出最近的车道连接，并计算车辆与最近车道连接之间的距离。
   *
   * @param lon 指定的经度值
   * @param lat 指定的纬度值
   * @param distanceCurve 存储离最近车道连接的曲线距离
   * @param distancePedal 存储离最近车道连接的脚踏距离
   * @param _lOffset 存储离最近车道连接的起始偏移量
   * @return 返回最近的车道连接指针
   * @throws 无
   */
  static hadmap::txLaneLinkPtr GetLaneLinkForInit(const Base::txFloat lon, const Base::txFloat lat,
                                                  Base::txFloat& distanceCurve, Base::txFloat& distancePedal,
                                                  Base::txFloat& _lOffset) TX_NOEXCEPT;

  /**
   * @brief 通过给定的GPS坐标获取最相邻的车道信息
   *
   * 通过在地图中查找最近的车道信息，包括车道方向、起点横坐标、纵坐标、车道宽度等。
   *
   * @param aPos 输入的GPS坐标
   * @param locInfo 输出的相邻车道信息
   * @param _S 存储车道方向信息的变量，0表示向左，1表示向右
   * @param _TT 存储车道起点横坐标信息的变量
   * @return 返回成功或失败的状态
   * @throws 无
   */
  static Base::txBool AdsorbLaneInfo(Coord::txWGS84 aPos, Base::Info_Lane_t& locInfo, Base::txFloat& _S,
                                     Base::txFloat& _TT) TX_NOEXCEPT;

  /**
   * @brief 获取地图底部左侧的坐标
   *
   * 该函数用于获取地图底部左侧的坐标点。
   *
   * @return Coord::txENU 地图底部左侧的坐标点
   */
  static Coord::txENU MapBottomLeft() TX_NOEXCEPT { return s_bottom_left; }

  /**
   * @brief 获取地图顶部右侧的坐标
   *
   * 该函数用于获取地图顶部右侧的坐标点。
   *
   * @return Coord::txENU 地图顶部右侧的坐标点
   */
  static Coord::txENU MapTopRight() TX_NOEXCEPT { return s_top_right; }

  /**
   * @brief 检查道路在拐点附近
   *
   * 此函数用于检查指定道路是否位于拐点附近。
   *
   * @param _laneUid 道路的唯一ID
   * @return 如果道路位于拐点附近，返回 true；否则返回 false
   */
  static Base::txBool IsRoadNearJunction(const txLaneUId _laneUid) TX_NOEXCEPT;

 protected:
  /**
   * @brief 对给定位置沿着给定车道方向偏移一定距离
   *
   * 此函数用于根据给定位置和车道方向，计算并返回偏移一定距离后的新位置。
   *
   * @param vPos        给定的初始位置
   * @param vLaneDir    给定的车道方向
   * @param _offset     需要偏移的距离
   * @return txVec3      计算后的新位置
   */
  static txVec3 OffsetLocalPosWithLaneTangent(const txVec3& vPos, const txVec3& vLaneDir,
                                              const txFloat _offset) TX_NOEXCEPT;

 public:
  /**
   * @brief 通过给定的车道UID获取车道信息
   *
   * 此函数用于根据给定的车道UID，从缓存中查找对应的车道信息，并返回该信息的智能指针。
   *
   * @param _laneUid 给定的车道UID
   * @return 返回车道信息的智能指针，若未找到车道信息则返回空指针
   */
  static txLaneInfoInterfacePtr GetLaneInfoByUid(const txLaneUId& _laneUid) TX_NOEXCEPT;
  // static txLaneInfoInterfacePtr GetLaneLinkInfoById(const txLaneLinkID& _lanelinkId) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道信息
   *
   * 根据给定的车道ID，从缓存中获取对应的车道信息，并以智能指针形式返回。
   *
   * @param _lane_loc_info 车道ID
   * @return 返回车道信息的智能指针，若未找到车道信息则返回空指针
   */
  static txLaneInfoInterfacePtr GetGeomInfoById(const Base::Info_Lane_t& _lane_loc_info) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的形状信息
   *
   * 根据给定的车道信息，从缓存中获取对应的车道形状信息，并以枚举形式返回。
   *
   * @param _lane_loc_info 车道信息
   * @return 返回车道形状信息的枚举值，若未找到车道信息则返回空指针
   */
  static txLaneInfoInterface::LaneShape GetGeomShape(const Base::Info_Lane_t& _lane_loc_info) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路信息的 Geometry 信息.
   *
   * 通过提供的道路信息，找到对应的道路 Geometry 信息，并且对结果进行缓存。
   * 若查询到的道路信息不存在，则返回空指针。
   *
   * @param _road_loc_info 要查询的道路信息.
   * @return txLaneInfoInterfacePtr 返回查询到的道路 Geometry 信息，若查询失败则返回空指针.
   */
  static txLaneInfoInterfacePtr GetRoadGeomInfoById(const Base::Info_Road_t& _road_loc_info) TX_NOEXCEPT;

  /**
   * @brief 使用给定的 txLaneLinkPtr 构造一个 LaneLocInfo，并进行逻辑校验，校验的内容包括 Link
   * 是否为空，如果为空则返回一个空的 Info_Lane_t，否则返回一个正确的 Info_Lane_t。
   *
   * @param _linkptr 给定的 txLaneLinkPtr，用于构造 Info_Lane_t。
   * @return Info_Lane_t 如果给定的 _linkptr 为空，则返回一个空的 Info_Lane_t；否则返回一个正确的 Info_Lane_t。
   */
  static Base::Info_Lane_t MakeLaneLocInfoByLinkId_uncheck(const hadmap::txLaneLinkPtr _linkptr) TX_NOEXCEPT;

 public:
  enum class SwitchDir : txInt { eLeft, eRight };

  /**
   * @brief 检查是否允许改变当前路线
   *
   * 对于传入的道路数据列表，检查是否允许改变当前路线。
   *
   * @param roadPtrVec 传入的道路数据列表
   */
  static void CheckRefuseChangeLane(const hadmap::txRoads& roadPtrVec) TX_NOEXCEPT;

  /**
   * @brief 判断是否禁止改变左侧路线
   *
   * 判断指定的道路是否被禁止改变左侧路线。
   *
   * @param _laneUid 要判断的道路 UID
   * @return 若禁止改变左侧路线返回 true，否则返回 false
   */
  static Base::txBool IsRefuseChangeLane_Left(const txLaneUId& _laneUid) TX_NOEXCEPT;

  /**
   * @brief 检查是否禁止改变右侧道路
   *
   * 检查指定的道路是否被禁止改变右侧道路。
   *
   * @param _laneUid 要检查的道路 UID
   * @return 若禁止改变右侧道路返回 true，否则返回 false
   */
  static Base::txBool IsRefuseChangeLane_Right(const txLaneUId& _laneUid) TX_NOEXCEPT;

  /**
   * @brief 为所有车道添加不允许改变的标志
   *
   * 为指定的车道添加不允许改变的标志。
   *
   * @param _laneUid 要修改的车道ID
   */
  static void AddRefuseChangeLane_All(const txLaneUId& _laneUid) TX_NOEXCEPT;

  /**
   * @brief 向映射缓存中添加指定方向的不允许改变车道标志
   *
   * 该函数用于向映射缓存中添加指定方向的不允许改变车道标志。
   * 这些标志会影响到自动驾驶系统的行驶路径选择，防止车辆在某些情况下跳出车道。
   *
   * @param vec_cfg_laneUid 要添加的车道ID列表
   * @param _dir 要添加的车道方向，分为正向和反向
   */
  static void AddRefuseChangeLane(const std::vector<Base::txLaneUId>& vec_cfg_laneUid,
                                  const SwitchDir _dir) TX_NOEXCEPT;

  using evId2RefuseChangeLaneSet =
      std::map<Base::txSysId, std::vector<std::tuple<Base::txLaneUId, txFloat, txFloat> > >;

  /**
   * @brief 为指定的事件添加特殊的不允许改变车道的标志
   *
   * 此函数为指定的事件添加不允许改变车道的标志。这些标志用于
   * 控制自动驾驶车辆在行驶过程中的行驶路径选择，以防止车辆在某些
   * 情况下跳出车道。
   *
   * @param evId 要添加标志的事件ID
   * @param vec_cfg_laneUid 要添加的车道ID列表
   * @param _dir 要添加的车道方向，分为正向和反向
   */
  static void AddSpecialRefuseChangeLane(const Base::txSysId evId,
                                         evId2RefuseChangeLaneSet::mapped_type& vec_cfg_laneUid,
                                         const SwitchDir _dir) TX_NOEXCEPT;

  /**
   * @brief 检查给定的车道是否为左侧特殊不允许改变车道的车道
   *
   * 这个函数用于检查给定的车道是否为左侧特殊不允许改变车道的车道。在自动驾驶系统运行过程中，有些情况下车辆不能改变车道，这种车道被认为是特殊不允许改变车道的车道。
   *
   * @param _laneUid 要检查的车道ID
   * @param distOnCurve 当前车辆在车道上的距离
   * @return 如果车道是左侧特殊不允许改变车道的车道，则返回 true，否则返回 false
   */
  static Base::txBool IsSpecialRefuseChangeLane_Left(const txLaneUId& _laneUid, const txFloat distOnCurve) TX_NOEXCEPT;

  /**
   * @brief 判断给定的车道是否为右侧特殊不允许改变车道的车道
   *
   * 此函数用于检查给定的车道是否为右侧特殊不允许改变车道的车道。在自动驾驶系统运行过程中，有些情况下车辆不能改变车道，这种车道被认为是特殊不允许改变车道的车道。
   *
   * @param _laneUid 要检查的车道ID
   * @param distOnCurve 当前车辆在车道上的距离
   * @return 如果车道是右侧特殊不允许改变车道的车道，则返回 true，否则返回 false
   */
  static Base::txBool IsSpecialRefuseChangeLane_Right(const txLaneUId& _laneUid, const txFloat distOnCurve) TX_NOEXCEPT;

  /**
   * @brief 从特殊不允许改变车道的车道列表中移除指定的事件
   *
   * 当某个事件在特殊不允许改变车道的车道中，需要在事件结束后将其从列表中移除，以避免其他与之相关的操作引用无效的问题。
   *
   * @param evId 要移除的事件ID
   */
  static void RemoveSpecialRefuseChangeLane(const Base::txSysId evId) TX_NOEXCEPT;

  /**
   * @brief 展示特殊不允许改变车道的车道信息
   *
   * 当自动驾驶系统遇到特殊不允许改变车道的车道时，需要展示这些车道的详细信息，以便用户和开发人员能够快速了解和处理这些情况。
   */
  static void ShowRefuseChangeLaneInfo() TX_NOEXCEPT;
  using lanelinkExpandVisionType = tbb::concurrent_hash_map<
      Base::Info_Lane_t,
      std::unordered_map<Base::Info_Lane_t, std::tuple<txFloat, txFloat>, Utils::Info_Lane_t_HashCompare>,
      Utils::Info_Lane_t_HashCompare>;

  /**
   * @brief 检查是否需要扩展视野以便获取链接的信息
   *
   * 这个函数用于检查是否需要扩展链接的视野以便获取与链接有关的信息。
   *
   * @param link_locInfo 包含链接位置信息的结构体
   * @param res_expand_vision 扩展视野信息结果
   * @return 返回扩展视野状态
   */
  static Base::txBool CheckExpandVision(const Base::Info_Lane_t& link_locInfo,
                                        lanelinkExpandVisionType::mapped_type& res_expand_vision) TX_NOEXCEPT;

 public:
  struct HadmapFilter {
    struct link_from_to {
      Base::txString fromLaneUid, toLaneUid;
      /**
       * @brief 返回字符串形式的数据
       *
       * 此函数返回当前数据结构的字符串表示，以方便输出和调试。
       *
       * @return 当前数据结构的字符串表示
       */
      txString Str() const TX_NOEXCEPT;
      friend std::ostream& operator<<(std::ostream& os, const link_from_to& v) TX_NOEXCEPT {
        os << v.Str();
        return os;
      }
    };

    struct link_from_to_distance {
      Base::txString fromLaneUid, toLaneUid;
      Base::txFloat distance = 0.0;
      /**
       * @brief 返回字符串形式的数据
       *
       * 此函数返回当前数据结构的字符串表示，以方便输出和调试。
       *
       * @return 当前数据结构的字符串表示
       */
      txString Str() const TX_NOEXCEPT;
      friend std::ostream& operator<<(std::ostream& os, const link_from_to_distance& v) TX_NOEXCEPT {
        os << v.Str();
        return os;
      }
    };

    struct link_from_to_road {
      Base::txRoadID fromRoadId, toRoadId;
      /**
       * @brief 返回字符串形式的数据
       *
       * 此函数返回当前数据结构的字符串表示，以方便输出和调试。
       *
       * @return 当前数据结构的字符串表示
       */
      txString Str() const TX_NOEXCEPT;
      friend std::ostream& operator<<(std::ostream& os, const link_from_to_road& v) TX_NOEXCEPT {
        os << v.Str();
        return os;
      }
    };

    struct cfg_special_hdmap {
      void Clear() TX_NOEXCEPT {
        HDMapNameWithExtern.clear();
        /*blacklist_lanelinkId.clear();*/
        blacklist_from_to_laneUid.clear();

        refuse_switch_dual_direction_lane.clear();
        refuse_switch_left_lane.clear();
        refuse_switch_right_lane.clear();

        refuse_switch_dual_direction_road.clear();
        refuse_switch_left_road.clear();
        refuse_switch_right_road.clear();
      }

      txString HDMapNameWithExtern;
      /*std::vector< Base::txLaneLinkID > blacklist_lanelinkId;*/
      std::vector<link_from_to> blacklist_from_to_laneUid;
      std::vector<link_from_to_road> blacklist_from_to_roadId;

      std::vector<txString> refuse_switch_dual_direction_lane;
      std::vector<txString> refuse_switch_left_lane;
      std::vector<txString> refuse_switch_right_lane;

      std::vector<Base::txRoadID> refuse_switch_dual_direction_road;
      std::vector<Base::txRoadID> refuse_switch_left_road;
      std::vector<Base::txRoadID> refuse_switch_right_road;

      struct expand_vision_node {
        link_from_to source_link;
        std::vector<link_from_to_distance> expand_links;
        /**
         * @brief 返回字符串形式的数据
         *
         * 此函数返回当前数据结构的字符串表示，以方便输出和调试。
         *
         * @return 当前数据结构的字符串表示
         */
        txString Str() const TX_NOEXCEPT;
        friend std::ostream& operator<<(std::ostream& os, const expand_vision_node& v) TX_NOEXCEPT {
          os << v.Str();
          return os;
        }
      };

      std::vector<expand_vision_node> expand_vision;

      /**
       * @brief 返回字符串形式的数据
       *
       * 此函数返回当前数据结构的字符串表示，以方便输出和调试。
       *
       * @return 当前数据结构的字符串表示
       */
      txString Str() const TX_NOEXCEPT;
      friend std::ostream& operator<<(std::ostream& os, const cfg_special_hdmap& v) TX_NOEXCEPT {
        os << v.Str();
        return os;
      }
    };

    std::vector<cfg_special_hdmap> hdmap_advanced_cfg;
    void Clear() TX_NOEXCEPT {
      for (auto& ref_cfg : hdmap_advanced_cfg) {
        ref_cfg.Clear();
      }
    }

    /**
     * @brief 解析指定名称的外部数据文件
     *
     * 这个函数解析指定的外部数据文件（如 d2d_20190726.sqlite），并在内部缓存一些重要的属性。
     *
     * @param map_name_with_extern 包含外部数据文件名的字符串
     */
    void Parse(const Base::txString map_name_with_extern /*eg: d2d_20190726.sqlite*/) TX_NOEXCEPT;

    /**
     * @brief 返回字符串表示的数据
     *
     * 该函数返回当前数据结构的字符串表示。这是一个便于输出和调试的功能。
     *
     * @return 当前数据结构的字符串表示
     */
    txString Str() const TX_NOEXCEPT;

    /**
     * @brief 判断当前缓存是否有效
     *
     * @return true 当前缓存有效
     * @return false 当前缓存无效
     */
    txBool IsValid() const TX_NOEXCEPT { return hdmap_advanced_cfg.size() > 0; }

    /**
     * @brief 获取当前缓存的HD地图名称
     * @return 返回当前缓存的HD地图名称
     */
    txString HdMapName() const TX_NOEXCEPT;

    /*std::vector< Base::txLaneLinkID > BlacklistLaneLinkId() const TX_NOEXCEPT;*/

    /**
     * @brief 获取黑名单位置信息
     * @return 一个包含所有黑名单位置信息的数组
     */
    std::vector<Base::Info_Lane_t> BlacklistLocInfo() const TX_NOEXCEPT;

    /**
     * @brief 获取黑名单中的路径ID及对应的从点和到点
     * @return 一个包含所有黑名单路径的数组
     */
    std::vector<link_from_to_road> BlacklistFromToRoadId() const TX_NOEXCEPT;

    /**
     * @brief 获取不能切换至左侧车道的车道编号
     * @return 包含不能切换至左侧车道车道编号的数组
     */
    std::vector<Base::txLaneUId> RefuseSwitchLeftLane() const TX_NOEXCEPT;

    /**
     * @brief 获取不能切换至右侧车道的车道编号
     * @return 包含不能切换至右侧车道车道编号的数组
     */
    std::vector<Base::txLaneUId> RefuseSwitchRightLane() const TX_NOEXCEPT;

    /**
     * @brief 展开视野
     *
     * 展开当前车辆的视野，返回一个包含所有车道及其对应相对位移的二维向量。
     *
     * @return 二维向量，包含所有车道及其对应相对位移的二维向量
     */
    std::vector<std::tuple<std::tuple<Base::txLaneUId, Base::txLaneUId>,
                           std::vector<std::tuple<Base::txLaneUId, Base::txLaneUId, Base::txFloat> > > >
    ExpandVision() const TX_NOEXCEPT;
    friend std::ostream& operator<<(std::ostream& os, const HadmapFilter& v) TX_NOEXCEPT {
      os << v.Str();
      return os;
    }
  };

  /**
   * @brief 初始化Hadmap过滤器
   *
   * 根据配置文件和地图文件初始化Hadmap过滤器，返回初始化是否成功的布尔值。
   * 配置文件需要包含HadmapFilter的相关参数，例如数据读取速率、滤波器类型等。
   * 地图文件通常包含Hadmap的地图信息，例如车道、交叉口、灯光等。
   *
   * @param cfg_path 配置文件路径
   * @param map_path 地图文件路径
   * @return 初始化是否成功的布尔值
   */
  static Base::txBool InitializeHadmapFilter(const Base::txString cfg_path, const Base::txString map_path) TX_NOEXCEPT;

  /**
   * @brief 处理Hadmap数据的过滤工作
   *
   * 此函数用于处理Hadmap数据的过滤工作，包括提取与过滤与感兴趣区域相关的数据，如车道信息、交叉口信息等。
   *
   * @param lanePtrVec 车道指针向量，存储需要过滤的车道信息
   * @param lanelinkPtrVec 车道链接指针向量，存储与车道相关的交叉口信息
   * @param roadPtrVec 道路指针向量，存储需要过滤的道路信息
   */
  static void Hadmap_Filter(hadmap::txLanes& lanePtrVec, hadmap::txLaneLinks& lanelinkPtrVec,
                            hadmap::txRoads& roadPtrVec) TX_NOEXCEPT;

  /**
   * @brief 反转车道数据
   *
   * 此函数反转输入的车道、车道链接和道路数据，使其具有相反方向的行驶方向。
   *
   * @param[in] lanePtrVec 车道指针向量，存储需要反转的车道信息
   * @param[in] lanelinkPtrVec 车道链接指针向量，存储与车道相关的交叉口信息
   * @param[in] roadPtrVec 道路指针向量，存储需要反转的道路信息
   */
  static void ReverseLane(hadmap::txLanes& lanePtrVec, hadmap::txLaneLinks& lanelinkPtrVec,
                          hadmap::txRoads& roadPtrVec) TX_NOEXCEPT;

  /**
   * @brief 清除并添加暂时禁止变更车道的目的地
   *
   * 此函数清除已经存在的暂时禁止变更车道的目的地，并将新的目的地添加进去。
   *
   * @return void
   */
  static void ClearAddDstRefuseLaneChange() TX_NOEXCEPT;

  /**
   * @brief 将目的地添加到左侧的禁止变更车道列表中
   *
   * 将指定的车道添加到左侧的禁止变更车道列表中，以确保相关的车辆不会在循环路线中产生错误的行驶方向。
   *
   * @param _laneUid 目的地车道的 Unique ID
   * @return void
   */
  static void AddDstRefuseLaneChange_Left(const Base::txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 添加右侧车道到禁止变更车道列表
   *
   * 将指定的右侧车道添加到禁止变更车道列表中，以确保相关的车辆不会在循环路线中产生错误的行驶方向。
   *
   * @param _laneUid 目的地车道的 Unique ID
   * @return void
   */
  static void AddDstRefuseLaneChange_Right(const Base::txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 判断目的地左侧车道是否需要禁止变更
   *
   * 判断目的地左侧车道是否需要禁止变更，用于确保车辆在循环路线中不会产生错误的行驶方向。
   *
   * @param _laneUid 目的地车道的 Unique ID
   * @return 若目的地左侧车道需要禁止变更，则返回 true，否则返回 false
   */
  static Base::txBool IsDstRefuseLaneChange_Left(const Base::txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 检查目的地右侧车道是否需要禁止变更
   *
   * 该函数用于检查目的地右侧车道是否需要禁止变更，以确保车辆在循环路线中不会产生错误的行驶方向。
   *
   * @param _laneUid 目的地车道的 Unique ID
   * @return 若目的地右侧车道需要禁止变更，则返回 true，否则返回 false
   */
  static Base::txBool IsDstRefuseLaneChange_Right(const Base::txLaneUId _laneUid) TX_NOEXCEPT;

  /**
   * @brief 制作一致性
   *
   * 该函数用于在并发操作时保证地图数据的一致性。
   *
   * @return void
   */
  static void MakeConsistency() TX_NOEXCEPT;

 protected:
  /**
   * @brief 生成一个缓存区范围
   *
   * 这个函数用于根据给定的参考点和偏移量，生成一个缓存区范围，该范围用于与地图数据进行交互。
   *
   * @param ref_origin_GPS 参考点，以GPS坐标表示
   * @param Envelope_Offset 地图的偏移量，用于生成缓存区范围
   * @return hadmap::PointVec 生成的缓存区范围的点集合
   */
  static hadmap::PointVec MakeEnvelope(const hadmap::txPoint& ref_origin_GPS,
                                       const Base::txFloat Envelope_Offset) TX_NOEXCEPT;

 protected:
  static hadmap::txMapHandle* pMapHandle;
  static Base::txBool sValid;
  static hadmap::PointVec s_envelope;

 public:
  using id2roadPtrType = tbb::concurrent_hash_map<txRoadID, hadmap::txRoadPtr, RoadIdHashCompare>;

 protected:
  static id2roadPtrType s_id2roadPtr;

 public:
  using sectionUid2sectionPtrType = tbb::concurrent_hash_map<txSectionUId, hadmap::txSectionPtr, SectionUIdHashCompare>;

 protected:
  static sectionUid2sectionPtrType s_sectionUid2sectionPtr;

  using id2lanePtrType = tbb::concurrent_hash_map<txLaneUId, hadmap::txLanePtr, LaneUIdHashCompare>;
  static id2lanePtrType s_id2lanePtr;

  using id2lanelinkPtrType = tbb::concurrent_hash_map<txLaneLinkID, hadmap::txLaneLinkPtr, LandLinkIDHashCompare>;
  static id2lanelinkPtrType s_id2lanelinkPtr;

  using fromUid_toUid2lanelinkPtrType =
      tbb::concurrent_hash_map<std::pair<txLaneUId, txLaneUId>, hadmap::txLaneLinkPtr, FromToUidHashCompare>;
  static fromUid_toUid2lanelinkPtrType s_fromUidtoUid_lanelinkPtr;

  using laneLocInfo2LaneInfoType =
      tbb::concurrent_hash_map<Base::Info_Lane_t, txLaneInfoInterfacePtr, Info_Lane_t_HashCompare>;
  static laneLocInfo2LaneInfoType s_lanelocinfo2GeomInfo;

  using laneLocInfo2LaneShapeType =
      tbb::concurrent_hash_map<Base::Info_Lane_t, txLaneInfoInterface::LaneShape, Info_Lane_t_HashCompare>;
  static laneLocInfo2LaneShapeType s_laneLocInfo2LaneShapeType;

  using RoadLocInfo2GeomInfoType =
      tbb::concurrent_hash_map<Base::Info_Road_t, txLaneInfoInterfacePtr, Info_Road_t_HashCompare>;
  static RoadLocInfo2GeomInfoType s_roadlocinfo2GeomInfo;

  using road2StartEndType = tbb::concurrent_hash_map<txRoadID, RoadStartEndInfo, RoadIdHashCompare>;
  static road2StartEndType s_roadId2roadStartEnd;

  using uid2LaneLinkSetType = tbb::concurrent_hash_map<txLaneUId, Base::LocInfoSet, LaneUIdHashCompare>;
  static uid2LaneLinkSetType s_lane_from_linkset;
  static uid2LaneLinkSetType s_lane_to_linkset;

  using lanelinkLocInfo2LaneSetType =
      tbb::concurrent_hash_map<Base::Info_Lane_t, std::set<Base::txLaneUId>, Info_Lane_t_HashCompare>;
  static lanelinkLocInfo2LaneSetType s_link_from_laneset;
  static lanelinkLocInfo2LaneSetType s_link_to_laneset;

  using uid2LengthType = tbb::concurrent_hash_map<txLaneUId, txFloat, LaneUIdHashCompare>;
  static uid2LengthType s_uid2LaneLength;

  using uid2LaneMarkType =
      tbb::concurrent_hash_map<txLaneUId, std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK>, LaneUIdHashCompare>;
  static uid2LaneMarkType s_uid2LaneMark;

  using lanelinkId2LengthType = tbb::concurrent_hash_map<Base::Info_Lane_t, txFloat, Utils::Info_Lane_t_HashCompare>;
  static lanelinkId2LengthType s_linkId2LinkLength;

  using uid2LaneUidSet = tbb::concurrent_hash_map<txLaneUId, std::set<txLaneUId>, LaneUIdHashCompare>;
  static uid2LaneUidSet s_lane_next_laneset;
  static uid2LaneUidSet s_lane_pre_laneset;

  using uid2HdMapLocateInfo = tbb::concurrent_hash_map<txLaneUId, txWGS84, LaneUIdHashCompare>;
  static uid2HdMapLocateInfo s_dead_end_road_flag_map_dead_point;

  using road2RoadSetType = tbb::concurrent_hash_map<txRoadID, std::set<txRoadID>, RoadIdHashCompare>;
  static road2RoadSetType s_RoadId2NextRoadSet;
  static road2RoadSetType s_RoadId2PreRoadSet;

  using roadIdPair2LaneLinkSetType = tbb::concurrent_hash_map<RoadPair, Base::LocInfoSet, RoadPairHashCompare>;
  static roadIdPair2LaneLinkSetType s_fromRoadIdToRoadId2LaneLinkIdSet, s_fromRoadIdToRoadId2LaneLinkIdSet4Route;

  using LocInfoBlackListType = tbb::concurrent_unordered_set<Base::Info_Lane_t, Utils::Info_Lane_t_HashCompare>;
  static LocInfoBlackListType s_locInfo_blacklist;

  using roadId2LaneLinkSetType = tbb::concurrent_hash_map<txRoadID, std::set<Base::Info_Lane_t>, RoadIdHashCompare>;
  static roadId2LaneLinkSetType s_fromRoadId2LaneLinkSet;

  using laneBoundaryId2LaneBoundaryPtrType =
      tbb::concurrent_hash_map<txBoundaryID, hadmap::txLaneBoundaryPtr, LaneBoundaryIDHashCompare>;
  static laneBoundaryId2LaneBoundaryPtrType s_id2BoundaryPtr;

  using uid2RefuseChangeLane = tbb::concurrent_hash_map<txLaneUId, hadmap::txPoint, LaneUIdHashCompare>;
  static uid2RefuseChangeLane s_laneUid2RefuseChangeLane_Left;
  static uid2RefuseChangeLane s_laneUid2RefuseChangeLane_Right;

  static evId2RefuseChangeLaneSet s_special_refuse_change_lane_left;
  static evId2RefuseChangeLaneSet s_special_refuse_change_lane_right;

  static lanelinkExpandVisionType s_lanelink_expand_vision;

  static HadmapFilter s_hadmap_filer;

  static Coord::txENU s_bottom_left, s_top_right;

  using DstReRefuseLaneChangeSet = tbb::concurrent_hash_map<Base::txLaneUId, bool, Utils::LaneUIdHashCompare>;
  static DstReRefuseLaneChangeSet s_dst_refuse_lane_change_left, s_dst_refuse_lane_change_right;

  using road2junctionType = tbb::concurrent_hash_map<txRoadID, txWGS84, RoadIdHashCompare>;
  static road2junctionType s_roadId2junction;

  using hdmapTrick_LaneUidLeftRightType =
      tbb::concurrent_hash_map<Base::txLaneUId, std::array<Base::txLaneUId, 2 /*left :0, right: 1*/>,
                               Utils::LaneUIdHashCompare>;
  static hdmapTrick_LaneUidLeftRightType s_hdmapTrick_LaneUidLeftRightType;

  using hdmapTrick_RoadLaneKeepTimeType = tbb::concurrent_hash_map<txRoadID, Base::txFloat, RoadIdHashCompare>;
  static hdmapTrick_RoadLaneKeepTimeType s_hdmapTrick_RoadLaneKeepTimeType;

 public:
  /**
   * @brief 获取所有道路的集合
   *
   * 返回包含所有道路的集合，用于查询或操作道路数据。
   *
   * @return s_id2roadPtr 一个包含所有道路的映射，键为道路ID，值为指向道路对象的指针
   */
  static const id2roadPtrType& getAllRoads() TX_NOEXCEPT { return s_id2roadPtr; }

  /**
   * @brief 根据道路ID获取该道路的所有车道连接
   *
   * 此函数用于获取指定道路ID对应的车道连接。
   * 返回一个包含所有车道连接的set，里面的元素是一个pair，pair的first为车道ID，second为该车道的车道连接信息。
   *
   * @param _roadId 指定道路的ID
   * @return 包含指定道路ID对应的所有车道连接的set
   */
  static roadId2LaneLinkSetType::value_type::second_type GetLaneLinksFromRoad(const Base::txRoadID _roadId) TX_NOEXCEPT;

  /**
   * @brief 初始化一个使用HdMapTrick的地图缓存
   *
   * 这个函数用于初始化一个使用HdMapTrick的地图缓存。在地图缓存初始化完成后，它将成为一个可用的HdMapTrick缓存。
   *
   * @param map_path 地图文件的路径
   * @return 如果初始化成功，返回True；否则返回False
   */
  static Base::txBool InitializeHdMapTrick(const Base::txString map_path) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的左右车道信息
   *
   * 通过输入车道ID，函数将返回指定车道的左右车道ID。
   *
   * @param srcLaneUid 需要查询的车道ID
   * @param leftUid 返回左侧车道ID
   * @param rightUid 返回右侧车道ID
   * @return 函数执行成功时返回true，否则返回false
   */
  static Base::txBool QueryTrick_LR(const Base::txLaneUId& srcLaneUid, Base::txLaneUId& leftUid,
                                    Base::txLaneUId& rightUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定车道的左侧车道信息
   *
   * 通过输入车道ID，函数将返回指定车道的左侧车道ID。
   *
   * @param srcLaneUid 需要查询的车道ID
   * @param leftUid 返回左侧车道ID
   * @return 函数执行成功时返回true，否则返回false
   */
  static Base::txBool QueryTrick_L(const Base::txLaneUId& srcLaneUid, Base::txLaneUId& leftUid) TX_NOEXCEPT;

  /**
   * @brief 查询指定车道的右侧车道信息
   *
   * 通过输入车道ID，函数将返回指定车道的右侧车道ID。
   *
   * @param srcLaneUid 需要查询的车道ID
   * @param rightUid 返回右侧车道ID
   * @return 函数执行成功时返回true，否则返回false
   */
  static Base::txBool QueryTrick_R(const Base::txLaneUId& srcLaneUid, Base::txLaneUId& rightUid) TX_NOEXCEPT;

  /**
   * @brief 获取指定道路的车道保持时间
   *
   * 根据给定的道路ID，查询并返回该道路的车道保持时间
   *
   * @param rid 指定的道路ID
   * @param spLaneKeepTime 返回保持时间
   * @return 函数执行成功时返回true，否则返回false
   */
  static Base::txBool QueryTrick_LaneKeepTime(const txRoadID rid, Base::txFloat& spLaneKeepTime) TX_NOEXCEPT;
};

TX_NAMESPACE_CLOSE(HdMap)

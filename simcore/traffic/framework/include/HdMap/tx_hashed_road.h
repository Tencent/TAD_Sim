// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tbb/tbb.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <list>
#include <set>
#include <vector>
#include "HdMap/tx_lane_geom_info.h"
#include "tbb/concurrent_hash_map.h"
#include "tx_header.h"
#include "tx_signal_element.h"
#include "tx_traffic_element_base.h"
#include "tx_vehicle_element.h"
#if USE_HashedRoadNetwork
#  include "HdMap/hashed_lane_info_orthogonal_list.h"
#  include "HdMap/tx_hashed_lane_info.h"

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)
class HashedRoadNetworkRTree2D {
 public:
  static HashedRoadNetworkRTree2D& getInstance() {
    static HashedRoadNetworkRTree2D instance;
    return instance;
  }

 private:
  HashedRoadNetworkRTree2D() TX_DEFAULT;
  ~HashedRoadNetworkRTree2D() TX_DEFAULT;
  /**
   * @brief HashedRoadNetworkRTree2D 类的拷贝构造函数
   *
   * 禁止使用此函数，禁止复制 HashedRoadNetworkRTree2D 类的实例
   *
   * @param other 待复制的 HashedRoadNetworkRTree2D 类实例
   */
  HashedRoadNetworkRTree2D(const HashedRoadNetworkRTree2D&) TX_DELETE;

  /**
   * @brief HashedRoadNetworkRTree2D 类的赋值运算符重载函数
   *
   * 禁止使用此函数，禁止复制 HashedRoadNetworkRTree2D 类的实例
   *
   * @param other 待复制的 HashedRoadNetworkRTree2D 类实例
   * @return HashedRoadNetworkRTree2D& 返回当前 HashedRoadNetworkRTree2D 类实例的引用
   */
  HashedRoadNetworkRTree2D& operator=(const HashedRoadNetworkRTree2D&) TX_DELETE;

 public:
  enum { max_element_size = 16 };
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using HashedLaneInfoPtr = Geometry::SpatialQuery::HashedLaneInfoPtr;
  using point_t = boost::geometry::model::point<Base::txFloat, 2, boost::geometry::cs::cartesian>;
  using box_t = boost::geometry::model::box<point_t>;
  using poly_t = boost::geometry::model::polygon<point_t>;
  TX_MARK("outer is clock-wise.");

  using value_t = std::pair<point_t, HashedLaneInfoPtr>;
  using rtree = boost::geometry::index::rtree<value_t, boost::geometry::index::rstar<max_element_size> >;

 public:
  /**
   * @brief 清除哈希道路信息
   *
   * 这个函数清除当前哈希道路的信息，使其变为空。
   *
   */
  void Clear() TX_NOEXCEPT { m_rtree_hashedpoint.clear(); }

  /**
   * @brief 释放哈希道路信息
   *
   * 这个函数释放当前哈希道路的信息，使其变为空。
   */
  void Release() TX_NOEXCEPT { Clear(); }

  /**
   * @brief 获取哈希道路信息的大小
   *
   * 此函数返回当前哈希道路信息的大小，即哈希点的数量。
   *
   * @return 返回哈希道路信息的大小
   */
  Base::txSize Size() const TX_NOEXCEPT { return m_rtree_hashedpoint.size(); }

  /**
   * @brief 初始化函数
   *
   * 此函数用于初始化txHashedRoad类的实例。
   * 输入参数包括：
   *   - ref_lanesVec: 包含所有道路信息的集合
   *   - ref_lanelinksVec: 包含所有道路连接信息的向量
   *
   * @param ref_lanesVec 包含所有道路信息的集合
   * @param ref_lanelinksVec 包含所有道路连接信息的向量
   * @return 函数是否成功执行
   */
  Base::txBool Initialize(const hadmap::txLanes& ref_lanesVec, const hadmap::txLaneLinks& ref_lanelinksVec) TX_NOEXCEPT;

  /**
   * @brief 插入一个已经存在的HashedLaneInfo结构体到指定位置
   *
   * 将已经存在的HashedLaneInfo结构体插入到输入参数的坐标位置，并用指向该结构体的指针作为参数传递
   *
   * @param localPt_0 输入参数的坐标位置
   * @param _elem_ptr 指向要插入的HashedLaneInfo结构体的指针
   */
  void Insert(const Base::txVec2& localPt_0, HashedLaneInfoPtr _elem_ptr) TX_NOEXCEPT;

  /**
   * @brief 查找给定圆心周围包含的所有哈希点
   *
   * 此函数根据输入的中心点、循环顺时针封闭的localPts的顺序，在内部计算并找出哈希点，将结果存储在输出参数resultElementPtr中。
   *
   * @param centerPt 输入中心点
   * @param localPts_clockwise_close 输入循环顺时针封闭的localPts
   * @param resultElementPtr 输出参数，将在此函数中存储找到的哈希点
   * @return 成功查找哈希点返回true，否则返回false
   */
  Base::txBool FindHashedPointInArea(const Base::txVec2& centerPt,
                                     const std::vector<Base::txVec2>& localPts_clockwise_close,
                                     std::vector<HashedLaneInfoPtr>& resultElementPtr) TX_NOEXCEPT;

  /**
   * @brief 顺时针生成包含待处理的原始点vOriginal的一系列待处理点，并将结果存储在txVec2类型的std::vector中
   * @param vOriginal 需要在序列中包含的原始点
   * @param vAhead 待处理的点向量，顺时针排列
   * @return 返回一个std::vector<txVec2>类型的结果，其中包含了对应的待处理点
   */
  static std::vector<Base::txVec2> GenerateSearchClockWise(const Base::txVec2& vOriginal,
                                                           const Base::txVec3& vAhead) TX_NOEXCEPT;

  /**
   * @brief 搜索指定对象的长度
   *
   * 从文件 tx_hashed_road.h 中引入。
   * 搜索指定对象的长度。
   * @return txFloat 返回指定对象的长度
   * @retval FLAGS_DETECT_OBJECT_DISTANCE 默认值，指定对象的长度
   */
  static Base::txFloat SearchLength() TX_NOEXCEPT { return FLAGS_DETECT_OBJECT_DISTANCE; }

  /**
   * @brief 返回指定对象的宽度
   *
   * 来自文件: tx_hashed_road.h
   *
   * 该函数用于计算指定对象的宽度。
   *
   * @return Base::txFloat 返回指定对象的宽度
   * @retval FLAGS_default_lane_width 默认值，指定对象的宽度
   */
  static Base::txFloat SearchWidth() TX_NOEXCEPT { return 2.5 * FLAGS_default_lane_width; }

 protected:
  rtree m_rtree_hashedpoint;
  tbb::mutex tbbMutex_rtree_hashedpoint;
};

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)

TX_NAMESPACE_OPEN(HdMap)

class HashedRoadCacheConCurrent {
 public:
  using HashedLaneInfo = Geometry::SpatialQuery::HashedRoadNetworkRTree2D::HashedLaneInfo;
  using HashedLaneInfoPtr = Geometry::SpatialQuery::HashedRoadNetworkRTree2D::HashedLaneInfoPtr;
  using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  using hashedRoad2OrthogonalList =
      tbb::concurrent_hash_map<HashedLaneInfo, HashedLaneInfoOrthogonalListPtr, HashedLaneInfo::HashCompare>;
  using hashedRoad2CloseLaneEvent =
      tbb::concurrent_hash_map<HashedLaneInfo, std::set<Base::txSysId /*event_id*/>, HashedLaneInfo::HashCompare>;
  using hashedRoad2SurrouingHashedPt =
      tbb::concurrent_hash_map<HashedLaneInfo, std::list<HashedLaneInfo>, HashedLaneInfo::HashCompare>;

  using MeetingHashLaneInfoPtrList = std::list<HashedLaneInfoPtr>;
  using hashedRoad2MeetingHashedInfo =
      tbb::concurrent_hash_map<HashedLaneInfo, MeetingHashLaneInfoPtrList, HashedLaneInfo::HashCompare>;
  using hashedRoad2MeetingGPS = tbb::concurrent_hash_map<HashedLaneInfo, Coord::txENU, HashedLaneInfo::HashCompare>;

  using HashedLaneInfoSet = std::unordered_set<HashedLaneInfo, HashedLaneInfo::HashCompare>;
  using roadId2LaneLinkSetToThisRoadType =
      tbb::concurrent_hash_map<Base::txRoadID, HashedLaneInfoSet, Utils::RoadIdHashCompare>;
  using VehicleContainer = HashedLaneInfoOrthogonalList::VehicleContainer;
  using PedestrianContainer = HashedLaneInfoOrthogonalList::PedestrianContainer;
  using ObstacleContainer = HashedLaneInfoOrthogonalList::ObstacleContainer;

 public:
  /**
   * @brief 释放HashedRoadNetwork对象
   *
   * 用于释放分配给HashedRoadNetwork对象的内存资源，避免内存泄漏。
   *
   * @param 无参数
   * @return 无返回值
   * @exception 无异常
   */
  static void Release() TX_NOEXCEPT;

  /**
   * @brief 判断HashedRoad对象是否有效
   *
   * 判断HashedRoad对象是否已经初始化且具有有效的属性值。
   * 当对象无效时，某些操作可能无法正常进行。
   *
   * @return 如果对象有效，返回true；否则返回false。
   * @exception 无异常抛出
   */
  static Base::txBool IsValid() TX_NOEXCEPT { return sValid; }

  /**
   * @brief 清除注册元素操作
   *
   * 该方法用于注册清除元素操作。当没有被使用的元素被创建时，应该调用此方法将其注册到对应的元素池中。
   *
   * @param 无参数
   * @return 无返回值
   * @exception 无异常抛出
   */
  static void RegisterElementClear() TX_NOEXCEPT;

  /**
   * @brief 显示调试信息
   *
   * 此方法用于在控制台输出一些调试信息，例如汇编器的注册表和宏定义表。
   *
   * @param 无参数
   * @return 无返回值
   * @exception 无异常抛出
   */
  static void ShowDebugInfo() TX_NOEXCEPT;

  /**
   * @brief 向散列后的道路系统注册车辆
   *
   * 此方法用于向已经将道路散列后的系统注册车辆，提供车辆相关的信息和指针。
   *
   * @param _objInfo 散列后的道路信息
   * @param _veh_ptr 车辆对象指针，用于后续的车辆操作
   * @return 无返回值
   * @exception 无异常抛出
   */
  static void RegisterVehicle(const HashedLaneInfo& _objInfo, Base::IVehicleElementPtr _veh_ptr) TX_NOEXCEPT;

  /**
   * @brief 解除对哈希后的道路的注册车辆
   *
   * 此方法用于解除已经散列后的道路系统中对车辆的注册，从而使这个车辆的相关信息和所在道路进行解除关联。
   *
   * @param _objInfo 散列后的道路信息，表示要解除注册的道路。
   * @param _veh_sysId 步行者的系统ID，表示要解除注册的车辆。
   * @return 无返回值。
   * @exception 不抛出异常。
   */
  static void UnRegisterVehicle(const HashedLaneInfo& _objInfo, Base::txSysId _veh_sysId) TX_NOEXCEPT;

  /**
   * @brief 为指定的哈希道路注册步行者
   *
   * 此方法用于将步行者注册到已经散列后的道路系统中。
   * 注册步行者将使这个步行者的相关信息和所在道路进行关联。
   *
   * @param _objInfo 散列后的道路信息
   * @param _ped_ptr 表示步行者的交通元素指针
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void RegisterPedestrian(const HashedLaneInfo& _objInfo, Base::ITrafficElementPtr _ped_ptr) TX_NOEXCEPT;

  /**
   * @brief 解除对哈希后的道路的注册，使其不再接受步行者。
   *
   * 此方法用于解除已经哈希后的道路系统中对步行者的注册，从而使这个步行者的相关信息和所在道路进行解除关联。
   *
   * @param _objInfo 哈希后的道路信息，表示要解除注册的道路。
   * @param _ped_sysId 步行者的系统ID，表示要解除注册的步行者。
   * @return 无返回值。
   * @exception 不抛出异常。
   */
  static void UnRegisterPedestrian(const HashedLaneInfo& _objInfo, Base::txSysId _ped_sysId) TX_NOEXCEPT;

  /**
   * @brief 为指定的哈希道路注册某个障碍物。
   *
   * 此函数将指定的障碍物（通过 Base::ITrafficElementPtr 传入）与哈希道路信息相关联，并在道路上注册这个障碍物。
   *
   * @param _objInfo 哈希后的道路信息，表示要注册的道路。
   * @param _obs_ptr 障碍物的智能指针，用于标识要注册的障碍物。
   * @return 无返回值。
   * @exception 不抛出异常。
   */
  static void RegisterObstacle(const HashedLaneInfo& _objInfo, Base::ITrafficElementPtr _obs_ptr) TX_NOEXCEPT;

  /**
   * @brief 获取哈希道路上的所有车辆
   *
   * 此函数用于获取给定的哈希道路信息（通过 _objInfo 传入）中所有车辆的智能指针，存储在 refVecContainer 中。
   *
   * @param _objInfo 哈希后的道路信息，表示要获取车辆的道路。
   * @param refVecContainer 存储获取到的车辆智能指针的容器。
   * @return 无返回值。
   * @exception 不抛出异常。
   */
  static void GetHashedPtVehicles(const HashedLaneInfo& _objInfo, VehicleContainer& refVecContainer) TX_NOEXCEPT;

  /**
   * @brief 获取哈希道路上的所有行人
   *
   * 此函数用于获取给定的哈希道路信息（通过 _objInfo 传入）中所有行人的智能指针，存储在 refVecContainer 中。
   *
   * @param _objInfo 哈希后的道路信息，表示要获取行人的道路。
   * @param refVecContainer 存储获取到的行人智能指针的容器。
   * @return 无返回值。
   * @exception 不抛出异常。
   */
  static void GetHashedPtPedestrians(const HashedLaneInfo& _objInfo, PedestrianContainer& refVecContainer) TX_NOEXCEPT;

  /**
   * @brief 根据给定的哈希道路信息，生成所有的哈希点障碍物
   *
   * 此函数用于根据给定的哈希道路信息，生成所有的哈希点障碍物，并将这些障碍物存储在 refVecContainer 中。
   *
   * @param _objInfo 给定的哈希道路信息
   * @param refVecContainer 存储生成的哈希点障碍物的容器
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void GetHashedPtObstacles(const HashedLaneInfo& _objInfo, ObstacleContainer& refVecContainer) TX_NOEXCEPT;

  /**
   * @brief 取消注册所有元素
   *
   * 此函数用于取消注册所有已注册的元素。
   *
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void UngisterAllElements() TX_NOEXCEPT;

 public:
  /**
   * @brief 将相关的哈希道路信息添加到指定的哈希道路信息中
   *
   * 这个函数用于将指定的相关哈希道路信息添加到给定的哈希道路信息中。这些相关的哈希道路信息用于构建多重哈希道路。
   *
   * @param _objInfo 指定的哈希道路信息
   * @param vecRelatedPt 相关的哈希道路信息列表
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void AddRelatedHashedPt(const HashedLaneInfo& _objInfo,
                                 const std::list<HashedLaneInfo>& vecRelatedPt) TX_NOEXCEPT;

  /**
   * @brief 查询与给定哈希道路信息相关的其他哈希道路信息
   *
   * 该函数用于查询与给定哈希道路信息相关的其他哈希道路信息，并将这些相关哈希道路信息添加到 refRelativeHashedLanes
   * 列表中。
   *
   * @param _objInfo 指定的哈希道路信息
   * @param refRelativeHashedLanes 用于存储与 _objInfo 相关的哈希道路信息列表
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void QueryRelatedHashedPt(const HashedLaneInfo& _objInfo,
                                   std::list<HashedLaneInfo>& refRelativeHashedLanes) TX_NOEXCEPT;

  /**
   * @brief 查询与指定哈希道路信息相关的其他哈希道路信息，并将这些相关哈希道路信息添加到指定的车辆容器中。
   *
   * @param refRelativeHashedLanes 指定的哈希道路信息列表，包含了与指定哈希道路信息相关的哈希道路信息。
   * @param refVecContainer 指定的车辆容器，将存储与指定哈希道路信息相关的车辆信息。
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void QueryRegisterVehicles(const std::list<HashedLaneInfo>& refRelativeHashedLanes,
                                    VehicleContainer& refVecContainer) TX_NOEXCEPT;

  /**
   * @brief 查询与指定哈希道路信息相关的其他哈希道路信息，并将这些相关哈希道路信息添加到指定的车辆容器中。
   *
   * 此函数用于查询与指定哈希道路信息相关的其他哈希道路信息，并将这些相关哈希道路信息添加到指定的车辆容器中。
   *
   * @param refRelativeHashedLanes 指定的哈希道路信息列表，包含了与指定哈希道路信息相关的哈希道路信息。
   * @param refVecContainer 指定的车辆容器，将存储与指定哈希道路信息相关的车辆信息。
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void QueryRegisterVehicles(const HashedLaneInfoSet& refRelativeHashedLanes,
                                    VehicleContainer& refVecContainer) TX_NOEXCEPT;

  /**
   * @brief 查询指定哈希道路信息的周边车辆信息。
   *
   * 此函数用于查询指定哈希道路信息的周边车辆信息，并将结果存储在 refVecContainer 中。
   *
   * @param _objInfo 指定的哈希道路信息。
   * @param refVecContainer 存储周边车辆信息的容器。
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void QuerySurroundingVehicles(const HashedLaneInfo& _objInfo, VehicleContainer& refVecContainer) TX_NOEXCEPT;

 public: /*FBLR*/
  /**
   * @brief 添加一条对角线的车道信息至车道列表中
   *
   * 该函数会根据传入的哈希道路信息，将它对应的对角线车道信息加入车道列表中。
   *
   * @param _objInfo 指定的哈希道路信息
   * @return 无返回值
   * @exception 不抛出异常
   */
  static void Add_OrthogonalList(const HashedLaneInfo& _objInfo) TX_NOEXCEPT;

  /**
   * @brief 根据哈希道路信息查询对角线车道信息列表
   *
   * 根据指定的哈希道路信息，该函数会在内部数据结构中查找对应的对角线车道信息列表，并返回该列表。
   *
   * @param _objInfo 指定的哈希道路信息
   * @return 返回对角线车道信息列表的指针，如果未找到，则返回空指针
   * @exception 不抛出异常
   */
  static HashedLaneInfoOrthogonalListPtr Query_OrthogonalList(const HashedLaneInfo& _objInfo) TX_NOEXCEPT;

  /**
   * @brief 生成对角线车道信息列表
   *
   * 该函数用于生成对角线车道信息列表，将其存储在静态内部数据结构中，方便后续使用。
   *
   * @return 无
   * @exception 不抛出异常
   */
  static void Generate_OrthogonalList() TX_NOEXCEPT;

  /**
   * @brief 检查对角线车道信息列表
   *
   * 该函数用于检查内部存储的对角线车道信息列表，验证其正确性和完整性。
   *
   * @return 无
   * @exception 不抛出异常
   */
  static void Check_OrthogonalList() TX_NOEXCEPT;

  /**
   * @brief 检查对角线车道信息列表的左侧节点
   *
   * 该函数用于检查内部存储的对角线车道信息列表的左侧节点，验证其正确性和完整性。
   *
   * @param curOrthogonalListPtr 当前对角线车道信息列表指针
   * @return 无
   * @exception 不抛出异常
   */
  static void Check_OrthogonalList_LeftNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr) TX_NOEXCEPT;

  /**
   * @brief 检查对角线车道信息列表的右侧节点
   *
   * 该函数用于检查内部存储的对角线车道信息列表的右侧节点，验证其正确性和完整性。
   *
   * @param curOrthogonalListPtr 当前对角线车道信息列表指针
   * @return 无
   * @exception 不抛出异常
   */
  static void Check_OrthogonalList_RightNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr) TX_NOEXCEPT;

  /**
   * @brief 检查对角线车道信息列表的头节点
   *
   * 该函数用于检查内部存储的对角线车道信息列表的头节点，验证其正确性和完整性。
   *
   * @param curOrthogonalListPtr 当前对角线车道信息列表指针
   * @return 无
   * @exception 不抛出异常
   */
  static void Check_OrthogonalList_FrontNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr) TX_NOEXCEPT;

  /**
   * @brief 检查对角线车道信息列表的后节点
   *
   * 该函数用于检查内部存储的对角线车道信息列表的后节点，验证其正确性和完整性。
   *
   * @param curOrthogonalListPtr 当前对角线车道信息列表指针
   * @return 无
   * @exception 不抛出异常
   */
  static void Check_OrthogonalList_BackNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr) TX_NOEXCEPT;

 public:
  /**
   * @brief 注册关闭车道事件
   *
   * 此函数用于注册关闭车道事件，该事件在相应的车道处关闭车流。
   * @param eventId 事件ID
   * @param hashedPtVec 散列车道信息向量
   */
  static void RegisterCloseLaneEvent(const Base::txSysId eventId,
                                     const std::vector<HashedLaneInfo>& hashedPtVec) TX_NOEXCEPT;

  /**
   * @brief 取消注册关闭车道事件
   *
   * 此函数用于取消注册关闭车道事件，该事件在相应的车道处关闭车流。
   *
   * @param eventId 事件ID
   */
  static void UnRegisterCloseLaneEvent(const Base::txSysId eventId) TX_NOEXCEPT;

  /**
   * @brief 判断车道是否关闭
   *
   * 此函数用于判断指定的车道是否关闭，关闭后车辆将无法通过该车道。
   *
   * @param hashedPt 哈希后的车道信息
   * @return 返回一个布尔值，表示车道是否关闭
   */
  static Base::txBool IsCloseLane(const HashedLaneInfo& hashedPt) TX_NOEXCEPT;

  /**
   * @brief 检查指定哈希车道信息的车道是否关闭
   *
   * 该函数用于检查指定哈希车道信息的车道是否关闭。关闭后，车辆将无法通过该车道。
   *
   * @param hashedPt 哈希后的车道信息
   * @param eventSet 事件集合，用于存储关闭车道的事件 ID
   * @return 返回一个布尔值，表示车道是否关闭
   */
  static Base::txBool IsCloseLane(const HashedLaneInfo& hashedPt,
                                  std::set<Base::txSysId /*event_id*/>& eventSet) TX_NOEXCEPT;

  static void IteratorOrthogonalList(const HashedRoadCacheConCurrent::HashedLaneInfoOrthogonalListPtr _nodePtr,
                                     const Base::txInt _level,
                                     std::list<HashedRoadCacheConCurrent::HashedLaneInfo>& pt_list,
                                     const Base::txBool isFront) TX_NOEXCEPT;

 public:
  /**
   * @brief 获取指定道路ID的车道连接到该道路的车道信息
   *
   * 这个函数根据给定的道路ID，返回包含车道连接信息的容器。车道连接信息包括连接车道的道路ID、连接车道的索引、连接车道的类型等。
   *
   * @param _roadId 要获取车道连接信息的道路ID
   * @return roadId2LaneLinkSetToThisRoadType::value_type::second_type 包含车道连接信息的容器
   */
  static roadId2LaneLinkSetToThisRoadType::value_type::second_type GetLaneLinksToRoad(const Base::txRoadID _roadId)
      TX_NOEXCEPT;

  /**
   * @brief 将给定的车道连接路径信息添加到指定道路ID的HashedLaneInfo数据结构中
   *
   * 这个函数为给定的道路ID添加车道连接路径信息，这些车道连接路径信息包含了相关车道的索引、类型、偏移量等信息。
   *
   * @param _roadId 目标道路ID
   * @param HashedLaneInfo 包含车道连接路径信息的数据结构
   */
  static void Add_LaneLinkLocInfoToRoad(const Base::txRoadID _roadId, const HashedLaneInfo) TX_NOEXCEPT;

#  if __Meeting__

 public:
  /**
   * @brief AddMeetingHashedInfo 在给定的哈希网格路径上添加新的交叉感知信息
   * @param _objInfo 要添加的交叉感知信息
   * @param listMeetingHashedInfoPtr 存储交叉感知信息的指针列表
   * @param meetingPt 交叉感知信息的坐标点
   */
  static void AddMeetingHashedInfo(const HashedLaneInfo& _objInfo,
                                   const MeetingHashLaneInfoPtrList& listMeetingHashedInfoPtr,
                                   const Coord::txENU meetingPt) TX_NOEXCEPT;

  /**
   * @brief 查询指定HashedLaneInfo的相邻车道MeetingHashLaneInfo
   *
   * 该函数根据给定的HashedLaneInfo信息，查询与之相邻的车道MeetingHashLaneInfo信息，并将其存储在refMeetingHashedLanePtrList中。
   *
   * @param[in] _objInfo 指定的HashedLaneInfo
   * @param[out] refMeetingHashedLanePtrList 存储与指定HashedLaneInfo相邻的车道MeetingHashLaneInfo的指针列表
   * @return 无返回值
   * @retval TX_NOEXCEPT 该函数不会抛出异常
   */
  static void QueryMeetingHashedInfo(const HashedLaneInfo& _objInfo,
                                     MeetingHashLaneInfoPtrList& refMeetingHashedLanePtrList) TX_NOEXCEPT;

  /**
   * @brief 根据给定的车道信息，查询当前相邻车道的交通信息
   *
   * 该函数根据给定的 HashedLaneInfo 对象，查询当前相邻的交通信息（包括车辆信息和行人信息），并将其存储在
   * refVecContainer 中。
   *
   * @param[in] _objInfo 给定的车道信息
   * @param[out] refVecContainer 存储查询结果的交通信息容器
   * @return 无返回值
   * @retval TX_NOEXCEPT 该函数不会抛出异常
   */
  static void QueryMeetingVehicles(const HashedLaneInfo& _objInfo, VehicleContainer& refVecContainer) TX_NOEXCEPT;

  /**
   * @brief 查询给定车道信息下的车辆信息
   *
   * 该函数用于根据传入的车道信息，查询所有与该车道相邻的车辆信息，并将结果存储在 refVecContainer 中。
   *
   * @param[in] _objInfo 传入的车道信息
   * @param[out] refVecContainer 存储查询结果的车辆信息容器
   * @return 无返回值
   */
  static void QueryRegisterVehicles(const MeetingHashLaneInfoPtrList& refRelativeHashedLanePtrList,
                                    VehicleContainer& refVecContainer) TX_NOEXCEPT;

  /**
   * @brief 检查给定的车道信息是否在遇见区域内
   *
   * 遇见区域的定义为车道内各个道口之间的交叉区域，该函数用于判断给定的车道信息是否在遇见区域内。
   *
   * @param[in] _objInfo 待检查的车道信息
   * @param[out] meetingPt 遇见区域内遇到的交叉点坐标
   * @return 如果给定的车道信息在遇见区域内，返回 true，否则返回 false
   */
  static Base::txBool IsInMeetingRegion(const HashedLaneInfo& _objInfo, Coord::txENU& meetingPt) TX_NOEXCEPT;

  /**
   * @brief 检查给定的车道信息是否在遇见区域内
   *
   * 遇见区域的定义为车道内各个道口之间的交叉区域，该函数用于判断给定的车道信息是否在遇见区域内。
   *
   * @param[in] _objInfo 待检查的车道信息
   * @return 如果给定的车道信息在遇见区域内，返回 true，否则返回 false
   */
  static Base::txBool IsInMeetingRegion(const HashedLaneInfo& _objInfo) TX_NOEXCEPT;

  /**
   * @brief 生成遇见区域
   *
   * 遇见区域是指两个交叉口之间的区域。通过调用该函数，可以生成遇见区域。
   *
   * @return void
   */
  static void Generate_Meeting() TX_NOEXCEPT;
#  endif /*__Meeting__*/

 protected:
  static Base::txBool sValid;
  static hashedRoad2OrthogonalList s_hashedPt2OrthogonalListPtr;
  static hashedRoad2CloseLaneEvent s_close_lane_event;
  static hashedRoad2SurrouingHashedPt s_hashedPt2RelatedHashedPts;
  static roadId2LaneLinkSetToThisRoadType s_lanelinkSetToRoadId;
#  if __Meeting__
  static hashedRoad2MeetingHashedInfo s_hashed2MeetingHashedInfo;
  static hashedRoad2MeetingGPS s_hashed2MeetingGPS;
#  endif /*__Meeting__*/
};

TX_NAMESPACE_CLOSE(HdMap)

#endif /*USE_HashedRoadNetwork*/

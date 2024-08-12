// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tbb/tbb.h>
#include "HdMap/tx_hashed_lane_info.h"
#include "tx_class_counter.h"
#include "tx_vehicle_element.h"
#if USE_HashedRoadNetwork

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

class HashedLaneInfoOrthogonalList;
using HashedLaneInfoOrthogonalListPtr = std::shared_ptr<HashedLaneInfoOrthogonalList>;
using Weak_HashedLaneInfoOrthogonalListPtr = std::weak_ptr<HashedLaneInfoOrthogonalList>;
class HashedLaneInfoOrthogonalList {
  Utils::Counter<HashedLaneInfoOrthogonalList> _c;

 public:
  using AdjoinContainerType =
      std::unordered_map<HashedLaneInfo, HashedLaneInfoOrthogonalListPtr, HashedLaneInfo::HashCompare>;
  using WeakAdjoinContainerType =
      std::unordered_map<HashedLaneInfo, Weak_HashedLaneInfoOrthogonalListPtr, HashedLaneInfo::HashCompare>;
  using VehicleContainer = std::map<Base::txSysId, Base::IVehicleElementPtr>;
  using PedestrianContainer = std::map<Base::txSysId, Base::ITrafficElementPtr>;
  using ObstacleContainer = std::map<Base::txSysId, Base::ITrafficElementPtr>;

 public:
  explicit HashedLaneInfoOrthogonalList(const HashedLaneInfo& self_info) : m_self_hashedLaneInfo(self_info) {}
  virtual ~HashedLaneInfoOrthogonalList() { Clear(); }
  /**
   * @brief 清除动态元素信息
   *
   * 该方法会清除当前 HashedLaneInfoOrthogonalList 实例中所有动态元素的信息。
   *
   * @return void 无返回值
   */
  void ClearDynamicElementInfo() TX_NOEXCEPT {
    {
      tbb::mutex::scoped_lock lock(tbbMutex_vehicles);
      m_map_vehicles.clear();
    }
    {
      tbb::mutex::scoped_lock lock(tbbMutex_pedestrians);
      m_map_pedestrians.clear();
    }
    {
      tbb::mutex::scoped_lock lock(tbbMutex_obstacles);
      m_map_obstacles.clear();
    }
  }

  /**
   * @brief 清除当前 HashedLaneInfoOrthogonalList 的列表
   *
   * 这个方法会清除当前 HashedLaneInfoOrthogonalList 实例中存储的所有具有对称性的元素。
   *
   * @return 无返回值
   */
  void ClearOrthogonalList() TX_NOEXCEPT {
    m_left_node_list.reset();
    m_right_node_list.reset();
    m_front_node_list.clear();
    m_back_node_list.clear();
  }

  /**
   * @brief 清除当前 HashedLaneInfoOrthogonalList 的所有具有对称性的元素
   *
   * 这个方法用于清除当前 HashedLaneInfoOrthogonalList 实例中所有具有对称性的元素，以便重新使用该实例。
   *
   * @return 无返回值
   */
  void Clear() TX_NOEXCEPT {
    m_self_hashedLaneInfo.SetValid(false);
    ClearDynamicElementInfo();
    ClearOrthogonalList();
  }

  /**
   * @brief 获取当前哈希表中的所有车辆信息
   *
   * 此函数用于获取当前哈希表中所有车辆的信息，并将其存储在一个与输入参数对应的容器中。
   *
   * @param[out] refVehicleContainer 一个指向包含所有车辆信息的容器的引用
   * @return 无返回值
   */
  void GetRegisterVehicles(VehicleContainer& refVehicleContainer) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_vehicles);
    refVehicleContainer = m_map_vehicles;
  }

  /**
   * @brief 获取当前哈希表中的所有行人信息
   *
   * 此函数用于获取当前哈希表中所有行人的信息，并将其存储在一个与输入参数对应的容器中。
   *
   * @param[out] refPedestrianContainer 一个指向包含所有行人信息的容器的引用
   * @return 无返回值
   */
  void GetRegisterPedestrians(PedestrianContainer& refPedestrianContainer) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_pedestrians);
    refPedestrianContainer = m_map_pedestrians;
  }

  /**
   * @brief 获取当前哈希表中的所有障碍物信息
   *
   * 此函数用于获取当前哈希表中所有障碍物的信息，并将其存储在一个与输入参数对应的容器中。
   *
   * @param[out] refObstacleContainer 一个指向包含所有障碍物信息的容器的引用
   * @return 无返回值
   */
  void GetRegisterObstacles(ObstacleContainer& refObstacleContainer) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_obstacles);
    refObstacleContainer = m_map_obstacles;
  }

  /**
   * @brief 将一辆车注册到哈希表中
   *
   * 该函数将一辆车的指针注册到哈希表中，用于后续操作。该函数适用于具有唯一系统ID的车辆。
   *
   * @param _veh_ptr 要注册的车辆指针
   */
  void RegisterVehicle(Base::IVehicleElementPtr _veh_ptr) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_vehicles);
    m_map_vehicles[_veh_ptr->SysId()] = _veh_ptr;
  }

  /**
   * @brief 注册行人
   *
   * 此函数注册一个行人对象到哈希表中，该对象用于后续操作。行人的唯一标识符被用作键。
   *
   * @param _ped_ptr 要注册的行人对象指针
   */
  void RegisterPedestrian(Base::ITrafficElementPtr _ped_ptr) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_pedestrians);
    m_map_pedestrians[_ped_ptr->SysId()] = _ped_ptr;
  }

  /**
   * @brief 注册障碍物
   *
   * 将一个障碍物对象注册到哈希表中，用于后续操作。障碍物的唯一标识符被用作键。
   *
   * @param _obs_ptr 要注册的障碍物对象指针
   */
  void RegisterObstacle(Base::ITrafficElementPtr _obs_ptr) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_obstacles);
    m_map_obstacles[_obs_ptr->SysId()] = _obs_ptr;
  }

  /**
   * @brief 取消注册车辆
   *
   * 从哈希表中移除指定车辆的信息。
   *
   * @param _vehId 车辆的唯一标识符
   */
  void UnRegisterVehicle(Base::txSysId _vehId) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_vehicles);
    auto itr = m_map_vehicles.find(_vehId);
    if (m_map_vehicles.end() != itr) {
      m_map_vehicles.erase(itr);
    }
  }

  /**
   * @brief 注销行人
   *
   * 从哈希表中移除指定行人的信息。
   *
   * @param _vehId 行人的唯一标识符
   */
  void UnRegisterPedestrian(Base::txSysId _vehId) TX_NOEXCEPT {
    tbb::mutex::scoped_lock lock(tbbMutex_pedestrians);
    auto itr = m_map_pedestrians.find(_vehId);
    if (m_map_pedestrians.end() != itr) {
      m_map_pedestrians.erase(itr);
    }
  }

  /**
   * @brief 设置左侧节点列表
   *
   * 设置当前节点的左侧节点列表指针，并将左侧节点列表设置为 s_ptr。
   *
   * @param s_ptr 左侧节点列表指针
   */
  void SetLeftNode(HashedLaneInfoOrthogonalListPtr s_ptr) TX_NOEXCEPT { m_left_node_list = s_ptr; }

  /**
   * @brief 设置右侧节点列表
   *
   * 设置当前节点的右侧节点列表指针，并将右侧节点列表设置为 s_ptr。
   *
   * @param s_ptr 右侧节点列表指针
   */
  void SetRightNode(HashedLaneInfoOrthogonalListPtr s_ptr) TX_NOEXCEPT { m_right_node_list = s_ptr; }

  /**
   * @brief 设置前面的节点列表
   *
   * 该函数用于设置当前节点的前面节点列表。
   *
   * @param sharedPtrMap 前面节点列表的共享指针容器
   */
  void SetFrontNodes(AdjoinContainerType sharedPtrMap) TX_NOEXCEPT;

  /**
   * @brief 设置当前节点的后面节点列表
   *
   * 该函数用于设置当前节点的后面节点列表。
   *
   * @param sharedPtrMap 后面节点列表的共享指针容器
   */
  void SetBackNodes(AdjoinContainerType sharedPtrMap) TX_NOEXCEPT;

  /**
   * @brief 获取当前节点的左侧节点列表
   *
   * 这个函数用于获取当前节点的左侧节点列表。
   *
   * @return 返回左侧节点列表的指针
   */
  HashedLaneInfoOrthogonalListPtr GetLeftNode() const TX_NOEXCEPT;

  /**
   * @brief 获取当前节点的右侧节点列表
   *
   * 该函数用于获取当前节点的右侧节点列表。
   *
   * @return 右侧节点列表的指针
   */
  HashedLaneInfoOrthogonalListPtr GetRightNode() const TX_NOEXCEPT;

  /**
   * @brief 获取当前节点的前面节点列表
   *
   * 该函数用于获取当前节点的前面节点列表。
   *
   * @return 前面节点列表的指针
   */
  AdjoinContainerType GetFrontNodes() const TX_NOEXCEPT;

  /**
   * @brief 获取当前节点的后面节点列表
   *
   * 该函数用于获取当前节点的后面节点列表。
   *
   * @return 后面节点列表的指针
   */
  AdjoinContainerType GetBackNodes() const TX_NOEXCEPT;

  /**
   * @brief 获取当前对象的哈希信息
   *
   * 获取当前对象的哈希信息，通常用于输出或者打印信息
   *
   * @return 返回当前对象的哈希信息
   */
  const HashedLaneInfo& GetSelfHashedInfo() const TX_NOEXCEPT { return m_self_hashedLaneInfo; }

  /**
   * @brief 将当前的 HashedLaneInfoOrthogonalList 转换成字符串表示
   *
   * 该函数将当前的 HashedLaneInfoOrthogonalList 转换成字符串表示，方便输出或者打印
   *
   * @return 返回一个字符串，表示当前的 HashedLaneInfoOrthogonalList
   */
  Base::txString Str() const TX_NOEXCEPT;
  friend std::ostream& operator<<(std::ostream& os, const HashedLaneInfoOrthogonalList& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }

 public:
  using txSurroundVehiclefo = Base::txSurroundVehiclefo;
  struct OrthogonalListSearchCmd {
    /**
     * @brief 判断当前对象是否有效
     *
     * 判断当前对象是否有效，如果有效则返回 true，否则返回 false。
     * 该对象有效的条件是指针非空，原始距离值在有效范围内，有效距离值大于 0，长度值大于等于 0。
     *
     * @return 如果当前对象有效则返回 true，否则返回 false。
     */
    Base::txBool IsValid() const TX_NOEXCEPT {
      return NonNull_Pointer(curNodePtr) && original_s_in_node >= 0.0 && valid_dist_along_original > 0.0 &&
             length_in_node >= 0.0;
    }
    HashedLaneInfoOrthogonalListPtr curNodePtr = nullptr;
    Base::txFloat original_s_in_node = 0.0;
    Base::txFloat length_in_node = 0.0;
    Base::txFloat valid_dist_along_original = 0.0;
    Base::txFloat raw_original_search_dist = 0.0;

    /**
     * @brief Set the original S value in HashedLaneInfoOrthogonalList object.
     *
     * This function sets the original S value in the HashedLaneInfoOrthogonalList object.
     *
     * @param _s The new value of original S.
     * @return OrthogonalListSearchCmd& A reference to the same object, allowing for chaining.
     */
    OrthogonalListSearchCmd& SetOrthogonalListPtr(HashedLaneInfoOrthogonalListPtr _ptr) TX_NOEXCEPT {
      curNodePtr = _ptr;
      return (*this);
    }

    /**
     * @brief Set the original S value in the HashedLaneInfoOrthogonalList object.
     *
     * This function sets the original S value in the HashedLaneInfoOrthogonalList object.
     *
     * @param _s The new value of original S.
     * @return OrthogonalListSearchCmd& A reference to the same object, allowing for chaining.
     */
    OrthogonalListSearchCmd& SetOriginalS(const Base::txFloat _s) TX_NOEXCEPT {
      original_s_in_node = _s;
      return (*this);
    }

    /**
     * @brief 设置node的长度
     *
     * @param _dist 所要设置的长度
     * @return OrthogonalListSearchCmd& 返回当前对象
     */
    OrthogonalListSearchCmd& SetNodeLength(const Base::txFloat _dist) TX_NOEXCEPT {
      length_in_node = _dist;
      return (*this);
    }

    /**
     * @brief Set the valid distance along the base node of the hashed lane info orthogonal list object.
     *
     * This function sets the valid distance along the base node of the hashed lane info orthogonal list object.
     *
     * @param _dist The new valid distance value along the base node.
     * @return OrthogonalListSearchCmd& A reference to the same object, allowing for chaining.
     */
    OrthogonalListSearchCmd& SetValidDistanceBaseNode(const Base::txFloat _dist) TX_NOEXCEPT {
      valid_dist_along_original = _dist;
      return (*this);
    }

    /**
     * @brief 设置原始搜索距离
     *
     * @param _dist 要设置的距离
     * @return OrthogonalListSearchCmd& 返回当前对象的引用
     */
    OrthogonalListSearchCmd& SetRawSearchDist(const Base::txFloat _dist) TX_NOEXCEPT {
      raw_original_search_dist = _dist;
      return (*this);
    }

    /**
     * @brief HashedLaneInfoOrthogonalList 的字符串表示形式
     *
     * 此函数返回 HashedLaneInfoOrthogonalList 的字符串表示形式。
     * @return 一个包含 HashedLaneInfoOrthogonalList 信息的字符串
     */
    Base::txString Str() const TX_NOEXCEPT {
      std::ostringstream oss;
      oss << TX_VARS(curNodePtr->GetSelfHashedInfo().Str()) << TX_VARS(original_s_in_node) << TX_VARS(length_in_node)
          << TX_VARS(valid_dist_along_original) << TX_VARS(raw_original_search_dist);
      return oss.str();
    }
  };

  /**
   * @brief 在同一坐标系下，搜索指定车辆在同一坐标系下存在的第一个车辆
   *
   * @param elemSysId 车辆的系统ID，用于区分同一车辆的不同端口
   * @param elemId 车辆的ID
   * @param elemLength 车辆的长度
   * @param cmd 搜索指令
   * @param res_gap_vehPtr 搜索到的车辆对象
   *
   * @return 如果搜索成功，则返回 true，否则返回 false
   */
  static Base::txBool SearchOrthogonalListFront(const Base::txSysId elemSysId, const Base::txSysId elemId,
                                                const Base::txFloat elemLength, const OrthogonalListSearchCmd& cmd,
                                                txSurroundVehiclefo& res_gap_vehPtr) TX_NOEXCEPT;

  /**
   * @brief 在指定的车辆信息列表中搜索最接近前方车辆的那个车辆信息
   *
   * @param elemSysId 车辆的系统ID，用于区分同一车辆的不同端口
   * @param elemId 车辆的ID
   * @param elemLength 车辆的长度
   * @param selfStableHashedLaneInfo 当前车辆的哈希车道信息
   * @param center_node_ptr 要搜索的车辆信息列表的中心节点
   * @param resQueryInfo 搜索到的最接近前方车辆的车辆信息
   * @param search_distance 搜索的范围，单位：米
   *
   * @return 如果搜索成功，则返回 true，否则返回 false
   */
  static void SearchNearestFrontElement(const Base::txSysId elemSysId, const Base::txSysId elemId,
                                        const Base::txFloat elemLength, const HashedLaneInfo& selfStableHashedLaneInfo,
                                        const HashedLaneInfoOrthogonalListPtr& center_node_ptr,
                                        txSurroundVehiclefo& resQueryInfo,
                                        const Base::txFloat search_distance) TX_NOEXCEPT;

  /**
   * @brief 根据给定的条件，在哈希后的车道信息列表中搜索最近的车辆
   *
   * @param elemSysId 目标车辆的系统ID
   * @param elemId 目标车辆的ID
   * @param elemLength 目标车辆的长度
   * @param cmd 搜索条件
   * @param res_gap_vehPtr 搜索结果，包括最近车辆的信息
   *
   * @return 如果搜索成功，则返回 true，否则返回 false
   */
  static Base::txBool SearchOrthogonalListRear(const Base::txSysId elemSysId, const Base::txSysId elemId,
                                               const Base::txFloat elemLength, const OrthogonalListSearchCmd& cmd,
                                               txSurroundVehiclefo& res_gap_vehPtr) TX_NOEXCEPT;

  /**
   * @brief 根据给定的条件，在哈希后的车道信息列表中搜索最近的尾部车辆
   *
   * @param elemSysId 目标车辆的系统ID
   * @param elemId 目标车辆的ID
   * @param elemLength 目标车辆的长度
   * @param selfStableHashedLaneInfo 哈希后的车道信息列表
   * @param center_node_ptr 目标车辆所在的哈希车道信息节点
   * @param resQueryInfo 搜索结果，包括最近车辆的信息
   * @param search_distance 搜索距离
   *
   * @return 如果搜索成功，则返回 true，否则返回 false
   */
  static void SearchNearestRearElement(const Base::txSysId elemSysId, const Base::txSysId elemId,
                                       const Base::txFloat elemLength, const HashedLaneInfo& selfStableHashedLaneInfo,
                                       const HashedLaneInfoOrthogonalListPtr& center_node_ptr,
                                       txSurroundVehiclefo& resQueryInfo,
                                       const Base::txFloat search_distance) TX_NOEXCEPT;

  /**
   * @brief 搜索给定哈希车道信息节点中的旋转相邻车辆信息
   *
   * @param center_node_ptr 中心车辆哈希车道信息节点的指针
   * @param post_search_distance 搜索距离
   * @param results 存储搜索结果的容器
   *
   * @return 若搜索成功，则返回 true，否则返回 false
   */
  static Base::txBool SearchOrthogonalListPost(const HashedLaneInfoOrthogonalListPtr& center_node_ptr,
                                               const Base::txFloat post_search_distance,
                                               AdjoinContainerType& results) TX_NOEXCEPT;

  /**
   * @brief 搜索旋转相邻车辆信息
   *
   * @param center_node_ptr 待搜索的哈希车道信息节点的指针
   * @param post_search_distance 搜索距离
   * @param results 存储搜索结果的容器
   * @param checkOp 用于过滤搜索结果的函数，以确定是否满足搜索条件
   *
   * @return 若搜索成功，则返回 true，否则返回 false
   */
  static Base::txBool SearchOrthogonalListPost(const HashedLaneInfoOrthogonalListPtr& center_node_ptr,
                                               const Base::txFloat post_search_distance, AdjoinContainerType& results,
                                               std::function<Base::txBool(const Base::txRoadID)> checkOp) TX_NOEXCEPT;

  /**
   * @brief 搜索给定中心节点的旋转相邻头部车辆信息
   *
   * @param center_node_ptr 待搜索的哈希车道信息节点的指针
   * @param front_search_distance 搜索距离
   * @param results 存储搜索结果的容器
   * @return 若搜索成功，则返回 true，否则返回 false
   */
  static Base::txBool SearchOrthogonalListFront(const HashedLaneInfoOrthogonalListPtr& center_node_ptr,
                                                const Base::txFloat front_search_distance,
                                                AdjoinContainerType& results) TX_NOEXCEPT;

  /**
   * @brief 按级别搜索给定中心节点的旋转相邻车辆信息
   *
   * @param center_node_ptr 待搜索的哈希车道信息节点的指针
   * @param front_search_distance 搜索距离
   * @param level_idx 搜索的级别
   * @param results_by_level 存储搜索结果的容器，根据级别进行分类
   * @return 若搜索成功，则返回 true，否则返回 false
   */
  static Base::txBool SearchOrthogonalListFrontByLevel(
      const HashedLaneInfoOrthogonalListPtr& center_node_ptr, const Base::txFloat front_search_distance,
      const Base::txInt level_idx, std::vector<HashedLaneInfoOrthogonalListPtr>& results_by_level) TX_NOEXCEPT;

 protected:
  HashedLaneInfo m_self_hashedLaneInfo;
  Weak_HashedLaneInfoOrthogonalListPtr m_left_node_list;
  Weak_HashedLaneInfoOrthogonalListPtr m_right_node_list;
  WeakAdjoinContainerType m_front_node_list;
  WeakAdjoinContainerType m_back_node_list;

  VehicleContainer m_map_vehicles;
  PedestrianContainer m_map_pedestrians;
  ObstacleContainer m_map_obstacles;
  tbb::mutex tbbMutex_vehicles;
  tbb::mutex tbbMutex_pedestrians;
  tbb::mutex tbbMutex_obstacles;
};

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)

#endif /*USE_HashedRoadNetwork*/

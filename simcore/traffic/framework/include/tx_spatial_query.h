// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tbb/tbb.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/iterator/function_output_iterator.hpp>
#include <set>
#include <vector>
#include "HdMap/tx_lane_geom_info.h"
#include "tbb/concurrent_vector.h"
#include "tx_header.h"
#include "tx_signal_element.h"
#include "tx_traffic_element_base.h"

#define _CloseRTree2dSignal_ (0)
#define _UseWeakSignal_ (!_CloseRTree2dSignal_)
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

#define _UseInsert_ (0)
class RTree2DLite {
 public:
  struct RTree2DLiteNode {
   public:
    /**
     * @brief txSpatialQuery 节点类型枚举
     */
    enum class NodeType : Base::txInt { eObstacle = 0, eVehicleInput, eSignal };

    /**
     * @brief Construct a new RTree2DLiteNode object
     *
     * @param _t The type of the node
     * @param _l The lane information of the node
     * @param _enu2d The ENU space coordinate of the node
     */
    RTree2DLiteNode(NodeType _t, Base::Info_Lane_t _l, Base::txVec2 _enu2d) : mType(_t), mLocInfo(_l), mEnu2d(_enu2d) {}

    /**
     * @brief GetType 获取节点类型
     * @return NodeType 节点类型
     */
    NodeType GetType() const TX_NOEXCEPT { return mType; }

    /**
     * @brief GetLocInfo 获取位置信息
     * @return const Info_Lane_t& 位置信息
     */
    const Base::Info_Lane_t &GetLocInfo() const TX_NOEXCEPT { return mLocInfo; }

    /**
     * @brief 获取ENU空间中的2D坐标
     *
     * @return Base::txVec2 返回ENU空间中的2D坐标
     */
    Base::txVec2 GetEnu2D() const TX_NOEXCEPT { return mEnu2d; }

   protected:
    NodeType mType;
    Base::Info_Lane_t mLocInfo;
    Base::txVec2 mEnu2d;
  };
  using RTree2DLiteNodePtr = std::shared_ptr<RTree2DLiteNode>;

 public:
  enum { max_element_size = 16 };
  using point_t = boost::geometry::model::point<Base::txFloat, 2, boost::geometry::cs::cartesian>;
  using box_t = boost::geometry::model::box<point_t>;
  using poly_t = boost::geometry::model::polygon<point_t>;
  TX_MARK("outer is clock-wise.");
  using value_t = std::pair<point_t, RTree2DLiteNodePtr>;
  using rtree = boost::geometry::index::rtree<value_t, boost::geometry::index::rstar<max_element_size>>;

 public:
  struct circle_filter_squared {
    using point_t = RTree2DLite::point_t;
    template <typename Value>
    bool operator()(Value v) const {
      const point_t elemPoint = v.first;
      const Base::txVec2 center2d = Base::txVec2(circle_center.get<0>(), circle_center.get<1>());
      const Base::txVec2 elem2d = Base::txVec2(elemPoint.get<0>(), elemPoint.get<1>());
      const Base::txFloat squared_dist = (center2d - elem2d).squaredNorm();
      const Base::Info_Lane_t &surround_elem_locInfo = v.second->GetLocInfo();
      if ((squared_dist < squared_radius) &&
          ((surround_elem_locInfo.onLaneUid.roadId == center_locInfo.onLaneUid.roadId) ||
           (RTree2DLiteNode::NodeType::eVehicleInput == v.second->GetType()))) {
        return true;
      } else {
        return false;
      }
    }

    circle_filter_squared(const point_t &_circle_center, const Base::txFloat _squared_radius,
                          const Base::Info_Lane_t _locInfo)
        : circle_center(_circle_center), squared_radius(_squared_radius), center_locInfo(_locInfo) {}

   protected:
    const point_t &circle_center;
    const Base::txFloat squared_radius;
    const Base::Info_Lane_t center_locInfo;
  };

 public:
  RTree2DLite() TX_DEFAULT;
  ~RTree2DLite() { Clear(); }
  void Clear() TX_NOEXCEPT;

  /**
   * @brief 将一个点注册到指定的空间索引结构中
   *
   * @param localPt 指定的坐标点
   * @param _type 注册的点的类型
   * @param _locInfo 指定点的汽车信息
   */
  void RegisterPoint(const Base::txVec2 &localPt, RTree2DLiteNode::NodeType _type,
                     Base::Info_Lane_t _locInfo) TX_NOEXCEPT;

  /**
   * @brief 根据给定的圆心和半径在空间索引中查找圆内的元素
   *
   * @param centerPt 圆心坐标
   * @param locInfo 点的相关信息，例如高速公路上的车道号
   * @param CircleRadius 圆的半径
   * @param resultElementPtr 存储查找到的圆内元素指针的向量
   *
   * @return true 表示找到至少一个符合条件的元素
   * @return false 表示圆内没有符合条件的元素
   */
  Base::txBool FindElementsInCircle(const Base::txVec2 &centerPt, const Base::Info_Lane_t locInfo,
                                    const Base::txFloat _CircleRadius,
                                    std::vector<RTree2DLiteNodePtr> &resultElementPtr) TX_NOEXCEPT;

  /**
   * @brief 根据给定的圆心和半径查找圆内的元素数量
   *
   * @param centerPt 圆心坐标
   * @param locInfo 点的相关信息，例如高速公路上的车道号
   * @param CircleRadius 圆的半径
   *
   * @return Base::txSize 返回圆内元素数量
   */
  Base::txSize CountElementInCircle(const Base::txVec2 &centerPt, const Base::Info_Lane_t locInfo,
                                    const Base::txFloat _CircleRadius) TX_NOEXCEPT;

 protected:
  rtree bg_tree;
#if USE_TBB
  tbb::mutex tbbMutex_bg_tree;
#endif
};

class RTree2D {
 public:
  /**
   * @brief 获取单例对象的引用
   *
   * @return RTree2D& 返回 RTree2D 类型的单例对象的引用
   */
  static RTree2D &getInstance() {
    static RTree2D instance;
    return instance;
  }

 private:
  RTree2D();
  ~RTree2D();
  RTree2D(const RTree2D &) TX_DELETE;
  RTree2D &operator=(const RTree2D &) TX_DELETE;

 protected:
  struct RTree2DImpl;
  std::shared_ptr<RTree2DImpl> pImpl;

 protected:
  using point_t = boost::geometry::model::point<Base::txFloat, 2, boost::geometry::cs::cartesian>;
  using value_t = std::pair<point_t, Base::ITrafficElementPtr>;
  static tbb::concurrent_vector<value_t> conVec_bulk_element_info;

 public:
  /**
   * 释放大批量对象.
   *
   * 当需要一次性释放大量对象时，可以使用此函数，以提高性能。
   *
   * @return void 不返回任何值，仅释放对象.
   */
  static void ReleaseBulk() TX_NOEXCEPT;

  /**
   * @brief 向地理查询表中插入一个批量的点和相关元素
   *
   * 当大量点和相关元素需要插入地理查询表时，可以使用此函数一次性插入多个点和元素，以提高性能。
   *
   * @param localPt 插入的地理坐标点
   * @param _elem_ptr 插入的相关元素指针
   * @return void 没有返回值，仅插入元素
   */
  static void InsertBulk(const Base::txVec2 &localPt, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;

  /**
   * @brief 向地理查询表中插入一组批量的点和相关元素
   *
   * 当大量点和相关元素需要插入地理查询表时，可以使用此函数一次性插入多个点和元素，以提高性能。
   *
   * @param localPts 插入的地理坐标点集合
   * @param _elem_ptr 插入的相关元素指针
   * @return void 无返回值，仅插入元素
   */
  static void InsertBulk(const std::vector<Base::txVec2> &localPts, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;

  /**
   * @brief 向地理查询表中插入一组批量的点和相关元素
   *
   * 当大量点和相关元素需要插入地理查询表时，可以使用此函数一次性插入多个点和元素，以提高性能。
   *
   * @param localPt_0 插入的地理坐标点1
   * @param localPt_1 插入的地理坐标点2
   * @param localPt_2 插入的地理坐标点3
   * @param localPt_3 插入的地理坐标点4
   * @param _elem_ptr 插入的相关元素指针
   * @return void 无返回值，仅插入元素
   */
  static void InsertBulk(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
                         const Base::txVec2 &localPt_3, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;

  /**
   * @brief 向地理查询表中插入一组批量的点和相关元素
   *
   * 当大量点和相关元素需要插入地理查询表时，可以使用此函数一次性插入多个点和元素，以提高性能。
   *
   * @param localPt_0 插入的地理坐标点1
   * @param localPt_1 插入的地理坐标点2
   * @param localPt_2 插入的地理坐标点3
   * @param localPt_3 插入的地理坐标点4
   * @param localPt_center 插入的地理坐标点的中心点
   * @param _elem_ptr 插入的相关元素指针
   * @return 无返回值，仅插入元素
   */
  static void InsertBulk(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
                         const Base::txVec2 &localPt_3, const Base::txVec2 &localPt_center,
                         Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;

  /**
   * @brief 大批量加载数据
   *
   * 对于特定的数据加载场景，此函数允许一次性加载大量数据，提高数据加载性能。
   *
   * @param TX_NOEXCEPT 可能抛出异常
   */
  void LoadBulk() TX_NOEXCEPT;

 public:
  void Release() TX_NOEXCEPT;
  Base::txSize Size() const TX_NOEXCEPT;
#if _UseInsert_
  void Insert(const Base::txVec2 &localPt, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;
  void Insert(const std::vector<Base::txVec2> &localPts, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;
  void Insert(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
              const Base::txVec2 &localPt_3, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;
  void Insert(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
              const Base::txVec2 &localPt_3, const Base::txVec2 &localPt_center,
              Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT;
#endif

  /**
   * @brief 查找指定区域内符合条件的路口或道路元素
   *
   * 该函数用于查找指定区域内满足元素类型条件的路口或道路元素。区域由包含两个点的向量描述，具体的描述顺序由传入的参数
   * `localPts_clockwise` 指定，但要求顺时针方向排列。
   *
   * @param centerPt         区域中心点
   * @param srcSysId         数据系统 ID
   * @param localPts_clockwise 区域边界点列表，顺序为顺时针方向
   * @param type_filter_set   元素类型过滤器集合，仅检索属于该集合中任一类型的元素
   * @param resultElementPtr  存储查找到的符合条件的元素指针列表
   *
   * @return bool    函数调用是否成功
   *
   */
  Base::txBool FindElementsInAreaByType(const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
                                        const std::vector<Base::txVec2> &localPts_clockwise,
                                        const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                        std::vector<Base::ITrafficElementPtr> &resultElementPtr) TX_NOEXCEPT;

  Base::txBool FindElementsInCircleByType(const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
                                          const Base::txFloat _CircleRadius,
                                          const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                          std::vector<Base::ITrafficElementPtr> &resultElementPtr) TX_NOEXCEPT;

  Base::txBool FindElementsInAreaByType(const Base::txSysId srcSysId,
                                        const std::vector<Base::txVec2> &localPts_clockwise,
                                        const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                        std::vector<Base::ITrafficElementPtr> &resultElementPtr) TX_NOEXCEPT;

  Base::txBool FindElementsInAreaByType_IDM(const Base::txSysId srcSysId,
                                            const std::vector<Base::txVec2> &localPts_clockwise,
                                            const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                            std::vector<Base::ITrafficElementPtr> &resultElementPtr) TX_NOEXCEPT;

  Base::ITrafficElementPtr FindNearestElementsByType(
      const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
      const std::set<Base::ITrafficElement::ElementType> &type_filter_set) TX_NOEXCEPT;

  static std::vector<Base::txVec2> GenerateSearchTriangleClosed(const Base::txVec3 &vOriginal,
                                                                const Base::txVec3 &vAhead,
                                                                const Base::txFloat halfSearchAngle,
                                                                const Base::txFloat distance) TX_NOEXCEPT;
  static std::vector<Base::txVec2> GenerateSearchTrapezoidClosed(const Base::txVec3 &vOriginal_head,
                                                                 const Base::txVec3 &vOriginal_tail,
                                                                 const Base::txVec3 &vAhead,
                                                                 const Base::txFloat halfSearchAngle,
                                                                 const Base::txFloat distance) TX_NOEXCEPT;
};

#if 1
class RTree2D_Signal {
 public:
  static RTree2D_Signal &getInstance() {
    static RTree2D_Signal instance;
    return instance;
  }

 private:
  RTree2D_Signal() TX_DEFAULT;
  ~RTree2D_Signal() TX_DEFAULT;
  RTree2D_Signal(const RTree2D_Signal &) TX_DELETE;
  RTree2D_Signal &operator=(const RTree2D_Signal &) TX_DELETE;

 public:
  void Clear() TX_NOEXCEPT;

  /**
   * @brief 释放对象，释放所有资源
   *
   * 当对象不再使用时，调用此方法释放所有资源。
   */
  void Release() TX_NOEXCEPT;

  /**
   * @brief 将信号添加到随机查询列表中
   *
   * 这个函数将信号添加到搜索列表中，以便我们可以快速查找。
   *
   * @param[in] localPt 信号的本地坐标
   * @param[in] _signal_ptr 信号指针
   */
  void Insert_Signal(const Base::txVec2 &localPt, Base::ISignalLightElementPtr _signal_ptr) TX_NOEXCEPT;

  /**
   * @brief 查找圆形区域内的交通元素
   *
   * 该函数用于查找给定圆形区域内的交通元素，并将结果存储在指定的 std::vector 中。
   *
   * @param[in] centerPt 圆心坐标
   * @param[in] radius 圆的半径
   * @param[out] resultSignalVec 储存查找结果的 std::vector 类型的指针
   * @return 是否成功查找到符合条件的交通元素
   * @retval true 查找成功
   * @retval false 查找失败
   */
  Base::txBool FindSignalInCircle(const Base::txVec2 &centerPt, const Base::txFloat radius,
                                  std::vector<Base::ITrafficElementPtr> &resultSignalVec) TX_NOEXCEPT;

  /**
   * @brief GenerateSearchTriangleClosed 生成搜索三角形内的顶点集合
   *
   * 此函数将生成一个顶点集合，并存储在输出向量中。此向量用于包含原点、结束点、搜索角度一半和指定距离的搜索三角形的顶点。
   *
   * @param vOriginal 原始点位置
   * @param vAhead 搜索前瞻点位置
   * @param halfSearchAngle 搜索角度的一半
   * @param distance 顶点距离
   * @return std::vector<Base::txVec2> 包含搜索三角形顶点的向量
   */
  static std::vector<Base::txVec2> GenerateSearchTriangleClosed(const Base::txVec3 &vOriginal,
                                                                const Base::txVec3 &vAhead,
                                                                const Base::txFloat halfSearchAngle,
                                                                const Base::txFloat distance) TX_NOEXCEPT;

  /**
   * @brief 根据给定的坐标参数生成一个封闭的搜索梯形的顶点集合
   *
   * 该函数用于生成一个封闭的搜索梯形的顶点集合，包含原始头尾点、搜索前瞻点、搜索角度一半和指定距离的搜索梯形的顶点。
   *
   * @param vOriginal_head 原始头点坐标
   * @param vOriginal_tail 原始尾点坐标
   * @param vAhead 搜索前瞻点坐标
   * @param halfSearchAngle 搜索角度的一半
   * @param distance 顶点距离
   * @return 包含搜索梯形顶点的向量
   */
  static std::vector<Base::txVec2> GenerateSearchTrapezoidClosed(const Base::txVec3 &vOriginal_head,
                                                                 const Base::txVec3 &vOriginal_tail,
                                                                 const Base::txVec3 &vAhead,
                                                                 const Base::txFloat halfSearchAngle,
                                                                 const Base::txFloat distance) TX_NOEXCEPT;

  /**
   * @brief 将一个信号点插入到哈德地图中
   *
   * 该函数将一个指定的局部坐标点插入到地图中，作为信号点。
   * 该信号点关联指定的 hadmap 对象。
   *
   * @param localPt 要插入的信号点的局部坐标
   * @param _sign_ptr 关联的 hadmap 对象
   */
  void Insert_Sign(const Base::txVec2 &localPt, hadmap::txObjectPtr _sign_ptr) TX_NOEXCEPT;

  /**
   * @brief 查找圆形区域内的某些信号点
   *
   * 该函数接受一个中心点和半径，在地图中查找圆形区域内所有信号点的位置。
   * 这些信号点需要关联到特定的地图对象，并返回它们的位置。
   *
   * @param centerPt 圆心的位置
   * @param radius 圆的半径
   * @param resultSignVec 存储结果的信号点向量
   * @return Base::txBool 查找成功时返回 true，否则返回 false
   */
  Base::txBool FindSignInCircle(const Base::txVec2 &centerPt, const Base::txFloat radius,
                                hadmap::txObjects &resultSignVec) TX_NOEXCEPT;

  /**
   * @brief 将一个新的点插入地图数据库
   *
   * 这个函数接收一个参数：一个局部坐标点(x, y)，用于定位公园。同时需要
   * 一个找到的指向一个地图对象指针。在插入公园之前，首先检查该局部坐标点是否已经存在于数据库中。
   * 如果已经存在，插入操作将被拒绝。在插入操作成功后，返回 true，否则返回 false。
   *
   * @param localPt 点的局部坐标点
   * @param _sign_ptr 指向地图对象的指针
   * @return bool 插入成功则返回 true，否则返回 false
   */
  void Insert_Park(const Base::txVec2 &localPt, hadmap::txObjectPtr _sign_ptr) TX_NOEXCEPT;
  Base::txBool FindParkInCircle(const Base::txVec2 &centerPt, const Base::txFloat radius,
                                hadmap::txObjects &resultParkVec) TX_NOEXCEPT;

 public:
  enum { max_element_size = 16 };
  using point_t = boost::geometry::model::point<Base::txFloat, 2, boost::geometry::cs::cartesian>;
  using box_t = boost::geometry::model::box<point_t>;
  using poly_t = boost::geometry::model::polygon<point_t>;
  TX_MARK("outer is clock-wise.");
  using value_t = std::pair<point_t, Base::ITrafficElementPtr>;
  using value_signal_t = std::pair<point_t, Base::ISignalLightElementPtr>;
  using value_signs_t = std::pair<point_t, hadmap::txObjectPtr>;
  using rtree_signal = boost::geometry::index::rtree<value_signal_t, boost::geometry::index::rstar<max_element_size>>;
  using rtree_signs = boost::geometry::index::rtree<value_signs_t, boost::geometry::index::rstar<max_element_size>>;

 protected:
  rtree_signal bg_signal_tree;
  rtree_signs bg_signs_tree, bg_park_tree;
};
#endif
#if USE_GetLaneNearBy
class RTree2D_NearestReferenceLine {
 public:
  static RTree2D_NearestReferenceLine &getInstance() {
    static RTree2D_NearestReferenceLine instance;
    return instance;
  }

 private:
  RTree2D_NearestReferenceLine();
  ~RTree2D_NearestReferenceLine() TX_DEFAULT;

  /**
   * @brief 找到最近的参考线
   *
   * 给定一个 2D 空间中的坐标点，找到最近的参考线
   *
   * @param pnt 要查找的坐标点
   * @return double 返回最近的参考线的坐标值
   */
  RTree2D_NearestReferenceLine(const RTree2D_NearestReferenceLine &) TX_DELETE;
  RTree2D_NearestReferenceLine &operator=(const RTree2D_NearestReferenceLine &) TX_DELETE;

 public:
  struct NearestReferenceLineResult {
    Base::Info_Lane_t locInfo;
    /*Base::txLaneUId onLaneUid;
    Base::txBool isOnLaneLink = false;
    Base::txLaneLinkID onLinkId;*/
    hadmap::txLanePtr lanePtr = nullptr;
    hadmap::txLaneLinkPtr lanelinkPtr = nullptr;
    Base::txFloat distanceCurve = 0.0;

    /**
     * @brief 检查当前坐标点是否位于沿道路连接的车道上
     *
     * @return true 当前坐标点在道路连接的车道上
     * @return false 当前坐标点不在道路连接的车道上
     */
    Base::txBool isOnLaneLink() const TX_NOEXCEPT { return locInfo.IsOnLaneLink(); }

    /**
     * @brief 获取当前坐标点所在的车道UID
     *
     * @return Base::txLaneUId 当前坐标点所在的车道UID
     */
    Base::txLaneUId onLaneUid() const TX_NOEXCEPT { return locInfo.onLaneUid; }
  };

  /**
   * @brief 根据给定的经纬度获取距离最近的车道
   *
   * 这个函数用于根据给定的经纬度获取离它最近的车道，并且返回它在车道上的距离。
   *
   * @param _lon 给定的经度
   * @param _lat 给定的纬度
   * @param distanceAlongCurve 返回距离最近的车道在车道上的距离
   * @return hadmap::txLanePtr 返回最近的车道的指针
   */
  hadmap::txLanePtr GetLaneNearBy(const Base::txFloat _lon, const Base::txFloat _lat,
                                  Base::txFloat &distanceAlongCurve) TX_NOEXCEPT;

  /**
   * @brief 根据给定的经纬度获取车道起点的经纬度
   *
   * 该函数通过传入的经纬度坐标，获取车道起点的经纬度。
   *
   * @param[in] _lon 给定的经度坐标
   * @param[in] _lat 给定的纬度坐标
   * @param[out] start_lon 返回的车道起点的经度坐标
   * @param[out] start_lat 返回的车道起点的纬度坐标
   */
  void GetLaneStartPoint(const Base::txFloat _lon, const Base::txFloat _lat, Base::txFloat &start_lon,
                         Base::txFloat &start_lat) TX_NOEXCEPT;

  /**
   * @brief 通过给定的世界坐标点获取关于同一路段起点的距离，返回的距离为曲线上的点离道路起点的距离
   *
   * 通过该函数，可以根据给定的世界坐标点，在相应道路中找到最近的具有道路信息的点，并计算出它与车道起点之间的距离。
   *
   * @param[in] aPos 给定的世界坐标点
   * @param[out] locInfo 根据返回的关于当前道路的信息
   * @param[out] distanceAlongCurve 车道起点与给定点之间的距离
   * @return true 表示成功
   * @return false 表示失败，常见的原因为传入的坐标点不在道路范围内
   */
  Base::txBool Get_S_Coord_By_Enu_Pt(Coord::txWGS84 aPos, Base::Info_Lane_t &locInfo,
                                     Base::txFloat &distanceAlongCurve) TX_NOEXCEPT;

  /**
   * @brief 获取给定WGS84坐标点的中心相对于车道中心的坐标点
   *
   * 该函数用于获取给定WGS84坐标点的中心相对于车道中心的坐标点，包括车道标识和道路属性。
   *
   * @param aPos 给定的WGS84坐标点
   * @param locInfo 通过返回的车道信息
   * @param _S 车道中心与给定坐标点的距离
   * @param _TT 坐标点与道路中心之间的角度
   * @return true 获取成功
   * @return false 获取失败，可能的原因包括坐标点不在道路范围内
   */
  Base::txBool Get_ST_Coord_By_Enu_Pt(Coord::txWGS84 aPos, Base::Info_Lane_t &locInfo, Base::txFloat &_S,
                                      Base::txFloat &_TT) TX_NOEXCEPT;

  /**
   * @brief 更新R树结构
   *
   * 更新R树结构，根据给定的坐标点`gPos`更新相应的节点。该方法用于更新`R`树结构中与`gPos`所在区域相关的节点，以确保查询结果的准确性。
   *
   * @param gPos 给定的地理坐标点
   * @return 无返回值
   */
  void UpdateRtree(const hadmap::txPoint &gPos) TX_NOEXCEPT;

  void Clear() TX_NOEXCEPT;

  /**
   * @brief 初始化参数，建立地理坐标系与r-tree索引之间的映射关系。
   *
   * 初始化参数，预先在内存中构建一个存储地理坐标点的spatial index，并建立其与地理坐标系之间的映射关系。
   *
   * @param ref_map_laneInfo 一个包含所有实际道路信息的地理坐标点的Map，用于建立映射关系。
   * @return 无返回值
   */
  Base::txBool Initialize(const std::map<Base::txLaneUId, hadmap::txLanePtr> &ref_map_laneInfo,
                          const std::map<Base::txLaneLinkID, hadmap::txLaneLinkPtr> &ref_map_lanelinkInfo) TX_NOEXCEPT;

  /**
   * @brief 初始化查询引擎，建立映射关系
   *
   * 初始化参数ref_lanesVec和ref_lanelinksVec后，构建索引数据结构，建立地理坐标系与r-tree索引之间的映射关系，用于后续查询。
   *
   * @param ref_lanesVec 存储所有实际道路信息的地理坐标点的Map
   * @param ref_lanelinksVec 存储所有道路连接信息的Map
   * @return 成功则返回true，否则返回false
   */
  Base::txBool Initialize(const hadmap::txLanes &ref_lanesVec, const hadmap::txLaneLinks &ref_lanelinksVec) TX_NOEXCEPT;

  /**
   * @brief 将车道注册到地图中
   *
   * 把一个车道注册到地图中，根据其坐标信息，即 (x,y) 将其存储到内存中的索引结构体中
   *
   * @param lanePtr 要注册的道路指针
   * @return 成功返回 true，否则返回 false
   */
  Base::txBool RegisterLane(hadmap::txLanePtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 注册道路连接
   *
   * 将道路连接注册到地图中，根据其坐标信息，即 (x,y) 将其存储到内存中的索引结构体中。
   *
   * @param lanePtr 要注册的道路连接指针
   * @return 成功返回 true，否则返回 false
   */
  Base::txBool RegisterLaneLink(hadmap::txLaneLinkPtr lanePtr) TX_NOEXCEPT;

  /**
   * @brief 判断给定的坐标位置是否已被注册
   *
   * 判断给定的坐标位置是否已经被注册，即根据输入的坐标信息 (x, y) 检查是否存在于内存中的索引结构体中。
   *
   * @param locInfo 待检查的坐标位置信息
   * @return 如果给定位置已被注册，则返回 true，否则返回 false
   */
  Base::txBool HasRegister(const Base::Info_Lane_t locInfo) TX_NOEXCEPT;
  /*Base::txBool HasRegister(const Base::txLaneLinkID lanelinkId) const TX_NOEXCEPT;
  Base::txBool HasRegister(const Base::txLaneUId laneUid) const TX_NOEXCEPT;*/

  /**
   * @brief 判断给定的gps坐标点是否已被同一车道上的其他车辆（包括车辆本身）覆盖，判断标准是车道距离和车辆大小
   *
   * 这个函数接受一个gps坐标点、车辆大小和车道信息，输出是否被其他车辆覆盖。
   * 车道信息包含车道宽度、前后停车道距离、左右道宽度等信息。
   * 判断是否被其他车辆覆盖的标准包括车道距离和车辆大小，也就是判断该车辆是否会与其他车辆发生碰撞。
   *
   * @param gpsMap 需要判断的gps坐标点
   * @param _sMap 车辆大小
   * @param infoMap 车道信息
   * @return true 表示可以被覆盖，false 表示不能被覆盖
   */
  Base::txBool TestAbsorb(const hadmap::txPoint gpsMap, const Base::txFloat _sMap,
                          const Base::Info_Lane_t &infoMap) TX_NOEXCEPT;

  /**
   * @brief CheckAbsorb 检查当前车道尽可能吸收其他车道
   *
   * 检查当前车道（self_laneVec）能否通过尽可能多的链接，将其它车道（ref_lanesVec）吸收到自己车道中。
   * 这个函数会使用 ref_lanelinksVec 中的链接信息，并将结果保存在 self_laneVec 中。
   *
   * @param ref_lanesVec 被吸收的车道集合，以 ref_lane 为基础数据结构存储
   * @param ref_lanelinksVec 链接信息集合，包含了车道之间的链接信息
   */
  void CheckAbsorb(const hadmap::txLanes &ref_lanesVec, const hadmap::txLaneLinks &ref_lanelinksVec) TX_NOEXCEPT;

 protected:
  struct RTree2D_NearestReferenceLine_Impl;
  std::shared_ptr<RTree2D_NearestReferenceLine_Impl> pImpl;

#  if USE_TBB

 protected:
  tbb::mutex tbbMutex_RTree2D_NearestReferenceLine;
#  endif
};
#endif /*USE_GetLaneNearBy*/

extern void ClearSpatialQueryInfo() TX_NOEXCEPT;
extern void ClearSpatialQueryElementInfo() TX_NOEXCEPT;
extern void ClearSpatialQuerySignalInfo() TX_NOEXCEPT;
extern void ClearSpatialQueryReferenceLineInfo() TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)

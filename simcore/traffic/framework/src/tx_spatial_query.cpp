// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_spatial_query.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/iterator/function_output_iterator.hpp>
#include <unordered_set>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "HdMap/tx_lane_geom_info.h"
#include "tx_frame_utils.h"
#include "tx_hadmap_utils.h"
#include "tx_hash_utils.h"
#include "tx_locate_info.h"
#include "tx_math.h"
#include "tx_parallel_def.h"
#include "tx_signal_element.h"
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

void ClearSpatialQueryElementInfo() TX_NOEXCEPT {
  RTree2D::getInstance().Release();
  RTree2D::ReleaseBulk();
}

void ClearSpatialQuerySignalInfo() TX_NOEXCEPT { RTree2D_Signal::getInstance().Release(); }

void ClearSpatialQueryReferenceLineInfo() TX_NOEXCEPT { RTree2D_NearestReferenceLine::getInstance().Clear(); }

void ClearSpatialQueryInfo() TX_NOEXCEPT {
  // 清空元素空间查询信息
  ClearSpatialQueryElementInfo();
  // 清空信号灯空间查询信息
  ClearSpatialQuerySignalInfo();
  ClearSpatialQueryReferenceLineInfo();
}
#if __TX_Mark__("cloud search circle signal")
void RTree2D_Signal::Clear() TX_NOEXCEPT { Release(); }

void RTree2D_Signal::Release() TX_NOEXCEPT {
  // 清空信号灯树
  bg_signal_tree.clear();
  // 清空标志物树
  bg_signs_tree.clear();
  // 清空停车位树
  bg_park_tree.clear();
}

void RTree2D_Signal::Insert_Signal(const Base::txVec2 &localPt, Base::ISignalLightElementPtr _signal_ptr) TX_NOEXCEPT {
  bg_signal_tree.insert(std::make_pair(point_t(localPt.x(), localPt.y()), _signal_ptr));
}

struct circle_filter_squared {
  using point_t = RTree2D_Signal::point_t;
  template <typename Value>
  bool operator()(Value v) const {
    const Base::txFloat squared_dist =
        (Base::txVec2(circle_center.get<0>(), circle_center.get<1>()) - (v.second->GetLocation().ToENU().ENU2D()))
            .squaredNorm();

    // 如果元素与圆心的平方距离小于圆的平方半径，且元素的系统ID与源系统ID不同，且元素存活
    if ((squared_dist < squared_radius) && (srcSysId != (v.second->SysId())) && (v.second->IsAlive())) {
      return true;
    } else {
      return false;
    }
  }

  circle_filter_squared(const point_t &_circle_center, const Base::txFloat _squared_radius, const Base::txSysId _SysId)
      : circle_center(_circle_center), squared_radius(_squared_radius), srcSysId(_SysId) {}

 protected:
  const point_t &circle_center;
  const Base::txFloat squared_radius;
  const Base::txSysId srcSysId;
};

Base::txBool RTree2D_Signal::FindSignalInCircle(const Base::txVec2 &centerPt, const Base::txFloat radius,
                                                std::vector<Base::ITrafficElementPtr> &resultSignalVec) TX_NOEXCEPT {
  namespace bgi = boost::geometry::index;
  // 清空结果信号灯向量
  resultSignalVec.clear();
  // 将圆心坐标转换为point_t类型
  point_t center_pt(centerPt.x(), centerPt.y());
  std::map<Base::txSysId, Base::ITrafficElementPtr> result_sighalPtr_Map;
  // 定义一个lambda函数，用于将查询结果添加到映射中
  std::function<void(value_t)> f = [&result_sighalPtr_Map](value_t v) {
    result_sighalPtr_Map[v.second->SysId()] = (v.second);
  };
  // 在信号灯树中查询满足条件的元素，并将结果添加到映射中
  bgi::query(bg_signal_tree, bgi::satisfies(circle_filter_squared(center_pt, radius * radius, 0)),
             boost::make_function_output_iterator(f));
  if (result_sighalPtr_Map.size() > 0) {
    // 将映射中的信号灯添加到结果信号灯向量中
    for (const auto sysid_ptr_pair : result_sighalPtr_Map) {
      resultSignalVec.emplace_back(sysid_ptr_pair.second);
    }
  }
  return true;
}

void RTree2D_Signal::Insert_Sign(const Base::txVec2 &localPt, hadmap::txObjectPtr _sign_ptr) TX_NOEXCEPT {
  bg_signs_tree.insert(std::make_pair(point_t(localPt.x(), localPt.y()), _sign_ptr));
}

struct circle_filter_squared_sign {
  using point_t = RTree2D_Signal::point_t;
  template <typename Value>
  bool operator()(Value v) const {
    const point_t &objPt = v.first;
    // 计算元素与圆心的平方距离
    const Base::txFloat squared_dist =
        (Base::txVec2(circle_center.get<0>(), circle_center.get<1>()) - (Base::txVec2(objPt.get<0>(), objPt.get<1>())))
            .squaredNorm();

    // 如果元素与圆心的平方距离小于圆的平方半径
    if ((squared_dist < squared_radius)) {
      return true;
    } else {
      return false;
    }
  }

  circle_filter_squared_sign(const point_t &_circle_center, const Base::txFloat _squared_radius)
      : circle_center(_circle_center), squared_radius(_squared_radius) {}

 protected:
  const point_t &circle_center;
  const Base::txFloat squared_radius;
};

Base::txBool RTree2D_Signal::FindSignInCircle(const Base::txVec2 &centerPt, const Base::txFloat radius,
                                              hadmap::txObjects &resultSignVec) TX_NOEXCEPT {
  namespace bgi = boost::geometry::index;
  resultSignVec.clear();
  // 将圆心坐标转换为point_t类型
  point_t center_pt(centerPt.x(), centerPt.y());
  std::map<hadmap::objectpkid, hadmap::txObjectPtr> result_sign_Map;
  // 定义一个lambda函数，用于将查询结果添加到映射中
  std::function<void(value_signs_t)> f = [&result_sign_Map](value_signs_t v) {
    result_sign_Map[v.second->getId()] = (v.second);
  };
  // 在信号灯树中查询满足条件的元素，并将结果添加到映射中
  bgi::query(bg_signs_tree, bgi::satisfies(circle_filter_squared_sign(center_pt, radius * radius)),
             boost::make_function_output_iterator(f));
  // 如果映射中有结果
  if (result_sign_Map.size() > 0) {
    for (const auto sysid_ptr_pair : result_sign_Map) {
      resultSignVec.emplace_back(sysid_ptr_pair.second);
    }
  }
  return _NonEmpty_(resultSignVec);
}

void RTree2D_Signal::Insert_Park(const Base::txVec2 &localPt, hadmap::txObjectPtr _sign_ptr) TX_NOEXCEPT {
  bg_park_tree.insert(std::make_pair(point_t(localPt.x(), localPt.y()), _sign_ptr));
}

struct circle_filter_squared_park {
  using point_t = RTree2D_Signal::point_t;
  // 判断元素是否满足条件
  template <typename Value>
  bool operator()(Value v) const {
    const point_t &objPt = v.first;
    const Base::txFloat squared_dist =
        (Base::txVec2(circle_center.get<0>(), circle_center.get<1>()) - (Base::txVec2(objPt.get<0>(), objPt.get<1>())))
            .squaredNorm();

    // 如果元素与圆心的平方距离小于圆的平方半径
    if ((squared_dist < squared_radius)) {
      return true;
    } else {
      return false;
    }
  }

  circle_filter_squared_park(const point_t &_circle_center, const Base::txFloat _squared_radius)
      : circle_center(_circle_center), squared_radius(_squared_radius) {}

 protected:
  const point_t &circle_center;
  const Base::txFloat squared_radius;
};

Base::txBool RTree2D_Signal::FindParkInCircle(const Base::txVec2 &centerPt, const Base::txFloat radius,
                                              hadmap::txObjects &resultParkVec) TX_NOEXCEPT {
  namespace bgi = boost::geometry::index;
  // 清空结果停车场向量
  resultParkVec.clear();
  point_t center_pt(centerPt.x(), centerPt.y());
  std::map<hadmap::objectpkid, hadmap::txObjectPtr> result_park_Map;
  // 定义一个lambda函数，用于将查询结果添加到映射中
  std::function<void(value_signs_t)> f = [&result_park_Map](value_signs_t v) {
    result_park_Map[v.second->getId()] = (v.second);
  };
  // 在停车场树中查询满足条件的元素，并将结果添加到映射中
  bgi::query(bg_park_tree, bgi::satisfies(circle_filter_squared_park(center_pt, radius * radius)),
             boost::make_function_output_iterator(f));
  if (result_park_Map.size() > 0) {
    // 将映射中的停车场添加到结果停车场向量中
    for (const auto sysid_ptr_pair : result_park_Map) {
      resultParkVec.emplace_back(sysid_ptr_pair.second);
    }
  }
  return _NonEmpty_(resultParkVec);
}

std::vector<Base::txVec2> RTree2D_Signal::GenerateSearchTriangleClosed(const Base::txVec3 &vOriginal,
                                                                       const Base::txVec3 &vAhead,
                                                                       const Base::txFloat halfSearchAngle,
                                                                       const Base::txFloat distance) TX_NOEXCEPT {
  // 计算左侧搜索方向
  const Base::txVec3 vLeftTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(-1.0 * halfSearchAngle), Utils::Axis_Enu_Up());
  // 计算右侧搜索方向
  const Base::txVec3 vRightTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(halfSearchAngle), Utils::Axis_Enu_Up());

  // 初始化搜索三角形的顶点
  std::vector<Base::txVec2> localPts_clockwise_close;
  localPts_clockwise_close.resize(4);
  const Base::txVec3 leftTopPt = vOriginal + vLeftTestDir * distance;
  const Base::txVec3 rightTopPt = vOriginal + vRightTestDir * distance;
  // 设置三角形的顶点
  localPts_clockwise_close[0] = Base::txVec2(vOriginal.x(), vOriginal.z());
  localPts_clockwise_close[1] = Base::txVec2(leftTopPt.x(), leftTopPt.z());
  localPts_clockwise_close[2] = Base::txVec2(rightTopPt.x(), rightTopPt.z());
  localPts_clockwise_close[3] = localPts_clockwise_close[0];
  return localPts_clockwise_close;
}

// 设置三角形的顶点
std::vector<Base::txVec2> RTree2D_Signal::GenerateSearchTrapezoidClosed(const Base::txVec3 &leftBottomPt,
                                                                        const Base::txVec3 &rightBottomPt,
                                                                        const Base::txVec3 &vAhead,
                                                                        const Base::txFloat halfSearchAngle,
                                                                        const Base::txFloat distance) TX_NOEXCEPT {
  // 计算左侧右侧搜索方向
  const Base::txVec3 vLeftTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(-1.0 * halfSearchAngle), Utils::Axis_Enu_Up());
  const Base::txVec3 vRightTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(halfSearchAngle), Utils::Axis_Enu_Up());

  // 初始化搜索梯形的顶点
  std::vector<Base::txVec2> localPts_clockwise_close;
  localPts_clockwise_close.resize(5);
  const Base::txVec3 leftTopPt = leftBottomPt + vLeftTestDir * distance;
  const Base::txVec3 rightTopPt = rightBottomPt + vRightTestDir * distance;

  // 设置梯形的顶点
  localPts_clockwise_close[0] = Base::txVec2(leftBottomPt.x(), leftBottomPt.z());
  localPts_clockwise_close[1] = Base::txVec2(leftTopPt.x(), leftTopPt.z());
  localPts_clockwise_close[2] = Base::txVec2(rightTopPt.x(), rightTopPt.z());
  localPts_clockwise_close[3] = Base::txVec2(rightBottomPt.x(), rightBottomPt.z());
  localPts_clockwise_close[4] = localPts_clockwise_close[0];
  // 返回搜索梯形的顶点
  return localPts_clockwise_close;
}
#endif
struct RTree2D::RTree2DImpl {
 public:
  enum { max_element_size = 16 };
  using point_t = RTree2D::point_t;
  using poly_t = boost::geometry::model::polygon<point_t>;
  TX_MARK("outer is clock-wise.");

  using value_t = RTree2D::value_t;
  using rtree = boost::geometry::index::rtree<value_t, boost::geometry::index::rstar<max_element_size>>;

  using value_signal_t = std::pair<point_t, Base::ISignalLightElementPtr>;
  using rtree_signal = boost::geometry::index::rtree<value_signal_t, boost::geometry::index::rstar<max_element_size>>;

 public:
  void Release() TX_NOEXCEPT {
#if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#endif
    bg_tree.clear();
  }
#if _UseInsert_
  void Insert(const Base::txVec2 &localPt, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
#  if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#  endif
    bg_tree.insert(std::make_pair(point_t(localPt.x(), localPt.y()), _elem_ptr));
  }

  void Insert(const std::vector<Base::txVec2> &localPts, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
#  if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#  endif
    for (const auto &pt : localPts) {
      bg_tree.insert(std::make_pair(point_t(pt.x(), pt.y()), _elem_ptr));
    }
  }

  void Insert(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
              const Base::txVec2 &localPt_3, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
#  if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#  endif
    bg_tree.insert(std::make_pair(point_t(localPt_0.x(), localPt_0.y()), _elem_ptr));
    bg_tree.insert(std::make_pair(point_t(localPt_1.x(), localPt_1.y()), _elem_ptr));
    bg_tree.insert(std::make_pair(point_t(localPt_2.x(), localPt_2.y()), _elem_ptr));
    bg_tree.insert(std::make_pair(point_t(localPt_3.x(), localPt_3.y()), _elem_ptr));
  }

  void Insert(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
              const Base::txVec2 &localPt_3, const Base::txVec2 &localPt_4,
              Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
#  if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#  endif
    bg_tree.insert(std::make_pair(point_t(localPt_4.x(), localPt_4.y()), _elem_ptr));
  }
#endif
  struct type_filter {
    template <typename Value>
    bool operator()(Value const &v) const {
      const auto elemPtr = v.second;
      /*if (Null_Pointer(elemPtr)) {
          LOG(WARNING) << TX_VARS(v.first.get<0>()) << TX_VARS(v.first.get<1>()) << TX_COND(NonNull_Pointer(elemPtr));
      }*/
      /*LOG(WARNING) << TX_VARS_NAME(query_result_id, elemPtr->Id())
          << TX_VARS(elemPtr->SysId()) << TX_COND(elemPtr->IsAlive());*/
      /*if (2 == srcSysId) {
      }*/
      // 如果元素指针不为空
      if ((type_filter_set.count(elemPtr->Type()) > 0) && (srcSysId != (elemPtr->SysId())) && (elemPtr->IsAlive())) {
        return true;
      } else {
        return false;
      }
    }

    type_filter(const std::set<Base::ITrafficElement::ElementType> &_type_set, const Base::txSysId _SysId)
        : type_filter_set(_type_set), srcSysId(_SysId) {}

   protected:
    const std::set<Base::ITrafficElement::ElementType> &type_filter_set;
    const Base::txSysId srcSysId;
  };

  struct circle_filter_squared {
    template <typename Value>
    bool operator()(Value v) const {
      const Base::txFloat squared_dist =
          (Base::txVec2(circle_center.get<0>(), circle_center.get<1>()) - (v.second->GetLocation().ToENU().ENU2D()))
              .squaredNorm();

      // 如果元素与圆心的平方距离小于圆的平方半径，且元素的系统ID与给定的系统ID不同，且元素存活
      if ((squared_dist < squared_radius) && (srcSysId != (v.second->SysId())) && (v.second->IsAlive())) {
        return true;
      } else {
        return false;
      }
    }

    circle_filter_squared(const point_t &_circle_center, const Base::txFloat _squared_radius,
                          const Base::txSysId _SysId)
        : circle_center(_circle_center), squared_radius(_squared_radius), srcSysId(_SysId) {}

   protected:
    const point_t &circle_center;
    const Base::txFloat squared_radius;
    const Base::txSysId srcSysId;
  };

  Base::txBool FindElementsInCircleByType(const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
                                          const Base::txFloat _radius,
                                          const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                          std::vector<Base::ITrafficElementPtr> &resultElementPtrVec) TX_NOEXCEPT {
    resultElementPtrVec.clear();
    resultElementPtrVec.reserve(20);
    // 如果半径大于0
    if (_radius > 0.0) {
      namespace bgi = boost::geometry::index;
      point_t center_pt(centerPt.x(), centerPt.y());
      // 定义结果元素指针映射
      std::map<Base::txSysId, Base::ITrafficElementPtr> result_elemPtr_Map;
      // 定义一个lambda函数，用于将满足条件的元素添加到结果映射中
      std::function<void(value_t)> f = [&result_elemPtr_Map](value_t v) {
        result_elemPtr_Map[v.second->SysId()] = (v.second);
      };

      // 查询满足条件的元素
      bgi::query(bg_tree,
                 bgi::satisfies(circle_filter_squared(center_pt, (_radius * _radius), srcSysId)) &&
                     bgi::satisfies(type_filter(type_filter_set, srcSysId)),
                 boost::make_function_output_iterator(f));

      if (result_elemPtr_Map.size() > 0) {
        // 将结果映射中的元素添加到结果元素指针向量中
        for (const auto sysid_ptr_pair : result_elemPtr_Map) {
          resultElementPtrVec.emplace_back(sysid_ptr_pair.second);
        }
      }
      return true;
    } else {
      return false;
    }
  }

  Base::txBool FindElementsInAreaByType(const Base::txSysId srcSysId,
                                        const std::vector<Base::txVec2> &localPts_clockwise_close,
                                        const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                        std::vector<Base::ITrafficElementPtr> &resultElementPtrVec) TX_NOEXCEPT {
#if USE_TBB
    // 使用TBB库的互斥锁
    tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#endif
    resultElementPtrVec.clear();
    resultElementPtrVec.reserve(20);
    if (localPts_clockwise_close.size() > 3) {
      namespace bgi = boost::geometry::index;
      poly_t _polygon;
      // 将多边形顶点添加到多边形中
      for (const auto &pt : localPts_clockwise_close) {
        boost::geometry::append(_polygon.outer(), point_t(pt.x(), pt.y()));
      }
      // 定义结果元素指针映射
      std::map<Base::txSysId, Base::ITrafficElementPtr> result_elemPtr_Map;
      // 定义一个lambda函数，用于将满足条件的元素添加到结果映射中
      std::function<void(value_t)> f = [&result_elemPtr_Map](value_t v) {
        result_elemPtr_Map[v.second->SysId()] = (v.second);
      };
      /*std::function<void(value_t)> f = [&resultElementPtrVec](value_t v) { resultElementPtrVec.push_back((v.second));
       * };*/
      // 查询满足条件的元素
      bgi::query(bg_tree, bgi::intersects(_polygon) && bgi::satisfies(type_filter(type_filter_set, srcSysId)),
                 boost::make_function_output_iterator(f));
      // 如果结果映射不为空
      if (CallFail(result_elemPtr_Map.empty())) {
        // 将结果映射中的元素添加到结果元素指针向量中
        for (const auto sysid_ptr_pair : result_elemPtr_Map) {
          resultElementPtrVec.emplace_back(sysid_ptr_pair.second);
        }
      }
      return true;
    } else {
      return false;
    }
  }

  Base::txBool FindElementsInAreaByType_IDM(const Base::txSysId srcSysId,
                                            const std::vector<Base::txVec2> &localPts_clockwise_close,
                                            const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                            std::vector<Base::ITrafficElementPtr> &resultElementPtrVec) TX_NOEXCEPT {
    resultElementPtrVec.clear();
    resultElementPtrVec.reserve(20);
    // 如果多边形顶点数量大于3
    if (localPts_clockwise_close.size() > 3) {
      namespace bgi = boost::geometry::index;
      poly_t _polygon;
      // 将多边形顶点添加到多边形中
      for (const auto &pt : localPts_clockwise_close) {
        boost::geometry::append(_polygon.outer(), point_t(pt.x(), pt.y()));
      }
      // 定义结果元素指针映射
      std::map<Base::txSysId, Base::ITrafficElementPtr> result_elemPtr_Map;
      // 定义一个lambda函数，用于将满足条件的元素添加到结果映射中
      std::function<void(value_t)> f = [&result_elemPtr_Map](value_t v) {
        result_elemPtr_Map[v.second->SysId()] = (v.second);
      };
      /*std::function<void(value_t)> f = [&resultElementPtrVec](value_t v) { resultElementPtrVec.push_back((v.second));
       * };*/
      bgi::query(/*bg_tree_IDM*/ bg_tree,
                 bgi::intersects(_polygon) && bgi::satisfies(type_filter(type_filter_set, srcSysId)),
                 boost::make_function_output_iterator(f));
      // 如果结果映射不为空
      if (CallFail(result_elemPtr_Map.empty())) {
        // 将结果映射中的元素添加到结果元素指针向量中
        for (const auto sysid_ptr_pair : result_elemPtr_Map) {
          resultElementPtrVec.emplace_back(sysid_ptr_pair.second);
        }
      }
      return true;
    } else {
      return false;
    }
  }

  Base::txBool FindElementsInAreaByType(const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
                                        const std::vector<Base::txVec2> &localPts_clockwise_close,
                                        const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                        std::vector<Base::ITrafficElementPtr> &resultElementPtrVec) TX_NOEXCEPT {
    resultElementPtrVec.clear();
    if (localPts_clockwise_close.size() > 3) {
      namespace bgi = boost::geometry::index;
      poly_t _polygon;
      // 将多边形顶点添加到多边形中
      for (const auto &pt : localPts_clockwise_close) {
        boost::geometry::append(_polygon.outer(), point_t(pt.x(), pt.y()));
      }
      point_t center_pt(centerPt.x(), centerPt.y());
      std::map<Base::txSysId, Base::ITrafficElementPtr> result_elemPtr_Map;
      // 定义一个lambda函数，用于将满足条件的元素添加到结果映射中
      std::function<void(value_t)> f = [&result_elemPtr_Map](value_t v) {
        result_elemPtr_Map[v.second->SysId()] = (v.second);
      };
      bgi::query(bg_tree, bgi::intersects(_polygon) && bgi::satisfies(type_filter(type_filter_set, srcSysId)),
                 boost::make_function_output_iterator(f));
      // 如果结果映射不为空
      if (result_elemPtr_Map.size() > 0) {
        // 将结果映射中的元素添加到结果元素指针向量中
        for (const auto sysid_ptr_pair : result_elemPtr_Map) {
          resultElementPtrVec.emplace_back(sysid_ptr_pair.second);
        }
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  Base::ITrafficElementPtr FindNearestElementsByType(
      const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
      const std::set<Base::ITrafficElement::ElementType> &type_filter_set) TX_NOEXCEPT {
    namespace bgi = boost::geometry::index;

    point_t center_pt(centerPt.x(), centerPt.y());
    std::map<Base::txSysId, Base::ITrafficElementPtr> result_elemPtr_Map;
    // 定义一个lambda函数，用于将满足条件的元素添加到结果映射中
    std::function<void(value_t)> f = [&result_elemPtr_Map](value_t v) {
      result_elemPtr_Map[v.second->SysId()] = (v.second);
    };
    // 查询满足条件的元素
    bgi::query(bg_tree, bgi::nearest(center_pt, 1) && bgi::satisfies(type_filter(type_filter_set, srcSysId)),
               boost::make_function_output_iterator(f));
    // 如果结果映射不为空
    if (result_elemPtr_Map.size() > 0) {
      return (*result_elemPtr_Map.begin()).second;
    } else {
      return nullptr;
    }
  }

  Base::txSize bg_tree_size() const TX_NOEXCEPT { return bg_tree.size(); }

  Base::txSize bg_tree_IDM_size() const TX_NOEXCEPT { return bg_tree_size(); }

 public:
  rtree bg_tree;

#if USE_TBB
  tbb::mutex tbbMutex_bg_tree;
#endif
};
#if _UseInsert_
void RTree2D::Insert(const Base::txVec2 &localPt, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  pImpl->Insert(localPt, _elem_ptr);
}

void RTree2D::Insert(const std::vector<Base::txVec2> &localPts, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  pImpl->Insert(localPts, _elem_ptr);
}

void RTree2D::Insert(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
                     const Base::txVec2 &localPt_3, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  pImpl->Insert(localPt_0, localPt_1, localPt_2, localPt_3, _elem_ptr);
}

void RTree2D::Insert(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
                     const Base::txVec2 &localPt_3, const Base::txVec2 &localPt_center,
                     Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  /*namespace bgi = boost::geometry::index;
  using point_t = boost::geometry::model::point<Base::txFloat, 2, boost::geometry::cs::cartesian>;
  pImpl->m_vec_bulk_load.push_back(std::make_pair(point_t(localPt_center.x(), localPt_center.y()), _elem_ptr));*/
  pImpl->Insert(localPt_0, localPt_1, localPt_2, localPt_3, localPt_center, _elem_ptr);
}
#endif
Base::txBool RTree2D::FindElementsInCircleByType(const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
                                                 const Base::txFloat _CircleRadius,
                                                 const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                                 std::vector<Base::ITrafficElementPtr> &resultElementPtr) TX_NOEXCEPT {
  return pImpl->FindElementsInCircleByType(centerPt, srcSysId, _CircleRadius, type_filter_set, resultElementPtr);
}

Base::txBool RTree2D::FindElementsInAreaByType(const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
                                               const std::vector<Base::txVec2> &localPts_clockwise,
                                               const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                               std::vector<Base::ITrafficElementPtr> &resultElementPtrVec) TX_NOEXCEPT {
  return pImpl->FindElementsInAreaByType(centerPt, srcSysId, localPts_clockwise, type_filter_set, resultElementPtrVec);
}

Base::txBool RTree2D::FindElementsInAreaByType(const Base::txSysId srcSysId,
                                               const std::vector<Base::txVec2> &localPts_clockwise,
                                               const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
                                               std::vector<Base::ITrafficElementPtr> &resultElementPtrVec) TX_NOEXCEPT {
  return pImpl->FindElementsInAreaByType(srcSysId, localPts_clockwise, type_filter_set, resultElementPtrVec);
}

Base::txBool RTree2D::FindElementsInAreaByType_IDM(
    const Base::txSysId srcSysId, const std::vector<Base::txVec2> &localPts_clockwise,
    const std::set<Base::ITrafficElement::ElementType> &type_filter_set,
    std::vector<Base::ITrafficElementPtr> &resultElementPtr) TX_NOEXCEPT {
  return pImpl->FindElementsInAreaByType_IDM(srcSysId, localPts_clockwise, type_filter_set, resultElementPtr);
}

Base::ITrafficElementPtr RTree2D::FindNearestElementsByType(
    const Base::txVec2 &centerPt, const Base::txSysId srcSysId,
    const std::set<Base::ITrafficElement::ElementType> &type_filter_set) TX_NOEXCEPT {
  return pImpl->FindNearestElementsByType(centerPt, srcSysId, type_filter_set);
}

RTree2D::RTree2D() : pImpl(std::make_shared<RTree2D::RTree2DImpl>()) {}

RTree2D::~RTree2D() {}

void RTree2D::Release() TX_NOEXCEPT { pImpl->Release(); }

Base::txSize RTree2D::Size() const TX_NOEXCEPT { return pImpl->bg_tree_size(); }

// 生成搜索三角形的顶点
std::vector<Base::txVec2> RTree2D::GenerateSearchTriangleClosed(const Base::txVec3 &vOriginal,
                                                                const Base::txVec3 &vAhead,
                                                                const Base::txFloat halfSearchAngle,
                                                                const Base::txFloat distance) TX_NOEXCEPT {
  // 计算左右侧的搜索方向
  const Base::txVec3 vLeftTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(-1.0 * halfSearchAngle), Utils::Axis_Enu_Up());
  const Base::txVec3 vRightTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(halfSearchAngle), Utils::Axis_Enu_Up());

  std::vector<Base::txVec2> localPts_clockwise_close;
  // 预留空间，避免多次分配内存
  localPts_clockwise_close.resize(4);
  // 计算左右侧的顶点
  const Base::txVec3 leftTopPt = vOriginal + vLeftTestDir * distance;
  const Base::txVec3 rightTopPt = vOriginal + vRightTestDir * distance;
  // 将顶点添加到搜索三角形中
  localPts_clockwise_close[0] = Base::txVec2(vOriginal.x(), vOriginal.z());
  localPts_clockwise_close[1] = Base::txVec2(leftTopPt.x(), leftTopPt.z());
  localPts_clockwise_close[2] = Base::txVec2(rightTopPt.x(), rightTopPt.z());
  localPts_clockwise_close[3] = localPts_clockwise_close[0];
  return localPts_clockwise_close;
}

std::vector<Base::txVec2> RTree2D::GenerateSearchTrapezoidClosed(const Base::txVec3 &leftBottomPt,
                                                                 const Base::txVec3 &rightBottomPt,
                                                                 const Base::txVec3 &vAhead,
                                                                 const Base::txFloat halfSearchAngle,
                                                                 const Base::txFloat distance) TX_NOEXCEPT {
  // 计算左侧搜索方向
  const Base::txVec3 vLeftTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(-1.0 * halfSearchAngle), Utils::Axis_Enu_Up());
  const Base::txVec3 vRightTestDir =
      Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(halfSearchAngle), Utils::Axis_Enu_Up());

  std::vector<Base::txVec2> localPts_clockwise_close;
  // 预留空间，避免多次分配内存
  localPts_clockwise_close.resize(5);
  // 计算左侧顶点
  const Base::txVec3 leftTopPt = leftBottomPt + vLeftTestDir * distance;
  const Base::txVec3 rightTopPt = rightBottomPt + vRightTestDir * distance;

  // 将顶点添加到搜索梯形中
  localPts_clockwise_close[0] = Base::txVec2(leftBottomPt.x(), leftBottomPt.z());
  localPts_clockwise_close[1] = Base::txVec2(leftTopPt.x(), leftTopPt.z());
  localPts_clockwise_close[2] = Base::txVec2(rightTopPt.x(), rightTopPt.z());
  localPts_clockwise_close[3] = Base::txVec2(rightBottomPt.x(), rightBottomPt.z());
  localPts_clockwise_close[4] = localPts_clockwise_close[0];
  return localPts_clockwise_close;
}

#if USE_GetLaneNearBy
#  if __TX_Mark__("RTree2D_NearestReferenceLine")

struct RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl {
  using point_t = boost::geometry::model::point<Base::txFloat, 2, boost::geometry::cs::cartesian>;
  using segment_t = boost::geometry::model::segment<point_t>;
  using value_t = std::pair<segment_t, std::tuple<Base::txFloat, Base::txSize>>;
  using rtree = boost::geometry::index::rtree<value_t, boost::geometry::index::rstar<16>>;
  virtual ~RTree2D_NearestReferenceLine_Impl() { /*LOG(WARNING) << "[~] RTree2D_NearestReferenceLine_Impl";*/
  }
  // 定义R树变量
  rtree m_rtree_point2maplocationInfo;
  // 定义结果向量
  std::vector<RTree2D_NearestReferenceLine::NearestReferenceLineResult> m_locationInfoWithoutDistanceCurve;

  std::unordered_set<Base::Info_Lane_t, Utils::Info_Lane_t_HashCompare> m_set_locInfo;
  /*std::set< Base::txLaneLinkID > m_set_lanelinkid;
  std::set< Base::txLaneUId > m_set_laneuid;*/
};

Base::txBool RTree2D_NearestReferenceLine::HasRegister(const Base::Info_Lane_t locInfo) TX_NOEXCEPT {
#    if USE_TBB
  tbb::mutex::scoped_lock lock(tbbMutex_RTree2D_NearestReferenceLine);
#    endif /*USE_TBB*/
  return ((pImpl->m_set_locInfo).count(locInfo) > 0);
}

/*Base::txBool RTree2D_NearestReferenceLine::HasRegister(const Base::txLaneLinkID lanelinkId) const TX_NOEXCEPT {
    return ((pImpl->m_set_lanelinkid).count(lanelinkId) > 0);
}

Base::txBool RTree2D_NearestReferenceLine::HasRegister(const Base::txLaneUId laneUid) const TX_NOEXCEPT {
    return ((pImpl->m_set_laneuid).count(laneUid) > 0);
}*/

void RTree2D_NearestReferenceLine::Clear() TX_NOEXCEPT {
  pImpl->m_locationInfoWithoutDistanceCurve.clear();
  // 清空R树
  pImpl->m_rtree_point2maplocationInfo.clear();
  pImpl->m_set_locInfo.clear();
  /*pImpl->m_set_lanelinkid.clear();
  pImpl->m_set_laneuid.clear();*/
}
#    if __TX_Mark__("si chuan high way UpdateSwitchLaneRunning relocate shift")
Base::txBool RTree2D_NearestReferenceLine::RegisterLane(hadmap::txLanePtr srclanePtr) TX_NOEXCEPT {
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  // 如果车道有效且未注册
  if (Utils::IsLaneValid(srclanePtr) && CallFail(HasRegister(Base::Info_Lane_t(srclanePtr->getTxLaneId())))) {
#      if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_RTree2D_NearestReferenceLine);
#      endif /*USE_TBB*/
    // 获取车道信息
    const auto &laneUid = Base::Info_Lane_t(srclanePtr->getTxLaneId());
    HdMap::txLaneInfoInterfacePtr laneInfoPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(laneUid);
    if (Null_Pointer(laneInfoPtr)) {
      LOG(FATAL) << TX_VARS_NAME(laneUid, laneUid) << " HdMap::HadmapCacheConCurrent::GetLaneInfoByUid return nullptr.";
      return false;
    }
    // 创建NearestReferenceLineResult对象并添加到结果向量中
    NearestReferenceLineResult locationInfo;
    locationInfo.locInfo = laneUid;
    locationInfo.lanePtr = srclanePtr;
    pImpl->m_locationInfoWithoutDistanceCurve.push_back(locationInfo);
    const Base::txSize locationInfoIndex = pImpl->m_locationInfoWithoutDistanceCurve.size() - 1;

    // 计算车道长度和采样间隔
    const Base::txFloat curLaneLength = laneInfoPtr->GetLength();
    const Base::txFloat sampleInterval =
        ((curLaneLength / 10.0) > FLAGS_MapLocationInterval) ? (FLAGS_MapLocationInterval) : ((curLaneLength / 10.0));
    Base::txFloat curDistance = 0.0;
    std::vector<std::tuple<Base::txVec3, Base::txFloat>> vecSamplePoint;
    vecSamplePoint.reserve((curLaneLength / sampleInterval) + 2);

    // 采样车道上的点
    while (curDistance < curLaneLength) {
      const Base::txVec3 pos(laneInfoPtr->GetLocalPos(curDistance));
      vecSamplePoint.push_back(std::make_tuple(pos, curDistance));
      curDistance += sampleInterval;
    }
    const Base::txVec3 pos(laneInfoPtr->GetEndPt());
    vecSamplePoint.push_back(std::make_tuple(pos, curLaneLength));

    // 创建值类型向量并添加到R树中
    std::vector<value_t> vec_map_location_info;
    vec_map_location_info.reserve(vecSamplePoint.size());
    // 遍历采样点
    for (Base::txSize i = 1; i < vecSamplePoint.size(); ++i) {
      // 获取起始点和结束点的二维坐标
      const auto &startLocalPt = Utils::Vec3_Vec2(std::get<0>(vecSamplePoint[i - 1]));
      const auto &endLocalPt = Utils::Vec3_Vec2(std::get<0>(vecSamplePoint[i]));
      // 获取当前采样点的距离曲线
      const Base::txFloat distanceCurve = std::get<1>(vecSamplePoint[i - 1]);
      segment_t seg(point_t(startLocalPt.x(), startLocalPt.y()), point_t(endLocalPt.x(), endLocalPt.y()));
      vec_map_location_info.push_back(std::make_pair(seg, std::make_tuple(distanceCurve, locationInfoIndex)));
    }
    pImpl->m_rtree_point2maplocationInfo.insert(vec_map_location_info.begin(), vec_map_location_info.end());
    pImpl->m_set_locInfo.insert(laneUid);
    return true;
  } else {
    return false;
  }
}

Base::txBool RTree2D_NearestReferenceLine::RegisterLaneLink(hadmap::txLaneLinkPtr srclanelinkPtr) TX_NOEXCEPT {
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  // 如果车道连接有效且未注册
  if (Utils::IsLaneLinkValid(srclanelinkPtr) &&
      CallFail(HasRegister(
          Base::Info_Lane_t(srclanelinkPtr->getId(), srclanelinkPtr->fromTxLaneId(), srclanelinkPtr->toTxLaneId())))) {
#      if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_RTree2D_NearestReferenceLine);
#      endif /*USE_TBB*/
    // 获取车道连接信息
    const auto &lanelinkUid = HdMap::HadmapCacheConCurrent::MakeLaneLocInfoByLinkId_uncheck(srclanelinkPtr);
    HdMap::txLaneInfoInterfacePtr lanelinkInfoPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(lanelinkUid);
    if (Null_Pointer(lanelinkInfoPtr)) {
      LOG(FATAL) << TX_VARS(lanelinkUid) << " HdMap::HadmapCacheConCurrent::GetLaneLinkInfoById return nullptr.";
      return false;
    }

    // 创建NearestReferenceLineResult对象并添加到结果向量中
    NearestReferenceLineResult locationInfo;
    locationInfo.locInfo = lanelinkUid;
    locationInfo.lanelinkPtr = srclanelinkPtr;
    pImpl->m_locationInfoWithoutDistanceCurve.push_back(locationInfo);
    const Base::txSize locationInfoIndex = pImpl->m_locationInfoWithoutDistanceCurve.size() - 1;

    // 计算车道连接长度和采样间隔
    const Base::txFloat curLaneLinkLength = lanelinkInfoPtr->GetLength();
    const Base::txFloat sampleInterval = ((curLaneLinkLength / 10.0) > FLAGS_MapLocationInterval)
                                             ? (FLAGS_MapLocationInterval)
                                             : ((curLaneLinkLength / 10.0));
    Base::txFloat curDistance = 0.0;
    std::vector<std::tuple<Base::txVec3, Base::txFloat>> vecSamplePoint;
    vecSamplePoint.reserve((curLaneLinkLength / sampleInterval) + 2);
    // 采样车道连接上的点
    while (curDistance < curLaneLinkLength) {
      const Base::txVec3 pos(lanelinkInfoPtr->GetLocalPos(curDistance));
      vecSamplePoint.push_back(std::make_tuple(pos, curDistance));
      curDistance += sampleInterval;
    }
    const Base::txVec3 pos(lanelinkInfoPtr->GetEndPt());
    vecSamplePoint.push_back(std::make_tuple(pos, curLaneLinkLength));

    // 创建值类型向量并添加到R树中
    std::vector<value_t> vec_map_location_info;
    vec_map_location_info.reserve(vecSamplePoint.size());
    for (Base::txSize i = 1; i < vecSamplePoint.size(); ++i) {
      // 获取起始点和结束点
      const auto &startLocalPt = Utils::Vec3_Vec2(std::get<0>(vecSamplePoint[i - 1]));
      const auto &endLocalPt = Utils::Vec3_Vec2(std::get<0>(vecSamplePoint[i]));
      const Base::txFloat distanceCurve = std::get<1>(vecSamplePoint[i - 1]);
      segment_t seg(point_t(startLocalPt.x(), startLocalPt.y()), point_t(endLocalPt.x(), endLocalPt.y()));
      vec_map_location_info.push_back(std::make_pair(seg, std::make_tuple(distanceCurve, locationInfoIndex)));
    }
    pImpl->m_rtree_point2maplocationInfo.insert(vec_map_location_info.begin(), vec_map_location_info.end());
    pImpl->m_set_locInfo.insert(lanelinkUid);
    return true;
  } else {
    return false;
  }
}

#    else
Base::txBool RTree2D_NearestReferenceLine::RegisterLane(hadmap::txLanePtr lanePtr) TX_NOEXCEPT {
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  if (Utils::IsLaneValid(lanePtr) && CallFail(HasRegister(lanePtr->getTxLaneId()))) {
#      if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_RTree2D_NearestReferenceLine);
#      endif /*USE_TBB*/
    const auto &laneUid = lanePtr->getTxLaneId();
    NearestReferenceLineResult locationInfo;
    locationInfo.isOnLaneLink = false;
    locationInfo.onLaneUid = laneUid;
    locationInfo.lanePtr = lanePtr;
    locationInfo.onLinkId = 0;
    pImpl->m_locationInfoWithoutDistanceCurve.push_back(locationInfo);
    const Base::txSize locationInfoIndex = pImpl->m_locationInfoWithoutDistanceCurve.size() - 1;

    const Base::txFloat curLaneLength = lanePtr->getGeometry()->getLength();
    const Base::txFloat sampleInterval =
        ((curLaneLength / 10.0) > FLAGS_MapLocationInterval) ? (FLAGS_MapLocationInterval) : ((curLaneLength / 10.0));
    Base::txFloat curDistance = 0.0;
    std::vector<std::tuple<Coord::txWGS84, Base::txFloat>> vecSamplePoint;
    vecSamplePoint.reserve((curLaneLength / sampleInterval) + 2);

    while (curDistance < curLaneLength) {
      const Coord::txWGS84 pos(lanePtr->getGeometry()->getPoint(curDistance));
      vecSamplePoint.push_back(std::make_tuple(pos, curDistance));
      curDistance += sampleInterval;
    }
    const Coord::txWGS84 pos(lanePtr->getGeometry()->getEnd());
    vecSamplePoint.push_back(std::make_tuple(pos, curLaneLength));

    std::vector<value_t> vec_map_location_info;
    vec_map_location_info.reserve(vecSamplePoint.size());
    for (Base::txSize i = 1; i < vecSamplePoint.size(); ++i) {
      const auto &startLocalPt = std::get<0>(vecSamplePoint[i - 1]).ToENU().ENU2D();
      const auto &endLocalPt = std::get<0>(vecSamplePoint[i]).ToENU().ENU2D();
      const Base::txFloat distanceCurve = std::get<1>(vecSamplePoint[i - 1]);
      segment_t seg(point_t(startLocalPt.x(), startLocalPt.y()), point_t(endLocalPt.x(), endLocalPt.y()));
      vec_map_location_info.push_back(std::make_pair(seg, std::make_tuple(distanceCurve, locationInfoIndex)));
    }
    pImpl->m_rtree_point2maplocationInfo.insert(vec_map_location_info.begin(), vec_map_location_info.end());
    pImpl->m_set_laneuid.insert(laneUid);
    return true;
  } else {
    return false;
  }
}

Base::txBool RTree2D_NearestReferenceLine::RegisterLaneLink(hadmap::txLaneLinkPtr lanelinkPtr) TX_NOEXCEPT {
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  if (Utils::IsLaneLinkValid(lanelinkPtr) && CallFail(HasRegister(lanelinkPtr->getId()))) {
#      if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_RTree2D_NearestReferenceLine);
#      endif /*USE_TBB*/
    const auto &lanelinkUid = lanelinkPtr->getId();
    NearestReferenceLineResult locationInfo;
    locationInfo.isOnLaneLink = true;
    locationInfo.onLinkId = lanelinkUid;
    locationInfo.lanelinkPtr = lanelinkPtr;
    pImpl->m_locationInfoWithoutDistanceCurve.push_back(locationInfo);
    const Base::txSize locationInfoIndex = pImpl->m_locationInfoWithoutDistanceCurve.size() - 1;

    const Base::txFloat curLaneLinkLength = lanelinkPtr->getGeometry()->getLength();
    const Base::txFloat sampleInterval = ((curLaneLinkLength / 10.0) > FLAGS_MapLocationInterval)
                                             ? (FLAGS_MapLocationInterval)
                                             : ((curLaneLinkLength / 10.0));
    Base::txFloat curDistance = 0.0;
    std::vector<std::tuple<Coord::txWGS84, Base::txFloat>> vecSamplePoint;
    vecSamplePoint.reserve((curLaneLinkLength / sampleInterval) + 2);
    while (curDistance < curLaneLinkLength) {
      const Coord::txWGS84 pos(lanelinkPtr->getGeometry()->getPoint(curDistance));
      vecSamplePoint.push_back(std::make_tuple(pos, curDistance));
      curDistance += sampleInterval;
    }
    const Coord::txWGS84 pos(lanelinkPtr->getGeometry()->getEnd());
    vecSamplePoint.push_back(std::make_tuple(pos, curLaneLinkLength));

    std::vector<value_t> vec_map_location_info;
    vec_map_location_info.reserve(vecSamplePoint.size());
    for (Base::txSize i = 1; i < vecSamplePoint.size(); ++i) {
      const auto &startLocalPt = std::get<0>(vecSamplePoint[i - 1]).ToENU().ENU2D();
      const auto &endLocalPt = std::get<0>(vecSamplePoint[i]).ToENU().ENU2D();
      const Base::txFloat distanceCurve = std::get<1>(vecSamplePoint[i - 1]);
      segment_t seg(point_t(startLocalPt.x(), startLocalPt.y()), point_t(endLocalPt.x(), endLocalPt.y()));
      vec_map_location_info.push_back(std::make_pair(seg, std::make_tuple(distanceCurve, locationInfoIndex)));
    }
    pImpl->m_rtree_point2maplocationInfo.insert(vec_map_location_info.begin(), vec_map_location_info.end());
    pImpl->m_set_lanelinkid.insert(lanelinkUid);
    return true;
  } else {
    return false;
  }
}
#    endif

Base::txBool RTree2D_NearestReferenceLine::Initialize(
    const std::map<Base::txLaneUId, hadmap::txLanePtr> &ref_map_laneInfo,
    const std::map<Base::txLaneLinkID, hadmap::txLaneLinkPtr> &ref_map_lanelinkInfo) TX_NOEXCEPT {
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  std::vector<value_t> vec_map_location_info;
  Clear();
  pImpl->m_locationInfoWithoutDistanceCurve.reserve(ref_map_laneInfo.size() + ref_map_lanelinkInfo.size());
  // 注册车道连接
  for (const auto &pair_id_lanelinkPtr : ref_map_lanelinkInfo) {
    RegisterLaneLink(pair_id_lanelinkPtr.second);
  }

  // 注册车道
  for (const auto &pair_id_lanePtr : ref_map_laneInfo) {
    RegisterLane(pair_id_lanePtr.second);
  }

  // 将值类型向量插入到R树中
  pImpl->m_rtree_point2maplocationInfo.insert(vec_map_location_info.begin(), vec_map_location_info.end());
  return true;
}

Base::txBool RTree2D_NearestReferenceLine::Initialize(const hadmap::txLanes &ref_lanesVec,
                                                      const hadmap::txLaneLinks &ref_lanelinksVec) TX_NOEXCEPT {
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  std::vector<value_t> vec_map_location_info;
  Clear();
  pImpl->m_locationInfoWithoutDistanceCurve.reserve(ref_lanesVec.size() + ref_lanelinksVec.size());
  // 注册车道连接
  for (const auto &_lanelinkPtr : ref_lanelinksVec) {
    RegisterLaneLink(_lanelinkPtr);
  }
  // 注册车道
  for (const auto &_lanePtr : ref_lanesVec) {
    RegisterLane(_lanePtr);
  }
  pImpl->m_rtree_point2maplocationInfo.insert(vec_map_location_info.begin(), vec_map_location_info.end());

  // 检查吸收车道信息
  if (FLAGS_CheckAbsorbLaneInfo) {
    CheckAbsorb(ref_lanesVec, ref_lanelinksVec);
  } else {
    LOG(INFO) << "Ignore absorb lane info check.";
  }
  return true;
}

RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine()
    : pImpl(std::make_shared<RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl>()) {}

void RTree2D_NearestReferenceLine::UpdateRtree(const hadmap::txPoint &wgs84GpsPt) TX_NOEXCEPT {
  return;
  const hadmap::txPoint gpsPt3dAlt = __3d_Alt__(wgs84GpsPt);
  // 初始化车道和车道连接
  hadmap::txLanePtr initLane = nullptr;
  hadmap::txLaneLinkPtr initLaneLink = nullptr;
  // 获取车道和车道连接
  hadmap::getLane(HdMap::HadmapCacheConCurrent::GetMapHandler(), gpsPt3dAlt, initLane);
  hadmap::getLaneLink(HdMap::HadmapCacheConCurrent::GetMapHandler(), gpsPt3dAlt, initLaneLink);
  // 注册车道和车道连接
  Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().RegisterLane(initLane);
  Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().RegisterLaneLink(initLaneLink);
  LOG_IF(INFO, FLAGS_LogLevel_RTree) << _StreamPrecision_ << "call hadmap::getLane(mMapHdr, gpsPt, initLane);"
                                     << TX_VARS(Utils::ToString(gpsPt3dAlt))
                                     << TX_VARS_NAME(initLane_is_valid, (initLane ? "[valid]" : "[invalid]"))
                                     << ", call hadmap::getLaneLink(mMapHdr, gpsPt, initLaneLink);"
                                     << TX_VARS(Utils::ToString(gpsPt3dAlt))
                                     << TX_VARS_NAME(initLaneLink_is_valid, (initLaneLink ? "[valid]" : "[invalid]"));
}

hadmap::txLanePtr RTree2D_NearestReferenceLine::GetLaneNearBy(const Base::txFloat _lon, const Base::txFloat _lat,
                                                              Base::txFloat &distanceAlongCurve) TX_NOEXCEPT {
  Base::Info_Lane_t locInfo;
  // 如果获取成功并且不在车道连接上
  if (Get_S_Coord_By_Enu_Pt(Coord::txWGS84(hadmap::txPoint(_lon, _lat, FLAGS_default_altitude)), locInfo,
                            distanceAlongCurve) &&
      CallFail(locInfo.isOnLaneLink)) {
    return HdMap::HadmapCacheConCurrent::GetTxLanePtr(locInfo.onLaneUid);
  }
  return nullptr;
}

void RTree2D_NearestReferenceLine::GetLaneStartPoint(const Base::txFloat _lon, const Base::txFloat _lat,
                                                     Base::txFloat &start_lon, Base::txFloat &start_lat) TX_NOEXCEPT {
  Base::Info_Lane_t locInfo;
  Base::txFloat distanceAlongCurve;
  // 如果获取成功
  if (Get_S_Coord_By_Enu_Pt(Coord::txWGS84(hadmap::txPoint(_lon, _lat, FLAGS_default_altitude)), locInfo,
                            distanceAlongCurve)) {
    hadmap::txLanePtr lanePtr;
    // 如果不在车道连接上
    if (CallFail(locInfo.isOnLaneLink)) {
      lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(locInfo.onLaneUid);
    } else {
      lanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(locInfo.onLinkFromLaneUid);
    }

    // 如果车道指针有效
    if (NonNull_Pointer(lanePtr)) {
      start_lon = lanePtr->getGeometry()->getStart().x;
      start_lat = lanePtr->getGeometry()->getStart().y;
    }
  }
}

Base::txBool RTree2D_NearestReferenceLine::Get_S_Coord_By_Enu_Pt(Coord::txWGS84 aPos, Base::Info_Lane_t &locInfo,
                                                                 Base::txFloat &distanceAlongCurve) TX_NOEXCEPT {
  namespace bgi = boost::geometry::index;
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  // 更新R树
  UpdateRtree(aPos.WGS84());
  // 将WGS84坐标转换为ENU坐标
  const Base::txVec2 &enuPos2d = aPos.ToENU().ENU2D();
  std::vector<value_t> result;
  // 定义函数
  std::function<void(value_t)> f = [&result](value_t v) { result.push_back(v); };
  // 查询R树
  bgi::query(pImpl->m_rtree_point2maplocationInfo, bgi::nearest(point_t(enuPos2d.x(), enuPos2d.y()), 1),
             boost::make_function_output_iterator(f));

  // 如果结果向量大于0
  if (result.size() > 0) {
    Base::txVec2 sTmp;
    const segment_t &hitSeg = result[0].first;
    distanceAlongCurve = std::get<0>(result[0].second);
    // 计算点到线段的距离并更新距离曲线
    distanceAlongCurve += Math::ProjectPoint2Line(
        enuPos2d, Base::txVec2(hitSeg.first.get<0>(), hitSeg.first.get<1>()) TX_MARK("start pt"),
        Base::txVec2(hitSeg.second.get<0>(), hitSeg.second.get<1>()) TX_MARK("end pt"), sTmp);

    const Base::txSize info_index = std::get<1>(result[0].second);
    // 如果信息索引有效
    if (info_index >= 0 && info_index < pImpl->m_locationInfoWithoutDistanceCurve.size()) {
      // 获取引用R树位置信息
      const auto &refRTreeLocInfo = pImpl->m_locationInfoWithoutDistanceCurve[info_index];
      // 如果在车道连接上
      if (refRTreeLocInfo.isOnLaneLink()) {
        // 从车道连接信息更新位置信息
        locInfo.FromLaneLink(refRTreeLocInfo.locInfo.onLinkId_without_equal,
                             refRTreeLocInfo.lanelinkPtr->fromTxLaneId(), refRTreeLocInfo.lanelinkPtr->toTxLaneId());
      } else {
        // 从车道信息更新位置信息
        locInfo.FromLane(refRTreeLocInfo.onLaneUid());
      }
      return true;
    } else {
      LOG(WARNING) << "RTree find, index error.";
      return false;
    }
  } else {
    LOG(WARNING) << "RTree do not find.";
    return false;
  }
}

Base::txBool RTree2D_NearestReferenceLine::Get_ST_Coord_By_Enu_Pt(Coord::txWGS84 aPos, Base::Info_Lane_t &locInfo,
                                                                  Base::txFloat &_S, Base::txFloat &_TT) TX_NOEXCEPT {
  namespace bgi = boost::geometry::index;
  using point_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::point_t;
  using segment_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::segment_t;
  using value_t = RTree2D_NearestReferenceLine::RTree2D_NearestReferenceLine_Impl::value_t;
  // 更新R树
  UpdateRtree(aPos.WGS84());
  // 将WGS84坐标转换为ENU坐标
  const Base::txVec2 &enuPos2d = aPos.ToENU().ENU2D();
  std::vector<value_t> result;
  std::function<void(value_t)> f = [&result](value_t v) { result.push_back(v); };
  // 查询R树
  bgi::query(pImpl->m_rtree_point2maplocationInfo, bgi::nearest(point_t(enuPos2d.x(), enuPos2d.y()), 1),
             boost::make_function_output_iterator(f));

  // 如果结果向量大于0
  if (result.size() > 0) {
    Base::txVec2 sTmp;
    // 获取结果向量中的第一个元素的线段
    const segment_t &hitSeg = result[0].first;

    // 获取线段的起点和终点
    const Base::txVec2 vStart(hitSeg.first.get<0>(), hitSeg.first.get<1>());
    const Base::txVec2 vEnd(hitSeg.second.get<0>(), hitSeg.second.get<1>());

    // 获取结果向量中的第一个元素的距离曲线
    _S = std::get<0>(result[0].second);
    // 计算点到线段的距离并更新距离曲线
    _S += Math::ProjectPoint2Line(enuPos2d, vStart TX_MARK("start pt"), vEnd TX_MARK("end pt"), sTmp);

    const auto sign = Utils::LOR2Sign(Utils::PointOnVector(vStart, vEnd, enuPos2d));

    // 计算TT坐标
    _TT = sign * (enuPos2d - sTmp).norm();

    const Base::txSize info_index = std::get<1>(result[0].second);
    // 如果信息索引有效
    if (info_index >= 0 && info_index < pImpl->m_locationInfoWithoutDistanceCurve.size()) {
      const auto &refRTreeLocInfo = pImpl->m_locationInfoWithoutDistanceCurve[info_index];
      // 如果在车道连接上
      if (refRTreeLocInfo.isOnLaneLink()) {
        // 从车道连接信息更新位置信息
        locInfo.FromLaneLink(refRTreeLocInfo.locInfo.onLinkId_without_equal,
                             refRTreeLocInfo.lanelinkPtr->fromTxLaneId(), refRTreeLocInfo.lanelinkPtr->toTxLaneId());
      } else {
        // 从车道信息更新位置信息
        locInfo.FromLane(refRTreeLocInfo.onLaneUid());
      }
      return true;
    } else {
      LOG(WARNING) << "RTree find, index error.";
      return false;
    }
  } else {
    LOG(WARNING) << "RTree do not find.";
    return false;
  }
}

Base::txBool RTree2D_NearestReferenceLine::TestAbsorb(const hadmap::txPoint gpsMap, const Base::txFloat _sMap,
                                                      const Base::Info_Lane_t &infoMap) TX_NOEXCEPT {
  // 引用位置信息
  Base::Info_Lane_t infoReference;
  Base::txFloat sReference = 0.0;
  Base::txFloat tReference = 0.0;
  // 获取引用S-T坐标
  if (Get_ST_Coord_By_Enu_Pt(Coord::txWGS84(gpsMap), infoReference, sReference, tReference)) {
    // 如果引用位置信息与给定位置信息不相等，或者S坐标差大于1.5
    if (CallFail(infoReference == infoMap) || (std::fabs(_sMap - sReference) > 1.5f)) {
      // 记录警告信息并返回失败
      LOG(WARNING) << "[#####################]" << TX_VARS_NAME(gpsMap, Utils::ToString(gpsMap))
                   << TX_VARS(infoReference) << TX_VARS(infoMap) << TX_VARS(sReference) << TX_VARS(_sMap)
                   << TX_VARS(tReference);
      return false;
    } else {
      return true;
    }
  } else {
    LOG(WARNING) << "[XXXXXXXXXXXXXXXX] Get_ST_Coord_By_Enu_Pt Error" << TX_VARS_NAME(gpsMap, Utils::ToString(gpsMap));
    return false;
  }
}

void RTree2D_NearestReferenceLine::CheckAbsorb(const hadmap::txLanes &ref_lanesVec,
                                               const hadmap::txLaneLinks &ref_lanelinksVec) TX_NOEXCEPT {
#    if 1
  // 遍历车道向量
  for (const auto refLane : ref_lanesVec) {
    // 如果车道和车道几何信息有效
    if (NonNull_Pointer(refLane) && NonNull_Pointer(refLane->getGeometry())) {
      Base::Info_Lane_t infoMap;
      // 从车道ID更新位置信息
      infoMap.FromLane(refLane->getTxLaneId());
      const auto refGeometry = refLane->getGeometry();
      const auto laneLen = refGeometry->getLength();
      const auto startGPS = refGeometry->getStart();
      const auto endGPS = refGeometry->getEnd();
      TestAbsorb(startGPS, 0.0, infoMap);
      Base::txFloat step = 1.0;
      // 遍历车道长度
      while (step < laneLen) {
        TestAbsorb(refGeometry->getPoint(step), step, infoMap);
        step += 1.0;
      }
      TestAbsorb(endGPS, laneLen, infoMap);
    }
  }

  // 遍历车道连接向量
  for (const auto refLink : ref_lanelinksVec) {
    // 如果车道连接和车道连接几何信息有效
    if (NonNull_Pointer(refLink) && NonNull_Pointer(refLink->getGeometry()) &&
        (refLink->getGeometry()->getLength() > 0.0)) {
      Base::Info_Lane_t infoMap;
      // 从车道连接ID更新位置信息
      infoMap.FromLaneLink(refLink->getId(), refLink->fromTxLaneId(), refLink->toTxLaneId());
      const auto refGeometry = refLink->getGeometry();
      const auto laneLen = refGeometry->getLength();
      const auto startGPS = refGeometry->getStart();
      const auto endGPS = refGeometry->getEnd();
      TestAbsorb(startGPS, 0.0, infoMap);
      Base::txFloat step = 1.0;
      // 遍历车道连接长度
      while (step < laneLen) {
        // 测试吸收车道连接上的点
        TestAbsorb(refGeometry->getPoint(step), step, infoMap);
        step += 1.0;
      }
      TestAbsorb(endGPS, laneLen, infoMap);
    }
  }
  LOG(INFO) << "Check absorb lane info finish.";
#    endif
}

#  endif /*__TX_Mark__("RTree2D_NearestReferenceLine")*/

#endif /*USE_GetLaneNearBy*/

#if __TX_Mark__("RTree2DLite")

void RTree2DLite::Clear() TX_NOEXCEPT { bg_tree.clear(); }

void RTree2DLite::RegisterPoint(const Base::txVec2 &localPt, RTree2DLite::RTree2DLiteNode::NodeType _type,
                                Base::Info_Lane_t _locInfo) TX_NOEXCEPT {
#  if USE_TBB
  tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#  endif
  RTree2DLite::RTree2DLiteNodePtr _elem_ptr = std::make_shared<RTree2DLite::RTree2DLiteNode>(_type, _locInfo, localPt);
  // 将点插入到R树中
  bg_tree.insert(std::make_pair(point_t(localPt.x(), localPt.y()), _elem_ptr));
}

Base::txBool RTree2DLite::FindElementsInCircle(const Base::txVec2 &centerPt, const Base::Info_Lane_t locInfo,
                                               const Base::txFloat _CircleRadius,
                                               std::vector<RTree2DLiteNodePtr> &resultElementPtrVec) TX_NOEXCEPT {
  resultElementPtrVec.clear();
  resultElementPtrVec.reserve(20);
  // 如果圆半径大于0
  if (_CircleRadius > 0.0) {
#  if USE_TBB
    tbb::mutex::scoped_lock lock(tbbMutex_bg_tree);
#  endif
    namespace bgi = boost::geometry::index;
    // 定义中心点
    point_t center_pt(centerPt.x(), centerPt.y());
    std::function<void(value_t)> f = [&resultElementPtrVec](value_t v) { resultElementPtrVec.emplace_back(v.second); };
    // 查询满足条件的元素
    bgi::query(bg_tree, bgi::satisfies(circle_filter_squared(center_pt, (_CircleRadius * _CircleRadius), locInfo)),
               boost::make_function_output_iterator(f));
    return true;
  } else {
    return false;
  }
}

Base::txSize RTree2DLite::CountElementInCircle(const Base::txVec2 &centerPt, const Base::Info_Lane_t locInfo,
                                               const Base::txFloat _CircleRadius) TX_NOEXCEPT {
  std::vector<RTree2DLiteNodePtr> resultElementPtrVec;
  // 查找圆内的元素
  if (CallSucc(FindElementsInCircle(centerPt, locInfo, _CircleRadius, resultElementPtrVec))) {
    // 将中心点从ENU坐标转换为WGS84坐标
    Coord::txWGS84 center_gps;
    center_gps.FromENU(Utils::Vec2_Vec3(centerPt));

    // 遍历结果元素向量
    for (const auto &refNode : resultElementPtrVec) {
      // 将其他点从ENU坐标转换为WGS84坐标
      Coord::txWGS84 other_gps;
      other_gps.FromENU(Utils::Vec2_Vec3(refNode->GetEnu2D()));
      /*LOG(INFO) << TX_VARS_NAME(centerPt, Utils::ToString(center_gps.WGS84()))
          << TX_VARS_NAME(center_locInfo, locInfo)
          << TX_VARS_NAME(elemType, Utils::to_underlying(refNode->GetType()))
          << TX_VARS_NAME(otherPt, Utils::ToString(other_gps.WGS84()))
          << TX_VARS_NAME(other_locInfo, refNode->GetLocInfo())
          << TX_VARS_NAME(dist, Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(center_gps, other_gps));*/
    }
    // 返回结果元素向量的大小
    return resultElementPtrVec.size();
  } else {
    return 0;
  }
}

#endif /*__TX_Mark__("RTree2DLite")*/

#if __TX_Mark__("Bulk")

tbb::concurrent_vector<RTree2D::value_t> RTree2D::conVec_bulk_element_info;

void RTree2D::ReleaseBulk() TX_NOEXCEPT { conVec_bulk_element_info.clear(); }

void RTree2D::InsertBulk(const Base::txVec2 &localPt, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  conVec_bulk_element_info.emplace_back(std::make_pair(point_t(localPt.x(), localPt.y()), _elem_ptr));
}

void RTree2D::InsertBulk(const std::vector<Base::txVec2> &localPts, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  for (const auto &pt : localPts) {
    InsertBulk(pt, _elem_ptr);
  }
}

void RTree2D::InsertBulk(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
                         const Base::txVec2 &localPt_3, Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  InsertBulk(localPt_0, _elem_ptr);
  InsertBulk(localPt_1, _elem_ptr);
  InsertBulk(localPt_2, _elem_ptr);
  InsertBulk(localPt_3, _elem_ptr);
}

// 插入批量元素
void RTree2D::InsertBulk(const Base::txVec2 &localPt_0, const Base::txVec2 &localPt_1, const Base::txVec2 &localPt_2,
                         const Base::txVec2 &localPt_3, const Base::txVec2 &localPt_center,
                         Base::ITrafficElementPtr _elem_ptr) TX_NOEXCEPT {
  InsertBulk(localPt_0, _elem_ptr);
  InsertBulk(localPt_1, _elem_ptr);
  InsertBulk(localPt_2, _elem_ptr);
  InsertBulk(localPt_3, _elem_ptr);
  InsertBulk(localPt_center, _elem_ptr);
}

void RTree2D::LoadBulk() TX_NOEXCEPT {
  std::vector<value_t> init_range_vec;
  init_range_vec.reserve(conVec_bulk_element_info.size() * 2);

  // 将conVec_bulk_element_info中的元素复制到init_range_vec中
  std::transform(conVec_bulk_element_info.begin(), conVec_bulk_element_info.end(), std::back_inserter(init_range_vec),
                 [&](const RTree2D::value_t &p) { return p; });
  // 将init_range_vec赋值给bg_tree
  pImpl->bg_tree = RTree2D::RTree2DImpl::rtree(init_range_vec);
}

#endif /*__TX_Mark__("Bulk")*/

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)

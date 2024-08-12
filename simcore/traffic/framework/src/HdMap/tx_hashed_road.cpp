// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/tx_hashed_road.h"
#include <math.h> /*scalbn*/
#include <boost/iterator/function_output_iterator.hpp>
#include <fstream>
#include <queue>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_MapSDK)
#if USE_HashedRoadNetwork
TX_NAMESPACE_OPEN(TBB)

class parallel_create_hashed_point_lane {
 public:
  using HashedRoadNetworkRTree2D = Geometry::SpatialQuery::HashedRoadNetworkRTree2D;
  using HashedLaneInfo = HashedRoadNetworkRTree2D::HashedLaneInfo;
  using HashedLaneInfoPtr = HashedRoadNetworkRTree2D::HashedLaneInfoPtr;

 public:
  // 为选中的代码添加中文注释
  void operator()(const tbb::blocked_range<size_t>& r) const {
    // 获取哈希范围长度
    const Base::txFloat scopeLen = HashedLaneInfo::HashScopeLen();
    // 获取子区间幂次
    const Base::txInt subSectionPower = -1 * FLAGS_SubSectionPower;
    TX_MARK("FLAGS_SubSectionPower = 4");
    // 遍历车道指针向量
    for (size_t i = r.begin(); i != r.end(); ++i) {
      // 获取车道指针
      const hadmap::txLanePtr refLanePtr = m_uniqueLanes[i];
      // 如果车道有效
      if (Utils::IsLaneValid(refLanePtr) TX_MARK("lane length > 0.0")) {
        // 获取车道信息指针
        HdMap::txLaneInfoInterfacePtr geom_ptr =
            HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(refLanePtr->getTxLaneId());
        // 获取车道长度
        const Base::txFloat dbLaneLength = geom_ptr->GetLength();
        // const Base::txFloat dbLaneLength = refLanePtr->getGeometry()->getLength();
        const Base::txSize nSize = scalbn(dbLaneLength, subSectionPower);
        // 初始化子区间起始位置和索引
        Base::txFloat scopeStart = 0.0;
        Base::txSize subsectionIdx = 0;
        // 遍历子区间
        for (; subsectionIdx < nSize; ++subsectionIdx, scopeStart += scopeLen) {
          // 创建哈希车道信息指针
          HashedLaneInfoPtr elemPtr = std::make_shared<HashedLaneInfo>();
          elemPtr->m_is_valid_without_hash = true;
          // 设置哈希车道信息车道长度
          elemPtr->m_curveLength = dbLaneLength;
          elemPtr->m_laneinfo.isOnLaneLink = false;
          elemPtr->m_laneinfo.onLaneUid = refLanePtr->getTxLaneId();
          // 设置哈希车道信息子区间索引
          elemPtr->m_subSectionIdx = subsectionIdx;
          // 设置哈希车道信息子区间范围
          elemPtr->m_subSectionScope = std::make_pair(scopeStart, scopeStart + scopeLen);
          elemPtr->m_geom_ptr_without_hash = geom_ptr;
          elemPtr->subSectionStartWGS84 = Coord::txWGS84(refLanePtr->getGeometry()->getPoint(scopeStart));
          // 将哈希车道信息插入到哈希车道网络RTree中
          HashedRoadNetworkRTree2D::getInstance().Insert(elemPtr->subSectionStartWGS84.ToENU().ENU2D(), elemPtr);
          // 将哈希车道信息添加到哈希车道缓存中
          HdMap::HashedRoadCacheConCurrent::Add_OrthogonalList(*elemPtr);
        }
        // 如果子区间起始位置小于车道长度，处理最后一个子区间
        if (scopeStart < dbLaneLength) {
          // 创建哈希车道信息指针
          HashedLaneInfoPtr elemPtr = std::make_shared<HashedLaneInfo>();
          elemPtr->m_is_valid_without_hash = true;
          // 设置哈希车道信息车道长度
          elemPtr->m_curveLength = dbLaneLength;
          elemPtr->m_laneinfo.isOnLaneLink = false;
          elemPtr->m_laneinfo.onLaneUid = refLanePtr->getTxLaneId();
          // 设置哈希车道信息子区间索引
          elemPtr->m_subSectionIdx = subsectionIdx;
          elemPtr->m_subSectionScope = std::make_pair(scopeStart, dbLaneLength);
          elemPtr->m_geom_ptr_without_hash = geom_ptr;
          // 设置哈希车道信息子区间起始经纬度坐标
          elemPtr->subSectionStartWGS84 = Coord::txWGS84(refLanePtr->getGeometry()->getPoint(scopeStart));
          // 将哈希车道信息插入到哈希车道网络RTree中
          HashedRoadNetworkRTree2D::getInstance().Insert(elemPtr->subSectionStartWGS84.ToENU().ENU2D(), elemPtr);
          // 将哈希车道信息添加到哈希车道缓存中
          HdMap::HashedRoadCacheConCurrent::Add_OrthogonalList(*elemPtr);
        }
      }
    }
  }

  explicit parallel_create_hashed_point_lane(const hadmap::txLanes& uniqueLanes) : m_uniqueLanes(uniqueLanes) {}

 protected:
  const hadmap::txLanes& m_uniqueLanes;
};

class parallel_create_hashed_point_lanelink {
 public:
  using HashedRoadNetworkRTree2D = Geometry::SpatialQuery::HashedRoadNetworkRTree2D;
  using HashedLaneInfo = HashedRoadNetworkRTree2D::HashedLaneInfo;
  using HashedLaneInfoPtr = HashedRoadNetworkRTree2D::HashedLaneInfoPtr;

 public:
  void operator()(const tbb::blocked_range<size_t>& r) const {
    // 获取哈希范围长度
    const Base::txFloat scopeLen = HashedLaneInfo::HashScopeLen();
    // 获取子区间幂次
    const Base::txInt subSectionPower = -1 * FLAGS_SubSectionPower;
    TX_MARK("FLAGS_SubSectionPower = 4");
    // 遍历车道连接指针向量
    for (size_t i = r.begin(); i != r.end(); ++i) {
      // 获取车道连接指针
      const hadmap::txLaneLinkPtr refLinkPtr = m_uniqueLaneLinks[i];
      // 如果车道连接有效
      if (Utils::IsLaneLinkValid(refLinkPtr) TX_MARK("lane length > 0.0")) {
        // 获取车道连接信息
        const Base::Info_Lane_t link_uid(refLinkPtr->getId(), refLinkPtr->fromTxLaneId(), refLinkPtr->toTxLaneId());
        // 获取车道连接信息指针
        HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(link_uid);
        // 获取车道连接长度
        const Base::txFloat dbLinkLength = geom_ptr->GetLength();
        // const Base::txFloat dbLinkLength = refLinkPtr->getGeometry()->getLength();
        const Base::txSize nSize = scalbn(dbLinkLength, subSectionPower);
        // 初始化子区间起始位置和索引
        Base::txFloat scopeStart = 0.0;
        Base::txSize subsectionIdx = 0;
        for (; subsectionIdx < nSize; ++subsectionIdx, scopeStart += scopeLen) {
          // 创建哈希车道信息指针
          HashedLaneInfoPtr elemPtr = std::make_shared<HashedLaneInfo>();
          // 设置哈希车道信息有效性
          elemPtr->m_is_valid_without_hash = true;
          // 设置哈希车道信息车道长度
          elemPtr->m_curveLength = dbLinkLength;
          // 设置哈希车道信息车道连接信息
          elemPtr->m_laneinfo =
              link_uid;  // .FromLaneLink(refLinkPtr->getId(), refLinkPtr->fromTxLaneId(), refLinkPtr->toTxLaneId());
          // 设置哈希车道信息子区间索引
          elemPtr->m_subSectionIdx = subsectionIdx;
          // 设置哈希车道信息子区间范围
          elemPtr->m_subSectionScope = std::make_pair(scopeStart, scopeStart + scopeLen);
          // 设置哈希车道信息车道信息指针
          elemPtr->m_geom_ptr_without_hash = geom_ptr;
          // 设置哈希车道信息子区间起始经纬度坐标
          elemPtr->subSectionStartWGS84 = Coord::txWGS84(refLinkPtr->getGeometry()->getPoint(scopeStart));
          // 将哈希车道信息插入到哈希车道网络RTree中
          HashedRoadNetworkRTree2D::getInstance().Insert(elemPtr->subSectionStartWGS84.ToENU().ENU2D(), elemPtr);
          // 将哈希车道信息添加到哈希车道缓存中
          HdMap::HashedRoadCacheConCurrent::Add_OrthogonalList(*elemPtr);
          // 将哈希车道信息添加到哈希车道连接缓存中
          HdMap::HashedRoadCacheConCurrent::Add_LaneLinkLocInfoToRoad(link_uid.onLinkToLaneUid.roadId, *elemPtr);
        }
        // 如果子区间起始位置小于车道连接长度，处理最后一个子区间
        if (scopeStart < dbLinkLength) {
          // 创建哈希车道信息指针
          HashedLaneInfoPtr elemPtr = std::make_shared<HashedLaneInfo>();
          // 设置哈希车道信息有效性
          elemPtr->m_is_valid_without_hash = true;
          // 设置哈希车道信息车道长度
          elemPtr->m_curveLength = dbLinkLength;
          // 设置哈希车道信息车道连接信息
          elemPtr->m_laneinfo =
              link_uid;  // .FromLaneLink(refLinkPtr->getId(), refLinkPtr->fromTxLaneId(), refLinkPtr->toTxLaneId());
          // 设置哈希车道信息子区间索引
          elemPtr->m_subSectionIdx = subsectionIdx;
          // 设置哈希车道信息子区间范围
          elemPtr->m_subSectionScope = std::make_pair(scopeStart, dbLinkLength);
          // 设置哈希车道信息车道信息指针
          elemPtr->m_geom_ptr_without_hash = geom_ptr;
          // 设置哈希车道信息子区间起始经纬度坐标
          elemPtr->subSectionStartWGS84 = Coord::txWGS84(refLinkPtr->getGeometry()->getPoint(scopeStart));
          // 将哈希车道信息插入到哈希车道网络RTree中
          HashedRoadNetworkRTree2D::getInstance().Insert(elemPtr->subSectionStartWGS84.ToENU().ENU2D(), elemPtr);
          // 将哈希车道信息添加到哈希车道缓存中
          HdMap::HashedRoadCacheConCurrent::Add_OrthogonalList(*elemPtr);
          // 将哈希车道信息添加到哈希车道连接缓存中
          HdMap::HashedRoadCacheConCurrent::Add_LaneLinkLocInfoToRoad(link_uid.onLinkToLaneUid.roadId, *elemPtr);
        }
      }
    }
  }

  explicit parallel_create_hashed_point_lanelink(const hadmap::txLaneLinks& uniqueLaneLinks)
      : m_uniqueLaneLinks(uniqueLaneLinks) {}

 protected:
  const hadmap::txLaneLinks& m_uniqueLaneLinks;
};

TX_NAMESPACE_CLOSE(TBB)

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(SpatialQuery)

#  if USE_HashedRoadNetwork

Base::txBool HashedRoadNetworkRTree2D::Initialize(const hadmap::txLanes& uniqueLanes,
                                                  const hadmap::txLaneLinks& uniqueLaneLinks) TX_NOEXCEPT {
  // 释放资源
  Release();
  using namespace tbb;
  // 获取车道数量
  const Base::txSize nLaneSize = uniqueLanes.size();
  // 使用TBB并行处理车道
  tbb::parallel_for(blocked_range<size_t>(0, nLaneSize), TBB::parallel_create_hashed_point_lane(uniqueLanes));

  // 获取车道连接数量
  const Base::txSize nLinkSize = uniqueLaneLinks.size();
  // 使用TBB并行处理车道连接
  tbb::parallel_for(blocked_range<size_t>(0, nLinkSize), TBB::parallel_create_hashed_point_lanelink(uniqueLaneLinks));

  // 创建哈希车道信息指针向量
  std::vector<HashedLaneInfoPtr> vecAllHashedPt;
  vecAllHashedPt.reserve(m_rtree_hashedpoint.size());
  // 输出哈希车道信息到文件
#    define _hashed_output_ (1)
#    if _hashed_output_
  std::ofstream outfile("d:/hashedpt.txt");
  outfile << "x,y" << std::endl;
#    endif /*_hashed_output_*/
  // 将哈希车道信息添加到向量中
  for (const auto& v : m_rtree_hashedpoint) {
    vecAllHashedPt.emplace_back(v.second);
#    if _hashed_output_
    outfile << _StreamPrecision_ << (v.second->subSectionStartWGS84.Lon()) << ","
            << (v.second->subSectionStartWGS84.Lat()) << std::endl;
#    endif /*_hashed_output_*/
  }
#    if _hashed_output_
  outfile.close();
#    endif /*_hashed_output_*/

#    if 0
  // 获取哈希车道信息数量
  const Base::txSize nHashedPtSize = vecAllHashedPt.size();
  // 使用TBB并行处理哈希车道信息
  tbb::parallel_for(blocked_range<size_t>(0, nHashedPtSize), TBB::parallel_create_hashed_point_related(vecAllHashedPt));
  tbb::parallel_for(blocked_range<size_t>(0, nHashedPtSize), TBB::parallel_create_hashed_point_fblr(vecAllHashedPt));
#    endif
  // 生成正交列表
  LOG(WARNING) << "before Generate_OrthogonalList "
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfo>::howMany())
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfoOrthogonalList>::howMany());
  HdMap::HashedRoadCacheConCurrent::Generate_OrthogonalList();
  LOG(WARNING) << "after Generate_OrthogonalList "
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfo>::howMany())
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfoOrthogonalList>::howMany());
  // 检查正交列表
  LOG(INFO) << "call HdMap::HashedRoadCacheConCurrent::Check_OrthogonalList()";
  HdMap::HashedRoadCacheConCurrent::Check_OrthogonalList();
  LOG(INFO) << "finish HdMap::HashedRoadCacheConCurrent::Check_OrthogonalList()";
  // 生成会议信息
#    if __Meeting__
  if (FLAGS_Generate_Meeting) {
    LOG(INFO) << "begin generate meeting...";
    HdMap::HashedRoadCacheConCurrent::Generate_Meeting();
    LOG(INFO) << "generate meeting finish";
  } else {
    LOG(WARNING) << "do not generate meeting.";
  }
#    endif /*__Meeting__*/
  return true;
}

void HashedRoadNetworkRTree2D::Insert(const Base::txVec2& localPt,
                                      HashedRoadNetworkRTree2D::HashedLaneInfoPtr _elem_ptr) TX_NOEXCEPT {
  // LOG(WARNING) << TX_VARS_NAME(localPt, Utils::ToString(localPt));
  tbb::mutex::scoped_lock lock(tbbMutex_rtree_hashedpoint);
  // 将哈希车道信息插入到哈希车道网络RTree中
  m_rtree_hashedpoint.insert(std::make_pair(point_t(localPt.x(), localPt.y()), _elem_ptr));
}

Base::txBool HashedRoadNetworkRTree2D::FindHashedPointInArea(
    const Base::txVec2& centerPt, const std::vector<Base::txVec2>& localPts_clockwise_close,
    std::vector<HashedRoadNetworkRTree2D::HashedLaneInfoPtr>& resultElementPtrVec) TX_NOEXCEPT {
  // 清空结果向量
  resultElementPtrVec.clear();
  // 预留空间
  resultElementPtrVec.reserve(30);
  // 如果指定区域的顶点数量大于3，继续处理
  if (localPts_clockwise_close.size() > 3) {
    namespace bgi = boost::geometry::index;
    // 创建多边形
    poly_t _polygon;
    for (const auto& pt : localPts_clockwise_close) {
      boost::geometry::append(_polygon.outer(), point_t(pt.x(), pt.y()));
    }

    // 创建哈希车道信息映射
    std::unordered_map<HashedLaneInfo, HashedLaneInfoPtr, HashedLaneInfo::HashCompare> result_HashedPt_Map;
    // 定义查询函数
    std::function<void(HashedRoadNetworkRTree2D::value_t)> f =
        [&result_HashedPt_Map](HashedRoadNetworkRTree2D::value_t v) { result_HashedPt_Map[*(v.second)] = (v.second); };
    /*std::function<void(value_t)> f = [&resultElementPtrVec](value_t v) { resultElementPtrVec.push_back((v.second));
     * };*/
    {
      tbb::mutex::scoped_lock lock(tbbMutex_rtree_hashedpoint);
      // 查询哈希车道网络RTree中与多边形相交的哈希车道信息
      bgi::query(m_rtree_hashedpoint, bgi::intersects(_polygon), boost::make_function_output_iterator(f));
    }
    // 如果查询结果非空，将结果添加到结果向量中
    if (_NonEmpty_(result_HashedPt_Map)) {
      for (const auto sysid_ptr_pair : result_HashedPt_Map) {
        resultElementPtrVec.emplace_back(sysid_ptr_pair.second);
      }
    }
    return true;
  } else {
    return false;
  }
}

std::vector<Base::txVec2> HashedRoadNetworkRTree2D::GenerateSearchClockWise(const Base::txVec2& center,
                                                                            const Base::txVec3& vAhead) TX_NOEXCEPT {
  // 初始化结果向量
  std::vector<Base::txVec2> retVec;
  retVec.resize(5);
  // 初始化轴向量
  Base::txVec2 Axis[2];
  // 设置轴向量
  Axis[0] = Utils::Vec3_Vec2(vAhead);
  Axis[1] =
      Utils::Vec3_Vec2(Utils::VetRotVecByDegree(vAhead, Unit::txDegree::MakeDegree(LeftOnENU), Utils::Axis_Enu_Up()));
  // 设置搜索区域半尺寸
  Base::txFloat halfSize[2] = {0.5 * SearchLength(), 0.5 * (SearchWidth())};

  // 计算顺时针搜索区域的顶点
  retVec[0] = (center + Axis[0] * halfSize[0] * (1.0) + Axis[1] * halfSize[1] * (1.0));   /*( 1,  1)*/
  retVec[1] = (center + Axis[0] * halfSize[0] * (1.0) + Axis[1] * halfSize[1] * (-1.0));  /*( 1, -1)*/
  retVec[2] = (center + Axis[0] * halfSize[0] * (-1.0) + Axis[1] * halfSize[1] * (-1.0)); /*(-1, -1)*/
  retVec[3] = (center + Axis[0] * halfSize[0] * (-1.0) + Axis[1] * halfSize[1] * (1.0));  /*(-1,  1)*/
  retVec[4] = retVec[0];
  return retVec;
}
#  endif /*USE_HashedRoadNetwork*/

TX_NAMESPACE_CLOSE(SpatialQuery)
TX_NAMESPACE_CLOSE(Geometry)

TX_NAMESPACE_OPEN(HdMap)

Base::txBool HashedRoadCacheConCurrent::sValid;
HashedRoadCacheConCurrent::hashedRoad2OrthogonalList HashedRoadCacheConCurrent::s_hashedPt2OrthogonalListPtr;
HashedRoadCacheConCurrent::hashedRoad2CloseLaneEvent HashedRoadCacheConCurrent::s_close_lane_event;
HashedRoadCacheConCurrent::hashedRoad2SurrouingHashedPt HashedRoadCacheConCurrent::s_hashedPt2RelatedHashedPts;
HashedRoadCacheConCurrent::roadId2LaneLinkSetToThisRoadType HashedRoadCacheConCurrent::s_lanelinkSetToRoadId;
#  if __Meeting__
HashedRoadCacheConCurrent::hashedRoad2MeetingHashedInfo HashedRoadCacheConCurrent::s_hashed2MeetingHashedInfo;
HashedRoadCacheConCurrent::hashedRoad2MeetingGPS HashedRoadCacheConCurrent::s_hashed2MeetingGPS;
#  endif /*__Meeting__*/
void HashedRoadCacheConCurrent::Release() TX_NOEXCEPT {
  sValid = false;
  s_hashedPt2OrthogonalListPtr.clear();
  s_close_lane_event.clear();
  s_hashedPt2RelatedHashedPts.clear();
  s_lanelinkSetToRoadId.clear();
#  if __Meeting__
  s_hashed2MeetingHashedInfo.clear();
  s_hashed2MeetingGPS.clear();
#  endif /*__Meeting__*/
}

// 注册清除哈希车道信息
void HashedRoadCacheConCurrent::RegisterElementClear() TX_NOEXCEPT {
  tbb::parallel_for(s_hashedPt2OrthogonalListPtr.range(), [](const hashedRoad2OrthogonalList::range_type& r) {
    // 遍历哈希车道信息
    for (hashedRoad2OrthogonalList::iterator itr = r.begin(); itr != r.end(); itr++) {
      // 清除哈希车道信息
      itr->second->Clear();
    }
  });
}

void HashedRoadCacheConCurrent::ShowDebugInfo() TX_NOEXCEPT {
  // 遍历哈希车道信息
  for (const auto& ref : s_hashedPt2OrthogonalListPtr) {
    LOG(INFO) << ref.first;
  }
}

void HashedRoadCacheConCurrent::Add_OrthogonalList(const HashedLaneInfo& _objInfo) TX_NOEXCEPT {
  hashedRoad2OrthogonalList::const_accessor ca;
  // 如果在哈希车道信息到正交列表中找不到指定的哈希车道信息
  if (CallFail(s_hashedPt2OrthogonalListPtr.find(ca, _objInfo))) {
    ca.release();
    // 创建哈希车道信息到正交列表的访问器
    hashedRoad2OrthogonalList::accessor a;
    // 在哈希车道信息到正交列表中插入指定的哈希车道信息
    s_hashedPt2OrthogonalListPtr.insert(a, _objInfo);
    // 创建一个新的正交列表并将其与哈希车道信息关联
    (a->second) = std::make_shared<HashedLaneInfoOrthogonalList>(_objInfo);
    a.release();
  }
}

HashedRoadCacheConCurrent::HashedLaneInfoOrthogonalListPtr HashedRoadCacheConCurrent::Query_OrthogonalList(
    const HashedLaneInfo& _objInfo) TX_NOEXCEPT {
  // 创建哈希车道信息到正交列表的常量访问器
  hashedRoad2OrthogonalList::const_accessor ca;
  // 如果在哈希车道信息到正交列表中找到指定的哈希车道信息
  if (CallSucc(s_hashedPt2OrthogonalListPtr.find(ca, _objInfo))) {
    return ca->second;
  } else {
    return nullptr;
  }
}

void HashedRoadCacheConCurrent::AddRelatedHashedPt(const HashedLaneInfo& _objInfo,
                                                   const std::list<HashedLaneInfo>& listRelatedPt) TX_NOEXCEPT {
  // 创建哈希车道信息到相关哈希点的常量访问器
  hashedRoad2SurrouingHashedPt::const_accessor ca;
  // 如果在哈希车道信息到相关哈希点中找不到指定的哈希车道信息
  if (CallFail(s_hashedPt2RelatedHashedPts.find(ca, _objInfo))) {
    // 释放访问器
    ca.release();
    // 创建哈希车道信息到相关哈希点的访问器
    hashedRoad2SurrouingHashedPt::accessor a;
    // 在哈希车道信息到相关哈希点中插入指定的哈希车道信息
    s_hashedPt2RelatedHashedPts.insert(a, _objInfo);
    // 将相关哈希点列表与哈希车道信息关联
    (a->second) = listRelatedPt;
    a.release();

#  define _related_hashed_output_ (0)
#  if _related_hashed_output_
    std::ostringstream oss;
    oss << "x,y" << std::endl;
    oss << _StreamPrecision_ << _objInfo.subSectionStartWGS84.Lon() << "," << _objInfo.subSectionStartWGS84.Lat()
        << std::endl;
    for (const auto ptr : vecRelatedPt) {
      oss << _StreamPrecision_ << (ptr->subSectionStartWGS84).Lon() << "," << (ptr->subSectionStartWGS84).Lat()
          << std::endl;
    }
    std::stringstream ss;
    ss << "D:/1.HashedRelated/" << _objInfo.m_laneinfo << "_" << _objInfo.m_subSectionIdx << ".txt";
    std::ofstream outfile(ss.str());
    outfile << oss.str();
    outfile.close();
#  endif /*_related_hashed_output_*/
#  undef _related_hashed_output_
  }
}

void HashedRoadCacheConCurrent::QuerySurroundingVehicles(const HashedLaneInfo& _objInfo,
                                                         VehicleContainer& refVecContainer) TX_NOEXCEPT {
  // 创建一个相关哈希车道信息列表
  std::list<HashedLaneInfo> refRelativeHashedLanes;
  // 查询与指定哈希车道信息相关的哈希车道信息列表
  QueryRelatedHashedPt(_objInfo, refRelativeHashedLanes);
  // 查询与相关哈希车道信息列表相关的车辆并将其添加到车辆容器中
  QueryRegisterVehicles(refRelativeHashedLanes, refVecContainer);
}

void HashedRoadCacheConCurrent::QueryRelatedHashedPt(const HashedLaneInfo& _objInfo,
                                                     std::list<HashedLaneInfo>& refRelativeHashedLanes) TX_NOEXCEPT {
  // 创建哈希车道信息到相关哈希点的常量访问器
  hashedRoad2SurrouingHashedPt::const_accessor ca;
  // 清空相关哈希车道信息列表
  refRelativeHashedLanes.clear();
  // 如果在哈希车道信息到相关哈希点中找到指定的哈希车道信息
  if (CallSucc(s_hashedPt2RelatedHashedPts.find(ca, _objInfo))) {
    refRelativeHashedLanes = ca->second;
    ca.release();
  }
}

#  if __Meeting__
void HashedRoadCacheConCurrent::AddMeetingHashedInfo(const HashedLaneInfo& _objInfo,
                                                     const MeetingHashLaneInfoPtrList& listMeetingHashedInfoPtr,
                                                     const Coord::txENU meetingPt) TX_NOEXCEPT {
  {
    // 创建哈希车道信息到会议哈希信息的访问器
    hashedRoad2MeetingHashedInfo::accessor a;
    // 在哈希车道信息到会议哈希信息的映射中插入指定的哈希车道信息
    s_hashed2MeetingHashedInfo.insert(a, _objInfo);
    // 将会议哈希信息列表添加到哈希车道信息对应的会议哈希信息列表中
    a->second.insert((a->second).end(), listMeetingHashedInfoPtr.begin(), listMeetingHashedInfoPtr.end());
    a.release();
  }
  {
    // 创建哈希车道信息到会议坐标的访问器
    hashedRoad2MeetingGPS::accessor a;
    // 在哈希车道信息到会议坐标的映射中插入指定的哈希车道信息
    s_hashed2MeetingGPS.insert(a, _objInfo);
    a->second = meetingPt;
    a.release();
  }
}

Base::txBool HashedRoadCacheConCurrent::IsInMeetingRegion(const HashedLaneInfo& _objInfo,
                                                          Coord::txENU& meetingPt) TX_NOEXCEPT {
  // 创建哈希车道信息到会议坐标的常量访问器
  hashedRoad2MeetingGPS::const_accessor ca;
  // 如果在哈希车道信息到会议坐标的映射中找到指定的哈希车道信息
  if (CallSucc(s_hashed2MeetingGPS.find(ca, _objInfo))) {
    // 将找到的会议坐标赋值给输出参数
    meetingPt = ca->second;
    return true;
  } else {
    return false;
  }
}

Base::txBool HashedRoadCacheConCurrent::IsInMeetingRegion(const HashedLaneInfo& _objInfo) TX_NOEXCEPT {
  hashedRoad2MeetingGPS::const_accessor ca;
  return CallSucc(s_hashed2MeetingGPS.find(ca, _objInfo));
}

void HashedRoadCacheConCurrent::QueryMeetingHashedInfo(
    const HashedLaneInfo& _objInfo, MeetingHashLaneInfoPtrList& refMeetingHashedLanePtrList) TX_NOEXCEPT {
  // 创建哈希车道信息到会议哈希信息的常量访问器
  hashedRoad2MeetingHashedInfo::const_accessor ca;
  refMeetingHashedLanePtrList.clear();
  // 如果在哈希车道信息到会议哈希信息的映射中找到指定的哈希车道信息
  if (CallSucc(s_hashed2MeetingHashedInfo.find(ca, _objInfo))) {
    // 将找到的会议哈希信息列表赋值给输出参数
    refMeetingHashedLanePtrList = ca->second;
    ca.release();
  }
}

void HashedRoadCacheConCurrent::QueryRegisterVehicles(const MeetingHashLaneInfoPtrList& refRelativeHashedLanePtrList,
                                                      VehicleContainer& refVecContainer) TX_NOEXCEPT {
  refVecContainer.clear();
  // 遍历会议哈希信息列表
  for (const auto& hashedPtr : refRelativeHashedLanePtrList) {
    // 如果会议哈希信息指针非空
    if (NonNull_Pointer(hashedPtr)) {
      VehicleContainer curContainer;
      // 获取与当前会议哈希信息相关的车辆
      GetHashedPtVehicles(*hashedPtr, curContainer);
      // 将找到的车辆添加到输出参数的车辆容器中
      refVecContainer.insert(curContainer.begin(), curContainer.end());
    }
  }
}

void HashedRoadCacheConCurrent::QueryMeetingVehicles(const HashedLaneInfo& _objInfo,
                                                     VehicleContainer& refVecContainer) TX_NOEXCEPT {
  // 创建一个会议哈希信息指针列表
  MeetingHashLaneInfoPtrList refRelativeHashedLanePtrList;
  // 查询与指定哈希车道信息相关的会议哈希信息
  QueryMeetingHashedInfo(_objInfo, refRelativeHashedLanePtrList);
  // 查询与指定会议哈希信息列表相关的车辆
  QueryRegisterVehicles(refRelativeHashedLanePtrList, refVecContainer);
}

void HashedRoadCacheConCurrent::Generate_Meeting() TX_NOEXCEPT {
  // 使用TBB并行遍历哈希车道信息到正交列表的映射
  tbb::parallel_for(s_hashedPt2OrthogonalListPtr.range(), [](const hashedRoad2OrthogonalList::range_type& r) {
    // 遍历哈希车道信息到正交列表的映射
    for (hashedRoad2OrthogonalList::iterator itr = r.begin(); itr != r.end(); itr++) {
      // 获取当前正交列表指针
      HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr = ((*itr).second);
      // 如果正交列表指针非空
      if (NonNull_Pointer(curOrthogonalListPtr)) {
        // 获取自身哈希车道信息
        const HashedLaneInfo& refSelfHashedInfo = curOrthogonalListPtr->GetSelfHashedInfo();
        // 将自身哈希车道信息的起始WGS84坐标转换为ENU坐标
        Coord::txENU meetingGPS;
        meetingGPS.FromWGS84(refSelfHashedInfo.subSectionStartWGS84.Lon(),
                             refSelfHashedInfo.subSectionStartWGS84.Lat());
        // 如果正交列表中的后续节点数量大于1
        if ((curOrthogonalListPtr->GetBackNodes()).size() > 1) {
          // 获取后续哈希车道信息列表
          std::list<HashedLaneInfoPtr> back_hashed;
          GetBackHashedLaneInfoList(refSelfHashedInfo, back_hashed, 5);
          // 遍历后续哈希车道信息列表
          for (HashedLaneInfoPtr refHashedNodePtr : back_hashed) {
            // 获取当前后续哈希车道信息
            const HashedLaneInfo& refHashedNode = *refHashedNodePtr;
            // 添加会议哈希信息
            AddMeetingHashedInfo(refHashedNode, back_hashed, meetingGPS);
          }
        }
      }
    }
  });
  LOG(INFO) << TX_VARS(s_hashed2MeetingHashedInfo.size());
  auto range = s_hashed2MeetingHashedInfo.range();
  if (FLAGS_LogLevel_MapSDK) {
    // 遍历会议哈希信息
    for (auto itr = range.begin(); itr != range.end(); ++itr) {
      // 输出会议哈希信息的起始点和哈希车道信息
      LOG(INFO) << "[meeting_debug]" << TX_VARS_NAME(start_pt, (*itr).first.subSectionStartWGS84.StrWGS84())
                << TX_VARS_NAME(hash_lane_info, (*itr).first.LaneInfo());
    }
  }
}
#  endif /*__Meeting__*/

void HashedRoadCacheConCurrent::Generate_OrthogonalList() TX_NOEXCEPT {
  auto range = s_hashedPt2OrthogonalListPtr.range();
  std::vector<HashedLaneInfoOrthogonalListPtr> vecAllOrthogonalListPtr;
  // 遍历哈希车道信息到正交列表的映射
  for (auto itr = range.begin(); itr != range.end(); ++itr) {
    // 将正交列表指针添加到向量中
    vecAllOrthogonalListPtr.emplace_back((*itr).second);
  }
#  if 1

  tbb::parallel_for(static_cast<std::size_t>(0), vecAllOrthogonalListPtr.size(), [&](const std::size_t idx) {
    // 获取当前索引对应的正交列表指针
    HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr = vecAllOrthogonalListPtr[idx];
    // 如果正交列表指针为空，则返回
    if (Null_Pointer(curOrthogonalListPtr)) {
      return;
    }
    // 清除动态元素信息
    curOrthogonalListPtr->ClearDynamicElementInfo();
    // 清除正交列表
    curOrthogonalListPtr->ClearOrthogonalList();
    // 获取自身哈希车道信息
    const HashedLaneInfo& refSelfHashedInfo = curOrthogonalListPtr->GetSelfHashedInfo();
    // 查询左侧哈希车道信息
    {
      TX_MARK("left node.");
      HashedLaneInfoPtr leftHashedLaneInfoPtr = nullptr;
      Geometry::SpatialQuery::GetLeftHashedLaneInfo(refSelfHashedInfo, leftHashedLaneInfoPtr);
      // 如果左侧哈希车道信息指针非空
      if (NonNull_Pointer(leftHashedLaneInfoPtr)) {
        // 设置左侧节点为查询到的正交列表
        curOrthogonalListPtr->SetLeftNode(HashedRoadCacheConCurrent::Query_OrthogonalList(*leftHashedLaneInfoPtr));
      }
    }
    // 查询右侧哈希车道信息
    {
      TX_MARK("right node.");
      HashedLaneInfoPtr rightHashedLaneInfoPtr = nullptr;
      Geometry::SpatialQuery::GetRightHashedLaneInfo(refSelfHashedInfo, rightHashedLaneInfoPtr);
      // 如果右侧哈希车道信息指针非空
      if (NonNull_Pointer(rightHashedLaneInfoPtr)) {
        // 设置右侧节点为查询到的正交列表
        curOrthogonalListPtr->SetRightNode(HashedRoadCacheConCurrent::Query_OrthogonalList(*rightHashedLaneInfoPtr));
      }
    }
    {
      TX_MARK("front node(s).");
      // 获取前方哈希车道信息列表
      std::list<HashedLaneInfoPtr> front_hashed;
      GetFrontHashedLaneInfoList(refSelfHashedInfo, front_hashed, 1 /*one ring*/);
      HashedLaneInfoOrthogonalList::AdjoinContainerType refFrontMap;
      // 遍历前方哈希车道信息列表
      for (const auto& refFrontHashInfoPtr : front_hashed) {
        // 如果前方哈希车道信息指针非空
        if (NonNull_Pointer(refFrontHashInfoPtr)) {
          // 将前方哈希车道信息添加到前方节点映射中
          refFrontMap[*refFrontHashInfoPtr] = HashedRoadCacheConCurrent::Query_OrthogonalList(*refFrontHashInfoPtr);
          if (CallFail((*refFrontHashInfoPtr).IsValid())) {
            LOG(WARNING) << "[cloud_debug][front_nodes] " << TX_VARS(refSelfHashedInfo);
          }
        }
      }
#    if 0
      if (refSelfHashedInfo.LaneInfo().IsOnLane() &&
          ((Base::txLaneUId(1668, 0, -1) == refSelfHashedInfo.LaneInfo().onLaneUid) ||
           (Base::txLaneUId(1668, 0, -2) == refSelfHashedInfo.LaneInfo().onLaneUid) ||
           (Base::txLaneUId(1668, 0, -3) == refSelfHashedInfo.LaneInfo().onLaneUid))) {
        std::ostringstream oss;
        oss << "[cloud_debug] " << TX_VARS(refSelfHashedInfo) << " : ";
        for (const auto& refInfo : refFrontMap) {
          oss << "{" << refInfo.first << ", "
              << ((NonNull_Pointer(refInfo.second)) ? ("has_OrthogonalList") : ("donot_has_OrthogonalList")) << "},";
        }
        LOG(WARNING) << oss.str();
      }
#    endif
      curOrthogonalListPtr->SetFrontNodes(refFrontMap);
    }
    {
      TX_MARK("back node(s).");
      // 获取后方哈希车道信息列表
      std::list<HashedLaneInfoPtr> back_hashed;
      GetBackHashedLaneInfoList(refSelfHashedInfo, back_hashed, 1 /*one ring*/);
      HashedLaneInfoOrthogonalList::AdjoinContainerType refBackMap;
      // 遍历后方哈希车道信息列表
      for (const auto& refBackHashInfoPtr : back_hashed) {
        // 如果后方哈希车道信息指针非空
        if (NonNull_Pointer(refBackHashInfoPtr)) {
          // 将后方哈希车道信息添加到后方节点映射中
          refBackMap[*refBackHashInfoPtr] = HashedRoadCacheConCurrent::Query_OrthogonalList(*refBackHashInfoPtr);
        }
      }
      curOrthogonalListPtr->SetBackNodes(refBackMap);
    }
  }); /*lamda function*/
      /* parallel_for */
#  endif

#  if __TX_Mark__("generate relative hash node.")
  tbb::parallel_for(s_hashedPt2OrthogonalListPtr.range(), [](const hashedRoad2OrthogonalList::range_type& r) {
    const Base::txInt nLevel = 4;
    // 遍历哈希车道信息到正交列表的映射
    for (hashedRoad2OrthogonalList::iterator itr = r.begin(); itr != r.end(); itr++) {
      // 获取当前索引对应的正交列表指针
      HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr = itr->second;
      // 如果正交列表指针非空
      if (NonNull_Pointer(curOrthogonalListPtr)) {
        // 创建一个相关哈希车道信息列表
        std::list<HashedLaneInfo> vecRelatedPt;
        const auto& curHashedLaneInfo = curOrthogonalListPtr->GetSelfHashedInfo();
        vecRelatedPt.emplace_back(curHashedLaneInfo);

        /*front*/
        HashedRoadCacheConCurrent::IteratorOrthogonalList(curOrthogonalListPtr, nLevel, vecRelatedPt, true);
        /*back*/
        HashedRoadCacheConCurrent::IteratorOrthogonalList(curOrthogonalListPtr, nLevel, vecRelatedPt, false);

        if (NonNull_Pointer(curOrthogonalListPtr->GetLeftNode())) {
          /*left*/
          vecRelatedPt.emplace_back(curOrthogonalListPtr->GetLeftNode()->GetSelfHashedInfo());
          /*left front*/
          std::list<HashedLaneInfo> pt_list;
          HashedRoadCacheConCurrent::IteratorOrthogonalList(curOrthogonalListPtr->GetLeftNode(), nLevel, vecRelatedPt,
                                                            true);
          /*left back*/
          HashedRoadCacheConCurrent::IteratorOrthogonalList(curOrthogonalListPtr->GetLeftNode(), nLevel, vecRelatedPt,
                                                            false);
        }
        if (NonNull_Pointer(curOrthogonalListPtr->GetRightNode())) {
          /*right*/
          vecRelatedPt.emplace_back(curOrthogonalListPtr->GetRightNode()->GetSelfHashedInfo());
          /*right_front*/
          HashedRoadCacheConCurrent::IteratorOrthogonalList(curOrthogonalListPtr->GetRightNode(), nLevel, vecRelatedPt,
                                                            true);
          /*right_back*/
          HashedRoadCacheConCurrent::IteratorOrthogonalList(curOrthogonalListPtr->GetRightNode(), nLevel, vecRelatedPt,
                                                            false);
        }
        HashedRoadCacheConCurrent::AddRelatedHashedPt(curHashedLaneInfo, vecRelatedPt);
      } else {
        LOG(INFO) << TX_VARS(itr->first) << " do not have orthogonal list.";
      }
    }
  });

#  endif
}

void HashedRoadCacheConCurrent::Check_OrthogonalList_LeftNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr)
    TX_NOEXCEPT {
  // 获取当前哈希车道信息
  const HashedLaneInfo& refSelfHashedInfo = curOrthogonalListPtr->GetSelfHashedInfo();
  // 获取左侧节点
  const auto& refConstLeftNodePtr = curOrthogonalListPtr->GetLeftNode();
  // 如果左侧节点非空
  if (NonNull_Pointer(refConstLeftNodePtr)) {
    // 获取左侧节点的右侧节点
    const auto& refConstLeftNodePtr_RightNodePtr = refConstLeftNodePtr->GetRightNode();
    // 如果左侧节点的右侧节点非空
    if (NonNull_Pointer(refConstLeftNodePtr_RightNodePtr)) {
      // 如果当前哈希车道信息与左侧节点的右侧节点的哈希车道信息不相等
      if (CallFail(refSelfHashedInfo == (refConstLeftNodePtr_RightNodePtr->GetSelfHashedInfo()))) {
        // 输出日志信息
        LogInfo << "[self_has_left][left_has_right][not_same]" << TX_VARS_NAME(self_node, refSelfHashedInfo)
                << TX_VARS_NAME(left_node, refConstLeftNodePtr->GetSelfHashedInfo())
                << TX_VARS_NAME(left_node_right_node, refConstLeftNodePtr_RightNodePtr->GetSelfHashedInfo());
      }
    } else {
      LogInfo << "[self_has_left][left_donot_has_right]" << TX_VARS_NAME(self_node, refSelfHashedInfo)
              << TX_VARS_NAME(left_node, refConstLeftNodePtr->GetSelfHashedInfo());
    }
  }
}

void HashedRoadCacheConCurrent::Check_OrthogonalList_RightNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr)
    TX_NOEXCEPT {
  // 获取当前哈希车道信息
  const HashedLaneInfo& refSelfHashedInfo = curOrthogonalListPtr->GetSelfHashedInfo();
  // 获取右侧节点
  const auto& refConstRightNodePtr = curOrthogonalListPtr->GetRightNode();
  // 如果右侧节点非空
  if (NonNull_Pointer(refConstRightNodePtr)) {
    // 获取右侧节点的左侧节点
    const auto& refConstRightNodePtr_LeftNodePtr = refConstRightNodePtr->GetLeftNode();
    // 如果右侧节点的左侧节点非空
    if (NonNull_Pointer(refConstRightNodePtr_LeftNodePtr)) {
      // 如果当前哈希车道信息与右侧节点的左侧节点的哈希车道信息不相等
      if (CallFail(refSelfHashedInfo == (refConstRightNodePtr_LeftNodePtr->GetSelfHashedInfo()))) {
        // 输出日志信息
        LogInfo << "[self_has_left][left_has_right][not_same]" << TX_VARS_NAME(self_node, refSelfHashedInfo)
                << TX_VARS_NAME(right_node, refConstRightNodePtr->GetSelfHashedInfo())
                << TX_VARS_NAME(right_node_left_node, refConstRightNodePtr_LeftNodePtr->GetSelfHashedInfo());
      }
    } else {
      LogInfo << "[self_has_right][right_donot_has_left]" << TX_VARS_NAME(self_node, refSelfHashedInfo)
              << TX_VARS_NAME(right_node, refConstRightNodePtr->GetSelfHashedInfo());
    }
  }
}

void HashedRoadCacheConCurrent::Check_OrthogonalList_FrontNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr)
    TX_NOEXCEPT {
  // 获取当前哈希车道信息
  const HashedLaneInfo& refSelfHashedInfo = curOrthogonalListPtr->GetSelfHashedInfo();
  // 获取前方节点映射
  const HashedLaneInfoOrthogonalList::AdjoinContainerType& refConstFrontNodes = curOrthogonalListPtr->GetFrontNodes();
  // 遍历前方节点映射
  for (const auto& refPairHashedNode2ConstFrontPtr : refConstFrontNodes) {
    // const auto& refFrontHashNode = refPairHashedNode2ConstFrontPtr.first;
    const auto& refFrontPtr = refPairHashedNode2ConstFrontPtr.second;
    // 如果前方节点的正交列表指针非空
    if (NonNull_Pointer(refFrontPtr)) {
      // 获取前方节点的后方节点映射
      const auto& FrontNode_BackNodes_Map = refFrontPtr->GetBackNodes();
      // 如果当前哈希车道信息不在前方节点的后方节点映射中
      if (FrontNode_BackNodes_Map.end() == FrontNode_BackNodes_Map.find(refSelfHashedInfo)) {
        // 输出日志信息
        LogInfo << "[self_has_front][front_has_back][not_same]" << TX_VARS_NAME(self_node, refSelfHashedInfo)
                << TX_VARS_NAME(front_node, refFrontPtr->GetSelfHashedInfo());
      }
    } else {
      LogInfo << "refPairHashedNode2ConstFrontPtr.second is nullptr."
              << TX_VARS_NAME(front_node, refPairHashedNode2ConstFrontPtr.first);
    }
  }
}

void HashedRoadCacheConCurrent::Check_OrthogonalList_BackNode(HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr)
    TX_NOEXCEPT {
  // 获取当前哈希车道信息
  const HashedLaneInfo& refSelfHashedInfo = curOrthogonalListPtr->GetSelfHashedInfo();
  // 获取后方节点映射
  const HashedLaneInfoOrthogonalList::AdjoinContainerType& refConstBackNodes = curOrthogonalListPtr->GetBackNodes();
  // 遍历后方节点映射
  for (const auto& refPairHashedNode2ConstBackPtr : refConstBackNodes) {
    // const auto& refFrontHashNode = refPairHashedNode2ConstFrontPtr.first;
    // 获取后方节点的正交列表指针
    const auto& refBackPtr = refPairHashedNode2ConstBackPtr.second;
    // 如果后方节点的正交列表指针非空
    if (NonNull_Pointer(refBackPtr)) {
      const auto& BackNode_FrontNodes_Map = refBackPtr->GetFrontNodes();
      // 如果当前哈希车道信息不在后方节点的前方节点映射中
      if (BackNode_FrontNodes_Map.end() == BackNode_FrontNodes_Map.find(refSelfHashedInfo)) {
        LogInfo << "[self_has_back][back_has_front][not_same]" << TX_VARS_NAME(self_node, refSelfHashedInfo)
                << TX_VARS_NAME(back_node, refBackPtr->GetSelfHashedInfo());
      }
    } else {
      LogInfo << "refPairHashedNode2ConstFrontPtr.second is nullptr."
              << TX_VARS_NAME(back_node, refPairHashedNode2ConstBackPtr.first);
    }
  }
}

void HashedRoadCacheConCurrent::IteratorOrthogonalList(
    const HashedRoadCacheConCurrent::HashedLaneInfoOrthogonalListPtr _nodePtr, const Base::txInt _level,
    std::list<HashedRoadCacheConCurrent::HashedLaneInfo>& pt_list, const Base::txBool isFront) TX_NOEXCEPT {
  if (Null_Pointer(_nodePtr)) {
    return;
  }
  // 如果遍历层数小于等于0，则返回
  if (_level <= 0) {
    return;
  }

  // 获取前方或后方节点映射
  const auto& front_node_ptr_set = ((isFront) ? (_nodePtr->GetFrontNodes()) : (_nodePtr->GetBackNodes()));
  // 遍历前方或后方节点映射
  for (const auto& ptr : front_node_ptr_set) {
    // 如果节点指针非空
    if (NonNull_Pointer(ptr.second)) {
      // 将节点的哈希车道信息添加到列表中
      pt_list.push_back(ptr.second->GetSelfHashedInfo());
      // 递归遍历下一层的前方或后方节点
      IteratorOrthogonalList(ptr.second, _level - 1, pt_list, isFront);
    }
  }
}

void HashedRoadCacheConCurrent::Check_OrthogonalList() TX_NOEXCEPT {
  tbb::parallel_for(s_hashedPt2OrthogonalListPtr.range(), [](const hashedRoad2OrthogonalList::range_type& r) {
    // 遍历正交列表
    for (hashedRoad2OrthogonalList::iterator itr = r.begin(); itr != r.end(); itr++) {
      // 获取当前正交列表指针
      HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr = itr->second;
      // 检查当前正交列表的左侧节点
      Check_OrthogonalList_LeftNode(curOrthogonalListPtr);
      // 检查当前正交列表的右侧节点
      Check_OrthogonalList_RightNode(curOrthogonalListPtr);
      // 检查当前正交列表的前方节点
      Check_OrthogonalList_FrontNode(curOrthogonalListPtr);
      // 检查当前正交列表的后方节点
      Check_OrthogonalList_BackNode(curOrthogonalListPtr);
    }
  });

  if (FLAGS_export_hashed_debug_info) {
    tbb::parallel_for(s_hashedPt2OrthogonalListPtr.range(), [](const hashedRoad2OrthogonalList::range_type& r) {
      const Base::txInt nLevel = 4;
      for (hashedRoad2OrthogonalList::iterator itr = r.begin(); itr != r.end(); itr++) {
        HashedLaneInfoOrthogonalListPtr curOrthogonalListPtr = itr->second;
        // 如果当前正交列表指针非空
        if (NonNull_Pointer(curOrthogonalListPtr)) {
          // 创建一个字符串流对象，用于存储坐标信息
          std::ostringstream oss, oss_info;
          oss << "x,y" << std::endl;
          // 获取当前正交列表的哈希车道信息
          const auto& curHashedLaneInfo = curOrthogonalListPtr->GetSelfHashedInfo();
          // 将当前哈希车道信息的坐标添加到字符串流中
          oss << _StreamPrecision_ << curHashedLaneInfo.subSectionStartWGS84.Lon() << ","
              << curHashedLaneInfo.subSectionStartWGS84.Lat() << ", self" << std::endl;
          // 将当前哈希车道信息添加到信息字符串流中
          oss_info << "[self]" << curHashedLaneInfo << std::endl;

          // 创建一个列表，用于存储前方节点的哈希车道信息
          std::list<HashedLaneInfo> pt_list;
          // 遍历当前正交列表的前方节点
          IteratorOrthogonalList(curOrthogonalListPtr, nLevel, pt_list, true);
          // 将前方节点的哈希车道信息添加到字符串流中
          for (const auto front_pt : pt_list) {
            oss << _StreamPrecision_ << (front_pt.subSectionStartWGS84.Lon()) << ","
                << (front_pt.subSectionStartWGS84.Lat()) << ", front" << std::endl;
            oss_info << "[front]" << front_pt << std::endl;
          }
          // 清空列表，用于存储后方节点的哈希车道信息
          pt_list.clear();
          // 遍历当前正交列表的后方节点
          IteratorOrthogonalList(curOrthogonalListPtr, nLevel, pt_list, false);
          // 将后方节点的哈希车道信息添加到字符串流中
          for (const auto front_pt : pt_list) {
            oss << _StreamPrecision_ << (front_pt.subSectionStartWGS84.Lon()) << ","
                << (front_pt.subSectionStartWGS84.Lat()) << ", back" << std::endl;
            oss_info << "[back]" << front_pt << std::endl;
          }
          // 如果当前正交列表的左侧节点非空
          if (NonNull_Pointer(curOrthogonalListPtr->GetLeftNode())) {
            // 获取左侧节点的哈希车道信息
            const auto leftHashedLaneInfo = curOrthogonalListPtr->GetLeftNode()->GetSelfHashedInfo();
            // 将左侧节点的哈希车道信息的坐标添加到字符串流中
            oss << _StreamPrecision_ << (leftHashedLaneInfo.subSectionStartWGS84).Lon() << ","
                << (leftHashedLaneInfo.subSectionStartWGS84).Lat() << ", left" << std::endl;
            oss_info << "[left]" << leftHashedLaneInfo << std::endl;

            // 创建一个列表，用于存储左侧节点的前方节点的哈希车道信息
            std::list<HashedLaneInfo> pt_list;
            // 遍历左侧节点的前方节点
            IteratorOrthogonalList(curOrthogonalListPtr->GetLeftNode(), nLevel, pt_list, true);
            // 将左侧节点的前方节点的哈希车道信息添加到字符串流中
            for (const auto front_pt : pt_list) {
              oss << _StreamPrecision_ << (front_pt.subSectionStartWGS84.Lon()) << ","
                  << (front_pt.subSectionStartWGS84.Lat()) << ", left_front" << std::endl;
              oss_info << "[left_front]" << front_pt << std::endl;
            }
            pt_list.clear();
            // 遍历左侧节点的后方节点
            IteratorOrthogonalList(curOrthogonalListPtr->GetLeftNode(), nLevel, pt_list, false);
            // 将左侧节点的后方节点的哈希车道信息添加到字符串流中
            for (const auto front_pt : pt_list) {
              oss << _StreamPrecision_ << (front_pt.subSectionStartWGS84.Lon()) << ","
                  << (front_pt.subSectionStartWGS84.Lat()) << ", left_back" << std::endl;
              oss_info << "[left_back]" << front_pt << std::endl;
            }
          }
          // 如果当前正交列表的右侧节点非空
          if (NonNull_Pointer(curOrthogonalListPtr->GetRightNode())) {
            // 获取右侧节点的哈希车道信息
            const auto rightHashedLaneInfo = curOrthogonalListPtr->GetRightNode()->GetSelfHashedInfo();
            // 将右侧节点的哈希车道信息的坐标添加到字符串流中
            oss << _StreamPrecision_ << (rightHashedLaneInfo.subSectionStartWGS84).Lon() << ","
                << (rightHashedLaneInfo.subSectionStartWGS84).Lat() << ", right" << std::endl;
            oss_info << "[right]" << rightHashedLaneInfo << std::endl;

            // 创建一个列表，用于存储右侧节点的前方节点的哈希车道信息
            std::list<HashedLaneInfo> pt_list;
            // 遍历右侧节点的前方节点
            IteratorOrthogonalList(curOrthogonalListPtr->GetRightNode(), nLevel, pt_list, true);
            // 将右侧节点的前方节点的哈希车道信息添加到字符串流中
            for (const auto front_pt : pt_list) {
              oss << _StreamPrecision_ << (front_pt.subSectionStartWGS84.Lon()) << ","
                  << (front_pt.subSectionStartWGS84.Lat()) << ", right_front" << std::endl;
              oss_info << "[right_front]" << front_pt << std::endl;
            }
            pt_list.clear();
            // 遍历右侧节点的后方节点
            IteratorOrthogonalList(curOrthogonalListPtr->GetRightNode(), nLevel, pt_list, false);
            for (const auto front_pt : pt_list) {
              oss << _StreamPrecision_ << (front_pt.subSectionStartWGS84.Lon()) << ","
                  << (front_pt.subSectionStartWGS84.Lat()) << ", right_back" << std::endl;
              oss_info << "[right_back]" << front_pt << std::endl;
            }
          }
          // 创建一个字符串流对象，用于存储文件路径
          std::stringstream ss;
          // 设置文件路径
          ss << FLAGS_export_hashed_debug_info_path << "/" << curHashedLaneInfo.m_laneinfo << "_"
             << curHashedLaneInfo.m_subSectionIdx << "_fblr.txt";
          std::ofstream outfile(ss.str());
          outfile << oss.str();
          outfile.close();

          // 设置文件路径
          ss.str("");
          ss << FLAGS_export_hashed_debug_info_path << "/" << curHashedLaneInfo.m_laneinfo << "_"
             << curHashedLaneInfo.m_subSectionIdx << "_info.txt";
          std::ofstream outfileinfo(ss.str());
          outfileinfo << oss_info.str();
          outfileinfo.close();
        } else {
          LOG(INFO) << TX_VARS(itr->first) << " do not have orthogonal list.";
        }
      }
    });
  }
}

void HashedRoadCacheConCurrent::RegisterVehicle(const HashedLaneInfo& _objInfo,
                                                Base::IVehicleElementPtr _veh_ptr) TX_NOEXCEPT {
  hashedRoad2OrthogonalList::const_accessor ca;
  // 设置文件路径
  if (s_hashedPt2OrthogonalListPtr.find(ca, _objInfo)) {
    // 在正交列表中注册车辆
    (ca->second)->RegisterVehicle(_veh_ptr);
    ca.release();
  } else {
    LOG(INFO) << _objInfo;
  }
}

void HashedRoadCacheConCurrent::UnRegisterVehicle(const HashedLaneInfo& _objInfo,
                                                  Base::txSysId _veh_sysId) TX_NOEXCEPT {
  hashedRoad2OrthogonalList::accessor a;
  // 如果找到了与给定哈希车道信息对应的正交列表
  if (CallSucc(s_hashedPt2OrthogonalListPtr.find(a, _objInfo))) {
    // 在正交列表中注销车辆
    (a->second)->UnRegisterVehicle(_veh_sysId);
  }
  a.release();
}

void HashedRoadCacheConCurrent::RegisterPedestrian(const HashedLaneInfo& _objInfo,
                                                   Base::ITrafficElementPtr _ped_ptr) TX_NOEXCEPT {
  hashedRoad2OrthogonalList::accessor a;
  // 在正交列表中插入哈希车道信息
  s_hashedPt2OrthogonalListPtr.insert(a, _objInfo);
  // 在正交列表中注册行人
  (a->second)->RegisterPedestrian(_ped_ptr);
  a.release();
}

void HashedRoadCacheConCurrent::UnRegisterPedestrian(const HashedLaneInfo& _objInfo,
                                                     Base::txSysId _ped_sysId) TX_NOEXCEPT {
  // 创建一个访问器，用于访问正交列表
  hashedRoad2OrthogonalList::accessor a;
  // 如果找到了与给定哈希车道信息对应的正交列表
  if (CallSucc(s_hashedPt2OrthogonalListPtr.find(a, _objInfo))) {
    // 在正交列表中注销行人
    (a->second)->UnRegisterPedestrian(_ped_sysId);
  }
  a.release();
}

void HashedRoadCacheConCurrent::RegisterObstacle(const HashedLaneInfo& _objInfo,
                                                 Base::ITrafficElementPtr _obs_ptr) TX_NOEXCEPT {
  hashedRoad2OrthogonalList::accessor a;
  // 在正交列表中插入哈希车道信息
  s_hashedPt2OrthogonalListPtr.insert(a, _objInfo);
  // 在正交列表中注册障碍物
  (a->second)->RegisterObstacle(_obs_ptr);
  a.release();
}

void HashedRoadCacheConCurrent::GetHashedPtVehicles(const HashedLaneInfo& _objInfo,
                                                    VehicleContainer& refVecContainer) TX_NOEXCEPT {
  // 创建一个常量访问器，用于访问正交列表
  hashedRoad2OrthogonalList::const_accessor ca;
  // 如果找到了与给定哈希车道信息对应的正交列表
  if (CallSucc(s_hashedPt2OrthogonalListPtr.find(ca, _objInfo))) {
    // 获取正交列表中的车辆
    ca->second->GetRegisterVehicles(refVecContainer);
    ca.release();
  } else {
    // 如果没有找到对应的正交列表，清空车辆容器
    refVecContainer.clear();
  }
}

void HashedRoadCacheConCurrent::GetHashedPtPedestrians(const HashedLaneInfo& _objInfo,
                                                       PedestrianContainer& refVecContainer) TX_NOEXCEPT {
  // 创建一个常量访问器，用于访问正交列表
  hashedRoad2OrthogonalList::const_accessor ca;
  // 如果找到了与给定哈希车道信息对应的正交列表
  if (CallSucc(s_hashedPt2OrthogonalListPtr.find(ca, _objInfo))) {
    // 获取正交列表中的行人
    ca->second->GetRegisterPedestrians(refVecContainer);
    ca.release();
  } else {
    // 如果没有找到对应的正交列表，清空行人容器
    refVecContainer.clear();
  }
}

void HashedRoadCacheConCurrent::GetHashedPtObstacles(const HashedLaneInfo& _objInfo,
                                                     ObstacleContainer& refVecContainer) TX_NOEXCEPT {
  hashedRoad2OrthogonalList::const_accessor ca;
  // 如果找到了与给定哈希车道信息对应的正交列表
  if (CallSucc(s_hashedPt2OrthogonalListPtr.find(ca, _objInfo))) {
    // 获取正交列表中的障碍物
    ca->second->GetRegisterObstacles(refVecContainer);
    ca.release();
  } else {
    // 如果没有找到对应的正交列表，清空障碍物容器
    refVecContainer.clear();
  }
}

void HashedRoadCacheConCurrent::QueryRegisterVehicles(const std::list<HashedLaneInfo>& refRelativeHashedLanes,
                                                      VehicleContainer& refVecContainer) TX_NOEXCEPT {
  refVecContainer.clear();
  // 遍历哈希车道信息列表
  for (const auto& hashedInfo : refRelativeHashedLanes) {
    // 创建一个临时的车辆容器
    VehicleContainer curContainer;
    // 获取当前哈希车道信息对应的车辆
    GetHashedPtVehicles(hashedInfo, curContainer);
    refVecContainer.insert(curContainer.begin(), curContainer.end());
  }
}

void HashedRoadCacheConCurrent::QueryRegisterVehicles(const HashedLaneInfoSet& refRelativeHashedLanes,
                                                      VehicleContainer& refVecContainer) TX_NOEXCEPT {
  refVecContainer.clear();
  // 遍历哈希车道信息集合
  for (const auto& hashedInfo : refRelativeHashedLanes) {
    VehicleContainer curContainer;
    // 获取当前哈希车道信息对应的车辆
    GetHashedPtVehicles(hashedInfo, curContainer);
    // 将当前哈希车道信息对应的车辆插入到结果容器中
    refVecContainer.insert(curContainer.begin(), curContainer.end());
  }
}

// 将当前哈希车道信息对应的车辆插入到结果容器中
void HashedRoadCacheConCurrent::UngisterAllElements() TX_NOEXCEPT {
  tbb::parallel_for(s_hashedPt2OrthogonalListPtr.range(), [](const hashedRoad2OrthogonalList::range_type& r) {
    // 遍历映射中的每个元素
    for (hashedRoad2OrthogonalList::iterator itr = r.begin(); itr != r.end(); itr++) {
      // 如果当前元素的正交列表不为空
      if (NonNull_Pointer(itr->second)) {
        itr->second->ClearDynamicElementInfo();
      }
    }
  });
}

void HashedRoadCacheConCurrent::RegisterCloseLaneEvent(const Base::txSysId eventId,
                                                       const std::vector<HashedLaneInfo>& hashedPtVec) TX_NOEXCEPT {
  // 遍历哈希车道信息列表
  for (const HashedLaneInfo& _objInfo : hashedPtVec) {
    // 创建一个访问器，用于访问关闭车道事件映射
    hashedRoad2CloseLaneEvent::accessor a;
    // 将哈希车道信息插入到关闭车道事件映射中
    s_close_lane_event.insert(a, _objInfo);
    // 将事件ID插入到关闭车道事件集合中
    (a->second).insert(eventId);
    a.release();
  }
}

void HashedRoadCacheConCurrent::UnRegisterCloseLaneEvent(const Base::txSysId eventId) TX_NOEXCEPT {
  s_close_lane_event.clear();
}

Base::txBool HashedRoadCacheConCurrent::IsCloseLane(const HashedLaneInfo& hashedPt) TX_NOEXCEPT {
  // 创建一个常量访问器，用于访问关闭车道事件映射
  hashedRoad2CloseLaneEvent::const_accessor ca;
  // 查找给定的哈希车道信息是否在关闭车道事件映射中
  return (s_close_lane_event.find(ca, hashedPt));
}

Base::txBool HashedRoadCacheConCurrent::IsCloseLane(const HashedLaneInfo& hashedPt,
                                                    std::set<Base::txSysId /*event_id*/>& eventSet) TX_NOEXCEPT {
  // 创建一个常量访问器，用于访问关闭车道事件映射
  hashedRoad2CloseLaneEvent::const_accessor ca;
  // 查找给定的哈希车道信息是否在关闭车道事件映射中
  if (CallSucc(s_close_lane_event.find(ca, hashedPt))) {
    // 如果找到了给定的哈希车道信息，将关闭车道事件集合赋值给eventSet，并返回true
    eventSet = ca->second;
    return true;
  } else {
    return false;
  }
}

HashedRoadCacheConCurrent::roadId2LaneLinkSetToThisRoadType::value_type::second_type
HashedRoadCacheConCurrent::GetLaneLinksToRoad(const Base::txRoadID _to_roadId) TX_NOEXCEPT {
  // 如果找到了给定的哈希车道信息，将关闭车道事件集合赋值给eventSet，并返回true
  roadId2LaneLinkSetToThisRoadType::value_type::second_type res;
  // 创建一个常量访问器，用于访问道路ID到车道链接集合的映射
  roadId2LaneLinkSetToThisRoadType::const_accessor ca;
  // 查找给定的道路ID是否在道路ID到车道链接集合的映射中
  if (CallSucc(s_lanelinkSetToRoadId.find(ca, _to_roadId))) {
    // 如果找到了给定的道路ID，将车道链接集合赋值给res
    res = ca->second;
    ca.release();
  }
  return std::move(res);
}

// 将车道链接位置信息添加到给定道路ID的车道链接集合中
void HashedRoadCacheConCurrent::Add_LaneLinkLocInfoToRoad(const Base::txRoadID _to_roadId,
                                                          const HashedLaneInfo _locInfo) TX_NOEXCEPT {
  // 创建一个访问器，用于访问道路ID到车道链接集合的映射
  roadId2LaneLinkSetToThisRoadType::accessor a;
  // 将给定的道路ID插入到道路ID到车道链接集合的映射中
  s_lanelinkSetToRoadId.insert(a, _to_roadId);
  // 将车道链接位置信息插入到给定道路ID的车道链接集合中
  (a->second).insert(_locInfo);
  a.release();
}

TX_NAMESPACE_CLOSE(HdMap)

#endif /*USE_HashedRoadNetwork*/

// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <set>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "HdMap/tx_lane_geom_info.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_predef.h"
#include "tx_header.h"
#include "tx_locate_info.h"
TX_NAMESPACE_OPEN(Base)
TX_NAMESPACE_OPEN(Component)
class Location;
TX_NAMESPACE_CLOSE(Component)
TX_NAMESPACE_CLOSE(Base)

TX_NAMESPACE_OPEN(HdMap)

class MapTracker {
 public:
  using txBool = Base::txBool;

 public:
  explicit MapTracker(Base::txSysId _id) : mId(_id) {}

  MapTracker(Base::txSysId _id, const hadmap::txLanePtr _lanePtr) : mId(_id) { relocate(_lanePtr); }

  MapTracker(Base::txSysId _id, const hadmap::txLaneLinkPtr _linkPtr) : mId(_id) { relocate(_linkPtr); }

  ~MapTracker() TX_DEFAULT;

  /**
   * @brief 获取对象的唯一ID
   *
   * @return Base::txSysId 对象的唯一ID
   */
  Base::txSysId getId() const TX_NOEXCEPT { return mId; }

  /**
   * @brief 获取当前道路的信息
   *
   * @return Base::Info_Lane_t 当前道路的信息
   */
  const Base::Info_Lane_t getLaneInfo() const TX_NOEXCEPT { return m_LaneInfo; }

  /**
   * @brief 检查当前路段是否已经到达末尾
   *
   * @return Base::txBool 当前路段是否已经到达末尾
   */
  Base::txBool isReachedRoadLastSection() const TX_NOEXCEPT;

  /**
   * @brief 获取当前道路信息
   *
   * @return 当前道路指针
   */
  const hadmap::txLanePtr getCurrentLane() const TX_NOEXCEPT { return currentLanePtr; }

  /**
   * @brief 获取下一个交叉口的所有车道
   *
   * @return 一个包含所有下一个交叉口道路的 std::set<txLaneUId> 类型的引用
   */
  const std::set<Base::txLaneUId> getNextLanes() const TX_NOEXCEPT;

  /**
   * @brief 获取当前的车道连接
   *
   * @return const hadmap::txLaneLinkPtr
   */
  const hadmap::txLaneLinkPtr getCurrentLaneLink() const TX_NOEXCEPT { return currentLaneLinkPtr; }

  /**
   * @brief 判断是否在左侧边界上
   * @return 返回是否在左侧边界上
   */
  Base::txBool IsLeftBoundarySolid() const TX_NOEXCEPT { return mIsLeftBoundarySolid; }

  /**
   * @brief 判断是否在右侧边界上
   * @return 返回是否在右侧边界上
   */
  Base::txBool IsRightBoundarySolid() const TX_NOEXCEPT { return mIsRightBoundarySolid; }

  /**
   * @brief 判断是否存在左侧车道
   * @return 返回是否存在左侧车道
   */
  Base::txBool hasLeftLane() const TX_NOEXCEPT { return NonNull_Pointer(getLeftLane()); }

  /**
   * @brief 判断是否存在右侧车道
   * @return 返回是否存在右侧车道
   */
  Base::txBool hasRightLane() const TX_NOEXCEPT { return NonNull_Pointer(getRightLane()); }

  /**
   * @brief 获取左侧车道
   * @return 返回左侧车道的指针，如果不存在则返回空指针
   */
  hadmap::txLanePtr getLeftLane() const TX_NOEXCEPT { return currentLeftLanePtr; }

  /**
   * @brief 获取右侧车道
   * @return 返回右侧车道的指针，如果不存在则返回空指针
   */
  hadmap::txLanePtr getRightLane() const TX_NOEXCEPT { return currentRightLanePtr; }

  /**
   * @brief 获取下一个车道连接点
   * @return 返回下一个车道连接点的集合
   */
  const Base::LocInfoSet getNextLaneLinks() const TX_NOEXCEPT;

  /**
   * @brief 获取当前曲线
   * @return 返回当前曲线的指针
   */
  const hadmap::txLineCurve* getCurrentCurve() const TX_NOEXCEPT;

  /**
   * @brief 获取当前车道信息
   * @return 返回当前车道信息的指针
   */
  const HdMap::txLaneInfoInterfacePtr getLaneGeomInfo() const TX_NOEXCEPT { return mLaneGeomIntoPtr; }

  /**
   * @brief 获取左侧道路信息
   * @return 返回左侧道路信息的指针
   */
  const HdMap::txLaneInfoInterfacePtr get_Left_LaneGeomInfo() const TX_NOEXCEPT { return m_Left_LaneGeomIntoPtr; }

  /**
   * @brief 获取右侧道路信息
   * @return 返回右侧道路信息的指针
   */
  const HdMap::txLaneInfoInterfacePtr get_Right_LaneGeomInfo() const TX_NOEXCEPT { return m_Right_LaneGeomIntoPtr; }

  /**
   * @brief 获取道路信息
   * @return 返回道路信息的指针
   */
  const HdMap::txLaneInfoInterfacePtr getRoadGeomInfo() const TX_NOEXCEPT { return mRoadGeomInfoPtr; }

  /**
   * @brief 更新车道信息
   * @param _lanePtr 输入参数，需要更新的车道信息的指针
   * @param _lanelinkPtr 输入参数，车道信息的连接信息
   * @return 无返回值
   */
  void updateLaneInfo(TX_MARK("Ensure execution order") const hadmap::txLanePtr& _lanePtr,
                      const hadmap::txLaneLinkPtr& _lanelinkPtr) TX_NOEXCEPT;

  /**
   * @brief 更新左右车道信息
   * @param[in] _locInfo 输入参数，需要更新的车道信息
   * @param[in] nLaneCnt 输入参数，车道数量
   * @return 无返回值
   */
  void update_Left_Right_Lane(TX_MARK("Ensure execution order") const Base::Info_Lane_t& _locInfo,
                              const Base::txSize nLaneCnt) TX_NOEXCEPT;

  /**
   * @brief 更新左右车道边界信息
   *
   * @param[in] curLanePtr 当前车道信息的指针
   *
   * @return 无返回值
   */
  void update_Left_Right_Boundary(const hadmap::txLanePtr& curLanePtr) TX_NOEXCEPT;

  /**
   * @brief 获取当前的分段数量
   *
   * @return Base::txSize 当前分段数量
   */
  Base::txSize GetCurrentSectionCount() const TX_NOEXCEPT { return mCurrentSectionSize; }

  /**
   * @brief 获取当前的车道数量
   *
   * @return Base::txSize 当前车道数量
   */
  Base::txSize GetCurrentLaneCount() const TX_NOEXCEPT { return mCurrentLaneSize; }

 protected:
  friend class Base::Component::Location;

  /**
   * @brief 使用给定的lanePtr重新定位车辆
   *
   * @param lanePtr 要使用的lanePtr
   * @return true 如果成功重定位
   * @return false 如果重定位失败
   */
  Base::txBool relocate(const hadmap::txLanePtr& lanePtr) TX_NOEXCEPT;

  /**
   * @brief 根据给定的 laneLinkPtr 重新定位车辆
   *
   * @param laneLinkPtr 要使用的 laneLinkPtr
   * @return true 重定位成功
   * @return false 重定位失败
   */
  Base::txBool relocate(const hadmap::txLaneLinkPtr& laneLinkPtr) TX_NOEXCEPT;

 private:
  Base::txSysId mId;
  Base::Info_Lane_t m_LaneInfo;
  hadmap::txLanePtr currentLanePtr = nullptr;
  hadmap::txLanePtr currentLeftLanePtr = nullptr;
  hadmap::txLanePtr currentRightLanePtr = nullptr;
  hadmap::txLaneLinkPtr currentLaneLinkPtr = nullptr;
  hadmap::txSectionPtr currentSectionPtr = nullptr;
  hadmap::txRoadPtr currentRoadPtr = nullptr;
  Base::txSize mCurrentSectionSize = 0;
  TX_MARK("uninit : 0; onlink:1");
  Base::txSize mCurrentLaneSize = 0;
  TX_MARK("uninit : 0; onlink:1");
  Base::txBool mIsLeftBoundarySolid = true;
  Base::txBool mIsRightBoundarySolid = true;
  HdMap::txLaneInfoInterfacePtr mLaneGeomIntoPtr = nullptr;
  HdMap::txLaneInfoInterfacePtr m_Left_LaneGeomIntoPtr = nullptr;
  HdMap::txLaneInfoInterfacePtr m_Right_LaneGeomIntoPtr = nullptr;
  HdMap::txLaneInfoInterfacePtr mRoadGeomInfoPtr = nullptr;

 public:
  /**
   * @brief 将数据存储到存档中
   *
   * @param archive 存档对象
   */
  template <class Archive>
  void save(Archive& archive) const {
    archive(_MAKE_NVP_("Id", mId));
    archive(_MAKE_NVP_("LaneInfo", m_LaneInfo));

    if (NonNull_Pointer(currentLanePtr)) {
      archive(_MAKE_NVP_("currentLanePtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("currentLanePtr_Uid", currentLanePtr->getTxLaneId()));
    } else {
      archive(_MAKE_NVP_("currentLanePtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(currentLeftLanePtr)) {
      archive(_MAKE_NVP_("currentLeftLanePtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("currentLeftLanePtr_Uid", currentLeftLanePtr->getTxLaneId()));
    } else {
      archive(_MAKE_NVP_("currentLeftLanePtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(currentRightLanePtr)) {
      archive(_MAKE_NVP_("currentRightLanePtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("currentRightLanePtr_Uid", currentRightLanePtr->getTxLaneId()));
    } else {
      archive(_MAKE_NVP_("currentRightLanePtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(currentLaneLinkPtr)) {
      archive(_MAKE_NVP_("currentLaneLinkPtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("currentLaneLinkPtr_FromUid", currentLaneLinkPtr->fromTxLaneId()));
      archive(_MAKE_NVP_("currentLaneLinkPtr_ToUid", currentLaneLinkPtr->toTxLaneId()));
    } else {
      archive(_MAKE_NVP_("currentLaneLinkPtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(currentSectionPtr)) {
      archive(_MAKE_NVP_("currentSectionPtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("currentSectionPtr_roadid", currentSectionPtr->getRoadId()));
      archive(_MAKE_NVP_("currentSectionPtr_sectionid", currentSectionPtr->getId()));
    } else {
      archive(_MAKE_NVP_("currentSectionPtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(currentRoadPtr)) {
      archive(_MAKE_NVP_("currentRoadPtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("currentRoadPtr_Uid", currentRoadPtr->getId()));
    } else {
      archive(_MAKE_NVP_("currentRoadPtr_is_not_nullptr", false));
    }

    archive(_MAKE_NVP_("CurrentSectionSize", mCurrentSectionSize));
    archive(_MAKE_NVP_("CurrentLaneSize", mCurrentLaneSize));
    archive(_MAKE_NVP_("IsLeftBoundarySolid", mIsLeftBoundarySolid));
    archive(_MAKE_NVP_("IsRightBoundarySolid", mIsRightBoundarySolid));

    if (NonNull_Pointer(mLaneGeomIntoPtr)) {
      archive(_MAKE_NVP_("mLaneGeomIntoPtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("mLaneGeomIntoPtr_laneLocInfo", mLaneGeomIntoPtr->LaneLocInfo()));
    } else {
      archive(_MAKE_NVP_("mLaneGeomIntoPtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(mRoadGeomInfoPtr)) {
      archive(_MAKE_NVP_("mRoadGeomInfoPtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("mRoadGeomIntoPtr_RoadLocInfo", mRoadGeomInfoPtr->RoadLocInfo()));
    } else {
      archive(_MAKE_NVP_("mRoadGeomInfoPtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(m_Left_LaneGeomIntoPtr)) {
      archive(_MAKE_NVP_("m_Left_LaneGeomIntoPtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("m_Left_LaneGeomIntoPtr_laneLocInfo", m_Left_LaneGeomIntoPtr->LaneLocInfo()));
    } else {
      archive(_MAKE_NVP_("m_Left_LaneGeomIntoPtr_is_not_nullptr", false));
    }

    if (NonNull_Pointer(m_Right_LaneGeomIntoPtr)) {
      archive(_MAKE_NVP_("m_Right_LaneGeomIntoPtr_is_not_nullptr", true));
      archive(_MAKE_NVP_("m_Right_LaneGeomIntoPtr_laneLocInfo", m_Right_LaneGeomIntoPtr->LaneLocInfo()));
    } else {
      archive(_MAKE_NVP_("m_Right_LaneGeomIntoPtr_is_not_nullptr", false));
    }
  }

  /**
   * @brief 加载存档文件
   *
   * @param archive 要加载的存档对象
   */
  template <class Archive>
  void load(Archive& archive) {
    archive(_MAKE_NVP_("Id", mId));
    archive(_MAKE_NVP_("LaneInfo", m_LaneInfo));
    // ****************************************************************************************************
    txBool currentLanePtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("currentLanePtr_is_not_nullptr", currentLanePtr_is_not_nullptr));
    if (CallSucc(currentLanePtr_is_not_nullptr)) {
      Base::txLaneUId currentLanePtr_Uid;
      archive(_MAKE_NVP_("currentLanePtr_Uid", currentLanePtr_Uid));
      currentLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(currentLanePtr_Uid);
    } else {
      currentLanePtr = nullptr;
    }
    // ****************************************************************************************************
    txBool currentLeftLanePtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("currentLeftLanePtr_is_not_nullptr", currentLeftLanePtr_is_not_nullptr));
    if (CallSucc(currentLeftLanePtr_is_not_nullptr)) {
      Base::txLaneUId currentLeftLanePtr_Uid;
      archive(_MAKE_NVP_("currentLeftLanePtr_Uid", currentLeftLanePtr_Uid));
      currentLeftLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(currentLeftLanePtr_Uid);
    } else {
      currentLeftLanePtr = nullptr;
    }
    // ****************************************************************************************************
    txBool currentRightLanePtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("currentRightLanePtr_is_not_nullptr", currentRightLanePtr_is_not_nullptr));
    if (CallSucc(currentRightLanePtr_is_not_nullptr)) {
      Base::txLaneUId currentRightLanePtr_Uid;
      archive(_MAKE_NVP_("currentRightLanePtr_Uid", currentRightLanePtr_Uid));
      currentRightLanePtr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(currentRightLanePtr_Uid);
    } else {
      currentRightLanePtr = nullptr;
    }
    // ****************************************************************************************************
    txBool currentLaneLinkPtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("currentLaneLinkPtr_is_not_nullptr", currentLaneLinkPtr_is_not_nullptr));
    if (CallSucc(currentLaneLinkPtr_is_not_nullptr)) {
      Base::txLaneUId currentLaneLinkPtr_FromUid, currentLaneLinkPtr_ToUid;
      archive(_MAKE_NVP_("currentLaneLinkPtr_FromUid", currentLaneLinkPtr_FromUid));
      archive(_MAKE_NVP_("currentLaneLinkPtr_ToUid", currentLaneLinkPtr_ToUid));
      currentLaneLinkPtr =
          HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(currentLaneLinkPtr_FromUid, currentLaneLinkPtr_ToUid);
      /*LOG(WARNING) << "[serialize] " << TX_VARS(mId) << TX_VARS(currentLaneLinkPtr_FromUid) <<
       * TX_VARS(currentLaneLinkPtr_ToUid);*/
    } else {
      currentLaneLinkPtr = nullptr;
    }
    // ****************************************************************************************************
    txBool currentSectionPtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("currentSectionPtr_is_not_nullptr", currentSectionPtr_is_not_nullptr));
    if (CallSucc(currentSectionPtr_is_not_nullptr)) {
      Base::txRoadID roadId;
      Base::txSectionID secId;
      archive(_MAKE_NVP_("currentSectionPtr_roadid", roadId));
      archive(_MAKE_NVP_("currentSectionPtr_sectionid", secId));
      currentSectionPtr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(Base::txSectionUId(roadId, secId));
    } else {
      currentSectionPtr = nullptr;
    }
    // ****************************************************************************************************
    txBool currentRoadPtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("currentRoadPtr_is_not_nullptr", currentRoadPtr_is_not_nullptr));
    if (CallSucc(currentRoadPtr_is_not_nullptr)) {
      Base::txRoadID roadId;
      archive(_MAKE_NVP_("currentRoadPtr_Uid", roadId));
      currentRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(roadId);
    } else {
      currentRoadPtr = nullptr;
    }
    // ****************************************************************************************************
    archive(_MAKE_NVP_("CurrentSectionSize", mCurrentSectionSize));
    archive(_MAKE_NVP_("CurrentLaneSize", mCurrentLaneSize));
    archive(_MAKE_NVP_("IsLeftBoundarySolid", mIsLeftBoundarySolid));
    archive(_MAKE_NVP_("IsRightBoundarySolid", mIsRightBoundarySolid));
    // ****************************************************************************************************
    txBool mLaneGeomIntoPtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("mLaneGeomIntoPtr_is_not_nullptr", mLaneGeomIntoPtr_is_not_nullptr));
    if (CallSucc(mLaneGeomIntoPtr_is_not_nullptr)) {
      Base::Info_Lane_t laneLocInfo;
      archive(_MAKE_NVP_("mLaneGeomIntoPtr_laneLocInfo", laneLocInfo));
      mLaneGeomIntoPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(laneLocInfo);
    } else {
      mLaneGeomIntoPtr = nullptr;
    }
    // ****************************************************************************************************

    // ***************************************************************************************
    txBool mRoadGeomInfoPtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("mRoadGeomInfoPtr_is_not_nullptr", mRoadGeomInfoPtr_is_not_nullptr));
    if (CallSucc(mRoadGeomInfoPtr_is_not_nullptr)) {
      Base::Info_Road_t roadLocInfo;
      archive(_MAKE_NVP_("mRoadGeomIntoPtr_RoadLocInfo", roadLocInfo));
      mRoadGeomInfoPtr = HdMap::HadmapCacheConCurrent::GetRoadGeomInfoById(roadLocInfo);
    } else {
      mRoadGeomInfoPtr = nullptr;
    }
    // ***************************************************************************************
    txBool m_Left_LaneGeomIntoPtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("m_Left_LaneGeomIntoPtr_is_not_nullptr", m_Left_LaneGeomIntoPtr_is_not_nullptr));
    if (CallSucc(m_Left_LaneGeomIntoPtr_is_not_nullptr)) {
      Base::Info_Lane_t laneLocInfo;
      archive(_MAKE_NVP_("m_Left_LaneGeomIntoPtr_laneLocInfo", laneLocInfo));
      m_Left_LaneGeomIntoPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(laneLocInfo);
    } else {
      m_Left_LaneGeomIntoPtr = nullptr;
    }
    // ****************************************************************************************************
    txBool m_Right_LaneGeomIntoPtr_is_not_nullptr = false;
    archive(_MAKE_NVP_("m_Right_LaneGeomIntoPtr_is_not_nullptr", m_Right_LaneGeomIntoPtr_is_not_nullptr));
    if (CallSucc(m_Right_LaneGeomIntoPtr_is_not_nullptr)) {
      Base::Info_Lane_t laneLocInfo;
      archive(_MAKE_NVP_("m_Right_LaneGeomIntoPtr_laneLocInfo", laneLocInfo));
      m_Right_LaneGeomIntoPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(laneLocInfo);
    } else {
      m_Right_LaneGeomIntoPtr = nullptr;
    }
    // ****************************************************************************************************
  }
};

using MapTrackerPtr = std::shared_ptr<MapTracker>;

TX_NAMESPACE_CLOSE(HdMap)

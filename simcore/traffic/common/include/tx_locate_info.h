// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <atomic>
#include <boost/functional/hash.hpp>
#include <unordered_set>
#include "tx_class_counter.h"
#include "tx_hash_utils.h"
#include "tx_header.h"
#include "tx_serialization.h"
#include "tx_string_utils.h"
TX_NAMESPACE_OPEN(Base)

struct STCoord {
 public:
  using txFloat = Base::txFloat;
  using txString = Base::txString;
  enum { invalid_s = 100000 };
  STCoord() {
    s() = invalid_s + 1.0;
    t() = 0.0;
  }
  explicit STCoord(const Base::txVec2& vST) : mST(vST) {}
  STCoord(const txFloat _s, const txFloat _t) : STCoord(Base::txVec2(_s, _t)) {}

  txFloat dist(/*deprecated*/) const TX_NOEXCEPT { return s(); }

  /**
   * @brief 获取st坐标系的s值
   *
   * @return txFloat
   */
  txFloat s() const TX_NOEXCEPT { return mST[0]; }
  txFloat& s() TX_NOEXCEPT { return mST[0]; }

  /**
   * @brief 获取st坐标系下的t值
   *
   * @return txFloat
   */
  txFloat t() const TX_NOEXCEPT { return mST[1]; }
  txFloat& t() TX_NOEXCEPT { return mST[1]; }

  /**
   * @brief 获取实例对象的字符串描述
   *
   * @return txString
   */
  txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(s, s()) << TX_VARS_NAME(t, t()) << "}";
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const STCoord& c) {
    os << c.Str();
    return os;
  }

 protected:
  Base::txVec2 mST;
};

struct Info_Road_t {
  using txRoadID = Base::txRoadID;
  using txBool = Base::txBool;
  using txLaneUId = Base::txLaneUId;

  txRoadID fromRoadId = -1, toRoadId = -1;

  /**
   * @brief 检查是否位于连接上
   *
   * 若fromRoadId不等于toRoadId，则返回 true。否则返回 false。
   *
   * @return 是否位于链路上
   */
  txBool IsOnLink() const TX_NOEXCEPT { return fromRoadId != toRoadId; }

  /**
   * @brief 是否在道路上
   *
   * @return txBool 若fromRoadId等于toRoadId，则返回 true
   */
  txBool IsOnRoad() const TX_NOEXCEPT { return (fromRoadId == toRoadId); }

  Info_Road_t() TX_DEFAULT;
  explicit Info_Road_t(const txLaneUId& _laneUid) : Info_Road_t(_laneUid.roadId) {}
  explicit Info_Road_t(const txRoadID _r) : fromRoadId(_r), toRoadId(_r) {}
  Info_Road_t(const txRoadID _from_r, const txRoadID _to_r) : fromRoadId(_from_r), toRoadId(_to_r) {}
  Info_Road_t(const txLaneUId& _laneFromUid, const txLaneUId& _laneToUid)
      : Info_Road_t(_laneFromUid.roadId, _laneToUid.roadId) {}

  /**
   * @brief 将当前坐标从指定的道路转到另一个道路
   *
   * 该函数接受一个道路的UID作为参数，表示将当前坐标从该道路转到另一个道路。通常用于沿着道路链路进行车辆或行人的跟踪。
   *
   * @param _laneUid 要转到的道路的UID
   */
  void FromLane(const txLaneUId& _laneUid) TX_NOEXCEPT { toRoadId = fromRoadId = _laneUid.roadId; }

  void FromLaneLink(const txLaneUId& _laneFromUid, const txLaneUId& _laneToUid) TX_NOEXCEPT {
    fromRoadId = _laneFromUid.roadId;
    toRoadId = _laneToUid.roadId;
  }

  void Clear() TX_NOEXCEPT { toRoadId = fromRoadId = -1; }

  bool operator==(const Info_Road_t& o) const TX_NOEXCEPT {
    return (fromRoadId == o.fromRoadId) && (toRoadId == o.toRoadId);
  }

  bool operator!=(const Info_Road_t& o) const TX_NOEXCEPT { return !((*this) == o); }

  bool operator<(const Info_Road_t& o) const TX_NOEXCEPT {
    return (fromRoadId < o.fromRoadId) && (toRoadId < o.toRoadId);
  }

  friend std::ostream& operator<<(std::ostream& os, const Info_Road_t& v) TX_NOEXCEPT {
    if (v.IsOnLink()) {
      os << "Info_Road_t { on Link " << TX_VARS_NAME(fromRoadId, v.fromRoadId) << TX_VARS_NAME(toRoadId, v.toRoadId)
         << "}";
    } else {
      os << "Info_Lane { on Road " << TX_VARS_NAME(RoadId, v.fromRoadId) << "}";
    }
    return os;
  }

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("fromRoadId", fromRoadId), _MAKE_NVP_("toRoadId", toRoadId));
  }
};

struct Info_Lane_t {
  Base::txLaneUId onLaneUid;
  Base::txBool isOnLaneLink = false;
  TX_MARK("tracker relocate default link not init");
  Base::txLaneLinkID onLinkId_without_equal = 0;
  Base::txLaneUId onLinkFromLaneUid;
  Base::txLaneUId onLinkToLaneUid;

  Base::txBool IsOnLaneLink() const TX_NOEXCEPT { return isOnLaneLink; }
  Base::txBool IsOnLane() const TX_NOEXCEPT { return !IsOnLaneLink(); }

  Info_Lane_t() TX_DEFAULT;
  Info_Lane_t(const Base::txRoadID _r, const Base::txSectionID _s, const Base::txLaneID _l)
      : Info_Lane_t(Base::txLaneUId(_r, _s, _l)) {}
  explicit Info_Lane_t(const Base::txLaneUId& _laneUid) { FromLane(_laneUid); }

  Info_Lane_t(const Base::txLaneLinkID& _lane_link_id, const Base::txLaneUId& _laneFromUid,
              const Base::txLaneUId& _laneToUid) {
    FromLaneLink(_lane_link_id, _laneFromUid, _laneToUid);
  }

  void FromLane(const Base::txLaneUId& _laneUid) TX_NOEXCEPT {
    onLaneUid = _laneUid;
    isOnLaneLink = false;
    onLinkId_without_equal = 0;
    onLinkFromLaneUid = _laneUid;
    onLinkToLaneUid = _laneUid;
  }

  void FromLaneLink(const Base::txLaneLinkID _linkId, const Base::txLaneUId& _laneFromUid,
                    const Base::txLaneUId& _laneToUid) TX_NOEXCEPT {
    onLaneUid = _laneFromUid;
    isOnLaneLink = true;
    onLinkId_without_equal = _linkId;
    onLinkFromLaneUid = _laneFromUid;
    onLinkToLaneUid = _laneToUid;
  }

  void Clear() TX_NOEXCEPT {
    onLaneUid = Base::txLaneUId();
    isOnLaneLink = false;
    onLinkId_without_equal = 0;
    onLinkFromLaneUid = Base::txLaneUId();
    onLinkToLaneUid = Base::txLaneUId();
  }

  bool operator==(const Info_Lane_t& o) const TX_NOEXCEPT {
    if (o.isOnLaneLink == isOnLaneLink) {
      if (o.isOnLaneLink) {
        return (o.onLinkFromLaneUid == onLinkFromLaneUid) && (o.onLinkToLaneUid == onLinkToLaneUid);
      } else {
        return (o.onLaneUid == onLaneUid);
      }
    } else {
      return false;
    }
  }
  bool operator<(const Info_Lane_t& o) const TX_NOEXCEPT;

  Info_Road_t toRoadLocInfo() const TX_NOEXCEPT {
    if (IsOnLaneLink()) {
      return Info_Road_t(onLinkFromLaneUid.roadId, onLinkToLaneUid.roadId);
    } else {
      return Info_Road_t(onLaneUid.roadId);
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const Info_Lane_t& v) TX_NOEXCEPT {
    if (v.isOnLaneLink) {
      os << "Info_Lane { on LaneLink " << TX_VARS_NAME(onLinkFromLaneUid, Utils::ToString(v.onLinkFromLaneUid))
         << TX_VARS_NAME(onLinkToLaneUid, Utils::ToString(v.onLinkToLaneUid)) << "}";
    } else {
      os << "Info_Lane { on Lane " << TX_VARS_NAME(onLaneUid, Utils::ToString(v.onLaneUid)) << "}";
    }
    return os;
  }

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("isOnLaneLink", isOnLaneLink), _MAKE_NVP_("onLaneUid", onLaneUid),
       /*_MAKE_NVP_("onLinkId", onLinkId),*/
       _MAKE_NVP_("onLinkFromLaneUid", onLinkFromLaneUid), _MAKE_NVP_("onLinkToLaneUid", onLinkToLaneUid));
  }
};

TX_NAMESPACE_CLOSE(Base)

TX_NAMESPACE_OPEN(Utils)
struct Info_Lane_t_HashCompare {
  static size_t hash(const Base::Info_Lane_t& _lane_loc_info) {
    size_t seed = 0;
    if (_lane_loc_info.IsOnLane()) {
      boost::hash_combine(seed, _lane_loc_info.onLaneUid.roadId);
      boost::hash_combine(seed, _lane_loc_info.onLaneUid.sectionId);
      boost::hash_combine(seed, _lane_loc_info.onLaneUid.laneId);

      boost::hash_combine(seed, _lane_loc_info.onLaneUid.roadId);
      boost::hash_combine(seed, _lane_loc_info.onLaneUid.sectionId);
      boost::hash_combine(seed, _lane_loc_info.onLaneUid.laneId);
    } else {
      boost::hash_combine(seed, _lane_loc_info.onLinkFromLaneUid.roadId);
      boost::hash_combine(seed, _lane_loc_info.onLinkFromLaneUid.sectionId);
      boost::hash_combine(seed, _lane_loc_info.onLinkFromLaneUid.laneId);

      boost::hash_combine(seed, _lane_loc_info.onLinkToLaneUid.roadId);
      boost::hash_combine(seed, _lane_loc_info.onLinkToLaneUid.sectionId);
      boost::hash_combine(seed, _lane_loc_info.onLinkToLaneUid.laneId);
    }
    return seed;
  }

  size_t operator()(const Base::Info_Lane_t& _lane_loc_info) const {
    return Info_Lane_t_HashCompare::hash(_lane_loc_info);
  }

  static bool equal(const Base::Info_Lane_t& lv, const Base::Info_Lane_t& rv) { return lv == rv; }
};

struct Info_Road_t_HashCompare {
  static size_t hash(const Base::Info_Road_t& _road_loc_info) {
    size_t seed = 0;
    boost::hash_combine(seed, _road_loc_info.fromRoadId);
    boost::hash_combine(seed, _road_loc_info.toRoadId);
    return seed;
  }

  size_t operator()(const Base::Info_Road_t& _road_loc_info) const {
    return Info_Road_t_HashCompare::hash(_road_loc_info);
  }

  static bool equal(const Base::Info_Road_t& lv, const Base::Info_Road_t& rv) { return lv == rv; }
};

TX_NAMESPACE_CLOSE(Utils)

TX_NAMESPACE_OPEN(Base)
using LocInfoSet = std::set<Info_Lane_t>;
// using LocInfoSet = std::unordered_set<Info_Lane_t, Utils::Info_Lane_t_HashCompare>;
TX_NAMESPACE_CLOSE(Base)

TX_NAMESPACE_OPEN(Base)
struct JointPointId_t {
  Base::Info_Lane_t locInfo;
  txBool isStartPt = true;
#if 0
    txBool isOnLaneLink = false;
    txLaneUId onLaneUid;
    txLaneLinkID onLinkId = 0;
#else
  txBool isOnLaneLink() const TX_NOEXCEPT { return locInfo.IsOnLaneLink(); }
  /*txLaneLinkID onLinkId() const TX_NOEXCEPT { return locInfo.onLinkId; }*/
  txLaneUId onLaneUid() const TX_NOEXCEPT { return locInfo.onLaneUid; }
  txLaneUId onLinkFromLaneUid() const TX_NOEXCEPT { return locInfo.onLinkFromLaneUid; }
  txLaneUId onLinkToLaneUid() const TX_NOEXCEPT { return locInfo.onLinkToLaneUid; }
#endif
  static JointPointId_t MakeJointStartPointId(const Base::txLaneUId curLaneUid) TX_NOEXCEPT;
  static JointPointId_t MakeJointEndPointId(const Base::txLaneUId curLaneUid) TX_NOEXCEPT;
  static JointPointId_t MakeJointStartPointId(const Base::txLaneLinkID curLaneLinkId,
                                              const Base::txLaneUId& fromLaneUid,
                                              const Base::txLaneUId& toLaneUid) TX_NOEXCEPT;
  static JointPointId_t MakeJointEndPointId(const Base::txLaneLinkID curLaneLinkId, const Base::txLaneUId& fromLaneUid,
                                            const Base::txLaneUId& toLaneUid) TX_NOEXCEPT;

  txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    if (CallSucc(isOnLaneLink())) {
      oss << "[Link]" << TX_VARS_NAME(fromLaneUid, Utils::ToString(onLinkFromLaneUid()))
          << TX_VARS_NAME(toLaneUid, Utils::ToString(onLinkToLaneUid())) << (isStartPt ? ("][Start]") : ("][End]"));
    } else {
      oss << "[Land][Id = " << Utils::ToString(onLaneUid()) << (isStartPt ? ("][Start]") : ("][End]"));
    }
    return oss.str();
  }

  friend std::ostream& operator<<(std::ostream& os, const JointPointId_t& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }

#if 1
  bool operator==(const JointPointId_t& o) const TX_NOEXCEPT {
    if (o.isOnLaneLink() == isOnLaneLink() && o.isStartPt == isStartPt) {
      if (o.isOnLaneLink()) {
        return (o.onLinkFromLaneUid() == onLinkFromLaneUid()) && (o.onLinkToLaneUid() == onLinkToLaneUid());
        /*return o.onLinkId() == onLinkId();*/
      } else {
        return o.onLaneUid() == onLaneUid();
        /*return (o.onLaneUid.roadId == onLaneUid.roadId) &&
            (o.onLaneUid.sectionId == onLaneUid.sectionId) &&
            (o.onLaneUid.laneId == onLaneUid.laneId);*/
      }
    } else {
      return false;
    }
  }

  bool operator<(const JointPointId_t& o) const TX_NOEXCEPT {
    if (o.isOnLaneLink() == isOnLaneLink()) {
      if (o.isStartPt == isStartPt) {
        if (o.isOnLaneLink()) {
          return (onLinkFromLaneUid() < o.onLinkFromLaneUid()) && (onLinkToLaneUid() < o.onLinkToLaneUid());
          /*return onLinkId < o.onLinkId;*/
        } else {
          return onLaneUid() < o.onLaneUid();
          /*return (onLaneUid.roadId < o.onLaneUid.roadId) &&
              (onLaneUid.sectionId < o.onLaneUid.sectionId) &&
              (onLaneUid.laneId < o.onLaneUid.laneId);*/
        }
      } else {
        return (isStartPt /*left value*/ < o.isStartPt /*right value*/);
        TX_MARK("assert false < true");
      }
    } else {
      return (isOnLaneLink() < o.isOnLaneLink());
      TX_MARK("assert false < true");
    }
  }
#endif

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("isStartPt", isStartPt), _MAKE_NVP_("locInfo", locInfo));
  }
};

struct JointPointIdHashCompare {
  static size_t hash(const JointPointId_t& _joint_id) TX_NOEXCEPT {
    size_t seed = Utils::Info_Lane_t_HashCompare::hash(_joint_id.locInfo);
    boost::hash_combine(seed, _joint_id.isStartPt);

    /*boost::hash_combine(seed, _joint_id.isOnLaneLink);
boost::hash_combine(seed, _joint_id.isStartPt);
boost::hash_combine(seed, _joint_id.onLaneUid.roadId);
boost::hash_combine(seed, _joint_id.onLaneUid.sectionId);
boost::hash_combine(seed, _joint_id.onLaneUid.laneId);
    boost::hash_combine(seed, _joint_id.onLinkId);*/
    return seed;
  }

  static bool equal(const Base::JointPointId_t& lv, const Base::JointPointId_t& rv) TX_NOEXCEPT { return lv == rv; }

  size_t operator()(const JointPointId_t& interval) const TX_NOEXCEPT {
    return JointPointIdHashCompare::hash(interval);
  }
};

struct JointPointTreeNode;
using JointPointTreeNodePtr = std::shared_ptr<JointPointTreeNode>;
using JointPointTreeNodeWeakPtr = std::weak_ptr<JointPointTreeNode>;
struct JointPointTreeNode {
  explicit JointPointTreeNode(const JointPointId_t& _id) : isLeaf(true), mSysId(++sCounter) { mHashSetId.insert(_id); }

  JointPointTreeNode(JointPointTreeNodeWeakPtr leftPtr, JointPointTreeNodeWeakPtr rigthPtr)
      : isLeaf(false), left_child_ptr(leftPtr), right_child_ptr(rigthPtr), mSysId(++sCounter) {
    if (CallFail(left_child_ptr.expired()) /*NonNull_Pointer(left_child_ptr)*/) {
      mHashSetId.insert(left_child_ptr.lock()->mHashSetId.begin(), left_child_ptr.lock()->mHashSetId.end());
    } else {
      LOG(FATAL) << "left_child_ptr is nullptr.";
    }
    if (CallFail(right_child_ptr.expired()) /*NonNull_Pointer(right_child_ptr)*/) {
      mHashSetId.insert(right_child_ptr.lock()->mHashSetId.begin(), right_child_ptr.lock()->mHashSetId.end());
    } else {
      LOG(FATAL) << "right_child_ptr is nullptr.";
    }
  }
  virtual ~JointPointTreeNode() { /*LOG(WARNING) << "[~] JointPointTreeNode";*/ }

  std::unordered_set<JointPointId_t, JointPointIdHashCompare> mHashSetId;
  txBool isLeaf = false;
  JointPointTreeNodeWeakPtr parentPtr /* = nullptr*/;
  JointPointTreeNodeWeakPtr left_child_ptr /* = nullptr*/;
  JointPointTreeNodeWeakPtr right_child_ptr /* = nullptr*/;
  static std::atomic<txSize> sCounter;
  const txSize mSysId;
  JointPointTreeNodeWeakPtr ancestorPtr /* = nullptr*/;
  Utils::Counter<JointPointTreeNode> _c;
};

TX_NAMESPACE_CLOSE(Base)

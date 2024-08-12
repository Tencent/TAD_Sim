// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#if 0
#  include <proj_api.h>
#  include "tad_xosc.h"
#  include "txSceneLoader.h"
TX_NAMESPACE_OPEN(SceneLoader)

class OSC_SceneLoader : public Base::ISceneLoader {
 public:
  using txFloat = Base::txFloat;
  using txString = Base::txString;
  using txBool = Base::txBool;
  using txUShort = Base::txUShort;
  using txUInt = Base::txUInt;
  using txInt = Base::txInt;
  using txSysId = Base::txSysId;
  using VEHICLE_TYPE = Base::Enums::VEHICLE_TYPE;
  using VehicleBehavior = Base::Enums::VEHICLE_BEHAVIOR;
  using STATIC_ELEMENT_TYPE = Base::Enums::STATIC_ELEMENT_TYPE;
  using PEDESTRIAN_TYPE = Base::Enums::PEDESTRIAN_TYPE;

  static VehicleBehavior Str2Behavior(const txString& strBehavior) TX_NOEXCEPT;

  struct SimVehicle {
    using VEHICLE_TYPE = Base::Enums::VEHICLE_TYPE;
    txString m_strName;
    txString m_strID;
    VEHICLE_TYPE m_eType = VEHICLE_TYPE::Sedan;
    VehicleBehavior m_eBehavior = VehicleBehavior::eTadAI;
    txString m_strHeight;
    txString m_strWidth;
    txString m_strLength;
    txString m_strMaxVelocity;
    txString m_strStartTime;
    txString m_strStartVelocity;

    txString m_strLaneID; /* <0 on lane; >= 0 on link*/
    txString m_strStartShift;
    txString m_strOffset;
    txString m_strRouteID;

    txString m_strAccID;
    txString m_strMergeID;
  };

  struct SimObstacle {
    txString m_strName;
    txString m_strID;
    STATIC_ELEMENT_TYPE m_eType = STATIC_ELEMENT_TYPE::Box;
    txString m_strHeight;
    txString m_strWidth;
    txString m_strLength;

    txString m_strLaneID; /* <0 on lane; >= 0 on link*/
    txString m_strStartShift;
    txString m_strOffset;
    txString m_strRouteID;
    txString m_strDirection;
  };

  struct SimPedestrain {
    enum class PedestrainBehavior : txInt { Trajectory, UserDefine };
    txString m_strName;
    txString m_strID;
    PedestrainBehavior m_eBehavior = PedestrainBehavior::Trajectory;
    PEDESTRIAN_TYPE m_eType = PEDESTRIAN_TYPE::human;
    txString m_strStartTime;
    txString m_strEndTime;
    txString m_strMaxVelocity;
    txString m_strStartVelocity;

    txString m_strLaneID; /* <0 on lane; >= 0 on link*/
    txString m_strStartShift;
    txString m_strOffset;
    txString m_strRouteID;
  };

  struct sTagQueryInfoResult {
    enum class LocationType : txInt { onLane, onLink };
    LocationType eType = LocationType::onLane;
    txBool IsOnLane() const TX_NOEXCEPT { return LocationType::onLane == eType; }
    txBool IsOnLink() const TX_NOEXCEPT { return LocationType::onLink == eType; }

    Base::txLaneLinkID lanelinkID;
    Base::txLaneUId laneUid;
    txFloat dShift;
    txFloat dOffset;
    hadmap::txPoint dStartPoint;
  };

  struct sPathPoint {
    txString m_strLon;
    txString m_strLat;
    txString m_strAlt;

    hadmap::txPoint gps;
    txString toRouteStr() TX_NOEXCEPT {
      std::ostringstream oss;
      oss << _StreamPrecision_ << gps.x << "," << gps.y;
      return oss.str();
    }
  };

  using sPathPoints = std::vector<sPathPoint>;

  enum class RoutingActionType : txInt { AssignRouteAction_Type, FollowTrajectoryAction_Type };
  struct SimRoute {
    txString m_strRouteId;
    sPathPoints m_path;

    txString Str() const TX_NOEXCEPT {
      std::ostringstream oss;
      oss << TX_VARS_NAME(id, m_strRouteId) << TX_VARS_NAME(type, "start_end") << TX_VARS_NAME(start, m_strStart)
          << TX_VARS_NAME(end, m_strEnd) << TX_VARS_NAME(mid, m_strMid);
      return oss.str();
    }

    void ConvertToStr() TX_NOEXCEPT {
      m_strStart.clear();
      m_strEnd.clear();
      m_strMid.clear();

      sPathPoints tmp_path = m_path;
      if (tmp_path.size() > 0) {
        m_strStart = tmp_path.front().toRouteStr();
        if (tmp_path.size() > 1) {
          m_strEnd = tmp_path.back().toRouteStr();

          if (tmp_path.size() > 2) {
            txBool bflag = false;
            for (txInt i = 1; i < (tmp_path.size() - 1); ++i) {
              auto& ref = tmp_path[i];
              m_strMid += ref.toRouteStr() + ";";
              bflag = true;
            }
            if (bflag) {
              txAssert(';' == m_strMid.back());
              m_strMid = m_strMid.substr(0, m_strMid.size() - 1);
            }
          }
        }
      }
    }
    txString m_strStart;
    txString m_strEnd;
    txString m_strMid;

    txString m_strType = "start_end";
    txString m_strStartLon;
    txString m_strStartLat;
    txString m_strStartAlt;
  };

  enum class TrigConditionType : txInt {
    TCT_INVALID,
    TCT_TIME_ABSOLUTE,
    TCT_TIME_RELATIVE,
    TCT_POSITION_RELATIVE,
    TCT_POSITION_ABSOLUTE
  };

  enum class DistanceModeType : txInt { TDM_NONE, TDM_LANE_PROJECTION, TDM_EUCLIDEAN_DISTANCE };

  enum class AccEndType : txInt { AECT_NONE, AECT_TIME, AECT_VELOCITY };

  static txString AccelerationEndConditionTypeValue2Str(const AccEndType _e) TX_NOEXCEPT {
    switch (_e) {
      case AccEndType::AECT_TIME:
        return txString("Time");
      case AccEndType::AECT_VELOCITY:
        return txString("Velocity");
      default:
        return txString("None");
    }
  }

  struct TimeNode {
    TimeNode& operator=(const TimeNode& other) TX_NOEXCEPT {
      eTriggerType = other.eTriggerType;
      dTriggerValue = other.dTriggerValue;
      eDistanceMode = other.eDistanceMode;
      dAcc = other.dAcc;
      nCount = other.nCount;
      eStopType = other.eStopType;
      dStopValue = other.dStopValue;
      return (*this);
    }

    bool operator<(const TimeNode& other) const TX_NOEXCEPT {
      if (eTriggerType < other.eTriggerType) {
        return true;
      } else if (eTriggerType == other.eTriggerType && dTriggerValue < other.dTriggerValue) {
        return true;
      }

      return false;
    }

    bool operator==(const TimeNode& other) TX_NOEXCEPT {
      const TimeNode& lhs = *this;
      const TimeNode& rhs = other;
      if (lhs.eTriggerType != rhs.eTriggerType) {
        return false;
      }

      const double epsil = 0.000000001;
      double delta = lhs.dTriggerValue - rhs.dTriggerValue;
      if (delta > -1 * epsil && delta < epsil) {
        return true;
      }
      return false;
    }
    txFloat dTriggerValue;
    txFloat dAcc;
    TrigConditionType eTriggerType;
    DistanceModeType eDistanceMode;
    txInt nCount;
    AccEndType eStopType;
    txFloat dStopValue;
  };
  using TimeNodeSet = std::set<TimeNode>;

  struct SimAcceleration {
    txInt m_ID;
    TimeNodeSet m_nodes;
    txString m_strID;
    txString m_strProfilesTime;
    txString m_strProfilesEvent;
    txString m_strEndConditionTime;
    txString m_strEndConditionEvent;

    void ConvertToTimeStr() TX_NOEXCEPT {
      m_strProfilesTime.clear();
      m_strEndConditionTime.clear();

      m_strID = Utils::IntToString(m_ID);
      txInt nCount = 0;
      for (const auto& refNode : m_nodes) {
        if (TrigConditionType::TCT_TIME_ABSOLUTE == refNode.eTriggerType) {
          txString strTrigValue = Utils::FloatToString(refNode.dTriggerValue);
          txString strAcc = Utils::FloatToString(refNode.dAcc);
          txString strOneNode = strTrigValue + "," + strAcc;
          if (nCount > 0) {
            m_strProfilesTime.append(";");
          }

          m_strProfilesTime.append(strOneNode);

          txString strEndType = AccelerationEndConditionTypeValue2Str(refNode.eStopType);
          txString strEndValue = Utils::FloatToString(refNode.dStopValue);
          txString strOneCondition = strEndType + "," + strEndValue;
          if (nCount > 0) {
            m_strEndConditionTime.append(";");
          }

          m_strEndConditionTime.append(strOneCondition);

          nCount++;
        }
      }
    }
    void ConvertToEventStr() TX_NOEXCEPT {
      m_strProfilesEvent.clear();
      m_strEndConditionEvent.clear();
      m_strID = Utils::IntToString(m_ID);
      int nCount = 0;
      for (const auto& refNode : m_nodes) {
        if (TrigConditionType::TCT_TIME_ABSOLUTE != refNode.eTriggerType) {
          txString strTrigType = "ttc";
          if (TrigConditionType::TCT_POSITION_RELATIVE == refNode.eTriggerType) {
            strTrigType = "egoDistance";
          }
          txString strDistanceMode = "laneprojection";
          if (DistanceModeType::TDM_EUCLIDEAN_DISTANCE == refNode.eDistanceMode) {
            strDistanceMode = "euclideandistance";
          }

          txString strTrigValue = Utils::FloatToString(refNode.dTriggerValue);
          txString strAcc = Utils::FloatToString(refNode.dAcc);
          txString strCount = Utils::IntToString(refNode.nCount);
          txString strOneNode =
              strTrigType + " " + strDistanceMode + " " + strTrigValue + "," + strAcc + " [" + strCount + "]";

          if (nCount > 0) {
            m_strProfilesEvent.append(";");
          }

          m_strProfilesEvent.append(strOneNode);

          txString strEndType = AccelerationEndConditionTypeValue2Str(refNode.eStopType);
          txString strEndValue = Utils::FloatToString(refNode.dStopValue);
          txString strOneCondition = strEndType + "," + strEndValue;
          if (nCount > 0) {
            m_strEndConditionEvent.append(";");
          }

          m_strEndConditionEvent.append(strOneCondition);

          nCount++;
        }
      }
    }
  };
  enum class VEHICLE_MOVE_LANE_STATE : Base::txInt {
    VEHICLE_MOVE_LANE_RIGHT_IN_LANE = -2,
    VEHICLE_MOVE_LANE_RIGHT = -1,
    VEHICLE_MOVE_LANE_STRAIGHT = 0,
    VEHICLE_MOVE_LANE_LEFT = 1,
    VEHICLE_MOVE_LANE_LEFT_IN_LANE = 2,
    VEHICLE_MOVE_LANE_GOAL = 3
  };
  enum class MergeDirType : txInt {
    Straight = 0,
    LaneSwitch_Left = 1,
    LaneSwitch_Right = -1,
    LaneKeep_Left = 2,
    LaneKeep_Right = -2
  };
  static MergeDirType Int2MergeDirType(const txInt nType) TX_NOEXCEPT {
    switch (nType) {
      case 1:
        return MergeDirType::LaneSwitch_Left;
      case -1:
        return MergeDirType::LaneSwitch_Right;
      case 2:
        return MergeDirType::LaneKeep_Left;
      case -2:
        return MergeDirType::LaneKeep_Right;
      default:
        return MergeDirType::Straight;
    }
  }
  struct MergeNode {
    MergeNode& operator=(const MergeNode& other) TX_NOEXCEPT {
      eTrigType = other.eTrigType;
      dTrigValue = other.dTrigValue;
      eDistanceMode = other.eDistanceMode;
      eDir = other.eDir;
      dDuration = other.dDuration;
      dOffset = other.dOffset;
      nCount = other.nCount;
      return (*this);
    }

    bool operator<(const MergeNode& other) const TX_NOEXCEPT {
      if (eTrigType < other.eTrigType) {
        return true;
      } else if (eTrigType == other.eTrigType && dTrigValue < other.dTrigValue) {
        return true;
      }

      return false;
    }

    bool operator==(const MergeNode& other) TX_NOEXCEPT {
      const MergeNode& lhs = *this;
      const MergeNode& rhs = other;
      if (lhs.eTrigType != rhs.eTrigType) {
        return false;
      }
      const double epsil = 0.000001;
      double delta = lhs.dTrigValue - rhs.dTrigValue;
      if (delta > -1 * epsil && delta < epsil) {
        return true;
      }
      return false;
    }

    txFloat dTrigValue;
    txFloat dDuration;
    DistanceModeType eDistanceMode;
    MergeDirType eDir;
    txFloat dOffset;
    TrigConditionType eTrigType;
    txInt nCount;
  };
  using MergeNodeSet = std::set<MergeNode>;

  struct SimMerge {
    txInt m_ID;
    MergeNodeSet m_merges;
    txString m_strID;
    txString m_strProfileTime;
    txString m_strProfileEvent;

    void ConvertToTimeStr() TX_NOEXCEPT {
      m_strProfileTime.clear();
      m_strID = std::to_string(m_ID);
      int nCount = 0;
      for (const auto& refNode : m_merges) {
        if (TrigConditionType::TCT_TIME_ABSOLUTE == refNode.eTrigType) {
          txString strValue = Utils::FloatToString(refNode.dTrigValue);
          txString strDir = std::to_string(Utils::to_underlying(refNode.eDir));
          txString strDuration = Utils::FloatToString(refNode.dDuration);
          txString strOneNode;
          if (MergeDirType::LaneKeep_Left == refNode.eDir || MergeDirType::LaneKeep_Right == refNode.eDir) {
            txString strOffset = Utils::FloatToString(refNode.dOffset);
            strOneNode = strValue + "," + strDir + "," + strDuration + "," + strOffset;
          } else {
            strOneNode = strValue + "," + strDir + "," + strDuration;
          }
          if (nCount > 0) {
            m_strProfileTime.append(";");
          }
          m_strProfileTime.append(strOneNode);
          nCount++;
        }
      }
    }
    void ConvertToEventStr() TX_NOEXCEPT {
      m_strProfileEvent.clear();
      m_strID = std::to_string(m_ID);
      int nCount = 0;
      for (const auto& refNode : m_merges) {
        if (TrigConditionType::TCT_TIME_ABSOLUTE != refNode.eTrigType) {
          txString strType = "ttc";
          if (TrigConditionType::TCT_POSITION_RELATIVE == refNode.eTrigType) {
            strType = "egoDistance";
          }

          txString strDistanceMode = "laneprojection";
          if (DistanceModeType::TDM_EUCLIDEAN_DISTANCE == refNode.eDistanceMode) {
            strDistanceMode = "euclideandistance";
          }
          txString strValue = Utils::FloatToString(refNode.dTrigValue);
          txString strDir = std::to_string(Utils::to_underlying(refNode.eDir));
          txString strDuration = Utils::FloatToString(refNode.dDuration);
          txString strOffset = Utils::FloatToString(refNode.dOffset);
          txString strCount = std::to_string(refNode.nCount);

          txString strOneNode;

          if (MergeDirType::LaneKeep_Left == refNode.eDir || MergeDirType::LaneKeep_Right == refNode.eDir) {
            strOneNode = strType + " " + strDistanceMode + " " + strValue + "," + strDir + "," + strDuration + "," +
                         strOffset + " [" + strCount + "]";
          } else {
            strOneNode = strType + " " + strDistanceMode + " " + strValue + "," + strDir + "," + strDuration + ",v [" +
                         strCount + "]";
          }
          if (nCount > 0) {
            m_strProfileEvent.append(";");
          }
          m_strProfileEvent.append(strOneNode);
          nCount++;
        }
      }
    }
  };

  struct PedestrianVelocityNode {
    txFloat dTriggerValue;
    txInt nDirection;
    txFloat dVelocity;
    txInt nCount;
    DistanceModeType eDistanceMode;
    TrigConditionType eTriggerType;
  };
  using PedestrianVelocityNodeArray = std::vector<PedestrianVelocityNode>;
  struct SimPedestrianVelocity {
    txInt m_ID;
    txString m_strID;
    PedestrianVelocityNodeArray m_nodes;
    txString m_strProfileTime;
    txString m_strProfileEvent;

    void ConvertToTimeStr() TX_NOEXCEPT {
      m_strID = std::to_string(m_ID);
      m_strProfileTime.clear();
      int nCount = 0;
      for (auto& refNode : m_nodes) {
        if (TrigConditionType::TCT_TIME_ABSOLUTE == refNode.eTriggerType) {
          std::string strTrigValue = Utils::FloatToString(refNode.dTriggerValue);
          std::string strDirection = std::to_string(refNode.nDirection);
          std::string strVelocity = Utils::FloatToString(refNode.dVelocity);

          std::string strOneNode = strTrigValue + "," + strDirection + "," + strVelocity;
          if (nCount > 0) {
            strOneNode.append(";");
          }

          m_strProfileTime.append(strOneNode);
          nCount++;
        }
      }
    }
    void ConvertToEventStr() TX_NOEXCEPT {
      m_strID = std::to_string(m_ID);
      m_strProfileEvent.clear();
      int nCount = 0;
      txString m_strTriggerType;
      txString m_strDistanceMode;
      for (auto& refNode : m_nodes) {
        if (TrigConditionType::TCT_TIME_ABSOLUTE != refNode.eTriggerType) {
          if (refNode.eTriggerType == TrigConditionType::TCT_TIME_RELATIVE) {
            m_strTriggerType = "ttc";
          } else if (refNode.eTriggerType == TrigConditionType::TCT_POSITION_RELATIVE) {
            m_strTriggerType = "egoDistance";
          } else {
            m_strTriggerType = "ttc";
          }

          m_strDistanceMode = "laneprojection";
          if (DistanceModeType::TDM_EUCLIDEAN_DISTANCE == refNode.eDistanceMode) {
            m_strDistanceMode = "euclideandistance";
          }

          std::string strTrigValue = Utils::FloatToString(refNode.dTriggerValue);
          std::string strDirection = std::to_string(refNode.nDirection);
          std::string strVelocity = Utils::FloatToString(refNode.dVelocity);
          std::string strCount = std::to_string(refNode.nCount);
          std::string strOneNode = m_strTriggerType + " " + m_strDistanceMode + " " + strTrigValue + "," +
                                   strDirection + "," + strVelocity + " [" + strCount + "]";
          if (nCount > 0) {
            strOneNode.append(";");
          }
          m_strProfileEvent.append(strOneNode);
          nCount++;
        }
      }
    }
  };

  struct VelocityTimeNode {
    txFloat dVelocity;
    TrigConditionType eTriggerType;
    txFloat dTriggerValue;
    DistanceModeType eDistanceMode;
    txInt nCount;

    txString m_strTimeStr;
    txString m_strEventStr;

    void ConvertToEventStr() TX_NOEXCEPT {
      if (eTriggerType != TrigConditionType::TCT_TIME_ABSOLUTE) {
        m_strEventStr.clear();
        txString strTrigType = "ttc";
        if (eTriggerType == TrigConditionType::TCT_POSITION_RELATIVE) {
          strTrigType = "egoDistance";
        }

        txString strDistanceMode = "laneprojection";
        if (eDistanceMode == DistanceModeType::TDM_EUCLIDEAN_DISTANCE) {
          strDistanceMode = "euclideandistance";
        }
        txString strTrigValue = Utils::FloatToString(dTriggerValue);
        txString strVelocity = Utils::FloatToString(dVelocity);
        txString strCount = std::to_string(nCount);
        m_strEventStr =
            strTrigType + " " + strDistanceMode + " " + strTrigValue + "," + strVelocity + " [" + strCount + "]";
      }
    }

    void ConvertToTimeStr() TX_NOEXCEPT {
      if (eTriggerType == TrigConditionType::TCT_TIME_ABSOLUTE) {
        txString strTrigValue = Utils::FloatToString(dTriggerValue);
        txString strVelocity = Utils::FloatToString(dVelocity);
        m_strTimeStr = strTrigValue + "," + strVelocity;
      } else {
        m_strTimeStr = "";
      }
    }
  };

  struct SimVehicleVelocity {
    txString m_strID;
    // txString m_strProfiles;
    txString m_strProfilesTime;
    txString m_strProfilesEvent;
    uint64_t m_ID;
    std::vector<VelocityTimeNode> m_nodes;

    void ConvertToTimeStr() TX_NOEXCEPT {
      m_strProfilesTime.clear();
      m_strID = std::to_string(m_ID);
      int nCount = 0;
      for (auto& refNode : m_nodes) {
        if (refNode.eTriggerType == TrigConditionType::TCT_TIME_ABSOLUTE) {
          refNode.ConvertToTimeStr();
          if (nCount > 0) {
            m_strProfilesTime.append(";");
          }

          m_strProfilesTime.append(refNode.m_strTimeStr);
          nCount++;
        }
      }
    }

    void ConvertToEventStr() TX_NOEXCEPT {
      m_strProfilesEvent.clear();
      m_strID = std::to_string(m_ID);
      int nCount = 0;
      for (auto& refNode : m_nodes) {
        if (refNode.eTriggerType != TrigConditionType::TCT_TIME_ABSOLUTE) {
          refNode.ConvertToEventStr();
          if (nCount > 0) {
            m_strProfilesEvent.append(";");
          }
          m_strProfilesEvent.append(refNode.m_strEventStr);
          nCount++;
        }
      }
    }
  };

  struct OSC_AccelerationViewer : public IAccelerationViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_AccelerationViewer() TX_DEFAULT;
    virtual ~OSC_AccelerationViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString endCondition() const TX_NOEXCEPT TX_OVERRIDE;
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_acceleration_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual std::vector<EventEndCondition_t> end_condition_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_acceleration_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_acceleration_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class OSC_SceneLoader;
    void Init(const SimAcceleration& srcAcc, const EventActionType srcType) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimAcceleration m_acc;
    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using OSC_AccelerationViewerPtr = std::shared_ptr<OSC_AccelerationViewer>;

  struct OSC_MergeViewer : public IMergesViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_MergeViewer() TX_DEFAULT;
    virtual ~OSC_MergeViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
    timestamp_direction_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, DistanceProjectionType, Base::txFloat /*Duration*/,
                                   Base::txFloat /*offset*/, Base::txUInt> >
    ttc_direction_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, DistanceProjectionType, Base::txFloat /*Duration*/,
                                   Base::txFloat /*offset*/, Base::txUInt> >
    egodistance_direction_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class OSC_SceneLoader;
    void Init(SimMerge const& srcMerge, const EventActionType srcType) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimMerge m_merge;

    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using OSC_MergeViewerPtr = std::shared_ptr<OSC_MergeViewer>;

  struct OSC_VehiclesViewer : public IVehiclesViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_VehiclesViewer() TX_DEFAULT;
    virtual ~OSC_VehiclesViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsIdValid() const TX_NOEXCEPT;
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT { return true; }
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsLaneIdValid() const TX_NOEXCEPT;
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txSysId accID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsAccIdValid() const TX_NOEXCEPT;
    virtual Base::txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsMergeIdValid() const TX_NOEXCEPT;
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txSysId follow() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat mergeTime() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat offsetTime() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

   protected:
    friend class OSC_SceneLoader;
    void Init(const SimVehicle& srcVehicle) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimVehicle m_vehicle;
  };
  using OSC_VehiclesViewerPtr = std::shared_ptr<OSC_VehiclesViewer>;

  struct OSC_RouteViewer : public IRouteViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_RouteViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual ROUTE_TYPE type() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat startLon() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat startLat() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat endLon() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat endLat() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > midPoints() const TX_NOEXCEPT TX_OVERRIDE;
    /*virtual Base::txFloat midLon() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat midLat() const TX_NOEXCEPT TX_OVERRIDE;*/
    virtual Base::txRoadID roidId() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txSectionID sectionId() const TX_NOEXCEPT TX_OVERRIDE;

   protected:
    friend class OSC_SceneLoader;
    void Init(SimRoute const& srcRoute) TX_NOEXCEPT;
    template <typename t, typename p>
    t const& ConditionValue(t const& trueVal, p const& defaultVal) const TX_NOEXCEPT {
      return (m_inited) ? (trueVal) : (defaultVal);
    }

   protected:
    Base::txBool m_inited = false;
    SimRoute m_route;
  };
  using OSC_RouteViewerPtr = std::shared_ptr<OSC_RouteViewer>;

  struct OSC_VelocityViewer : public IVelocityViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_VelocityViewer() TX_DEFAULT;
    virtual ~OSC_VelocityViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class OSC_SceneLoader;
    void Init(const SimVehicleVelocity& srcMerge, const EventActionType _type) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimVehicleVelocity m_velocity;
    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using OSC_VelocityViewerPtr = std::shared_ptr<OSC_VelocityViewer>;

  struct OSC_EgoVehiclesViewer : public OSC_VehiclesViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_EgoVehiclesViewer() TX_DEFAULT;
    virtual ~OSC_EgoVehiclesViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsIdValid() const TX_NOEXCEPT;
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsLaneIdValid() const TX_NOEXCEPT;
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat lon() const TX_NOEXCEPT;
    virtual Base::txFloat lat() const TX_NOEXCEPT;
    virtual Base::txFloat alt() const TX_NOEXCEPT;
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txSysId accID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsAccIdValid() const TX_NOEXCEPT;
    virtual Base::txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsMergeIdValid() const TX_NOEXCEPT;
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;

   protected:
    friend class OSC_SceneLoader;
    void Init(sim_msg::Location const& srcEgo) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    sim_msg::Location m_ego;
    Base::txInt m_egoId = -1;
  };
  using OSC_EgoVehiclesViewerPtr = std::shared_ptr<OSC_EgoVehiclesViewer>;
#  if Use_TruckEgo
  struct OSC_TruckVehiclesViewer : public OSC_EgoVehiclesViewer {
   public:
    OSC_TruckVehiclesViewer() TX_DEFAULT;
    virtual ~OSC_TruckVehiclesViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
  };
  using OSC_TruckVehiclesViewerPtr = std::shared_ptr<OSC_TruckVehiclesViewer>;
#  endif /*Use_TruckEgo*/

  struct OSC_TrailerVehiclesViewer : public OSC_EgoVehiclesViewer {
   public:
    OSC_TrailerVehiclesViewer() TX_DEFAULT;
    virtual ~OSC_TrailerVehiclesViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
  };
  using OSC_TrailerVehiclesViewerPtr = std::shared_ptr<OSC_TrailerVehiclesViewer>;

  struct OSC_ObstacleViewer : public IObstacleViewer {
    using STATIC_ELEMENT_TYPE = Base::Enums::STATIC_ELEMENT_TYPE;
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_ObstacleViewer() TX_DEFAULT;
    virtual ~OSC_ObstacleViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;
    /*virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;*/
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat direction() const TX_NOEXCEPT TX_OVERRIDE;
    virtual STATIC_ELEMENT_TYPE type() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

   protected:
    friend class OSC_SceneLoader;
    void Init(SimObstacle const& srcObstacle) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimObstacle m_obstacle;
  };
  using OSC_ObstacleViewerPtr = std::shared_ptr<OSC_ObstacleViewer>;

  struct OSC_PedestriansViewer : public IPedestriansViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_PedestriansViewer() TX_DEFAULT;
    virtual ~OSC_PedestriansViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT { return true; }
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat end_t() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString type() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool hadDirection() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

   protected:
    friend class OSC_SceneLoader;
    void Init(SimPedestrain const& srcPedestrian) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimPedestrain m_pedestrian;
  };
  using OSC_PedestriansViewerPtr = std::shared_ptr<OSC_PedestriansViewer>;

  struct OSC_PedestriansEvent_time_velocity_Viewer : public IPedestriansEventViewer {
   public:
    using txFloat = Base::txFloat;
    using txInt = Base::txInt;
    using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_PedestriansEvent_time_velocity_Viewer() TX_DEFAULT;
    virtual ~OSC_PedestriansEvent_time_velocity_Viewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > timestamp_direction_speed_tuple_vector()
        const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class OSC_SceneLoader;
    void Init(SimPedestrianVelocity const& srcTimeVelocity, const EventActionType _t) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimPedestrianVelocity m_time_velocity;
    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using OSC_PedestriansEvent_time_velocity_ViewerPtr = std::shared_ptr<OSC_PedestriansEvent_time_velocity_Viewer>;

  struct OSC_PedestriansEvent_event_velocity_Viewer : public IPedestriansEventViewer {
   public:
    using txFloat = Base::txFloat;
    using txInt = Base::txInt;
    using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    OSC_PedestriansEvent_event_velocity_Viewer() TX_DEFAULT;
    virtual ~OSC_PedestriansEvent_event_velocity_Viewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;

    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > timestamp_direction_speed_tuple_vector()
        const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_direction_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_direction_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class OSC_SceneLoader;
    void Init(SimPedestrianVelocity const& srcEventVelocity, const EventActionType _t) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SimPedestrianVelocity m_event_velocity;
    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using OSC_PedestriansEvent_event_velocity_ViewerPtr = std::shared_ptr<OSC_PedestriansEvent_event_velocity_Viewer>;

 public:
  OSC_SceneLoader();
  virtual ~OSC_SceneLoader() TX_DEFAULT;
  virtual bool IsValid() TX_NOEXCEPT TX_OVERRIDE;
  virtual bool Load(const txString&) TX_NOEXCEPT TX_OVERRIDE;
  virtual bool LoadObjects() TX_NOEXCEPT TX_OVERRIDE;
  virtual EStatus Status() const TX_NOEXCEPT TX_OVERRIDE { return _status; }
  virtual Base::txLpsz ClassName() const TX_NOEXCEPT TX_OVERRIDE { return _class_name; }
  virtual Base::txString Source() const TX_NOEXCEPT TX_OVERRIDE { return _source; }
  virtual ESceneType GetSceneType() const TX_NOEXCEPT TX_OVERRIDE { return ESceneType::eOSC; }

 public:
  virtual Base::txBool GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
      TX_NOEXCEPT TX_OVERRIDE;
  virtual IRouteViewerPtr GetRouteData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IRouteViewerPtr GetEgoRouteData() TX_NOEXCEPT TX_OVERRIDE;
  virtual IAccelerationViewerPtr GetAccsData(Base::txSysId const _id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IAccelerationViewerPtr GetAccsEventData(Base::txSysId const _id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IMergesViewerPtr GetMergesData(Base::txSysId const _id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IMergesViewerPtr GetMergesEventData(Base::txSysId const _id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IVelocityViewerPtr GetVelocityData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IVelocityViewerPtr GetVelocityEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_TimeEvent(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_VelocityEvent(Base::txSysId const id)
      TX_NOEXCEPT TX_OVERRIDE;

  virtual IVehiclesViewerPtr GetVehicleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IVehiclesViewerPtr();
  };

  virtual std::unordered_map<Base::txSysId, IVehiclesViewerPtr> GetAllVehicleData() TX_NOEXCEPT TX_OVERRIDE;
  virtual IPedestriansViewerPtr GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IPedestriansViewerPtr();
  }
  virtual std::unordered_map<Base::txSysId, IPedestriansViewerPtr> GetAllPedestrianData() TX_NOEXCEPT TX_OVERRIDE;

  virtual ISignlightsViewerPtr GetSignlightData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return ISignlightsViewerPtr();
  }
  virtual std::unordered_map<Base::txSysId, ISignlightsViewerPtr> GetAllSignlightData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, ISignlightsViewerPtr>();
  }

  virtual IObstacleViewerPtr GetObstacleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IObstacleViewerPtr();
  }

  virtual std::unordered_map<Base::txSysId, IObstacleViewerPtr> GetAllObstacleData() TX_NOEXCEPT TX_OVERRIDE {
    std::unordered_map<Base::txSysId, Base::ISceneLoader::IObstacleViewerPtr> retMap;
    for (auto& refObstaclePair : m_mapObstacles) {
      auto retPtr = std::make_shared<OSC_ObstacleViewer>();
      if (retPtr) {
        retPtr->Init(refObstaclePair.second);
        retMap[retPtr->id()] = retPtr;
      }
    }
    return retMap;
  }

  virtual Base::txInt GetRandomSeed() const TX_NOEXCEPT TX_OVERRIDE {
    if (IsSelfDefineRandomSeed()) {
      return GetSelfDefineRandomSeed();
    } else {
      return FLAGS_default_random_seed;
    }
  }

  virtual IVehiclesViewerPtr GetEgoData() TX_NOEXCEPT TX_OVERRIDE;

  virtual IVehiclesViewerPtr GetEgoTrailerData() TX_NOEXCEPT TX_OVERRIDE;

 public:
  virtual void SetTADEgoData(sim_msg::Location const& _egoInfo) TX_NOEXCEPT;

 protected:
  struct MapProjParam {
    Base::txFloat m_north = 0.0;
    Base::txFloat m_south = 0.0;
    Base::txFloat m_east = 0.0;
    Base::txFloat m_west = 0.0;
    Base::txString m_proj =
        "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null "
        "+wktext +no_defs";
    /*"+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext
     * +no_defs"*/
    Base::txString m_Lonlat_Coordinator_Str = "+proj=longlat +datum=WGS84 +no_defs";
    projPJ pj_xodr;
    projPJ pj_lonlat;
    Base::txString Str() const TX_NOEXCEPT {
      std::ostringstream oss;
      oss << TX_VARS_NAME(north, m_north) << TX_VARS_NAME(south, m_south) << TX_VARS_NAME(east, m_east)
          << TX_VARS_NAME(west, m_west) << TX_VARS_NAME(proj, m_proj);
      return oss.str();
    }

    Base::txBool Initialize() TX_NOEXCEPT {
      pj_xodr = pj_init_plus(m_proj.c_str());
      pj_lonlat = pj_init_plus(m_Lonlat_Coordinator_Str.c_str());
      return true;
    }

    hadmap::txPoint ConvertXODR2LonLat(const Base::txFloat x, const Base::txFloat y) TX_NOEXCEPT {
      hadmap::txPoint gps;
      gps.z = 0.0;
      gps.x = x + m_west;
      gps.y = y + m_south;

      pj_transform(pj_xodr, pj_lonlat, 1, 1, &gps.x, &gps.y, NULL);

      gps.x *= RAD_TO_DEG;
      gps.y *= RAD_TO_DEG;
      return gps;
    }

    hadmap::txPoint ConvertXODR2LonLat(const Base::txString& str_x, const Base::txString& str_y) TX_NOEXCEPT {
      return ConvertXODR2LonLat(std::stod(str_x), std::stod(str_y));
    }
  };
  static MapProjParam GetXodrProjParam(const Base::txString str_xodr_file_path) TX_NOEXCEPT;
  struct TypeValue {
    Base::txString _parameterType;
    Base::txString _vaule;
  };
  using xmlFileHeader = SceneLoader::OSC_TADSim::xmlOpenSCENARIO::xmlFileHeader;
  using xmlParameterDeclarations = SceneLoader::OSC_TADSim::xmlOpenSCENARIO::xmlParameterDeclarations;
  using xmlEntities = SceneLoader::OSC_TADSim::xmlOpenSCENARIO::xmlEntities;
  using xmlStoryboard = SceneLoader::OSC_TADSim::xmlOpenSCENARIO::xmlStoryboard;
  using xmlPrivateAction = SceneLoader::OSC_TADSim::xmlPrivateAction;
  using PrivateAction = xmlPrivateAction;

  using xmlStartTrigger = SceneLoader::OSC_TADSim::xmlStartTrigger;
  using Trigger = xmlStartTrigger;

  void ParseFileHeader(xmlFileHeader& fht) TX_NOEXCEPT;
  void ParseParameterDeclaration(xmlParameterDeclarations& pas) TX_NOEXCEPT;
  void ParseEntities(xmlEntities& entities) TX_NOEXCEPT;
  void ParseStoryboard(xmlStoryboard& sb) TX_NOEXCEPT;
  enum class ElemTypeInXOSC : txInt {
    xosc_ego,
    xosc_vehicle,
    xosc_pedestrian,
    xosc_bike,
    xosc_obstacle,
    xosc_animal,
    xosc_undef
  };
  static std::tuple<OSC_SceneLoader::ElemTypeInXOSC, OSC_SceneLoader::txSysId> Parse_Type_Id_ByName(
      const Base::txString& strName) TX_NOEXCEPT;
  static txSysId ParseIdByName(const Base::txString& strName) TX_NOEXCEPT;
  Base::txString FindParameterDeclarationsValudeRet(const Base::txString& strKey) const TX_NOEXCEPT;
  void ParsePrivateAction(PrivateAction& pa, txString& strEntityRef, txString& strDynamicsValue,
                          txString& strTargetValue, txString& strTargetOffset, txBool& isAcc, txBool& isMerge,
                          txBool& isV, txString& Orientation, bool isInit = true) TX_NOEXCEPT;
  void ReadLongitudinalAction_SpeedAction_Absolute(PrivateAction& pa, txString& strDynamicsShape, txString& strValue,
                                                   txString& strDynamicsDimension, txString& strSpeedValue) TX_NOEXCEPT;
  void AddIniSpeed(txString& strEntityRef, txString& strSpeedV) TX_NOEXCEPT;
  void ReadTeleportAction_WorldPosition(PrivateAction& pa, txString& strX, txString& strY, txString& strZ,
                                        txString& strH) TX_NOEXCEPT;
  void AddIniWordPos(txString& strEntityRef, txString& strX, txString& strY, txString& strZ,
                     txString& strH) TX_NOEXCEPT;
  void ReadLateralAction_LaneChangeAction(PrivateAction& pa, txString& strDynamicsShape, txString& strValue,
                                          txString& strDynamicsDimension, txString& strTargetLane,
                                          txString& strTargetOffset) TX_NOEXCEPT;
  void ReadRoutingAction(PrivateAction& pa, const RoutingActionType type, sPathPoints& Vpoints,
                         txString& Orientation) TX_NOEXCEPT;
  void AddRoute(txString& strEntityRef, sPathPoints& Vpoints) TX_NOEXCEPT;
  void ParseStartTrigger(Trigger& st, txString& strEntityRef, TrigConditionType& nType, txString& strTrigerValue,
                         txString& strDistanceType, txString& strAlongRoute) TX_NOEXCEPT;
  void AddAccOrMerge(txString& strEntityRef, txString& strDynamicsValue, txString& strTargetValue, txString& strOffset,
                     const txBool isAcc, const txBool isMerge, const TrigConditionType nType, txString& strTrigerValue,
                     txString& strDistanceType, txString& strAlongRoute) TX_NOEXCEPT;
  void AddPedestrianVelocity(txString& strEntityRef, txString& strDynamicsValue, txString& strTargetValue,
                             const TrigConditionType nType, txString& strTrigerValue, txString& strDistanceType,
                             txString& strOrientation, txString& strAlongRoute) TX_NOEXCEPT;
  void AddVelocity(txString& strEntityRef, txString& strDynamicsValue, txString& strTargetValue,
                   const TrigConditionType nType, txString& strTrigerValue, txString& strDistanceType,
                   txString& strAlongRoute) TX_NOEXCEPT;

 protected:
  Base::txLpsz _class_name;
  SceneLoader::OSC_TADSim::OpenSCENARIO_ptr m_DataSource_XOSC = nullptr;
  Base::txString m_xodr_file_path;
  MapProjParam m_proj_param;

  Base::txString str_author;
  Base::txString str_date;
  Base::txString str_description;
  Base::txString str_revMajor;
  Base::txString srt_revMinor;

  std::map<Base::txString, TypeValue> m_ParameterDeclarations;

  std::unordered_map<Base::txString TX_MARK("V_1"), SimVehicle> m_mapVehicles;
  std::unordered_map<Base::txString TX_MARK("O_1"), SimObstacle> m_mapObstacles;
  std::unordered_map<Base::txString TX_MARK("P_1"), SimPedestrain> m_mapPedestrians;
  std::unordered_map<Base::txString, SimRoute> m_mapRoutes;
  std::unordered_map<Base::txString, SimAcceleration> m_mapAccs;
  std::unordered_map<Base::txString, SimMerge> m_mapMerges;
  std::unordered_map<Base::txString, SimPedestrianVelocity> m_mapPedestrianVelocitys;
  std::unordered_map<Base::txString, SimVehicleVelocity> m_mapVelocitys;
  SimRoute m_EgoRoute;
  hadmap::txPoint m_EgoStartPos = hadmap::txPoint(0.0, 0.0, 0.0);
  txFloat m_EgoStartHeading = 0.0;
  txFloat m_EgoStartSpeed = 0.0;

  sim_msg::Location m_EgoData;

  /*txInt m_nVehiclIDGeneartor = 1;
  txInt m_nObstacleIDGenerator = 1;
  txInt m_nPedestrianIDGenerator = 1;*/
  txInt m_nRouteIDGenerator = 1;
  TX_MARK("ego route is 0");
  txInt m_nAccIDGeneator = 1;
  txInt m_nMergeIDGenerator = 1;
  txInt m_nPedVIDGenerator = 1;
  txInt m_nVIDGenerator = 1;

  void Clear() TX_NOEXCEPT {
    ClearStates();
    ClearIdGeneartor();
  }

  void ClearStates() TX_NOEXCEPT {
    m_ParameterDeclarations.clear();
    m_mapVehicles.clear();
    m_mapObstacles.clear();
    m_mapPedestrians.clear();
    m_mapRoutes.clear();
    m_mapAccs.clear();
    m_mapMerges.clear();
    m_mapPedestrianVelocitys.clear();
    m_mapVelocitys.clear();
  }

  void ClearIdGeneartor() TX_NOEXCEPT {
    /*m_nVehiclIDGeneartor = 1;
    m_nObstacleIDGenerator = 1;
    m_nPedestrianIDGenerator = 1;*/
    m_nRouteIDGenerator = 1;
    TX_MARK("ego route is 0");
    m_nAccIDGeneator = 1;
    m_nMergeIDGenerator = 1;
    m_nPedVIDGenerator = 1;
    m_nVIDGenerator = 1;
  }

  void ConvertLonLat2SimPosition(const txFloat dLon, const txFloat dLat, sTagQueryInfoResult& result) TX_NOEXCEPT;

  static txBool EntityIsVechile(const txString& refEntity) TX_NOEXCEPT;
  static txBool EntityIsVechileEgo(const txString& refEntity) TX_NOEXCEPT;
  static txBool EntityIsPedestrian(const txString& refEntity) TX_NOEXCEPT;
  static txBool EntityIsObstacle(const txString& refEntity) TX_NOEXCEPT;
  static txBool EntityHasSpecialFirstChar(const txString& refEntity, const txString::value_type sCh) TX_NOEXCEPT;

 protected:
  static void ParseFileHeaderDesc(const Base::txString strDesc, Base::txString& refSimId,
                                  Base::txString& refDataVer) TX_NOEXCEPT;
  Base::txString m_strSimId = "0";
  Base::txString m_strDataVersion = "0";

 public:
  virtual std::ostream& PrintStream(std::ostream& os) TX_NOEXCEPT TX_OVERRIDE;
};

using OSC_SceneLoaderPtr = std::shared_ptr<OSC_SceneLoader>;
TX_NAMESPACE_CLOSE(SceneLoader)
#endif

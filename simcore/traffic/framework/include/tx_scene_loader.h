// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/optional.hpp>
#include <memory>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "location.pb.h"
#include "scene.pb.h"
#include "tx_enum_def.h"
#include "tx_locate_info.h"
#include "tx_marco.h"
#include "tx_serialization.h"
#include "vehicle_geometry.pb.h"
#define _LocationCloseLoop_ (1)
TX_NAMESPACE_OPEN(Base)

class ITrafficElement;
using ITrafficElementPtr = std::shared_ptr<ITrafficElement>;

// @brief 场景加载器基(接口)类
class ISceneLoader {
 public:
  using EStatus = Base::Enums::EStatus; /*enum class EStatus : txInt { None, Loading, Ready, Error };*/
  enum class eSceneEventVersion : txInt { V_0_0_0_0 = 0, V_1_0_0_0 = 1, V_1_1_0_0 = 2, V_1_2_0_0 = 3 };

 public:
  virtual ~ISceneLoader() TX_DEFAULT;

  /**
   * @brief 场景加载器是否有效
   *
   * @return txBool
   */
  virtual txBool IsValid() TX_NOEXCEPT = 0;
  virtual txBool Load(const txString&) TX_NOEXCEPT = 0;
  virtual txBool Load(const txString& _sim_path, const txString&, const txString&) TX_NOEXCEPT {
    return Load(_sim_path);
  }

  /**
   * @brief 释放资源
   *
   * @return txBool
   */
  virtual txBool Release() TX_NOEXCEPT { return false; }

  /**
   * @brief 解析场景事件
   *
   * @return txBool
   */
  virtual txBool ParseSceneEvent() TX_NOEXCEPT { return false; }

  /**
   * @brief 加载物体，包括场景中的物体和场景中的车辆
   *
   * @return txBool
   */
  virtual txBool LoadObjects() TX_NOEXCEPT = 0;

  /**
   * @brief 获取场景加载器状态
   *
   * @return EStatus
   */
  virtual EStatus Status() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取classname
   *
   * @return txLpsz
   */
  virtual txLpsz ClassName() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取source
   *
   * @return txString 一个字符串
   */
  virtual txString Source() const TX_NOEXCEPT = 0;

  /**
   * @brief 设置自身定义的随机种子
   *
   * @param _f
   * @param _seed
   */
  static void SetSelfDefineRandomSeed(const Base::txBool _f, const Base::txInt _seed) TX_NOEXCEPT {
    sBeSelfDefinedSeed = _f;
    snSelfDefinedSeed = _seed;
  }
  static Base::txBool IsSelfDefineRandomSeed() TX_NOEXCEPT { return sBeSelfDefinedSeed; }
  static Base::txBool sBeSelfDefinedSeed;
  static Base::txInt snSelfDefinedSeed;
  static Base::txInt GetSelfDefineRandomSeed() TX_NOEXCEPT;
  virtual std::ostream& PrintStream(std::ostream& os) TX_NOEXCEPT { return os; }
  friend std::ostream& operator<<(std::ostream& os, ISceneLoader& v) TX_NOEXCEPT { return v.PrintStream(os); }

 public:
  using DistanceProjectionType = Base::Enums::DistanceProjectionType;
  /*enum class DistanceProjectionType : Base::txInt { Euclidean = 0, Lane = 1 };*/
  using EventTriggerType = Base::Enums::EventTriggerType;
  /*enum class EventTriggerType : Base::txInt { Time = 0, TTC = 1, EgoDistance = 3};*/
  using VEHICLE_BEHAVIOR = Base::Enums::VEHICLE_BEHAVIOR;

  /**
   * @brief 根据字符串获取vehicle行为类型
   *
   * @param strBehavior
   * @return VEHICLE_BEHAVIOR
   */
  static VEHICLE_BEHAVIOR Str2VehicleBehavior(const txString& strBehavior) TX_NOEXCEPT {
    if (Base::txString("Planning") == strBehavior) {
      return _plus_(VEHICLE_BEHAVIOR::ePlanning);
    } else if (Base::txString("UserDefine") == strBehavior) {
      return _plus_(VEHICLE_BEHAVIOR::eUserDefined);
    } else if (Base::txString("TrafficVehicle") == strBehavior) {
      return _plus_(VEHICLE_BEHAVIOR::eTadAI);
    } else {
      return _plus_(VEHICLE_BEHAVIOR::undefined);
    }
  }

  /**
   * @brief 根据字符串获取对应的事件类型
   *
   * @param str
   * @return EventTriggerType
   */
  friend EventTriggerType Str2TriggerType(const Base::txString& str) TX_NOEXCEPT {
    if (FLAGS_EventTypeTTC == str) {
      return _plus_(EventTriggerType::TTC);
    } else if (FLAGS_EventTypeEgoDistance == str) {
      return _plus_(EventTriggerType::EgoDistance);
    } else {
      return _plus_(EventTriggerType::eTime);
    }
  }

  using acc_invalid_type = Base::Enums::acc_invalid_type;
  /*enum class acc_invalid_type : Base::txInt { None = 0, Time = 1,  Velocity = 3 };*/

  // @brief 结束条件
  struct EventEndCondition_t {
    Base::txBool m_IsValid = false;
    acc_invalid_type m_type = acc_invalid_type::eNone;
    Base::txFloat m_endCondition = 0.0;

    /**
     * @brief 返回结束条件的格式化字符串
     *
     * @return Base::txString
     */
    Base::txString Str() const TX_NOEXCEPT {
      std::ostringstream oss;
      oss << _StreamPrecision_ << "EventEndCondition_t " << TX_COND_NAME(IsValid, m_IsValid) << TX_VARS(m_type)
          << TX_VARS(m_endCondition);
      return oss.str();
    }
    friend std::ostream& operator<<(std::ostream& os, const EventEndCondition_t& v) TX_NOEXCEPT {
      os << v.Str();
      return os;
    }
  };

  struct EventInfoBase_t {
    Base::txBool m_IsValid = false;
    Base::txInt m_EndConditionIndex = -1;
  };

  // @brief 时间触发acc事件信息
  struct TimeTriggerAccEventInfo_t : public EventInfoBase_t {
    Base::txFloat m_Time = 0.0;
    Base::txFloat m_Acc = 0.0;
    friend std::ostream& operator<<(std::ostream& os, const TimeTriggerAccEventInfo_t& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "TimeTriggerAccEventInfo_t ";
      /*cereal::JSONOutputArchive archive(os);
      archive(_MAKE_NVP_("Valid", v.m_IsValid),
          _MAKE_NVP_("Time", v.m_Time),
          _MAKE_NVP_("Acc", v.m_Acc),
          _MAKE_NVP_("EndConditionIndex", v.m_EndConditionIndex));*/
      return os;
    }
  };

  // @brief 特殊触发acc事件信息
  struct SpecialTriggerAccEventInfo_t : public EventInfoBase_t {
    EventTriggerType m_TriggerType;
    Base::txFloat m_Condition_Threshold;
    Base::txFloat m_Acc = 0.0;
    DistanceProjectionType m_Proj;
    Base::txInt m_TriggerIdx = 1;

    friend std::ostream& operator<<(std::ostream& os, const SpecialTriggerAccEventInfo_t& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "TimeTriggerAccEventInfo_t ";
      /*cereal::JSONOutputArchive archive(os);
      archive(_MAKE_NVP_("Valid", v.m_IsValid),
          _MAKE_NVP_("TriggerType", v.m_TriggerType),
          _MAKE_NVP_("Condition_Threshold", v.m_Condition_Threshold),
          _MAKE_NVP_("Acc", v.m_Acc),
          _MAKE_NVP_("Proj", v.m_Proj),
          _MAKE_NVP_("TriggerIdx", v.m_TriggerIdx),
          _MAKE_NVP_("EndConditionIndex", v.m_EndConditionIndex));*/
      return os;
    }
  };

  using EgoType = Base::Enums::EgoType; /*enum class EgoType : txInt { Truck, Vehicle };*/
  using ESceneType = Base::Enums::ESceneType;
  using ROUTE_TYPE = Base::Enums::ROUTE_TYPE;
  using PEDESTRIAN_TYPE = Base::Enums::PEDESTRIAN_TYPE;
  using VEHICLE_TYPE = Base::Enums::VEHICLE_TYPE;
  using STATIC_ELEMENT_TYPE = Base::Enums::STATIC_ELEMENT_TYPE;
  using EventActionType = Base::Enums::EventActionType;

  // @brief 视图器接口
  struct IViewer {
    virtual ~IViewer() TX_DEFAULT;
    /**
     * @brief 获取视图系统id
     *
     * @return txSysId
     */
    virtual txSysId id() const TX_NOEXCEPT = 0;

    /**
     * @brief 视图是否已经初始化
     *
     * @return txBool
     */
    virtual txBool IsInited() const TX_NOEXCEPT = 0;

    /**
     * @brief 描述具体实现对象信息
     *
     * @return txString
     */
    virtual txString Str() const TX_NOEXCEPT { return txString("UnImplenment"); }
    static EventActionType EvaluateActionType(const txString& strProfile) TX_NOEXCEPT;
    static DistanceProjectionType EvaluateProjectionType(const txString& strProfile) TX_NOEXCEPT;
    virtual txString name() const TX_NOEXCEPT { return ""; }
  };

  using IViewerPtr = std::shared_ptr<IViewer>;

  // @brief route视图接口
  struct IRouteViewer : public IViewer {
    // @brief 控制路径节点
    struct control_path_node {
      using ControlPathGear = Base::Enums::ControlPathGear;
      hadmap::txPoint waypoint;
      txFloat speed_m_s = 0.0;
      ControlPathGear gear = _plus_(ControlPathGear::drive);

#if _LocationCloseLoop_
      Base::Info_Lane_t locInfo;
      txFloat dist_on_s;
#endif /*_LocationCloseLoop_*/

      /**
       * @brief 重写<< 描述轨迹点信息
       *
       * @param os
       * @param v
       * @return std::ostream&
       */
      friend std::ostream& operator<<(std::ostream& os, const control_path_node& v) TX_NOEXCEPT {
        os << "{" << TX_VARS_NAME(waypoint, Utils::ToString(v.waypoint)) << TX_VARS_NAME(speed_m_s, v.speed_m_s)
           << TX_VARS_NAME(gear, __enum2lpsz__(ControlPathGear, v.gear)) << "}";
        return os;
      }
    };
    using control_path_node_vec = std::vector<control_path_node>;

    IRouteViewer() TX_DEFAULT;
    virtual ~IRouteViewer() TX_DEFAULT;

    /**
     * @brief 获取route类型
     *
     * @return ROUTE_TYPE 返回route类型
     */
    virtual ROUTE_TYPE type() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取route开始点的经度
     *
     * @return txFloat 经度
     */
    virtual txFloat startLon() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取route起点纬度
     *
     * @return txFloat
     */
    virtual txFloat startLat() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取route结束点的经度
     *
     * @return txFloat
     */
    virtual txFloat endLon() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取route结束点的纬度
     *
     * @return txFloat
     */
    virtual txFloat endLat() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取route中间所有点经纬度
     *
     * @return std::vector< std::pair< txFloat, txFloat > >
     */
    virtual std::vector<std::pair<txFloat, txFloat> > midPoints() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取路id
     *
     * @return txRoadID
     */
    virtual txRoadID roidId() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取路段id
     *
     * @return txSectionID
     */
    virtual txSectionID sectionId() const TX_NOEXCEPT = 0;
    virtual control_path_node_vec control_path() const TX_NOEXCEPT { return control_path_node_vec(); }

    /**
     * @brief 获取相对路径的字符串表示
     *
     * @return txString
     */
    virtual txString relativePath() const TX_NOEXCEPT { return ""; }

    /**
     * @brief 输出绝对路径的字符串表示
     *
     * @return txString
     */
    virtual txString absolutePath() const TX_NOEXCEPT { return ""; }

    /**
     * @brief 获取起点gps
     *
     * @return hadmap::txPoint
     */
    virtual hadmap::txPoint startGPS() const TX_NOEXCEPT;

    /**
     * @brief 获取终点gps
     *
     * @return hadmap::txPoint
     */
    virtual hadmap::txPoint endGPS() const TX_NOEXCEPT;

    /**
     * @brief 获取中间点gps集合
     *
     * @return std::vector< hadmap::txPoint >
     */
    virtual std::vector<hadmap::txPoint> midGPS() const TX_NOEXCEPT;

    /**
     * @brief 输出整个route的字符串格式化信息
     *
     * @return txString
     */
    virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE {
      std::ostringstream oss_midpoints;
      const auto& refMidPoints = midPoints();

      for (const auto& refpair : refMidPoints) {
        oss_midpoints << _StreamPrecision_ << "(" << refpair.first << ", " << refpair.second << ")";
      }
      std::ostringstream oss;
      oss << _StreamPrecision_ << TX_VARS(id()) << TX_VARS(startLon()) << TX_VARS(startLat()) << TX_VARS(endLon())
          << TX_VARS(endLat()) << TX_VARS(oss_midpoints.str()) << TX_VARS(roidId()) << TX_VARS(sectionId())
          << TX_VARS(relativePath());
      return oss.str();
    }
  };
  using IRouteViewerPtr = std::shared_ptr<IRouteViewer>;

  // @brief acceleration视图接口
  struct IAccelerationViewer : public IViewer {
    IAccelerationViewer() TX_DEFAULT;
    virtual ~IAccelerationViewer() TX_DEFAULT;
    virtual txString profile() const TX_NOEXCEPT = 0;

    /**
     * @brief 结束条件获取
     *
     * @return Base::txString
     */
    virtual Base::txString endCondition() const TX_NOEXCEPT = 0;

    /**
     * @brief 事件行为类型
     *
     * @return EventActionType
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT = 0;
    virtual std::vector<std::pair<txFloat, txFloat> > timestamp_acceleration_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<EventEndCondition_t> end_condition_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_acceleration_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_acceleration_pair_vector() const TX_NOEXCEPT = 0;
  };
  using IAccelerationViewerPtr = std::shared_ptr<IAccelerationViewer>;

  // @brief Merges视图接口
  struct IMergesViewer : public IViewer {
    IMergesViewer() TX_DEFAULT;
    virtual ~IMergesViewer() TX_DEFAULT;
    virtual txString profile() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取事件行为类型
     *
     * @return EventActionType
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
    timestamp_direction_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txInt, DistanceProjectionType, Base::txFloat /*Duration*/,
                                   Base::txFloat /*offset*/, Base::txUInt> >
    ttc_direction_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txInt, DistanceProjectionType, Base::txFloat /*Duration*/,
                                   Base::txFloat /*offset*/, Base::txUInt> >
    egodistance_direction_pair_vector() const TX_NOEXCEPT = 0;
  };
  using IMergesViewerPtr = std::shared_ptr<IMergesViewer>;

  // @brief VelocityView接口
  struct IVelocityViewer : public IViewer {
    IVelocityViewer() TX_DEFAULT;
    virtual ~IVelocityViewer() TX_DEFAULT;
    virtual txString profile() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取事件行为类型
     *
     * @return EventActionType
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT = 0;
    virtual std::vector<std::pair<txFloat, txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txFloat, DistanceProjectionType, Base::txUInt> > ttc_speed_pair_vector()
        const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT = 0;
  };
  using IVelocityViewerPtr = std::shared_ptr<IVelocityViewer>;

  // @brief PedestrainsEventViewer接口
  struct IPedestriansEventViewer : public IViewer {
    IPedestriansEventViewer() TX_DEFAULT;
    virtual ~IPedestriansEventViewer() TX_DEFAULT;
    virtual txString profile() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取事件行为类型
     *
     * @return EventActionType
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_speed_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > timestamp_direction_speed_tuple_vector()
        const TX_NOEXCEPT = 0;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_direction_speed_pair_vector() const TX_NOEXCEPT {
      return std::vector<
          std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
    }
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_direction_speed_pair_vector() const TX_NOEXCEPT {
      return std::vector<
          std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
    }
  };
  using IPedestriansEventViewerPtr = std::shared_ptr<IPedestriansEventViewer>;

  // @brief SceneEventViewer接口
  struct ISceneEventViewer : public IViewer {
    using kvMap = std::map<Base::txString, Base::txString>;
    using kvMapVec = std::vector<kvMap>;
#if __SecenEventVersion_1_2_0_0__
    struct EventParam_t {
      kvMap conditionKVMap;
      kvMapVec endConditionKVMapVec;
      kvMapVec actionKVMapVec;
      Base::txBool IsValid() const TX_NOEXCEPT {
        return _NonEmpty_(conditionKVMap) && _NonEmpty_(endConditionKVMapVec) && _NonEmpty_(actionKVMapVec) &&
               endConditionKVMapVec.size() == actionKVMapVec.size();
      }
    };

    struct EventGroupParam_t {
      Base::txInt conditionNumber = 0;
      std::vector<kvMap> groupConditionVec;
      kvMapVec endConditionKVMapVec;
      kvMapVec actionKVMapVec;
      Base::txBool IsValid() const TX_NOEXCEPT {
        return CallSucc(IsGroupConditionValid()) && _NonEmpty_(endConditionKVMapVec) && _NonEmpty_(actionKVMapVec) &&
               endConditionKVMapVec.size() == actionKVMapVec.size();
      }
      Base::txBool IsGroupConditionValid() const TX_NOEXCEPT {
        return (_NonEmpty_(groupConditionVec) && groupConditionVec.size() == conditionNumber);
      }
    };
#endif /*__SecenEventVersion_1_2_0_0__*/
#if __TX_Mark__("Condition")
    // @brief 条件视图接口
    struct IConditionViewer {
      using ElementType = Base::Enums::ElementType;
      using SceneEventType = Base::Enums::SceneEventType;
      using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
      using ConditionDistanceType = Base::Enums::ConditionDistanceType;
      using ConditionSpeedType = Base::Enums::ConditionSpeedType;
      using ConditionEquationOp = Base::Enums::ConditionEquationOp;
      using SceneEventElementStatusType = Base::Enums::SceneEventElementStatusType;
      virtual ~IConditionViewer() TX_DEFAULT;

      /**
       * @brief 获取场景事件的类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT = 0;

      /**
       * @brief 返回条件视图描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT = 0;

      /**
       * @brief 条件视图初始化
       *
       * @param ref_kv_map 存储字段KV值用于初始化字段
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT = 0;

      /**
       * @brief 是否有效
       *
       * @return txBool
       */
      virtual txBool IsValid() const TX_NOEXCEPT { return mIsValid; }

      /**
       * @brief 当前触发的次数
       *
       * @return txSize&
       */
      virtual txSize& currentTriggerCount() TX_NOEXCEPT { return m_current_trigger_count; }
      virtual txSize currentTriggerCount() const TX_NOEXCEPT { return m_current_trigger_count; }

      /**
       * @brief 设置最后一次的value
       *
       * @param _v
       */
      virtual void setLastValue(txFloat _v) TX_NOEXCEPT {
        if (CallFail(m_op_last_value_list.is_initialized())) {
          m_op_last_value_list = std::list<txFloat>();
        }
        m_op_last_value_list->push_back(_v);
      }

      /**
       * @brief 获取可选的value集合
       *
       * @return boost::optional< std::list<txFloat> >
       */
      virtual boost::optional<std::list<txFloat> > lastValue() const TX_NOEXCEPT { return m_op_last_value_list; }

      /**
       * @brief 清空之前存储的last_alue
       */
      virtual void clearLastValue() TX_NOEXCEPT { m_op_last_value_list = boost::none; }

      /**
       * @brief 工具用于拆分元素类型和sysid
       *
       * @param refStr 元素type + id 的格式化字符串
       * @return std::tuple<ElementType, txSysId> 元素类型 + sysid
       */
      static std::tuple<ElementType, txSysId> splite_type_id(const txString& refStr) TX_NOEXCEPT;

      /**
       * @brief 拼接元素类型和id
       *
       * @param type_id 一个存储类型和id的元组
       * @return txString 拼接后的字符串
       */
      static txString combine_type_id(const std::tuple<ElementType, txSysId>& type_id) TX_NOEXCEPT;

     protected:
      txBool mIsValid = false;
      txSize m_current_trigger_count = 0;
      boost::optional<std::list<txFloat> > m_op_last_value_list;
    };
    using IConditionViewerPtr = std::shared_ptr<IConditionViewer>;
    using IConditionViewerPtrVec = std::vector<IConditionViewerPtr>;

    // @brief velocity事件触发条件视图接口
    struct IVelocityTriggerConditionViewer : public IConditionViewer {
      /**
       * @brief 场景事件类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::velocity_trigger);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取选择的速度类型，绝对，相对
       *
       * @return ConditionSpeedType
       */
      virtual ConditionSpeedType speed_type() const TX_NOEXCEPT { return mSpeedType; }

      /**
       * @brief 获取比较操作类型 = or > < ...
       *
       * @return ConditionEquationOp
       */
      virtual ConditionEquationOp op() const TX_NOEXCEPT { return mEquOp; }

      /**
       * @brief 获取距离类型
       *
       * @return ConditionDistanceType
       */
      virtual ConditionDistanceType distanceType() const TX_NOEXCEPT { return mDistType; }

      /**
       * @brief 获取边界条件类型
       *
       * @return ConditionBoundaryType
       */
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }

      /**
       * @brief 获取设置的速度阈值
       *
       * @return txFloat
       */
      virtual txFloat speed_threshold() const TX_NOEXCEPT { return m_speed_threshold; }

      /**
       * @brief 获取需要触发的次数
       *
       * @return txSize
       */
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }

      /**
       * @brief 获取目标元素类型
       *
       * @return ElementType
       */
      virtual ElementType target_type() const TX_NOEXCEPT { return m_target_element_type; }

      /**
       * @brief 获取目标元素sysid
       *
       * @return txSysId
       */
      virtual txSysId target_id() const TX_NOEXCEPT { return m_target_element_id; }

      /**
       * @brief 获取当前元素类型
       *
       * @return ElementType
       */
      virtual ElementType source_type() const TX_NOEXCEPT { return m_source_element_type; }

      /**
       * @brief 获取当前元素id
       *
       * @return txSysId
       */
      virtual txSysId source_id() const TX_NOEXCEPT { return m_source_element_id; }

     protected:
      ConditionSpeedType mSpeedType = _plus_(ConditionSpeedType::absolute);
      ConditionEquationOp mEquOp = _plus_(ConditionEquationOp::eq);
      ConditionDistanceType mDistType = _plus_(ConditionDistanceType::euclideandistance);
      ConditionBoundaryType mBoundaryType = _plus_(ConditionBoundaryType::none);
      txFloat m_speed_threshold = 0.0;
      txSize m_trigger_count = 1;
      ElementType m_target_element_type = _plus_(ElementType::Unknown);
      txSysId m_target_element_id = 0;
      ElementType m_source_element_type = _plus_(ElementType::Unknown);
      txSysId m_source_element_id = 0;
    };
    using IVelocityTriggerConditionViewerPtr = std::shared_ptr<IVelocityTriggerConditionViewer>;

    // @brief 绝对位置事件触发条件视图接口
    struct IReachAbsPositionConditionViewer : public IConditionViewer {
      using ConditionPositionType = Base::Enums::ConditionPositionType;

      /**
       * @brief 获取场景事件类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::reach_abs_position);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取条件 位置类型
       *
       * @return ConditionPositionType
       */
      virtual ConditionPositionType position_type() const TX_NOEXCEPT { return m_position_type; }

      /**
       * @brief 获取条件 具体位置
       *
       * @return hadmap::txPoint
       */
      virtual hadmap::txPoint point_gps() const TX_NOEXCEPT { return m_position; }

      /**
       * @brief 获取条件 位置的范围大小
       *
       * @return txFloat
       */
      virtual txFloat radius() const TX_NOEXCEPT { return m_radius; }

      /**
       * @brief 获取条件 目标类型
       *
       * @return ElementType
       */
      virtual ElementType target_type() const TX_NOEXCEPT { return m_target_element_type; }

      /**
       * @brief 获取目标id
       *
       * @return txSysId
       */
      virtual txSysId target_id() const TX_NOEXCEPT { return m_target_element_id; }

      /**
       * @brief 获取边界条件
       *
       * @return ConditionBoundaryType
       */
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }

      /**
       * @brief 获取需要触发的次数
       *
       * @return txSize
       */
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }

     protected:
      ConditionPositionType m_position_type;
      hadmap::txPoint m_position;
      txFloat m_radius = 0.0;
      ElementType m_target_element_type = _plus_(ElementType::Unknown);
      txSysId m_target_element_id = 0;
      ConditionBoundaryType mBoundaryType = _plus_(ConditionBoundaryType::none);
      txSize m_trigger_count = 1;
    };
    using IReachAbsPositionConditionViewerPtr = std::shared_ptr<IReachAbsPositionConditionViewer>;

    // @brief 绝对位置到达车道条件视图接口
    struct IReachAbsLaneConditionViewer : public IConditionViewer {
      using ConditionLaneType = Base::Enums::ConditionLaneType;

      /**
       * @brief 获取场景事件类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::reach_abs_lane);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取条件 位置类型
       *
       * @return ConditionLaneType
       */
      virtual ConditionLaneType position_type() const TX_NOEXCEPT { return m_position_type; }

      /**
       * @brief 获取条件 道路id
       *
       * @return Base::txRoadID
       */
      virtual Base::txRoadID road_id() const TX_NOEXCEPT { return m_road_id; }

      /**
       * @brief 获取条件 laneid
       *
       * @return Base::txLaneID
       */
      virtual Base::txLaneID lane_id() const TX_NOEXCEPT { return m_lane_id; }

      /**
       * @brief 获取目标类型
       *
       * @return ElementType
       */
      virtual ElementType target_type() const TX_NOEXCEPT { return m_target_element_type; }

      /**
       * @brief 获取条件 目标id
       *
       * @return txSysId
       */
      virtual txSysId target_id() const TX_NOEXCEPT { return m_target_element_id; }

      /**
       * @brief 获取条件 横向偏移
       *
       * @return txFloat
       */
      virtual txFloat lateral_offset() const TX_NOEXCEPT { return m_lateral_offset; }

      /**
       * @brief 获取条件 纵向偏移
       *
       * @return txFloat
       */
      virtual txFloat longitudinal_offset() const TX_NOEXCEPT { return m_longitudinal_offset; }

      /**
       * @brief 获取条件 容差
       *
       * @return txFloat
       */
      virtual txFloat tolerance() const TX_NOEXCEPT { return m_tolerance; }

      /**
       * @brief 获取条件 边界类型
       *
       * @return ConditionBoundaryType
       */
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }

      /**
       * @brief 获取条件 需要触发的次数
       *
       * @return txSize
       */
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }

     protected:
      ConditionLaneType m_position_type;
      Base::txRoadID m_road_id = 0;
      Base::txLaneID m_lane_id = 0;
      ElementType m_target_element_type = _plus_(ElementType::Unknown);
      txSysId m_target_element_id = 0;
      txFloat m_lateral_offset = 0.0;
      txFloat m_longitudinal_offset = 0.0;
      txFloat m_tolerance = 0.0;
      ConditionBoundaryType mBoundaryType = _plus_(ConditionBoundaryType::none);
      txSize m_trigger_count = 1;
    };
    using IReachAbsLaneConditionViewerPtr = std::shared_ptr<IReachAbsLaneConditionViewer>;

    // @brief 时间触发条件视图接口
    struct ITimeTriggerViewer : public IConditionViewer {
      /**
       * @brief 获取场景事件的类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::time_trigger);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取条件 需要触发的次数
       *
       * @return txFloat
       */
      virtual txFloat trigger_time() const TX_NOEXCEPT { return m_trigger_time; }

      /**
       * @brief 获取比较操作类型 = or > < ...
       *
       * @return ConditionEquationOp
       */
      virtual ConditionEquationOp op() const TX_NOEXCEPT { return mEquOp; }

     protected:
      txFloat m_trigger_time = -1.0;
      ConditionEquationOp mEquOp = _plus_(ConditionEquationOp::eq);
    };
    using ITimeTriggerViewerPtr = std::shared_ptr<ITimeTriggerViewer>;

    // @brief TTC触发条件视图接口
    struct ITTCTriggerViewer : public IConditionViewer {
      /**
       * @brief 获取场景事件类型
       *
       * @return * SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::ttc_trigger);
      }

      /**
       * @brief ttc触发条件的格式化返回
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 初始化当前结构
       *
       * @param ref_kv_map 各个字段的KV集合
       * @return txBool 初始化成功与否
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取设置的ttc阈值
       *
       * @return txFloat
       */
      virtual txFloat ttc_threshold() const TX_NOEXCEPT { return m_ttc_threshold; }

      /**
       * @brief 获取设置的比较类型，如>= == <= < >
       *
       * @return ConditionEquationOp
       */
      virtual ConditionEquationOp op() const TX_NOEXCEPT { return mEquOp; }

      /**
       * @brief 获取目标元素的类型
       *
       * @return ElementType
       */
      virtual ElementType target_type() const TX_NOEXCEPT { return m_target_element_type; }

      /**
       * @brief 获取目标元素的sysid
       *
       * @return txSysId
       */
      virtual txSysId target_id() const TX_NOEXCEPT { return m_target_element_id; }

      /**
       * @brief 获取距离类型 欧式 道路坐标系
       *
       * @return ConditionDistanceType
       */
      virtual ConditionDistanceType distanceType() const TX_NOEXCEPT { return mDistType; }

      /**
       * @brief 条件边界类型 上升 下降..
       *
       * @return ConditionBoundaryType
       */
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }

      /**
       * @brief 获取触发的次数
       *
       * @return txSize
       */
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }

     protected:
      txFloat m_ttc_threshold = 0.0;
      ConditionEquationOp mEquOp = _plus_(ConditionEquationOp::eq);
      ElementType m_target_element_type = _plus_(ElementType::Unknown);
      txSysId m_target_element_id = 0;
      ConditionDistanceType mDistType;
      ConditionBoundaryType mBoundaryType;
      txSize m_trigger_count = 1;
    };
    using ITTCTriggerViewerPtr = std::shared_ptr<ITTCTriggerViewer>;

    // @brief 距离事件触发条件视图接口
    struct IDistanceTriggerViewer : public IConditionViewer {
      /**
       * @brief 获取场景事件的类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::distance_trigger);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取条件 距离阈值
       *
       * @return txFloat
       */
      virtual txFloat distance_threshold() const TX_NOEXCEPT { return m_distance_threshold; }

      /**
       * @brief 获取比较操作类型 = > < ...
       *
       * @return ConditionEquationOp
       */
      virtual ConditionEquationOp op() const TX_NOEXCEPT { return mEquOp; }

      /**
       * @brief 获取条件 距离类型 欧式..
       *
       * @return ConditionDistanceType
       */
      virtual ConditionDistanceType distanceType() const TX_NOEXCEPT { return mDistType; }

      /**
       * @brief 获取目标类型
       *
       * @return ElementType
       */
      virtual ElementType target_type() const TX_NOEXCEPT { return m_target_element_type; }

      /**
       * @brief 获取目标sysid
       *
       * @return txSysId
       */
      virtual txSysId target_id() const TX_NOEXCEPT { return m_target_element_id; }

      /**
       * @brief 边界类型 获取
       *
       * @return * ConditionBoundaryType
       */
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }

      /**
       * @brief 需要触发的次数
       *
       * @return txSize
       */
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }

      /**
       * @brief source元素类型获取
       *
       * @return ElementType
       */
      virtual ElementType source_type() const TX_NOEXCEPT { return m_source_element_type; }

      /**
       * @brief 获取source元素id
       *
       * @return txSysId
       */
      virtual txSysId source_id() const TX_NOEXCEPT { return m_source_element_id; }

     protected:
      txFloat m_distance_threshold = 0.0;
      ConditionEquationOp mEquOp = _plus_(ConditionEquationOp::eq);
      ConditionDistanceType mDistType;
      ElementType m_target_element_type = _plus_(ElementType::Unknown);
      txSysId m_target_element_id = 0;
      ConditionBoundaryType mBoundaryType;
      txSize m_trigger_count = 1;
      ElementType m_source_element_type = _plus_(ElementType::Unknown);
      txSysId m_source_element_id = 0;
    };
    using IDistanceTriggerViewerPtr = std::shared_ptr<IDistanceTriggerViewer>;

    // @brief ego到达车道自定义条件视图接口
    struct IEgoAttachLaneUidCustomViewer : public IConditionViewer {
      /**
       * @brief 获取场景事件的类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::ego_attach_laneid_custom);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取车道id
       *
       * @return Base::txLaneUId
       */
      virtual Base::txLaneUId laneUid() const TX_NOEXCEPT { return mLaneUid; }

     protected:
      Base::txLaneUId mLaneUid;
    };
    using IEgoAttachLaneUidCustomViewerPtr = std::shared_ptr<IEgoAttachLaneUidCustomViewer>;

    // @brief 跟车时距触发条件视图接口
    struct ITimeHeadwayTriggerViewer : public IConditionViewer {
      /**
       * @brief 获取场景事件的类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::timeheadway_trigger);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 车头时距比较操作 > < = ..
       *
       * @return ConditionEquationOp
       */
      virtual ConditionEquationOp op() const TX_NOEXCEPT { return mEquOp; }

      /**
       * @brief 获取条件 边界类型
       *
       * @return ConditionBoundaryType
       */
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }

      /**
       * @brief 获取需要触发次数
       *
       * @return txSize
       */
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }

      /**
       * @brief 获取目标类型
       *
       * @return ElementType
       */
      virtual ElementType target_type() const TX_NOEXCEPT { return m_target_element_type; }

      /**
       * @brief 获取目标元素id
       *
       * @return txSysId
       */
      virtual txSysId target_id() const TX_NOEXCEPT { return m_target_element_id; }

      /**
       * @brief 获取source元素类型
       *
       * @return ElementType
       */
      virtual ElementType source_type() const TX_NOEXCEPT { return m_source_element_type; }

      /**
       * @brief 获取source元素id
       *
       * @return txSysId
       */
      virtual txSysId source_id() const TX_NOEXCEPT { return m_source_element_id; }

      /**
       * @brief 获取车头时距所设置阈值
       *
       * @return txFloat
       */
      virtual txFloat time_head_way_threshold() const TX_NOEXCEPT { return m_time_head_way_threshold; }

      /**
       * @brief 获取 freespace
       *
       * @return txBool
       */
      virtual txBool freespace() const TX_NOEXCEPT { return m_freespace; }

      /**
       * @brief 获取alongroute
       *
       * @return txBool
       */
      virtual txBool alongroute() const TX_NOEXCEPT { return m_alongroute; }

     protected:
      ConditionEquationOp mEquOp = _plus_(ConditionEquationOp::eq);
      ConditionBoundaryType mBoundaryType = _plus_(ConditionBoundaryType::none);
      txSize m_trigger_count = 1;
      ElementType m_target_element_type = _plus_(ElementType::Unknown);
      txSysId m_target_element_id = 0;
      ElementType m_source_element_type = _plus_(ElementType::Unknown);
      txSysId m_source_element_id = 0;

      txFloat m_time_head_way_threshold = 0.0;
      txBool m_freespace = false;
      txBool m_alongroute = false;
    };
    using ITimeHeadwayTriggerViewerPtr = std::shared_ptr<ITimeHeadwayTriggerViewer>;

    // @brief 元素状态触发事件条件视图接口
    struct IElementStateTriggerViewer : public IConditionViewer {
      /**
       * @brief 获取场景事件的类型
       *
       * @return SceneEventType
       */
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::element_state);
      }

      /**
       * @brief 当前条件的描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 数据结构初始化
       *
       * @param ref_kv_map 存储的kv值
       * @return txBool
       */
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取事件id+行为id
       *
       * @return txString
       */
      virtual txString eventId_actionId() const TX_NOEXCEPT { return m_dst_event_id_action_id; }

      /**
       * @brief 获取event id
       *
       * @return txSysId
       */
      virtual txSysId dst_event_id() const TX_NOEXCEPT { return m_dst_event_id; }

      /**
       * @brief 获取action id
       *
       * @return txInt
       */
      virtual txInt dst_event_action_id() const TX_NOEXCEPT { return m_dst_event_action_id; }

      /**
       * @brief 获取场景事件状态
       *
       * @return SceneEventElementStatusType
       */
      virtual SceneEventElementStatusType state() const TX_NOEXCEPT { return mState; }

     protected:
      txSysId m_dst_event_id = -1;
      txInt m_dst_event_action_id = -1;
      txString m_dst_event_id_action_id;
      SceneEventElementStatusType mState = _plus_(SceneEventElementStatusType::none);
    };

    using IElementStateTriggerViewerPtr = std::shared_ptr<IElementStateTriggerViewer>;
#endif /*__TX_Mark__("Condition")*/

#if __TX_Mark__("EndCondition")
    // @brief 结束条件视图接口
    struct IEndConditionViewer {
      using EndConditionKeyType = Base::Enums::EndConditionKeyType;
      using EndConditionType = std::tuple<EndConditionKeyType, Base::txFloat>;
      using EndConditionVec = std::vector<EndConditionType>;
      virtual ~IEndConditionViewer() TX_DEFAULT;

      /**
       * @brief 获取所有的endCondition
       *
       * @return EndConditionVec
       */
      virtual EndConditionVec endConditionList() const TX_NOEXCEPT { return mEndConditionVec; }

      /**
       * @brief 获取结束condition的个数
       *
       * @return Base::txSize
       */
      virtual Base::txSize endConditionCnt() const TX_NOEXCEPT { return mEndConditionVec.size(); }

      /**
       * @brief 获取结束condition类型
       *
       * @param idx 索引
       * @return EndConditionType
       */
      virtual EndConditionType getEndCondition(const Base::txSize idx) const TX_NOEXCEPT {
        return mEndConditionVec[idx];
      }

      /**
       * @brief 对象结构化描述
       *
       * @return txString
       */
      virtual txString Str() const TX_NOEXCEPT;

      /**
       * @brief 对象初始化
       *
       * @param ref_kv_map_vec
       * @return txBool
       */
      virtual txBool initialize(const kvMapVec& ref_kv_map_vec) TX_NOEXCEPT;

      /**
       * @brief 当前对象是否有效
       *
       * @return txBool
       */
      virtual txBool IsValid() const TX_NOEXCEPT { return mIsValid; }

     protected:
      EndConditionVec mEndConditionVec;
      txBool mIsValid = false;
    };
    using IEndConditionViewerPtr = std::shared_ptr<IEndConditionViewer>;
#endif /*__TX_Mark__("EndCondition")*/

#if __TX_Mark__("Action")
    // @brief 动作行为视图接口
    struct IActionViewer {
      using ElementType = Base::Enums::ElementType;
      using SceneEventActionType = Base::Enums::SceneEventActionType;
      using VehicleMoveLaneState = Base::Enums::VehicleMoveLaneState;
      using DrivingStatus = Base::Enums::DrivingStatus;
      struct EventActionInfo {
        /**
         * @brief 获取行为类型
         *
         * @return * SceneEventActionType
         */
        SceneEventActionType action_type() const TX_NOEXCEPT { return m_act_type; }

        /**
         * @brief 获取合并类型
         *
         * @return VehicleMoveLaneState
         */
        VehicleMoveLaneState merge_type() const TX_NOEXCEPT { return m_merge_type; }

        /**
         * @brief 获取值
         *
         * @return Base::txFloat
         */
        Base::txFloat value() const TX_NOEXCEPT { return m_value; }

        /**
         * @brief 获取lane offset
         *
         * @return Base::txFloat
         */
        Base::txFloat lane_offset() const TX_NOEXCEPT { return m_lane_offset; }

        /**
         * @brief 获取驾驶状态
         *
         * @return DrivingStatus
         */
        DrivingStatus driving_status() const TX_NOEXCEPT { return m_driving_status; }

        /**
         * @brief 获取行为id
         *
         * @return Base::txInt
         */
        Base::txInt action_id() const TX_NOEXCEPT { return m_action_id; }

        SceneEventActionType m_act_type;
        VehicleMoveLaneState m_merge_type;
        Base::txFloat m_value;
        Base::txFloat m_lane_offset;
        DrivingStatus m_driving_status;
        Base::txInt m_action_id;
#  if __TX_Mark__("lateralDistance")
        txFloat m_distance = 0.0;
        txFloat m_maxAcc = 3.5;
        txFloat m_maxDec = 3.5;
        txFloat m_maxSpeed = 20;
        txBool m_freespace = false;
        txBool m_continuous = false;
        ElementType m_target_element_type = _plus_(ElementType::Unknown);
        txSysId m_target_element_id = 0;
        txString m_target_element;
        ITrafficElementPtr m_target_element_ptr = nullptr;

        /**
         * @brief 获取距离
         *
         * @return txFloat
         */
        txFloat distance() const TX_NOEXCEPT { return m_distance; }

        /**
         * @brief 获取最大加速度
         *
         * @return txFloat
         */
        txFloat maxAcc() const TX_NOEXCEPT { return m_maxAcc; }

        /**
         * @brief 获取最大减速度
         *
         * @return txFloat
         */
        txFloat maxDec() const TX_NOEXCEPT { return m_maxDec; }

        /**
         * @brief 获取最大速度
         *
         * @return txFloat
         */
        txFloat maxSpeed() const TX_NOEXCEPT { return m_maxSpeed; }

        /**
         * @brief 获取是否是空闲空间
         *
         * @return txBool
         */
        txBool freespace() const TX_NOEXCEPT { return m_freespace; }

        /**
         * @brief 获取是否是连续
         *
         * @return txBool
         */
        txBool continuous() const TX_NOEXCEPT { return m_continuous; }

        /**
         * @brief 获取目标元素
         *
         * @return ITrafficElementPtr 返回的目标元素指针
         */
        ITrafficElementPtr target_element_ptr() const TX_NOEXCEPT { return m_target_element_ptr; }

        /**
         * @brief Set the target element ptr object
         *
         * @param ptr 设置的元素指针
         */
        void set_target_element_ptr(ITrafficElementPtr ptr) TX_NOEXCEPT { m_target_element_ptr = ptr; }

        /**
         * @brief 获取目标元素类型
         *
         * @return ElementType
         */
        ElementType target_element_type() const TX_NOEXCEPT { return m_target_element_type; }

        /**
         * @brief 获取目标元素id
         *
         * @return txSysId
         */
        txSysId target_element_id() const TX_NOEXCEPT { return m_target_element_id; }

        /**
         * @brief 获取目标元素名称
         *
         * @return txString
         */
        txString target_element() const TX_NOEXCEPT { return m_target_element; }
#  endif /*__TX_Mark__("lateralDistance")*/
      };
      static EventActionInfo make_EventActionInfo(const Base::txInt actId, const SceneEventActionType _type,
                                                  const VehicleMoveLaneState _merge_type, const Base::txFloat value,
                                                  const Base::txFloat lane_offset,
                                                  const DrivingStatus _state) TX_NOEXCEPT {
        EventActionInfo res;
        res.m_action_id = actId;
        res.m_act_type = _type;
        res.m_merge_type = _merge_type;
        res.m_value = value;
        res.m_lane_offset = lane_offset;
        res.m_driving_status = _state;
        return res;
      }
#  if __TX_Mark__("lateralDistance")
      static EventActionInfo make_EventActionInfo_lateralDistance(
          const Base::txInt actId, const SceneEventActionType _type, txFloat _distance, txFloat _maxAcc,
          txFloat _maxDec, txFloat _maxSpeed, txBool _freespace, txBool _continuous, ElementType _target_element_type,
          txSysId _target_element_id, txString _target_element) TX_NOEXCEPT {
        EventActionInfo res;
        res.m_action_id = actId;
        res.m_act_type = _type;
        res.m_distance = _distance;
        res.m_maxAcc = _maxAcc;
        res.m_maxDec = _maxDec;
        res.m_maxSpeed = _maxSpeed;
        res.m_freespace = _freespace;
        res.m_continuous = _continuous;
        res.m_target_element_type = _target_element_type;
        res.m_target_element_id = _target_element_id;
        res.m_target_element = _target_element;
        return res;
      }
#  endif /*__TX_Mark__("lateralDistance")*/
      // using EventActionInfo = std::tuple<SceneEventActionType, VehicleMoveLaneState, Base::txFloat/*lanechange
      // time*/, Base::txFloat/*lane offset*/, DrivingStatus>;
      using EventActionInfoVec = std::vector<EventActionInfo>;
      virtual ~IActionViewer() TX_DEFAULT;

      /**
       * @brief 获取动作数量
       *
       * @return Base::txSize
       */
      virtual Base::txSize actionCnt() const TX_NOEXCEPT { return mEventActionInfoVec.size(); }

      /**
       * @brief 根据索引获取action
       *
       * @param idx 指定索引
       * @return EventActionInfo
       */
      virtual EventActionInfo getAction(const Base::txSize idx) const TX_NOEXCEPT { return mEventActionInfoVec[idx]; }

      /**
       * @brief 获取所有的action集合
       *
       * @return EventActionInfoVec
       */
      virtual EventActionInfoVec actionList() const TX_NOEXCEPT { return mEventActionInfoVec; }

      /**
       * @brief 获取所有的action集合引用
       *
       * @return EventActionInfoVec&
       */
      virtual EventActionInfoVec& actionList() TX_NOEXCEPT { return mEventActionInfoVec; }
      virtual txString Str() const TX_NOEXCEPT;

      /**
       * @brief 初始化函数
       *
       * @param ref_kv_map_vec kv存储用于初始化的属性值
       * @return txBool
       */
      virtual txBool initialize(const kvMapVec& ref_kv_map_vec) TX_NOEXCEPT;

      /**
       * @brief 当前对象是否有效
       *
       * @return txBool
       */
      virtual txBool IsValid() const TX_NOEXCEPT { return mIsValid; }
      static txString actionType2Unit(const SceneEventActionType& _type) TX_NOEXCEPT;

     protected:
      EventActionInfoVec mEventActionInfoVec;
      txBool mIsValid = false;
    };
    using IActionViewerPtr = std::shared_ptr<IActionViewer>;

#endif /*__TX_Mark__("Action")*/

    ISceneEventViewer() TX_DEFAULT;
    virtual ~ISceneEventViewer() TX_DEFAULT;

    /**
     * 获取条件类型
     * @return 返回一个IConditionViewer::SceneEventType枚举值，表示条件类型
     */
    virtual IConditionViewer::SceneEventType conditionType() const TX_NOEXCEPT;

    /**
     * 获取结束条件
     * @return 返回一个IEndConditionViewerPtr类型的智能指针，表示结束条件
     */
    virtual IEndConditionViewerPtr endCondition() const TX_NOEXCEPT { return mEndConditionPtr; }

    /**
     * 获取动作
     * @return 返回一个IActionViewerPtr类型的智能指针，表示动作
     */
    virtual IActionViewerPtr action() const TX_NOEXCEPT { return mActionPtr; }
    virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;
#if __SecenEventVersion_1_2_0_0__

    /**
     * 初始化事件
     * @param _evId 事件ID
     * @param _ev_param 事件参数
     * @return 返回一个txBool类型的值，表示事件是否初始化成功
     */
    virtual txBool initialize(const txSysId _evId, const EventParam_t& _ev_param) TX_NOEXCEPT;

    /**
     * 初始化事件
     * @param _evId 事件ID
     * @param _ev_group_param 事件组参数
     * @return 返回一个txBool类型的值，表示事件是否初始化成功
     */
    virtual txBool initialize(const txSysId _evId, const EventGroupParam_t& _ev_group_param) TX_NOEXCEPT;

    /**
     * 获取条件集合
     * @return 返回一个IConditionViewerPtrVec类型的集合，表示条件集合
     */
    virtual IConditionViewerPtrVec conditionVec() const TX_NOEXCEPT { return mCondPtrVec; }

    /**
     * 初始化事件
     * @param _evId 事件ID
     * @param _condition 条件参数
     * @param _endcondition 结束条件参数
     * @param _action 动作参数
     * @return 返回一个txBool类型的值，表示事件是否初始化成功
     */
    virtual txBool initialize(const txSysId _evId, const kvMap& _condition, const kvMapVec& _endcondition,
                              const kvMapVec& _action) TX_NOEXCEPT;

    /**
     * 初始化条件
     * @param _condition 条件参数
     * @return 返回一个txBool类型的值，表示条件是否初始化成功
     */
    virtual txBool initialize_condition(const kvMap& _condition) TX_NOEXCEPT;

    /**
     * 初始化结束条件
     * @param _endcondition 结束条件参数
     * @return 返回一个txBool类型的值，表示结束条件是否初始化成功
     */
    virtual txBool initialize_end_condition(const kvMapVec& _endcondition) TX_NOEXCEPT;

    /**
     * 初始化动作
     * @param _action 动作参数
     * @return 返回一个txBool类型的值，表示动作是否初始化成功
     */
    virtual txBool initialize_action(const kvMapVec& _action) TX_NOEXCEPT;

    /**
     * 获取事件版本
     * @return 返回一个eSceneEventVersion类型的值，表示事件版本
     */
    virtual eSceneEventVersion version() const TX_NOEXCEPT { return mEventVersion; }

    /**
     * 设置事件版本
     * @param _ver 事件版本参数
     */
    virtual void set_version(const eSceneEventVersion _ver) TX_NOEXCEPT { mEventVersion = _ver; }
#endif /*__SecenEventVersion_1_2_0_0__*/

    /**
     * 获取事件ID
     * @return 返回一个txSysId类型的值，表示事件ID
     */
    virtual txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return m_event_id; }

    /**
     * 判断是否已初始化
     * @return 返回一个txBool类型的值，表示是否已初始化
     */
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE {
      return _NonEmpty_(mCondPtrVec) && NonNull_Pointer(mCondPtrVec.front()) && NonNull_Pointer(mEndConditionPtr) &&
             NonNull_Pointer(mActionPtr);
    }

    /**
     * 清除事件数据
     */
    virtual void clear() TX_NOEXCEPT {
      m_event_id = -1;
      mCondPtrVec.clear();
      mEndConditionPtr = nullptr;
      mActionPtr = nullptr;
    }

    /**
     * 生成键值对映射
     * @param _str 输入的字符串
     * @return 返回一个kvMap类型的值，表示键值对映射
     */
    static kvMap generateKVMap(txString _str /*, -> :*/) TX_NOEXCEPT;

    /**
     * 生成键值对映射集合
     * @param _str 输入的字符串，格式为 "key1:value1,key2:value2;key3:value3,key4:value4"
     * @return 返回一个kvMapVec类型的值，表示键值对映射集合
     */
    static kvMapVec generateKVMapVec(txString _str /* ; -> , -> : */) TX_NOEXCEPT;

    /**
     * 生成键值对映射集合（JSON格式）
     * @param _str 输入的字符串，格式为 "[{type:velocity,value:2},{}]"
     * @return 返回一个kvMapVec类型的值，表示键值对映射集合
     */
    static kvMapVec generateKVMapVec_Json(txString _str /* [{type:velocity,value:2},{}] */) TX_NOEXCEPT;

   protected:
#if __SecenEventVersion_1_2_0_0__
    IConditionViewerPtrVec mCondPtrVec;
    eSceneEventVersion mEventVersion;
#endif /*__SecenEventVersion_1_2_0_0__*/
    IEndConditionViewerPtr mEndConditionPtr = nullptr;
    IActionViewerPtr mActionPtr = nullptr;
    txSysId m_event_id = -1;
  };

  using ISceneEventViewerPtr = std::shared_ptr<ISceneEventViewer>;

  // @brief 交通车视图接口
  struct IVehiclesViewer : public IViewer {
    IVehiclesViewer() TX_DEFAULT;
    virtual ~IVehiclesViewer() TX_DEFAULT;
    virtual txSysId routeID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取获取车道id
     *
     * @return txLaneID
     */
    virtual txLaneID laneID() const TX_NOEXCEPT = 0;

    /**
     * @brief 开始时刻的距离
     *
     * @return txFloat
     */
    virtual txFloat start_s() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时间
     *
     * @return txFloat
     */
    virtual txFloat start_t() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始速度
     *
     * @return txFloat
     */
    virtual txFloat start_v() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取最大速度
     *
     * @return txFloat
     */
    virtual txFloat max_v() const TX_NOEXCEPT = 0;
    virtual txFloat l_offset() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车长
     *
     * @return txFloat
     */
    virtual txFloat length() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车宽
     *
     * @return txFloat
     */
    virtual txFloat width() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车高
     *
     * @return txFloat
     */
    virtual txFloat height() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取accid
     *
     * @return txSysId
     */
    virtual txSysId accID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取mergeid
     *
     * @return txSysId 返回的id
     */
    virtual txSysId mergeID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取行为
     *
     * @return txString 返回行为
     */
    virtual txString behavior() const TX_NOEXCEPT = 0;
    virtual txFloat aggress() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车辆类型
     *
     * @return txString
     */
    virtual txString vehicleType() const TX_NOEXCEPT = 0;
    virtual VEHICLE_BEHAVIOR behaviorEnum() const TX_NOEXCEPT;

    /**
     * @brief 获取follow的元素id
     *
     * @return txSysId
     */
    virtual txSysId follow() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取合并的时间
     *
     * @return txFloat
     */
    virtual txFloat mergeTime() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取偏移时间
     *
     * @return txFloat
     */
    virtual txFloat offsetTime() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取事件id集合
     *
     * @return std::vector< txSysId > 返回事件id集合
     */
    virtual std::vector<txSysId> eventId() const TX_NOEXCEPT { return std::vector<txSysId>(); }

    /**
     * @brief 获取角度
     *
     * @return txFloat
     */
    virtual txFloat angle() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取catalog的字符串
     *
     * @return txString
     */
    virtual txString catalog() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时角度
     *
     * @return txFloat
     */
    virtual txFloat start_angle() const TX_NOEXCEPT { return -1.0; }
    virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE final {
      LOG(WARNING) << "Call VehiclesViewer::Str";
      std::ostringstream oss_eventId;
      const auto eventIdVec = eventId();
      std::copy(eventIdVec.begin(), eventIdVec.end(), std::ostream_iterator<txSysId>(oss_eventId, ","));
      std::ostringstream oss;
      oss << _StreamPrecision_ << TX_VARS(id()) << TX_VARS(routeID()) << TX_VARS(laneID()) << TX_VARS(start_s())
          << TX_VARS(start_t()) << TX_VARS(start_v()) << TX_VARS(max_v()) << TX_VARS(l_offset()) << TX_VARS(length())
          << TX_VARS(width()) << TX_VARS(height()) << TX_VARS(accID()) << TX_VARS(mergeID()) << TX_VARS(behavior())
          << TX_VARS(vehicleType()) << TX_VARS(follow()) << TX_VARS(mergeTime()) << TX_VARS(offsetTime())
          << TX_VARS_NAME(eventId, oss_eventId.str()) << TX_VARS(angle()) << TX_VARS(catalog())
          << TX_VARS(start_angle());
      return oss.str();
    }
  };
  using IVehiclesViewerPtr = std::shared_ptr<IVehiclesViewer>;

  // @brief 行人视图接口
  struct IPedestriansViewer : public IViewer {
    IPedestriansViewer() TX_DEFAULT;
    virtual ~IPedestriansViewer() TX_DEFAULT;

    /**
     * @brief 获取routeid
     *
     * @return txSysId
     */
    virtual txSysId routeID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车道id
     *
     * @return txLaneID
     */
    virtual txLaneID laneID() const TX_NOEXCEPT = 0;

    /**
     * @brief 开始的距离
     *
     * @return txFloat
     */
    virtual txFloat start_s() const TX_NOEXCEPT = 0;

    /**
     * @brief 开始时的时间
     *
     * @return txFloat
     */
    virtual txFloat start_t() const TX_NOEXCEPT = 0;

    /**
     * @brief 结束时的时间
     *
     * @return txFloat
     */
    virtual txFloat end_t() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取偏移值
     *
     * @return txFloat
     */
    virtual txFloat l_offset() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取类型值
     *
     * @return txString
     */
    virtual txString type() const TX_NOEXCEPT = 0;

    /**
     * @brief 是否有方向
     *
     * @return txBool
     */
    virtual txBool hadDirection() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时的速度
     *
     * @return txFloat
     */
    virtual txFloat start_v() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取最大的速度
     *
     * @return txFloat
     */
    virtual txFloat max_v() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取行为
     *
     * @return txString
     */
    virtual txString behavior() const TX_NOEXCEPT = 0;
    virtual std::vector<txSysId> eventId() const TX_NOEXCEPT { return std::vector<txSysId>(); }
    virtual txFloat angle() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取catalog的字符串
     *
     * @return txString
     */
    virtual txString catalog() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时的角度
     *
     * @return txFloat
     */
    virtual txFloat start_angle() const TX_NOEXCEPT { return -1.0; }
    virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE final {
      LOG(WARNING) << "Call PedestriansViewer::Str";
      std::ostringstream oss_eventId;
      const auto eventIdVec = eventId();
      std::copy(eventIdVec.begin(), eventIdVec.end(), std::ostream_iterator<txSysId>(oss_eventId, ","));
      std::ostringstream oss;
      oss << _StreamPrecision_ << TX_VARS(id()) << TX_VARS(routeID()) << TX_VARS(laneID()) << TX_VARS(start_s())
          << TX_VARS(start_t()) << TX_VARS(end_t()) << TX_VARS(start_v()) << TX_VARS(max_v()) << TX_VARS(l_offset())
          << TX_VARS(type()) << TX_COND_NAME(hadDirection, hadDirection()) << TX_VARS(behavior())
          << TX_VARS_NAME(eventId, oss_eventId.str()) << TX_VARS(angle()) << TX_VARS(catalog())
          << TX_VARS(start_angle());
      return oss.str();
    }
    virtual IVehiclesViewerPtr vehicle_view_ptr() const TX_NOEXCEPT { return nullptr; }
  };
  using IPedestriansViewerPtr = std::shared_ptr<IPedestriansViewer>;

  // @brief 静态障碍物视图接口
  struct IObstacleViewer : public IViewer {
    IObstacleViewer() TX_DEFAULT;
    virtual ~IObstacleViewer() TX_DEFAULT;
    /**
     * @brief 获取路线id
     *
     * @return txSysId
     */
    virtual txSysId routeID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车道id
     *
     * @return txLaneID
     */
    virtual txLaneID laneID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时的时间
     *
     * @return txFloat
     */
    virtual txFloat start_s() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取横向偏移距离
     *
     * @return txFloat
     */
    virtual txFloat l_offset() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取长度
     *
     * @return txFloat
     */
    virtual txFloat length() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取宽度属性
     *
     * @return txFloat
     */
    virtual txFloat width() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取高度属性
     *
     * @return txFloat
     */
    virtual txFloat height() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取vehicle的type
     *
     * @return txString
     */
    virtual txString vehicleType() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取元素类型
     *
     * @return STATIC_ELEMENT_TYPE
     */
    virtual txString type() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取方向
     *
     * @return txFloat
     */
    virtual txFloat direction() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取行为
     *
     * @return txString
     */
    virtual txString behavior() const TX_NOEXCEPT { return ""; }

    /**
     * @brief 获取事件id集合
     *
     * @return std::vector< txSysId >
     */
    virtual std::vector<txSysId> eventId() const TX_NOEXCEPT { return std::vector<txSysId>(); }
    virtual txFloat angle() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取catalog字符串
     *
     * @return txString
     */
    virtual txString catalog() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时的角度
     *
     * @return txFloat
     */
    virtual txFloat start_angle() const TX_NOEXCEPT { return -1.0; }
  };
  using IObstacleViewerPtr = std::shared_ptr<IObstacleViewer>;

  // @brief 信号灯视图接口
  struct ISignlightsViewer : public IViewer {
    ISignlightsViewer() TX_DEFAULT;
    virtual ~ISignlightsViewer() TX_DEFAULT;

    /**
     * @brief 获取路由id
     *
     * @return txSysId
     */
    virtual txSysId routeID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车道id
     *
     * @return txLaneID 返回车道id
     */
    virtual txLaneID laneID() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时的道路距离
     *
     * @return txFloat
     */
    virtual txFloat start_s() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取开始时的时间
     *
     * @return txFloat
     */
    virtual txFloat start_t() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取道路的侧向偏移
     *
     * @return txFloat
     */
    virtual txFloat l_offset() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取绿灯时长
     *
     * @return txFloat 返回绿灯时长
     */
    virtual txFloat time_green() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取黄灯时长
     *
     * @return txFloat
     */
    virtual txFloat time_yellow() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取红灯时长
     *
     * @return txFloat
     */
    virtual txFloat time_red() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取方向
     *
     * @return txFloat
     */
    virtual txFloat direction() const TX_NOEXCEPT = 0;
#if USE_SignalByLane
    virtual txFloat compliance() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取车道表示
     *
     * @return txString
     */
    virtual txString lane() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取相位表示
     *
     * @return txString
     */
    virtual txString phase() const TX_NOEXCEPT = 0;

    /**
     * @brief 获取状态
     *
     * @return txString
     */
    virtual txString status() const TX_NOEXCEPT = 0;
#endif /*USE_SignalByLane*/

#if __TX_Mark__("signal control configure")

    /**
     * 获取信号控制计划
     * @return 返回信号控制计划
     */
    virtual txString plan() const TX_NOEXCEPT { return ""; }

    /**
     * 获取信号控制交叉口
     * @return 返回信号控制交叉口
     */
    virtual txString junction() const TX_NOEXCEPT { return ""; }

    /**
     * 获取信号控制相位数量
     * @return 返回信号控制相位数量
     */
    virtual txString phaseNumber() const TX_NOEXCEPT { return ""; }

    /**
     * 获取信号控制信号头
     * @return 返回信号控制信号头
     */
    virtual txString signalHead() const TX_NOEXCEPT { return ""; }

    /**
     * 获取信号控制事件ID
     * @return 返回信号控制事件ID
     */
    virtual txString eventId() const TX_NOEXCEPT { return ""; }
#endif /*signal control configure*/
  };
  using ISignlightsViewerPtr = std::shared_ptr<ISignlightsViewer>;
#if 1 /*def ON_CLOUD*/

  // @brief 位置视图接口
  struct ILocationViewer : public IViewer {
    ILocationViewer() TX_DEFAULT;
    virtual ~ILocationViewer() TX_DEFAULT;

    /**
     * 判断是否已初始化
     * @return 如果已初始化，返回true，否则返回false
     */
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return false; }

    /**
     * 获取系统ID
     * @return 返回系统ID
     */
    virtual Base::txSysId id() const TX_NOEXCEPT { return InvalidId; }

    /**
     * 获取GPS坐标
     * @return 返回GPS坐标
     */
    virtual hadmap::txPoint PosGPS() const TX_NOEXCEPT { return hadmap::txPoint(); }

    /**
     * 获取信息
     * @return 返回信息
     */
    virtual Base::txString info() const TX_NOEXCEPT { return "un-implement"; };
  };
  using ILocationViewerPtr = std::shared_ptr<ILocationViewer>;

  // 交通流量视图接口
  struct ITrafficFlowViewer : public IViewer {
    ITrafficFlowViewer() TX_DEFAULT;
    virtual ~ITrafficFlowViewer() TX_DEFAULT;

    /**
     * 判断是否已初始化
     * @return 如果已初始化，返回true，否则返回false
     */
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return false; }

    struct IVehType {
      // <VehType id="101" Type="Sedan" length="4.5" width="1.8" height="1.5" behavior="TrafficVehicle" />

      /**
       * 获取ID
       * @return 返回ID
       */
      virtual txInt id() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取车辆类型
       * @return 返回车辆类型
       */
      virtual Base::txString vehicleType() const TX_NOEXCEPT { return "undefined"; }

      /**
       * 获取车辆行为
       * @return 返回车辆行为
       */
      virtual VEHICLE_BEHAVIOR behavior() const TX_NOEXCEPT { return VEHICLE_BEHAVIOR::undefined; }

      /**
       * 获取车辆长度
       * @return 返回车辆长度
       */
      virtual txFloat length() const TX_NOEXCEPT { return FLAGS_EGO_Length; }

      /**
       * 获取车辆宽度
       * @return 返回车辆宽度
       */
      virtual txFloat width() const TX_NOEXCEPT { return FLAGS_EGO_Width; }

      /**
       * 获取车辆高度
       * @return 返回车辆高度
       */
      virtual txFloat height() const TX_NOEXCEPT { return FLAGS_EGO_Height; }
      virtual Base::txString Str() const TX_NOEXCEPT { return "un-implement"; }
    };
    using VehTypePtr = std::shared_ptr<IVehType>;

    /**
     * 获取车辆高度
     * @return 返回车辆高度
     */
    virtual VehTypePtr GetVehType(const txInt id) const TX_NOEXCEPT { return std::make_shared<IVehType>(); }

    /**
     * 根据车辆类型枚举获取车辆类型
     * @param _type 车辆类型枚举
     * @return 返回车辆类型的智能指针
     */
    virtual VehTypePtr GetVehType(const Base::Enums::VEHICLE_TYPE _type) const TX_NOEXCEPT {
      return std::make_shared<IVehType>();
    }
    struct IVehComp {
      // <VehComp id = "1001" Type1 = "101" Percentage1 = "80" Behavior1 = "1" Aggress1 = "0.5" Type2 = "102"
      // Percentage2 = "15" Behavior2 = "1" Aggress2 = "0.6" Type3 = "103" Percentage3 = "50" Behavior3 = "1" Aggress3 =
      // "0.1" Info = "Platoon1" / >

      /**
       * 获取车辆ID
       * @return 返回车辆ID
       */
      virtual txInt id() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取指定索引的车辆类型
       * @param idx 车辆索引
       * @return 返回车辆类型ID
       */
      virtual txInt Type(const txInt idx) const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取指定索引的车辆类型字符串
       * @param idx 车辆索引
       * @return 返回车辆类型字符串
       */
      virtual txString TypeStr(const txInt idx) const TX_NOEXCEPT { return ""; }

      /**
       * 获取指定索引的车辆完成百分比
       * @param idx 车辆索引
       * @return 返回车辆完成百分比
       */
      virtual txInt Percentage(const txInt idx) const TX_NOEXCEPT { return 0; }

      /**
       * 获取指定索引的车辆行为ID
       * @param idx 车辆索引
       * @return 返回车辆行为ID
       */
      virtual txInt Behavior(const txInt idx) const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取指定索引的车辆激进程度
       * @param idx 车辆索引
       * @return 返回车辆激进程度
       */
      virtual txFloat Aggress(const txInt idx) const TX_NOEXCEPT { return 0.0; }

      /**
       * 判断指定索引的车辆是否有效
       * @param idx 车辆索引
       * @return 如果车辆有效，返回true，否则返回false
       */
      virtual txBool IsValid(const txInt idx) const TX_NOEXCEPT { return false; }

      /**
       * 获取车辆属性数量
       * @return 返回车辆属性数量
       */
      virtual txInt PropCnt() const TX_NOEXCEPT { return 0; }
      /*virtual txInt Type2() const TX_NOEXCEPT { return InvalidId; }
      virtual txInt Percentage2() const TX_NOEXCEPT { return 0; }
      virtual txInt Behavior2() const TX_NOEXCEPT { return InvalidId; }
      virtual txFloat Aggress2() const TX_NOEXCEPT { return 0.0; }
      virtual txInt Type3() const TX_NOEXCEPT { return InvalidId; }
      virtual txInt Percentage3() const TX_NOEXCEPT { return 0; }
      virtual txInt Behavior3() const TX_NOEXCEPT { return InvalidId; }
      virtual txFloat Aggress3() const TX_NOEXCEPT { return InvalidId; }*/
      /**
       * 获取车辆信息
       * @return 返回车辆信息
       */
      virtual txString Info() const TX_NOEXCEPT { return "un-implement"; }
      virtual Base::txString Str() const TX_NOEXCEPT { return "un-implement"; }
    };
    using VehCompPtr = std::shared_ptr<IVehComp>;

    /**
     * 获取指定ID的车辆组件
     * @param id 车辆组件ID
     * @return 返回指定ID的车辆组件
     */
    virtual VehCompPtr GetVehComp(const txInt id) const TX_NOEXCEPT { return std::make_shared<IVehComp>(); }

    struct IVehInput {
      enum class Distribution : txInt { Fixed, Uniform, Exponential, TimeVarying, Normal, Undef };
      struct TimeVaryingDistributionParam {
        enum { random_number_size = 1000 };
        /* profile="0,1,0.2;300,2,0;600,0.5,0.01;"*/
        Base::txFloat start_time_s = 0.0;
        Base::txFloat mean_value = 0.0;
        Base::txFloat variance = 0.0;
        Base::txBool valid = false;
        std::vector<Base::txFloat> value_list;
        Base::txInt value_index = 0;
        Base::txBool IsValid() const TX_NOEXCEPT { return valid; }

        /**
         * 生成随机数列表
         */
        void Generate() TX_NOEXCEPT {
          valid = true;
          value_list.clear();
          value_index = 0;
          if (Distribution::Fixed == distribution()) {
            value_list.emplace_back(mean_value);
          } else if (Distribution::Normal == distribution()) {
            std::default_random_engine generator;
            std::normal_distribution<Base::txFloat> distribution(mean_value, variance);
            for (Base::txInt i = 0; i < random_number_size; ++i) {
              const Base::txFloat number = distribution(generator);
              if (number > 0.0) {
                value_list.emplace_back(number);
              }
            }
            if (0 == value_list.size()) {
              value_list.emplace_back(FLT_MAX);
            }
          } else {
            value_list.emplace_back(FLT_MAX);
            value_index = 0;
          }
        }

        /**
         * 获取下一个随机数值
         * @return 返回下一个随机数值
         */
        Base::txFloat NextValue() TX_NOEXCEPT {
          if (value_index >= value_list.size()) {
            value_index = 0;
          }
          return value_list[value_index++];
        }

        /**
         * 检查是否满足条件
         * @return 如果满足条件返回true，否则返回false
         */
        txBool Check() const TX_NOEXCEPT { return (start_time_s >= 0.0) && (variance >= 0.0); }
        Distribution distribution() const TX_NOEXCEPT {
          return (Math::isZero(variance)) ? (Distribution::Fixed) : (Distribution::Normal);
        }
        txString Str() const TX_NOEXCEPT {
          std::ostringstream oss;
          oss << TX_VARS(start_time_s) << TX_VARS(mean_value) << TX_VARS(variance);
          return oss.str();
        }
      };
      using TimeVaryingDistributionParamVec = std::vector<TimeVaryingDistributionParam>;
      // <VehInput id="10011" Location="800011" Composition="1001" start_v="12" max_v="15" Distribution="Fixed"
      // TimeHeadway="3.0" Duration="250" Cover="100" Info="Platoon10"/>

      /**
       * 获取ID
       * @return 返回ID
       */
      virtual txInt id() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取位置
       * @return 返回位置
       */
      virtual txInt location() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取组合ID
       * @return 返回组合ID
       */
      virtual txInt composition() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取起始V值
       * @return 返回起始V值
       */
      virtual txFloat start_v() const TX_NOEXCEPT { return 0.0; }

      /**
       * 获取最大V值
       * @return 返回最大V值
       */
      virtual txFloat max_v() const TX_NOEXCEPT { return 0.0; }

      /**
       * 获取V值半范围
       * @return 返回V值半范围
       */
      virtual txFloat halfRange_v() const TX_NOEXCEPT { return 0.3; }

      /**
       * 获取分布类型
       * @return 返回分布类型
       */
      virtual Distribution distribution() const TX_NOEXCEPT { return Distribution::Undef; }

      /**
       * 获取时间变化的分布参数
       * @return 返回时间变化的分布参数
       */
      virtual TimeVaryingDistributionParamVec TimeVaryingDistribution() const TX_NOEXCEPT {
        return TimeVaryingDistributionParamVec();
      }

      /**
       * 获取时间头道
       * @return 返回时间头道
       */
      virtual txFloat timeHeadway() const TX_NOEXCEPT { return 0.0; }

      /**
       * 获取持续时间
       * @return 返回持续时间
       */
      virtual txInt duration() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取覆盖的车道ID集合
       * @return 返回覆盖的车道ID集合
       */
      virtual std::set<Base::txLaneID> cover() const TX_NOEXCEPT { return std::set<Base::txLaneID>(); }

      /**
       * 获取覆盖的车道ID集合的字符串表示
       * @return 返回覆盖的车道ID集合的字符串表示
       */
      virtual txString cover_str() const TX_NOEXCEPT { return "un-implement"; }

      /**
       * 获取信息
       * @return 返回信息
       */
      virtual txString Info() const TX_NOEXCEPT { return "un-implement"; }

      /**
       * 获取字符串表示
       * @return 返回字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT { return "un-implement"; }
      friend std::ostream& operator<<(std::ostream& os, const Distribution v) {
        switch (v) {
          case Distribution::Fixed: {
            os << "Fixed";
            break;
          }
          case Distribution::Uniform: {
            os << "Uniform";
            break;
          }
          case Distribution::Exponential: {
            os << "Exponential";
            break;
          }
          case Distribution::Undef: {
            os << "Undef";
            break;
          }
        }
        return os;
      }
      static Distribution Str2Distribution(const txString& strDistribution) {
        if ("Fixed" == strDistribution) {
          return Distribution::Fixed;
        } else if ("Uniform" == strDistribution) {
          return Distribution::Uniform;
        } else if ("Exponential" == strDistribution) {
          return Distribution::Exponential;
        } else if ("TimeVarying" == strDistribution) {
          return Distribution::TimeVarying;
        }
        return Distribution::Undef;
      }
    };
    using VehInputPtr = std::shared_ptr<IVehInput>;

    /**
     * 获取指定ID的VehInput对象
     * @param id 要获取的VehInput对象的ID
     * @return 返回指定ID的VehInput对象
     */
    virtual VehInputPtr GetVehInput(const txInt id) const TX_NOEXCEPT { return std::make_shared<IVehInput>(); }

    struct IPedInput {
      // <PedInput id="30001" StartLocation="300001" ExitLocation="300002" Composition="3001" start_v="1.5" max_v="1.5"
      // Distribution="Fixed" TimeHeadway="1.5" Duration="100" Resp2Signal="8007" Start_Offset="2" Rate="0"
      // Info="Peds"/>
      using Distribution = IVehInput::Distribution;

      /**
       * 获取ID
       * @return 返回ID
       */
      virtual txInt id() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取起始位置
       * @return 返回起始位置
       */
      virtual txInt startLocation() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取结束位置
       * @return 返回结束位置
       */
      virtual txInt ExitLocation() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取组合ID
       * @return 返回组合ID
       */
      virtual txInt composition() const TX_NOEXCEPT { return InvalidId; }

      /**
       * @brief 获取开始的速度
       *
       * @return txFloat
       */
      virtual txFloat start_v() const TX_NOEXCEPT { return 0.0; }

      /**
       * @brief 获取最大的速度
       *
       * @return txFloat
       */
      virtual txFloat max_v() const TX_NOEXCEPT { return 0.0; }

      /**
       * 获取分布类型
       * @return 返回分布类型
       */
      virtual Distribution distribution() const TX_NOEXCEPT { return Distribution::Undef; }

      /**
       * @brief 获取车头时距
       *
       * @return txFloat
       */
      virtual txFloat timeHeadway() const TX_NOEXCEPT { return 0.0; }

      /**
       * @brief 获取持续时间
       *
       * @return txInt
       */
      virtual txInt duration() const TX_NOEXCEPT { return InvalidId; }
      virtual txInt Resp2Signal() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取信息
       * @return 返回信息
       */
      virtual txString Info() const TX_NOEXCEPT { return "un-implement"; }

      /**
       * 获取字符串
       * @return 返回字符串
       */
      virtual Base::txString Str() const TX_NOEXCEPT { return "un-implement"; }
    };
    using PedInputPtr = std::shared_ptr<IPedInput>;

    /**
     * 获取PedInput指针
     * @param id 输入的ID
     * @return 返回PedInput指针
     */
    virtual PedInputPtr GetPedInput(const txInt id) const TX_NOEXCEPT { return std::make_shared<IPedInput>(); }

    struct IBeh {
      // <Beh id="1" Type="Freeway" cc0="1.5" cc1="1.3" cc2="4"/>
      // <Beh id = "2" Type = "Arterial" AX = "2" BX_Add = "2" BX_Mult = "3" / >
      enum class BehType : txInt { Freeway, Arterial, UserDefine, Undef };

      /**
       * @brief 获取id
       *
       * @return txInt
       */
      virtual txInt id() const TX_NOEXCEPT { return InvalidId; }

      /**
       * @brief 获取类型
       *
       * @return BehType
       */
      virtual BehType type() const TX_NOEXCEPT { return BehType::Undef; }

      /**
       * @brief 获取参数cc0
       *
       * @return txFloat
       */
      virtual txFloat cc0() const TX_NOEXCEPT { return 0.0; }

      /**
       * @brief 获取参数cc1
       *
       * @return txFloat
       */
      virtual txFloat cc1() const TX_NOEXCEPT { return 0.0; }

      /**
       * @brief 获取参数cc2
       *
       * @return txFloat
       */
      virtual txFloat cc2() const TX_NOEXCEPT { return 0.0; }
      virtual txFloat AX() const TX_NOEXCEPT { return 0.0; }
      virtual txFloat BX_Add() const TX_NOEXCEPT { return 0.0; }
      virtual txFloat BX_Mult() const TX_NOEXCEPT { return 0.0; }
      virtual txFloat LCduration() const TX_NOEXCEPT { return 3.5; }
      virtual Base::txString Str() const TX_NOEXCEPT { return "un-implement"; }

      friend std::ostream& operator<<(std::ostream& os, const BehType v) {
        switch (v) {
          case BehType::Freeway: {
            os << "Freeway";
            break;
          }
          case BehType::Arterial: {
            os << "Arterial";
            break;
          }
          case BehType::UserDefine: {
            os << "UserDefine";
            break;
          }
          case BehType::Undef: {
            os << "Undef";
            break;
          }
        }
        return os;
      }
      static BehType Str2BehType(const txString& strBehType) {
        if ("Freeway" == strBehType) {
          return BehType::Freeway;
        } else if ("Arterial" == strBehType) {
          return BehType::Arterial;
        } else if ("UserDefined" == strBehType) {
          return BehType::UserDefine;
        }
        return BehType::Undef;
      }
    };
    using BehPtr = std::shared_ptr<IBeh>;
    virtual BehPtr GetBeh(const txInt id) const TX_NOEXCEPT { return std::make_shared<IBeh>(); }

    struct IVehExit {
      // <VehExit id = "20001" Location = "200001" Cover = "0" Info = "Exit1" / >
      /**
       * @brief 获取id
       *
       * @return txInt
       */
      virtual txInt id() const TX_NOEXCEPT { return InvalidId; }

      /**
       * @brief 获取位置
       *
       * @return txInt
       */
      virtual txInt location() const TX_NOEXCEPT { return InvalidId; }
      virtual std::set<Base::txLaneID> cover() const TX_NOEXCEPT { return std::set<Base::txLaneID>(); }
      virtual txString cover_str() const TX_NOEXCEPT { return "un-implement"; }

      /**
       * @brief 获取结构体info
       *
       * @return txString
       */
      virtual txString Info() const TX_NOEXCEPT { return "un-implement"; }
      virtual Base::txString Str() const TX_NOEXCEPT { return "un-implement"; }
    };
    using VehExitPtr = std::shared_ptr<IVehExit>;
    virtual VehExitPtr GetVehExit(const txInt id) const TX_NOEXCEPT { return std::make_shared<IVehExit>(); }

    struct IRouteGroup {
      IRouteGroup() TX_DEFAULT;
      ~IRouteGroup() TX_DEFAULT;
      // <RouteGroup id="8002" Start="800020" Mid1="0" End1="800021" Percentage1="30" Mid2="0" End2="800022"
      // Percentage2="40" Mid3="0" End3="800023" Percentage3="30" />

      /**
       * @brief 获取id
       *
       * @return txInt
       */
      virtual txInt id() const TX_NOEXCEPT { return InvalidId; }

      /**
       * @brief 获取start
       *
       * @return txInt
       */
      virtual txInt start() const TX_NOEXCEPT { return InvalidId; }

      /**
       * @brief 获取mid1
       *
       * @return txInt
       */
      virtual txInt mid1() const TX_NOEXCEPT { return InvalidId; }

      /**
       * @brief 获取end1
       *
       * @return txInt
       */
      virtual txInt end1() const TX_NOEXCEPT { return InvalidId; }

      /**
       * @brief 获取percentage1
       *
       * @return txInt
       */
      virtual txInt percentage1() const TX_NOEXCEPT { return 0; }

      /**
       * 获取mid2
       * @return 返回mid2的值
       */
      virtual txInt mid2() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取end2
       * @return 返回end2的值
       */
      virtual txInt end2() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取percentage2
       * @return 返回percentage2的值
       */
      virtual txInt percentage2() const TX_NOEXCEPT { return 0; }

      /**
       * 获取mid3
       * @return 返回mid3的值
       */
      virtual txInt mid3() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取end3
       * @return 返回end3的值
       */
      virtual txInt end3() const TX_NOEXCEPT { return InvalidId; }

      /**
       * 获取percentage3
       * @return 返回percentage3的值
       */
      virtual txInt percentage3() const TX_NOEXCEPT { return 0; }

      /**
       * 获取Str
       * @return 返回Str的值
       */
      virtual Base::txString Str() const TX_NOEXCEPT { return "un-implement"; }
    };
    using RouteGroupPtr = std::shared_ptr<IRouteGroup>;

    /**
     * 获取路由组
     * @param id 路由组的id
     * @return 返回路由组的智能指针
     */
    virtual RouteGroupPtr GetRouteGroup(const txInt id) const TX_NOEXCEPT { return std::make_shared<IRouteGroup>(); }

    /**
     * 获取系统id
     * @return 返回系统id的值
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return 0; }

    /**
     * 获取系统id
     * @return 返回系统id的值
     */
    virtual std::unordered_map<Base::txInt, VehInputPtr> GetAllVehInputData() const TX_NOEXCEPT {
      return std::unordered_map<Base::txInt, VehInputPtr>();
    }

    /**
     * 获取所有行人输入数据
     * @return 返回一个包含所有行人输入数据的无序映射
     */
    virtual std::unordered_map<Base::txInt, PedInputPtr> GetAllPedInputData() const TX_NOEXCEPT {
      return std::unordered_map<Base::txInt, PedInputPtr>();
    }

    /**
     * 获取所有路由组数据
     * @return 返回一个包含所有路由组数据的无序映射
     */
    virtual std::unordered_map<Base::txInt, RouteGroupPtr> GetAllRouteGroupData() const TX_NOEXCEPT {
      return std::unordered_map<Base::txInt, RouteGroupPtr>();
    }

    /**
     * 获取所有车辆出口数据
     * @return 返回一个包含所有车辆出口数据的无序映射
     */
    virtual std::unordered_map<Base::txInt, VehExitPtr> GetAllVehExitData() const TX_NOEXCEPT {
      return std::unordered_map<Base::txInt, VehExitPtr>();
    }
    // virtual Base::txFloat GetRuleComplianceProportion() const TX_NOEXCEPT { return
    // FLAGS_DefaultRuleComplianceProportion;}
  };
  using ITrafficFlowViewerPtr = std::shared_ptr<ITrafficFlowViewer>;
#endif /*ON_CLOUD*/

 public:
  /**
   * @brief 获取场景类型
   *
   * @return ESceneType
   */
  virtual ESceneType GetSceneType() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取ego类型
   *
   * @return EgoType
   */
  virtual EgoType GetEgoType() const TX_NOEXCEPT { return EgoType::eVehicle; }

  /**
   * @brief 获取routing信息
   *
   * @param refEgoData
   * @return Base::txBool
   */
  virtual Base::txBool GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT = 0;

  /**
   * @brief 获取地图管理器初始化参数
   *
   * @param refParams
   * @return Base::txBool
   */
  virtual Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams) TX_NOEXCEPT = 0;

  /**
   * @brief 获取路由视图
   *
   * @param id
   * @return IRouteViewerPtr
   */
  virtual IRouteViewerPtr GetRouteData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * @brief 获取ego route数据
   *
   * @return IRouteViewerPtr
   */
  virtual IRouteViewerPtr GetEgoRouteData() TX_NOEXCEPT = 0;

  /**
   * @brief 获取加速事件视图
   *
   * @param id
   * @return IAccelerationViewerPtr
   */
  virtual IAccelerationViewerPtr GetAccsData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * @brief 获取加速度s事件视图
   *
   * @param id
   * @return IAccelerationViewerPtr
   */
  virtual IAccelerationViewerPtr GetAccsEventData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的合并数据
   * @param id 要获取的合并数据的ID
   * @return 返回一个包含指定ID的合并数据的指针
   */
  virtual IMergesViewerPtr GetMergesData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的合并事件数据
   * @param id 要获取的合并事件数据的ID
   * @return 返回一个包含指定ID的合并事件数据的指针
   */
  virtual IMergesViewerPtr GetMergesEventData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的速度数据
   * @param id 要获取的速度数据的ID
   * @return 返回一个包含指定ID的速度数据的指针
   */
  virtual IVelocityViewerPtr GetVelocityData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的速度事件数据
   * @param id 要获取的速度事件数据的ID
   * @return 返回一个包含指定ID的速度事件数据的指针
   */
  virtual IVelocityViewerPtr GetVelocityEventData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的行人事件数据
   * @param id 要获取的行人事件数据的ID
   * @return 返回一个包含指定ID的行人事件数据的指针
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_TimeEvent(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的行人速度事件数据
   * @param id 要获取的行人速度事件数据的ID
   * @return 返回一个包含指定ID的行人速度事件数据的指针
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_VelocityEvent(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 判断是否有场景事件
   * @return 如果有场景事件返回true，否则返回false
   */
  virtual txBool HasSceneEvent() const TX_NOEXCEPT { return _NonEmpty_(_seceneEventVec); }

  /**
   * 获取场景事件向量
   * @return 返回一个包含场景事件的向量
   */
  virtual std::vector<ISceneEventViewerPtr> GetSceneEventVec() const TX_NOEXCEPT { return _seceneEventVec; }

  /**
   * 获取指定ID的车辆数据
   * @param id 要获取的车辆数据的ID
   * @return 返回一个包含指定ID的车辆数据的指针
   */
  virtual IVehiclesViewerPtr GetVehicleData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取所有车辆数据
   * @return 返回一个包含所有车辆数据的哈希表
   */
  virtual std::unordered_map<Base::txSysId, IVehiclesViewerPtr> GetAllVehicleData() TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的行人数据
   * @param id 要获取的行人数据的ID
   * @return 返回一个包含指定ID的行人数据的指针
   */
  virtual IPedestriansViewerPtr GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取所有行人数据
   * @return 返回一个包含所有行人数据的哈希表
   */
  virtual std::unordered_map<Base::txSysId, IPedestriansViewerPtr> GetAllPedestrianData() TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的信号灯数据
   * @param id 要获取的信号灯数据的ID
   * @return 返回一个包含指定ID的信号灯数据的指针
   */
  virtual ISignlightsViewerPtr GetSignlightData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取所有信号灯数据
   * @return 返回一个包含所有信号灯数据的哈希表
   */
  virtual std::unordered_map<Base::txSysId, ISignlightsViewerPtr> GetAllSignlightData() TX_NOEXCEPT = 0;

  /**
   * 获取指定ID的障碍物数据
   * @param id 要获取的障碍物数据的ID
   * @return 返回一个包含指定ID的障碍物数据的指针
   */
  virtual IObstacleViewerPtr GetObstacleData(Base::txSysId const id) TX_NOEXCEPT = 0;

  /**
   * 获取所有障碍物数据
   * @return 返回一个包含所有障碍物数据的哈希表
   */
  virtual std::unordered_map<Base::txSysId, IObstacleViewerPtr> GetAllObstacleData() TX_NOEXCEPT = 0;

  /**
   * 获取随机数种子
   * @return 返回一个整数类型的随机数种子
   */
  virtual Base::txInt GetRandomSeed() const TX_NOEXCEPT = 0;

  /**
   * 获取自车数据
   * @return 返回一个包含自车数据的指针
   */
  virtual IVehiclesViewerPtr GetEgoData() TX_NOEXCEPT = 0;

  /**
   * 获取自车拖车数据
   * @return 返回一个包含自车拖车数据的指针
   */
  virtual IVehiclesViewerPtr GetEgoTrailerData() TX_NOEXCEPT = 0;

  /**
   * 设置自车数据
   * @param _egoInfo 包含自车数据的结构体
   */
  virtual void SetEgoData(const sim_msg::Location& _egoInfo) TX_NOEXCEPT;

  /**
   * 获取规则遵循比例
   * @return 返回一个浮点数类型的规则遵循比例
   */
  virtual Base::txFloat GetRuleComplianceProportion() const TX_NOEXCEPT {
    return FLAGS_DefaultRuleComplianceProportion;
  }
#if 1 /*def ON_CLOUD*/

  /**
   * 获取指定ID的位置数据
   * @param id 要获取的位置数据的ID
   * @return 返回一个包含位置数据的指针
   */
  virtual ILocationViewerPtr GetLocationData(const Base::txUInt id) TX_NOEXCEPT {
    return nullptr; /* txMsg("unimplement"); return std::make_shared< ILocationViewer >();*/
  }

  /**
   * 获取交通流量数据
   * @return 返回一个包含交通流量数据的指针
   */
  virtual ITrafficFlowViewerPtr GetTrafficFlow() TX_NOEXCEPT {
    return nullptr; /*txMsg("unimplement"); return std::make_shared< ITrafficFlowViewer >();*/
  }

  /**
   * 获取所有位置数据
   * @return 返回一个包含所有位置数据的哈希表
   */
  virtual std::unordered_map<Base::txInt, ILocationViewerPtr> GetAllLocationData()
      TX_NOEXCEPT { /*txMsg("unimplement");*/
    return std::unordered_map<Base::txInt, ILocationViewerPtr>();
  }
#endif /*ON_CLOUD*/
#if __TX_Mark__("vehicle_measurements")

  /**
   * 获取指定ID和类型的车辆几何信息
   * @param egoId 要获取的车辆几何信息的ID
   * @param egoType 要获取的车辆几何信息的类型
   * @param refVehGeom 用于存储车辆几何信息的引用
   * @return 返回一个布尔值，表示操作是否成功
   */
  virtual txBool GetVehicleGeometory(const Base::txSysId egoId, const Base::Enums::EgoSubType egoType,
                                     sim_msg::VehicleGeometory& refVehGeom) const TX_NOEXCEPT;

  /**
   * 设置指定ID的车辆几何信息
   * @param egoId 要设置的车辆几何信息的ID
   * @param refVehGeomList 包含车辆几何信息的列表
   */
  virtual void SetVehicleGeometory(const Base::txSysId egoId,
                                   const sim_msg::VehicleGeometoryList& refVehGeomList) TX_NOEXCEPT;
#endif /*__TX_Mark__("vehicle_measurements")*/

  /**
   * 获取当前场景加载器的模拟ID
   * @return 返回一个字符串，表示当前场景加载器的模拟ID
   */
  virtual Base::txString Sim_Id() const TX_NOEXCEPT { return m_strSimId; }

  /**
   * 获取当前场景加载器的数据版本
   * @return 返回一个字符串，表示当前场景加载器的数据版本
   */
  virtual Base::txString Date_Version() const TX_NOEXCEPT { return m_strDataVersion; }

  /**
   * 获取规划器的起始速度
   * @return 返回一个浮点数，表示规划器的起始速度
   */
  virtual Base::txFloat PlannerStartV() const TX_NOEXCEPT { return 0.0; }

  /**
   * 获取规划器的起始角度
   * @return 返回一个浮点数，表示规划器的起始角度
   */
  virtual Base::txFloat PlannerTheta() const TX_NOEXCEPT { return 0.0; }

  /**
   * 获取规划器的最大速度
   * @return 返回一个浮点数，表示规划器的最大速度，单位为千米/小时
   */
  virtual Base::txFloat PlannerVelocityMax() const TX_NOEXCEPT { return 27.0 /*100km/h*/; }

  /**
   * 获取规划器的最大加速度
   * @return 返回一个浮点数，表示规划器的最大加速度，单位为m/s²
   */
  virtual Base::txFloat PlannerAccMax() const TX_NOEXCEPT { return 3.5 /*0.35*g*/; }

  /**
   * 获取规划器的最大减速度
   * @return 返回一个浮点数，表示规划器的最大减速度，单位为m/s²
   */
  virtual Base::txFloat PlannerDeceMax() const TX_NOEXCEPT { return -3.5 /*0.35*g*/; }

  /**
   * 设置场景事件版本
   * @param _t 一个eSceneEventVersion枚举值，表示场景事件的版本
   */
  void SetSceneEventVersion(const eSceneEventVersion _t) TX_NOEXCEPT { m_SceneEventVersion = _t; }

  /**
   * 获取场景事件版本
   * @return 返回一个eSceneEventVersion枚举值，表示场景事件的版本
   */
  eSceneEventVersion SceneEventVersion() const TX_NOEXCEPT { return m_SceneEventVersion; }

  /**
   * 获取当前活动的规划器名称
   * @return 返回一个字符串，表示当前活动的规划器名称
   */
  virtual Base::txString activePlan() const TX_NOEXCEPT { return ""; }

  /**
   * 判断规划器轨迹是否启用
   * @return 返回一个布尔值，表示规划器轨迹是否启用
   */
  virtual Base::txBool Planner_Trajectory_Enabled() const TX_NOEXCEPT { return false; }

  /**
   * 获取控制路径节点列表
   * @return 返回一个IRouteViewer::control_path_node_vec类型的列表，表示控制路径节点列表
   */
  virtual IRouteViewer::control_path_node_vec ControlPath() const TX_NOEXCEPT {
    return IRouteViewer::control_path_node_vec();
  }

  /**
   * @brief 获取场景pb结构
   *
   * @return sim_msg::Scene
   */
  virtual sim_msg::Scene ego_scene_pb() const TX_NOEXCEPT { return m_ego_scene_pb; }
  virtual sim_msg::Scene& ego_scene_pb() TX_NOEXCEPT { return m_ego_scene_pb; }

#if USE_EgoGroup

 public:
  // vehicle_measurements
  virtual txBool GetVehicleGeometory(const Base::txString egoGroup, const Base::Enums::EgoSubType egoType,
                                     sim_msg::VehicleGeometory& refVehGeom) const TX_NOEXCEPT;
  virtual void SetVehicleGeometory(const sim_msg::Scene& refSceneDesc) TX_NOEXCEPT;

  virtual std::vector<Base::txString> GetAllEgoGroups() const TX_NOEXCEPT { return std::vector<Base::txString>(); }
  virtual Base::txBool GetRoutingInfo(Base::txString egoGroup, sim_msg::Location& refEgoData) { return false; }
  virtual EgoType GetEgoType(Base::txString egoGroup) const TX_NOEXCEPT { return EgoType::eVehicle; }
  virtual Base::txInt GetEgoId(Base::txString egoGroup) const TX_NOEXCEPT { return FLAGS_Default_EgoVehicle_Id; };
  virtual IVehiclesViewerPtr GetEgoData(Base::txString egoGroup) { return nullptr; }
  virtual IVehiclesViewerPtr GetEgoTrailerData(Base::txString egoGroup) { return nullptr; }
  virtual IRouteViewerPtr GetEgoRouteData(Base::txString egoGroup) { return nullptr; }
  virtual Base::txFloat PlannerStartV(Base::txString egoGroup) const { return 0.0; }
  virtual Base::txFloat PlannerTheta(Base::txString egoGroup) const { return 0.0; }
  virtual Base::txFloat PlannerVelocityMax(Base::txString egoGroup) const TX_NOEXCEPT { return 27.0 /*100km/h*/; }
  virtual Base::txBool Planner_Trajectory_Enabled(Base::txString egoGroup) const TX_NOEXCEPT { return false; }
  virtual IRouteViewer::control_path_node_vec ControlPath(Base::txString egoGroup) const TX_NOEXCEPT {
    return IRouteViewer::control_path_node_vec();
  }
#endif

 protected:
  txString _source;
  sim_msg::Location m_EgoData;
  EStatus _status = EStatus::eNone;
  std::vector<ISceneEventViewerPtr> _seceneEventVec;
  std::map<Base::txSysId, sim_msg::VehicleGeometoryList> _id2VehGeomMap;
  Base::txString m_strSimId = "0";
  Base::txString m_strDataVersion = "0";
  eSceneEventVersion m_SceneEventVersion = eSceneEventVersion::V_0_0_0_0;
  sim_msg::Scene m_ego_scene_pb;
#if USE_EgoGroup
  std::map<Base::txString, sim_msg::VehicleGeometory> _group2VehGeomMap;
#endif
};

using ISceneLoaderPtr = std::shared_ptr<ISceneLoader>;

TX_NAMESPACE_CLOSE(Base)

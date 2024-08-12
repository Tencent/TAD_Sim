// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tbb/concurrent_hash_map.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_sim_time.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_SceneEvent_1_0_0_0 {
 public:
  using ISceneEventViewerPtr = Base::ISceneLoader::ISceneEventViewerPtr;
  using SceneEventElementStatusType = Base::Enums::SceneEventElementStatusType;
  virtual ~TAD_SceneEvent_1_0_0_0() TX_DEFAULT;

  /*virtual Base::txBool HandlerEvent(ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT { return false; }*/

  /**
   * @brief 获取自动驾驶模式
   * @return 返回当前自动驾驶模式
   */
  virtual Base::txBool HandlerEvent(Base::TimeParamManager const& timeMgr,
                                    ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 使用场景事件_1_0_0_0
   * @return 返回是否使用场景事件_1_0_0_0的布尔值
   */
  virtual Base::txBool UseSceneEvent_1_0_0_0() const TX_NOEXCEPT { return m_use_scene_event_1_0_0_0; }

  /**
   * @brief 获取场景事件列表
   * @return 返回一个指向整数向量的指针
   */
  virtual std::vector<ISceneEventViewerPtr> SceneEvent() const TX_NOEXCEPT { return m_vec_scene_event; }

  /**
   * @brief 场景事件是否存在
   *
   * @return Base::txBool
   */
  virtual Base::txBool HasSceneEvent() const TX_NOEXCEPT { return _NonEmpty_(m_vec_scene_event); }

  /**
   * @brief 获取当前场景是否存在车道改变
   * @return 返回一个整型变量的引用，表示是否存在车道改变
   */
  virtual Base::txInt& GetHasLaneChange() { return m_lanechange; }

  /**
   * @brief 清空场景事件
   *
   */
  virtual void ClearSceneEvent() TX_NOEXCEPT {
    m_vec_scene_event.clear();
    m_use_scene_event_1_0_0_0 = true;
    _elemAccViewPtr = nullptr;
    _elemAccEventViewPtr = nullptr;
    _elemMergeViewPtr = nullptr;
    _elemMergeEventViewPtr = nullptr;
    _elemVelocityViewPtr = nullptr;
    _elemVelocityEventViewPtr = nullptr;
  }

 public:
  /**
   * @brief 获取AccView视图指针
   *
   * @return Base::ISceneLoader::IAccelerationViewerPtr
   */
  virtual Base::ISceneLoader::IAccelerationViewerPtr getAccViewPtr() const TX_NOEXCEPT { return _elemAccViewPtr; }

  /**
   * @brief 获取AccEventView视图指针
   *
   * @return Base::ISceneLoader::IAccelerationViewerPtr
   */
  virtual Base::ISceneLoader::IAccelerationViewerPtr getAccEventViewPtr() const TX_NOEXCEPT {
    return _elemAccEventViewPtr;
  }

  /**
   * @brief 获取MergeView视图指针
   *
   * @return Base::ISceneLoader::IMergesViewerPtr
   */
  virtual Base::ISceneLoader::IMergesViewerPtr getMergeViewPtr() const TX_NOEXCEPT { return _elemMergeViewPtr; }

  /**
   * @brief 获取MergeEventView视图指针
   *
   * @return Base::ISceneLoader::IMergesViewerPtr
   */
  virtual Base::ISceneLoader::IMergesViewerPtr getMergeEventViewPtr() const TX_NOEXCEPT {
    return _elemMergeEventViewPtr;
  }

  /**
   * @brief 获取VelocityView视图指针
   *
   * @return Base::ISceneLoader::IVelocityViewerPtr
   */
  virtual Base::ISceneLoader::IVelocityViewerPtr getVelocityViewPtr() const TX_NOEXCEPT { return _elemVelocityViewPtr; }

  /**
   * @brief 获取VelocityEventView视图指针
   *
   * @return Base::ISceneLoader::IVelocityViewerPtr
   */
  virtual Base::ISceneLoader::IVelocityViewerPtr getVelocityEventViewPtr() const TX_NOEXCEPT {
    return _elemVelocityEventViewPtr;
  }

 public:
  struct AccEndConditionManager {
    Base::ISceneLoader::EventEndCondition_t m_endCondition;
    Base::txFloat m_remainingTime;
    Base::txFloat m_happenVelocity;
    Base::txFloat m_targetVelocity;
  };

  /**
   * @brief 判断当前场景是否存在Acc结束条件
   * @return 返回一个布尔值，表示当前场景是否存在累计结束条件
   */
  virtual Base::txBool HasAccEndCondition() const TX_NOEXCEPT { return m_optional_AccEndCondition.has_value(); }

  /**
   * @brief 获取累计结束条件管理器
   * @return 返回累计结束条件管理器
   */
  virtual AccEndConditionManager GetAccEndCondition() const TX_NOEXCEPT { return *m_optional_AccEndCondition; }

  /**
   * @brief 设置累计结束条件的剩余时间
   * @param _time 新的剩余时间
   */
  virtual void SetAccEndConditionRemainTime(const Base::txFloat _time) TX_NOEXCEPT {
    if (CallSucc(HasAccEndCondition())) {
      m_optional_AccEndCondition->m_remainingTime = _time;
    }
  }
  /*virtual void ClearAccEndCondition() TX_NOEXCEPT { m_optional_AccEndCondition = boost::none; }*/

  /**
   * @brief 清空acc结束条件
   *
   * @param timeMgr
   */
  virtual void ClearAccEndCondition(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
    m_optional_AccEndCondition = boost::none;
    ClearAccAction(timeMgr);
  }

 public:
  struct SceneEventStateInfo {
    Base::txString elemType_Id; /*vehicle_123*/
    SceneEventElementStatusType state = _plus_(Base::Enums::SceneEventElementStatusType::none);
    Base::txFloat time_stamp = 0;
  };

  /**
   * @brief 创建场景事件状态信息
   * @param _elemType_Id 元素类型ID
   * @param _state 元素状态
   * @param _time_stamp 时间戳
   * @return 返回场景事件状态信息
   */
  static SceneEventStateInfo Make_SceneEventStateInfo(const Base::txString& _elemType_Id,
                                                      const SceneEventElementStatusType _state,
                                                      const Base::txFloat _time_stamp) TX_NOEXCEPT;

  /**
   * @brief 创建场景事件状态信息
   * @param _elemType_Id 元素类型ID
   * @param _state 元素状态
   * @param _time_stamp 时间戳
   * @return 返回场景事件状态信息
   */
  static Base::txString Make_ElemType_ElemId(const Base::Enums::ElementType elemType,
                                             const Base::txSysId elemId) TX_NOEXCEPT;

  /**
   * @brief 根据给定的事件ID和动作ID生成一个唯一的字符串
   * @param evId 事件ID
   * @param actionId 动作ID
   * @return 返回由事件ID和动作ID组成的唯一字符串
   */
  static Base::txString Make_evId_actId(const Base::txSysId evId, const Base::txInt actionId) TX_NOEXCEPT;

  /**
   * @brief 更新场景事件状态信息
   * @param evId_actId 事件ID和动作ID组成的字符串
   * @param _info 场景事件状态信息
   */
  static void UpdateSceneEventStates(const Base::txString& evId_actId, const SceneEventStateInfo& _info) TX_NOEXCEPT;

  /**
   * @brief 查询场景事件状态信息
   * @param evId_actId 事件ID和动作ID组成的字符串
   * @param _info 场景事件状态信息的引用，用于存储查询到的状态信息
   * @return 是否成功查询到场景事件状态信息
   */
  static Base::txBool QuerySceneEventStates(const Base::txString& evId_actId, SceneEventStateInfo& _info) TX_NOEXCEPT;

  /**
   * @brief 清空场景事件状态信息
   *
   */
  static void ClearSceneEventStates() TX_NOEXCEPT { s_evIdactionId2StateInfo.clear(); }

 protected:
  using evIdactionId2StateInfo = tbb::concurrent_hash_map<Base::txString, SceneEventStateInfo>;
  static evIdactionId2StateInfo s_evIdactionId2StateInfo;

 protected:
  std::vector<ISceneEventViewerPtr> m_vec_scene_event;
  Base::txBool m_use_scene_event_1_0_0_0 = true;
  Base::txInt m_lanechange = 0;
  Base::ISceneLoader::IAccelerationViewerPtr _elemAccViewPtr = nullptr;
  Base::ISceneLoader::IAccelerationViewerPtr _elemAccEventViewPtr = nullptr;
  Base::ISceneLoader::IMergesViewerPtr _elemMergeViewPtr = nullptr;
  Base::ISceneLoader::IMergesViewerPtr _elemMergeEventViewPtr = nullptr;
  Base::ISceneLoader::IVelocityViewerPtr _elemVelocityViewPtr = nullptr;
  Base::ISceneLoader::IVelocityViewerPtr _elemVelocityEventViewPtr = nullptr;

  boost::optional<AccEndConditionManager> m_optional_AccEndCondition;

#if __TX_Mark__("MergeActionManager")
  class ActionManager {
    using txString = Base::txString;
    using txBool = Base::txBool;
    using txFloat = Base::txFloat;

   public:
    ActionManager() TX_DEFAULT;
    ~ActionManager() TX_DEFAULT;

    /**
     * @brief 查询场景事件状态信息
     * @param evId_actId 事件ID和动作ID组成的字符串
     * @param _info 场景事件状态信息的引用，用于存储查询到的状态信息
     * @return 是否成功查询到场景事件状态信息
     */
    Base::txBool Initialize(const txString _evId_actionId, const txString _self_elemType_elemId) TX_NOEXCEPT {
      m_evId_actionId = _evId_actionId;
      m_self_elemType_elemId = _self_elemType_elemId;
      return true;
    }

    /**
     * @brief 获取场景事件ID和动作ID组成的字符串
     * @return 场景事件ID和动作ID组成的字符串
     */
    Base::txString evId_actionId() const TX_NOEXCEPT { return m_evId_actionId; }

    /**
     * @brief 获取当前元素的类型和ID
     * @return 当前元素的类型和ID组成的字符串
     */
    Base::txString self_elemType_elemId() const TX_NOEXCEPT { return m_self_elemType_elemId; }

   protected:
    Base::txString m_evId_actionId;
    Base::txString m_self_elemType_elemId;
  };
  using ActionManagerPtr = std::shared_ptr<ActionManager>;

  ActionManagerPtr m_AccActionManagerPtr = nullptr;

  /**
   * @brief 清除操作
   * @param timeMgr 时间参数管理器
   */
  virtual void ClearAccAction(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
#endif /*__TX_Mark__("MergeActionManager")*/
};
using TAD_SceneEvent_1_0_0_0_Ptr = std::shared_ptr<TAD_SceneEvent_1_0_0_0>;

TX_NAMESPACE_CLOSE(TrafficFlow)

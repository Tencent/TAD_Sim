// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_scene_event_1_0_0_0.h"

TX_NAMESPACE_OPEN(TrafficFlow)

TAD_SceneEvent_1_0_0_0::evIdactionId2StateInfo TAD_SceneEvent_1_0_0_0::s_evIdactionId2StateInfo;

// 创建一个SceneEventStateInfo对象，并设置其元素类型ID、状态和时间戳
TAD_SceneEvent_1_0_0_0::SceneEventStateInfo TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
    const Base::txString& _elemType_Id, const SceneEventElementStatusType _state,
    const Base::txFloat _time_stamp) TX_NOEXCEPT {
  // 创建一个SceneEventStateInfo对象，设置对象的属性
  SceneEventStateInfo res;
  res.elemType_Id = _elemType_Id;
  res.state = _state;
  res.time_stamp = _time_stamp;
  return res;
}

// 创建一个字符串，表示元素类型和元素ID的组合
Base::txString TAD_SceneEvent_1_0_0_0::Make_ElemType_ElemId(const Base::Enums::ElementType elemType,
                                                            const Base::txSysId elemId) TX_NOEXCEPT {
  // 创建一个字符串流对象
  std::ostringstream oss;
  // 将元素类型和元素ID组合成一个字符串
  oss << elemType._to_string() << "_" << elemId;
  // 返回组合后的字符串
  return oss.str();
}

Base::txString TAD_SceneEvent_1_0_0_0::Make_evId_actId(const Base::txSysId evId,
                                                       const Base::txInt actionId) TX_NOEXCEPT {
  std::ostringstream oss;
  // 拼接eventid和actionid
  oss << evId << "_" << actionId;
  return oss.str();
}

// 更新场景事件状态
void TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(const Base::txString& evId_actId,
                                                    const SceneEventStateInfo& _info) TX_NOEXCEPT {
  // 创建一个evIdactionId2StateInfo的常量访问器
  evIdactionId2StateInfo::const_accessor ca;
  // 如果在evIdactionId2StateInfo中找不到指定的键
  if (CallFail(s_evIdactionId2StateInfo.find(ca, evId_actId))) {
    // 释放访问器
    ca.release();
    /*insert*/
    evIdactionId2StateInfo::accessor a;
    s_evIdactionId2StateInfo.insert(a, evId_actId);
    a->second = _info;
    a.release();
    return;
  }

  ca.release();
  // 创建一个evIdactionId2StateInfo的访问器
  evIdactionId2StateInfo::accessor a;
  // 如果在evIdactionId2StateInfo中找到指定的键
  if (CallSucc(s_evIdactionId2StateInfo.find(a, evId_actId))) {
    /*update*/
    a->second = _info;
    return;
  }
}

// 查询场景事件状态
Base::txBool TAD_SceneEvent_1_0_0_0::QuerySceneEventStates(const Base::txString& evId_actId,
                                                           SceneEventStateInfo& _info) TX_NOEXCEPT {
  // 创建一个evIdactionId2StateInfo的常量访问器
  evIdactionId2StateInfo::const_accessor ca;
  // 如果在evIdactionId2StateInfo中找到指定的键
  if (CallSucc(s_evIdactionId2StateInfo.find(ca, evId_actId))) {
    // 将找到的值赋给_info
    _info = ca->second;
    ca.release();
    return true;
  } else {
    return false;
  }
}

void TAD_SceneEvent_1_0_0_0::ClearAccAction(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 如果m_AccActionManagerPtr不为空
  if (NonNull_Pointer(m_AccActionManagerPtr)) {
    // 更新场景事件状态
    TAD_SceneEvent_1_0_0_0::UpdateSceneEventStates(
        m_AccActionManagerPtr->evId_actionId(),
        TAD_SceneEvent_1_0_0_0::Make_SceneEventStateInfo(
            m_AccActionManagerPtr->self_elemType_elemId(),
            _plus_(TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType::completeState), timeMgr.PassTime()));
    // 将m_AccActionManagerPtr设置为空
    m_AccActionManagerPtr = nullptr;
    LOG(INFO) << " acc action hit the target. stop action.";
  }
}
TX_NAMESPACE_CLOSE(TrafficFlow)

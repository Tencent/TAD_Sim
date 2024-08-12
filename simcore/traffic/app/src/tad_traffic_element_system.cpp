// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "TAD_TrafficElementSystem.h"
#include "tad_assembler_context.h"
#include "tad_linear_element_manager.h"
TX_NAMESPACE_OPEN(TrafficFlow)

// 初始化TAD_TrafficElementSystem
Base::txBool TAD_TrafficElementSystem::Initialize(Base::ISceneLoaderPtr loader) TX_NOEXCEPT {
  // 如果loader非空
  if (NonNull_Pointer(loader)) {
    m_SceneDataSource = loader;
    // 创建一个新的TAD_LinearElementManager对象并将其赋值给m_ElementMgr
    m_ElementMgr = std::make_shared<TAD_LinearElementManager>();
    // 创建一个新的Scene::TAD_AssemblerContext对象并将其赋值给m_AssemblerCtx
    m_AssemblerCtx = std::make_shared<Scene::TAD_AssemblerContext>();
    // 如果AssemblerCtx和m_ElementMgr非空且m_SceneDataSource->LoadObjects()和AssemblerCtx->GenerateScene(m_ElementMgr,
    // loader)成功
    if (NonNull_Pointer(AssemblerCtx) && NonNull_Pointer(m_ElementMgr) && CallSucc(m_SceneDataSource->LoadObjects()) &&
        CallSucc(AssemblerCtx->GenerateScene(m_ElementMgr, loader))) {
      sim_msg::Location pbRouteInfo;
      // 调用SendPlanningCarData函数
      SendPlanningCarData(0.0 TXSTMARK("unused"), pbRouteInfo);
      if (FLAGS_SimulateWithPnC) {
        TXSTMARK("Ego must be created before topological sort");
        // 调用RegisterPlanningCar函数
        RegisterPlanningCar(pbRouteInfo);
      }

      // 调用m_ElementMgr->Initialize函数
      m_ElementMgr->Initialize(loader);
      LOG(INFO) << _FootPrint_ << ", GenerateScene Success.";
      _isAlive = true;
      return true;
    } else {
      LOG(WARNING) << _FootPrint_ << ", GenerateScene Failure.";
      return false;
    }
  } else {
    LOG(WARNING) << _FootPrint_ << ", Data is Null.";
    return false;
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

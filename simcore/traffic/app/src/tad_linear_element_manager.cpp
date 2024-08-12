// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_linear_element_manager.h"
#include "tx_topological_graph.h"

TX_NAMESPACE_OPEN(TrafficFlow)
TAD_LinearElementManager::txBool TAD_LinearElementManager::Initialize(Base::ISceneLoaderPtr _dataSource) TX_NOEXCEPT {
  using namespace Geometry::Topology;
  m_DataSource = _dataSource;

  txTopologicalSort topoSort;
  txEdgeVec vtxPairVec;
  std::unordered_map<Base::txSysId, Base::ITrafficElementPtr> map_sysId2elemPtr;
  for (auto& refFlowElem : m_vec_VehiclesPtr) {
    // 使用系统ID作为 key，将线性元素指针存储到 map 中
    map_sysId2elemPtr[refFlowElem->SysId()] = refFlowElem;
    // 获取当前线性元素的顶点关系
    auto relationShip = refFlowElem->GetTopologyRelationShip();
    // 如果顶点关系非空
    if (_NonEmpty_(relationShip)) {
      // 将顶点关系添加到 vtxPairVec 中
      vtxPairVec.insert(vtxPairVec.end(), relationShip.begin(), relationShip.end());
      // 如果日志记录开启
      if (FLAGS_LogLevel_SceneLoader) {
        using Geometry::Topology::operator<<;
        std::stringstream ss;
        ss << "TopologyRelationShip " << TX_VARS_NAME(Id, refFlowElem->Id()) << " : ";
        for (const auto& e : relationShip) {
          ss << e;
        }
        LOG(INFO) << ss.str() << std::endl;
      }
    }
  }

  Base::txString errInfo;
  if (CallSucc(topoSort.Initialize(vtxPairVec, Base::ITrafficElement::SysIdCount())) &&
      CallFail(topoSort.HasCycle(errInfo))) {
    TX_MARK("init success & have not self-cycle.");
    std::vector<txTopologicalSort::VertexType> sorted_valid_vtx_sysId_vec, valid_vtx_sysId_vec, invalid_vtx_sysId_vec;
    sorted_valid_vtx_sysId_vec = topoSort.Sort(valid_vtx_sysId_vec, invalid_vtx_sysId_vec);

    // 在日志级别为 SceneLoader 时，输出排序后的有效顶点、有效顶点的系统ID、无效顶点的系统ID 和
    // Base::ITrafficElement::SysIdCount()
    if (FLAGS_LogLevel_SceneLoader) {
      // 将排序后的有效顶点系统ID复制到字符串流中
      std::stringstream ss;
      std::copy(sorted_valid_vtx_sysId_vec.begin(), sorted_valid_vtx_sysId_vec.end(),
                std::ostream_iterator<txTopologicalSort::VertexType>(ss, " "));
      LOG_IF(INFO, sorted_valid_vtx_sysId_vec.size() == Base::ITrafficElement::SysIdCount())
          << "sorted_valid_vtx_sysId_vec : " << ss.str();
      LOG_IF(WARNING, sorted_valid_vtx_sysId_vec.size() != Base::ITrafficElement::SysIdCount())
          << "sorted_valid_vtx_sysId_vec : " << ss.str();

      // 将有效顶点系统ID复制到字符串流中
      ss.str("");
      std::copy(valid_vtx_sysId_vec.begin(), valid_vtx_sysId_vec.end(),
                std::ostream_iterator<txTopologicalSort::VertexType>(ss, " "));
      LOG_IF(INFO, valid_vtx_sysId_vec.size() == Base::ITrafficElement::SysIdCount())
          << "valid_vtx_sysId_vec : " << ss.str();
      LOG_IF(WARNING, valid_vtx_sysId_vec.size() != Base::ITrafficElement::SysIdCount())
          << "valid_vtx_sysId_vec : " << ss.str();

      // 将无效顶点系统ID复制到字符串流中
      ss.str("");
      std::copy(invalid_vtx_sysId_vec.begin(), invalid_vtx_sysId_vec.end(),
                std::ostream_iterator<txTopologicalSort::VertexType>(ss, " "));
      LOG_IF(INFO, 0 == invalid_vtx_sysId_vec.size()) << "invalid_vtx_sysId_vec : " << ss.str();
      LOG_IF(WARNING, 0 != invalid_vtx_sysId_vec.size()) << "invalid_vtx_sysId_vec : " << ss.str();
      LOG(INFO) << "SysIdCount : " << Base::ITrafficElement::SysIdCount();
    }

    std::vector<Base::ITrafficElementPtr> tmp_vec_TrafficFlowElementPtr;
    for (const auto& sortedSysId : sorted_valid_vtx_sysId_vec) {
      // 如果系统ID存在于映射中
      if (map_sysId2elemPtr.end() != map_sysId2elemPtr.find(sortedSysId)) {
        TX_MARK("normal");
        // 将对应的流量元素指针添加到临时数组中
        tmp_vec_TrafficFlowElementPtr.push_back(map_sysId2elemPtr.at(sortedSysId));
      } else {
        LOG(WARNING) << TX_VARS(sortedSysId) << " is not in sorted id vector.";
      }
    }

    std::swap(tmp_vec_TrafficFlowElementPtr, m_vec_VehiclesPtr);
    if (FLAGS_LogLevel_SceneLoader) {
      LOG(INFO) << "simulation order : ";
      // 遍历所有车辆指针
      for (const auto& refElem : m_vec_VehiclesPtr) {
        // 打印车辆ID和系统ID
        LOG(INFO) << TX_VARS_NAME(elemId, refElem->Id()) << TX_VARS_NAME(sysId, refElem->SysId());
      }
    }
  } else {
    LOG(WARNING) << "Topology has Cycle. error info : " << errInfo;
    m_vec_VehiclesPtr.clear();
  }
  // 初始化时生成所有元素
  GenerateAllTrafficElement();
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)

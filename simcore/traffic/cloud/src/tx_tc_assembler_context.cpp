// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_assembler_context.h"
#include <fstream>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tad_ego_vehicle_element.h"
#include "tad_pedestrian_element.h"
#include "tad_signal_element.h"
#include "its_sim.pb.h"
#include "tx_cloud_stand_alone_event_dispatcher.h"
#include "tx_component.h"
#include "tx_spatial_query.h"
#include "tx_tc_ditw_tad_vehicle_ai_element.h"
#include "tx_tc_gflags.h"
#include "tx_tc_tad_element_generator.h"
#include "tx_tc_tad_obstacle_element.h"
#include "tx_tc_tad_pedestrian_element.h"
#include "tx_tc_tad_vehicle_ai_element.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneLoader)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(Scene)

Base::txBool TAD_Cloud_AssemblerContext::Init(Base::ISceneLoaderPtr _loader,
                                              const Base::map_range_t valid_map_range) TX_NOEXCEPT {
  mSceneLoader = _loader;
  if (mSceneLoader) {
    LOG(INFO) << "TAD_Cloud_AssemblerContext::Init";
    Base::ISceneLoader::ITrafficFlowViewerPtr tf_ptr = mSceneLoader->GetTrafficFlow();
    if (NonNull_Pointer(tf_ptr)) {
      mVehicleElementGenerator = std::make_shared<Scene::TAD_Cloud_ElementGenerator>();
      mVehicleElementGenerator->Initialize(_loader, valid_map_range);

      return true;
    } else {
      LOG(WARNING) << "TAD_Cloud_AssemblerContext::Init, ITrafficFlowViewerPtr is null.";
      return false;
    }
  } else {
    LOG(WARNING) << "TAD_Cloud_AssemblerContext::Init, param is null.";
    return false;
  }
}

Base::txBool TAD_Cloud_AssemblerContext::DynamicGenerateScene(Base::TimeParamManager const& timeMgr,
                                                              Base::IElementManagerPtr _elemMgr,
                                                              Base::ISceneLoaderPtr) TX_NOEXCEPT {
  if (mVehicleElementGenerator && _elemMgr) {
    return mVehicleElementGenerator->Generate(timeMgr, _elemMgr);
  }

  return true;
}

Base::txBool TAD_Cloud_AssemblerContext::DynamicChangeScene(Base::TimeParamManager const& timeMgr,
                                                            Base::IElementManagerPtr _elemMgr,
                                                            Base::ISceneLoaderPtr) TX_NOEXCEPT {
  if (mVehicleElementGenerator && _elemMgr) {
    Base::txBool bRet = true;
    bRet = bRet && (mVehicleElementGenerator->Erase(timeMgr, _elemMgr));
    bRet = bRet && (mVehicleElementGenerator->ReRoute(timeMgr, _elemMgr));
    return bRet;
  }

  return true;
}

Base::txInt TAD_Cloud_AssemblerContext::DynamicGenerateTrafficFlowElement(Base::TimeParamManager const&,
                                                                          Base::IElementManagerPtr,
                                                                          Base::ISceneLoaderPtr) TX_NOEXCEPT {
  return false;
}

Base::txInt TAD_Cloud_AssemblerContext::DynamicGenerateTrafficFlowElement_Vehicle(Base::TimeParamManager const&,
                                                                                  Base::IElementManagerPtr,
                                                                                  Base::ISceneLoaderPtr) TX_NOEXCEPT {
  return false;
}

Base::txInt TAD_Cloud_AssemblerContext::DynamicGenerateTrafficFlowElement_Pedestrian(
    Base::TimeParamManager const&, Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT {
  return false;
}

Base::txInt TAD_Cloud_AssemblerContext::DynamicGenerateTrafficEnvironmentElement(Base::TimeParamManager const&,
                                                                                 Base::IElementManagerPtr,
                                                                                 Base::ISceneLoaderPtr) TX_NOEXCEPT {
  return false;
}

Base::txInt TAD_Cloud_AssemblerContext::DynamicGenerateTrafficReferenceElement(Base::TimeParamManager const&,
                                                                               Base::IElementManagerPtr,
                                                                               Base::ISceneLoaderPtr) TX_NOEXCEPT {
  return false;
}

Base::txInt TAD_Cloud_AssemblerContext::GenerateTrafficEnvironmentElement(Base::IElementManagerPtr _elemMgr,
                                                                          Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  /*Traffic Light Data*/
  Base::txInt retCnt = 0;
  auto res = _loader->GetAllSignlightData();
  for (auto& tl : res) {
    TrafficFlow::TAD_SignalLightElementPtr signal_light = std::make_shared<TrafficFlow::TAD_SignalLightElement>();
    if (NonNull_Pointer(signal_light) && signal_light->Initialize(tl.second, _loader)) {
      _elemMgr->AddSignalPtr(signal_light);
#if __TX_Mark__("cloud search circle signal")
      Geometry::SpatialQuery::RTree2D_Signal::getInstance().Insert_Signal(signal_light->GetLocation().ToENU().ENU2D(),
                                                                          signal_light);
      LogInfo << TX_VARS_NAME(signalId, (signal_light->Id())) << TX_VARS_NAME(signalPos, signal_light->GetLocation());
#endif /*USE_RTree*/
      ++retCnt;
    } else {
      LogWarn << " Create Signal Light Element Failure.";
    }
  }
  Base::ISignalLightElement::RoadId2SignalIdSet roadId_with_signal;
  auto valid_signals = _elemMgr->GetAllSignal();
  for (auto signalPtr : valid_signals) {
    auto tad_signal_ptr = std::dynamic_pointer_cast<TrafficFlow::TAD_SignalLightElement>(signalPtr);
    if (NonNull_Pointer(tad_signal_ptr)) {
      roadId_with_signal[tad_signal_ptr->GetCurrentLaneInfo().onLaneUid.roadId].insert(tad_signal_ptr->Id());
    } else {
      LogWarn << " Create Signal Light Element Failure.";
    }
  }
  for (const auto& ref_pair : roadId_with_signal) {
    std::ostringstream oss;
    std::copy(ref_pair.second.begin(), ref_pair.second.end(), std::ostream_iterator<Base::txSysId>(oss, ","));
    LOG(INFO) << "[TAD_Signal_Road]" << TX_VARS_NAME(RoadId, ref_pair.first) << TX_VARS_NAME(SignalIds, oss.str());
  }

  for (auto signalPtr : valid_signals) {
    auto tad_signal_ptr = std::dynamic_pointer_cast<TrafficFlow::TAD_SignalLightElement>(signalPtr);
    if (NonNull_Pointer(tad_signal_ptr)) {
      if (CallFail(tad_signal_ptr->GenerateSignalControlLaneInfoList(roadId_with_signal))) {
        LogWarn << "[TAD_Signal_Road] " << TX_VARS_NAME(SignalId, tad_signal_ptr->Id()) << " initialize failure.";
      } else {
        tad_signal_ptr->CheckStart(Base::TimeParamManager(0, 0, 0, 0));
        LogInfo << "[Signal]" << TX_VARS_NAME(SignalId, tad_signal_ptr->Id()) << " create success.";
      }
    } else {
      LogWarn << " Create Signal Light Element Failure.";
    }
  }
  return retCnt;
}

Base::txInt TAD_Cloud_AssemblerContext::GenerateTrafficFlowElement(Base::IElementManagerPtr _elemMgr,
                                                                   Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;
  retCnt += GenerateTrafficFlowElement_Vehicle(_elemMgr, _loader);
  retCnt += GenerateTrafficFlowElement_Pedestrian(_elemMgr, _loader);
  retCnt += GenerateTrafficFlowElement_Obstacle(_elemMgr, _loader);
  return retCnt;
}

Base::txInt TAD_Cloud_AssemblerContext::GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr _elemMgr,
                                                                           Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;
  auto res = _loader->GetAllVehicleData();
  for (auto& pair_id_viewerPtr : res) {
    Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IVehiclesViewer>(pair_id_viewerPtr.second);
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      const Base::Enums::VEHICLE_BEHAVIOR behavorType = _elemAttrViewPtr->behaviorEnum();
      if ((_plus_(Base::Enums::VEHICLE_BEHAVIOR::eTadAI)) == behavorType ||
          (_plus_(Base::Enums::VEHICLE_BEHAVIOR::eTadAI_Arterial)) == behavorType) {
        TrafficFlow::TAD_Cloud_AI_VehicleElementPtr vehicle =
            std::make_shared<TrafficFlow::TAD_Cloud_AI_VehicleElement>();
        if (NonNull_Pointer(vehicle) && CallSucc(vehicle->Initialize(pair_id_viewerPtr.second, _loader))) {
          _elemMgr->AddVehiclePtr(vehicle);
          LogInfo << "[vehicle_event][TadAI] vehicle_id:" << (*vehicle).Id();
          ++retCnt;
        } else {
          LogWarn << " Create Vehicle Element Failure.";
        }
      }
    } else {
      LogWarn << "dynamic_pointer_cast IVehiclesViewer error.";
    }
  }
  LogInfo << "[Create_Vehicle]_Count = " << retCnt;
  return retCnt;
}

Base::txInt TAD_Cloud_AssemblerContext::GenerateTrafficFlowElement_Pedestrian(
    Base::IElementManagerPtr _elemMgr, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  Base::txInt retCnt = 0;
  auto res = _loader->GetAllPedestrianData();
  for (auto& entity : res) {
    TrafficFlow::TAD_PedestrianElementPtr pedestrian = std::make_shared<TrafficFlow::TAD_PedestrianElement>();
    if (NonNull_Pointer(pedestrian) && CallSucc(pedestrian->Initialize(entity.second, _loader))) {
      _elemMgr->AddPedestrianPtr(pedestrian);
      // LOG(INFO) << "[pedestrian]" << (*pedestrian); txPAUSE;
      auto tad_pede_viewptr = pedestrian->getPedestriansViewPtr();
      if (tad_pede_viewptr) {
        if (tad_pede_viewptr->hadDirection()) {
          TX_MARK("direction1 direction1_t direction1_v ");
          LOG(FATAL) << "deprecated pedestrian scene.";
        } else {
          TrafficFlow::TAD_SceneEvent_1_0_0_0_Ptr sceneEventPtr =
              std::dynamic_pointer_cast<TrafficFlow::TAD_SceneEvent_1_0_0_0>(pedestrian);
          if (NonNull_Pointer(sceneEventPtr) && CallSucc(sceneEventPtr->UseSceneEvent_1_0_0_0()) &&
              CallSucc(sceneEventPtr->HasSceneEvent())) {
            LogInfo << "[pedestrian_event][scene_event_1.0.0.0] pede_id:" << (*pedestrian).Id()
                    << TX_VARS_NAME(sceneEventCnt, sceneEventPtr->SceneEvent().size());
            TrafficFlow::CloudEventDispatcher::getInstance().RegisterVehicleSceneEvent(sceneEventPtr->SceneEvent(),
                                                                                       sceneEventPtr);
          } else {
            LogWarn << "[pedestrian_event][scene_event_1.0.0.0] pede_id:" << (*pedestrian).Id()
                    << " sceneEvent_1.0.0.0 is null.";
          }
        }
      }
      ++retCnt;
    } else {
      LogWarn << " Create Pedestrian Element Failure.";
    }
  }
  LogInfo << "[Create_Pedestrian]_Count = " << retCnt;
  return retCnt;
}

Base::txInt TAD_Cloud_AssemblerContext::GenerateTrafficFlowElement_Obstacle(Base::IElementManagerPtr _elemMgr,
                                                                            Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  if (CallFail(FLAGS_need_random_obs_pedes)) {
    return 0;
  }
  Base::txInt retCnt = 0;
  const Base::txInt rnd_seed = FLAGS_obs_rnd_seed;
  const Base::txFloat valid_section_length = FLAGS_obs_valid_section_length;
  const Base::txFloat valid_radius = FLAGS_rnd_obs_valid_radius;
  Geometry::SpatialQuery::RTree2DLite space_dict;
  /*if (NonNull_Pointer(mVehicleElementGenerator)) {
          auto input_region_pt_vec = mVehicleElementGenerator->InputRegionLocations();
          for (auto refPt : input_region_pt_vec) {
                  space_dict.RegisterPoint(refPt.ToENU().ENU2D(),
  Geometry::SpatialQuery::RTree2DLite::RTree2DLiteNode::NodeType::eVehicleInput, Base::Info_Lane_t());
          }
  }*/
  Base::Component::Pseudorandom rnd;
  rnd.Initialize(0, rnd_seed);
  tbb::concurrent_vector<hadmap::txSectionPtr> con_vec_secPtr;

  using id2roadPtrType = HdMap::HadmapCacheConCurrent::id2roadPtrType;
  const id2roadPtrType& refAllRoads = HdMap::HadmapCacheConCurrent::getAllRoads();
  LOG(INFO) << TX_VARS(refAllRoads.size());
  tbb::parallel_for(refAllRoads.range(), [&](const id2roadPtrType::const_range_type& r) {
    for (id2roadPtrType::const_iterator itr = r.begin(); itr != r.end(); itr++) {
      if (NonNull_Pointer(itr->second)) {
        const auto& refSectionVec = itr->second->getSections();
        for (const auto& refSection : refSectionVec) {
          if (refSection->getLength() > valid_section_length) {
            con_vec_secPtr.push_back(refSection);
          }
        }
      }
    }
  });

  for (Base::txSize secIdx = 0; secIdx < con_vec_secPtr.size(); ++secIdx) {
    const auto& refSecPtr = con_vec_secPtr[secIdx];
    if ((refSecPtr->getLanes().size()) > 2) {
      const Base::txSysId rnd_obs_id = 8800000 + secIdx;
      const Base::txInt seed = rnd.GetRandomInt();
      const Base::txInt laneIdx = seed % refSecPtr->getLanes().size();
      auto select_lane_ptr = refSecPtr->getLanes().at(laneIdx);
      if (Utils::IsLaneValidDriving(select_lane_ptr)) {
        auto geomPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(Base::Info_Lane_t(select_lane_ptr->getTxLaneId()));
        if (NonNull_Pointer(geomPtr)) {
          const Base::txFloat rnd_s = (geomPtr->GetLength() * 0.5 * rnd.GetRandomValue());
          Coord::txWGS84 lanePt;
          lanePt.FromENU(geomPtr->GetLocalPos(rnd_s));
          const Base::txVec2 lanePt2d = lanePt.ToENU().ENU2D();
          if (0 == space_dict.CountElementInCircle(lanePt2d, geomPtr->LaneLocInfo(), valid_radius)) {
            TrafficFlow::TAD_Cloud_ObstacleElementPtr obstacle =
                std::make_shared<TrafficFlow::TAD_Cloud_ObstacleElement>();
            if (obstacle && obstacle->InitializeOnCloud(rnd_obs_id, lanePt, _loader)) {
              _elemMgr->AddObstaclePtr(obstacle);
              space_dict.RegisterPoint(lanePt2d,
                                       Geometry::SpatialQuery::RTree2DLite::RTree2DLiteNode::NodeType::eObstacle,
                                       geomPtr->LaneLocInfo());
              ++retCnt;
            } else {
              LOG(WARNING) << ", Create Obstacle Element Failure.";
            }
          }
        }
      }
    }
  }
  return retCnt;
}

Base::txBool TAD_Cloud_AssemblerContext::GenerateScene(Base::IElementManagerPtr _elemMgr,
                                                       Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  if (NonNull_Pointer(_elemMgr) && NonNull_Pointer(_loader)) {
    Base::txInt const retCnt_FlowElem = GenerateTrafficFlowElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_FlowElem > 0) << "GenerateTrafficFlowElement " << retCnt_FlowElem;
    LOG_IF(WARNING, retCnt_FlowElem <= 0) << "GenerateTrafficFlowElement " << retCnt_FlowElem;

    const Base::txInt retCnt_EnvElem = GenerateTrafficEnvironmentElement(_elemMgr, _loader);
    LOG_IF(INFO, retCnt_EnvElem > 0) << "GenerateTrafficEnvironmentElement " << retCnt_EnvElem;
    LOG_IF(WARNING, retCnt_EnvElem <= 0) << "GenerateTrafficEnvironmentElement " << retCnt_EnvElem;

    return true;
  } else {
    LOG(WARNING) << " Param Error.";
    return false;
  }
}

Base::txBool TAD_Cloud_AssemblerContext::GenerateEgo(Base::IElementManagerPtr _elemMgr,
                                                     Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  if (_elemMgr && _loader) {
    const auto egoType = _loader->GetEgoType();
    TX_MARK("enum class EgoType : txInt { Truck, Vehicle, Cloud };");
    auto egoDataPtr = _loader->GetEgoData();
    TrafficFlow::TAD_EgoVehicleElementPtr ego = std::make_shared<TrafficFlow::TAD_EgoVehicleElement>();

    if (NonNull_Pointer(ego) && ego->InitializeOnCloud(egoDataPtr, _loader)) {
      _elemMgr->AddEgoPtr(ego);

      LOG(INFO) << " Create Vehicle Element Success.";
      return true;
    } else {
      LOG(WARNING) << " Create Vehicle Element Failure.";
      return false;
    }
  } else {
    LOG(WARNING) << " Param Error.";
    return false;
  }
}

Base::txBool TAD_Cloud_AssemblerContext::ReSetInputAgent(const std::vector<Base::map_range_t>& vec_valid_sim_range)
    TX_NOEXCEPT {
  if (NonNull_Pointer(mVehicleElementGenerator)) {
    Scene::TAD_Cloud_ElementGeneratorPtr elem_gen_ptr =
        std::dynamic_pointer_cast<Scene::TAD_Cloud_ElementGenerator>(mVehicleElementGenerator);
    if (NonNull_Pointer(elem_gen_ptr)) {
      return (elem_gen_ptr->ReSetInputAgent(vec_valid_sim_range));
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txBool TAD_Cloud_AssemblerContext::TestSerialization(const Base::TimeParamManager& timeMgr,
                                                           Base::IElementManagerPtr _elemMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(_elemMgr)) {
#if 1
    using sysId2Serialization = tbb::concurrent_hash_map<Base::txSysId, its::txVehicle>;
    using OutputArchive = cereal::BinaryOutputArchive;
    using InputArchive = cereal::BinaryInputArchive;
    sysId2Serialization map_sysId2String;
    auto& refAllVehicle = _elemMgr->GetAllVehiclePtr();
    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicle.size(), [&](const std::size_t idx) {
      TrafficFlow::TAD_Cloud_AI_VehicleElementPtr cloud_ai_veh_ptr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_Cloud_AI_VehicleElement>(refAllVehicle[idx]);
      if (NonNull_Pointer(cloud_ai_veh_ptr)) {
        {
          std::stringstream oss;
          oss << "./log/serialization/" << (cloud_ai_veh_ptr->SysId()) << "_" << timeMgr.AbsTime() << "_first.json";
          std::ofstream os(oss.str());
          os << cloud_ai_veh_ptr->json_archive();
          os.close();
        }

        sysId2Serialization::accessor a;
        map_sysId2String.insert(a, cloud_ai_veh_ptr->SysId());
        a->second.set_serialization_archive(cloud_ai_veh_ptr->binary_archive());
        // *(a->second.mutable_serialization_archive()) = ss.str();
      }
    }); /*lamda function*/
    /* parallel_for */

    for (const auto& ref : map_sysId2String) {
      LOG(INFO) << TX_VARS_NAME(SysId, ref.first) << TX_VARS_NAME(size, ref.second.serialization_archive().size());
    }

    tbb::parallel_for(static_cast<std::size_t>(0), refAllVehicle.size(), [&](const std::size_t idx) {
      TrafficFlow::TAD_Cloud_AI_VehicleElementPtr cloud_ai_veh_ptr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_Cloud_AI_VehicleElement>(refAllVehicle[idx]);
      if (NonNull_Pointer(cloud_ai_veh_ptr)) {
        sysId2Serialization::const_accessor ca;
        if (CallSucc(map_sysId2String.find(ca, cloud_ai_veh_ptr->SysId()))) {
          std::stringstream ss(ca->second.serialization_archive());
          InputArchive arin(ss);
          arin(*cloud_ai_veh_ptr);
          {
            std::stringstream oss;
            oss << "D:/log/serialization/" << (cloud_ai_veh_ptr->SysId()) << "_" << timeMgr.AbsTime() << "_second.json";
            std::ofstream os(oss.str());
            os << cloud_ai_veh_ptr->json_archive();
            os.close();
          }
        }
      }
    }); /*lamda function*/
        /* parallel_for */
#endif
    return true;
  } else {
    return false;
  }
}

void TAD_Cloud_AssemblerContext::Generate3rdVehicle(const Base::TimeParamManager& timeMgr, const ObjectInfo& _3rdData,
                                                    Base::IElementManagerPtr _elemMgr,
                                                    Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
  TrafficFlow::DITW_TAD_Cloud_AI_VehicleElementPtr ditw_cloud_vehicle_ptr =
      std::make_shared<TrafficFlow::DITW_TAD_Cloud_AI_VehicleElement>();
  if (NonNull_Pointer(ditw_cloud_vehicle_ptr) && ditw_cloud_vehicle_ptr->Initialize(timeMgr, _3rdData, _loader)) {
    _elemMgr->AddVehiclePtr(ditw_cloud_vehicle_ptr);
    LOG_IF(INFO, FLAGS_LogLevel_CloudDitw) << "[vehicle_event] vehicle_id:" << (ditw_cloud_vehicle_ptr->Id());
  } else {
    LogWarn << "Create Vehicle Element Failure.";
  }
}

TX_NAMESPACE_CLOSE(Scene)

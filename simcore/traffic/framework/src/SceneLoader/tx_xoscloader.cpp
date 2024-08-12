// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "SceneLoader/tx_xoscloader.h"
#if 0
#  include <boost/algorithm/string.hpp>
#  include "HdMap/txHdMapCacheConcurrent.h"
#  include "tinyxml2.h"
#  include "txMath.h"
#  include "txPathUtils.h"
#  include "txProfileUtils.h"
#  include "txStringUtils.h"
#  include "txTimerOnCPU.h"
#  define LOG_INFO LOG_IF(INFO, FLAGS_LogLevel_SceneReader || FLAGS_LogLevel_XOSC)
#  define LOG_WARN LOG(WARNING)
#  define XOSC_FLOAT_PRECISION (3)
#  define IsParameterDeclaration(_StrVarName_) (_NonEmpty_(_StrVarName_) && CallSucc('$' == _StrVarName_[0]))
#  define StrToRetValue(__var__, __true_value__, __false_value__) \
    (_NonEmpty_(__var__) ? (__true_value__) : (__false_value__))

TX_NAMESPACE_OPEN(Math)
Base::txFloat StringToDouble_LP(const Base::txString& inStr, const Base::txUInt inPrecision) TX_NOEXCEPT {
  std::stringstream streamObj3;
  streamObj3 << std::fixed;
  streamObj3 << std::setprecision(inPrecision);
  streamObj3 << stod(inStr);
  Base::txFloat retV = 0.0;
  if (!(streamObj3 >> retV)) {
    retV = 0.0;
  }
  return retV;
}
TX_NAMESPACE_CLOSE(Math)

inline Base::txFloat StrToFloatValueWithPrecision(const Base::txString& __var__,
                                                  const Base::txFloat __false_value__ = 0.0) TX_NOEXCEPT {
  return (_NonEmpty_(__var__) ? (Math::StringToDouble_LP(__var__, XOSC_FLOAT_PRECISION)) : (__false_value__));
}

TX_NAMESPACE_OPEN(SceneLoader)

OSC_SceneLoader::OSC_SceneLoader() : _class_name(__func__) {}

OSC_SceneLoader::VehicleBehavior OSC_SceneLoader::Str2Behavior(const txString& strBehavior) TX_NOEXCEPT {
  if (boost::iequals("TrafficVehicle", strBehavior)) {
    return VehicleBehavior::eTadAI;
  } else if (boost::iequals("TrafficVehicleArterial", strBehavior)) {
    return VehicleBehavior::eTadAI_Arterial;
  } else if (boost::iequals("UserDefine", strBehavior)) {
    return VehicleBehavior::eUserDefined;
  } else {
    LOG_WARN << "unknown behavior : " << quoted(strBehavior);
    return VehicleBehavior::eUserDefined;
  }
}

bool OSC_SceneLoader::IsValid() TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  return (_plus_(EStatus::eReady == Status())) && (exists(FilePath(_source)));
}

OSC_SceneLoader::txBool OSC_SceneLoader::EntityHasSpecialFirstChar(const txString& refEntity,
                                                                   const txString::value_type sCh) TX_NOEXCEPT {
  if (_NonEmpty_(refEntity)) {
    return (refEntity[0] == sCh);
  } else {
    return false;
  }
}

OSC_SceneLoader::txBool OSC_SceneLoader::EntityIsVechile(const txString& refEntity) TX_NOEXCEPT {
  return EntityHasSpecialFirstChar(refEntity, 'V');
}

OSC_SceneLoader::txBool OSC_SceneLoader::EntityIsVechileEgo(const txString& refEntity) TX_NOEXCEPT {
  return EntityHasSpecialFirstChar(refEntity, 'E');
}

OSC_SceneLoader::txBool OSC_SceneLoader::EntityIsPedestrian(const txString& refEntity) TX_NOEXCEPT {
  return EntityHasSpecialFirstChar(refEntity, 'P') || EntityHasSpecialFirstChar(refEntity, 'A') ||
         EntityHasSpecialFirstChar(refEntity, 'B') || EntityHasSpecialFirstChar(refEntity, 'M');
}

OSC_SceneLoader::txBool OSC_SceneLoader::EntityIsObstacle(const txString& refEntity) TX_NOEXCEPT {
  return EntityHasSpecialFirstChar(refEntity, 'O');
}

Base::txBool OSC_SceneLoader::GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
    TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_XOSC)) {
    refParams.strTrafficFilePath = _source;
    refParams.strHdMapFilePath = m_xodr_file_path;
    LOG(INFO) << _StreamPrecision_ << "xosc file : " << refParams.strTrafficFilePath << std::endl
              << ", xodr file : " << refParams.strHdMapFilePath;

    ParseParameterDeclaration(m_DataSource_XOSC->ParameterDeclarations);

    const auto& sbo = m_DataSource_XOSC->Storyboard;
    if (sbo.IsInited()) {
      const auto& sb = sbo;
      const auto& i = sb.Init;
      const auto& ias = i.Actions;
      const auto& ps = ias.Private;
      for (auto p : ps) {
        const txString& strEntityRef = p.entityRef;
        if ("Ego" == strEntityRef) {
          const auto& pas = p.PrivateAction;
          for (auto pa : pas) {
            if (pa.TeleportAction.IsInited()) {
              if (pa.TeleportAction.Position.WorldPosition.IsInited()) {
                txString strH, strX, strY, strZ;
                ReadTeleportAction_WorldPosition(pa, strX, strY, strZ, strH);

                m_EgoStartPos = refParams.SceneOriginGPS = m_proj_param.ConvertXODR2LonLat(strX, strY);

                m_EgoStartHeading = StrToFloatValueWithPrecision(strH);
                LOG_INFO << TX_VARS_NAME(SceneReferencePoint, (m_EgoStartPos));
              }
            } else if (pa.LongitudinalAction.IsInited() &&
                       pa.LongitudinalAction.SpeedAction.SpeedActionTarget.AbsoluteTargetSpeed.IsInited()) {
              txString strInitSpeed = pa.LongitudinalAction.SpeedAction.SpeedActionTarget.AbsoluteTargetSpeed.value;

              if (IsParameterDeclaration(strInitSpeed)) {
                strInitSpeed = FindParameterDeclarationsValudeRet(strInitSpeed);
              }

              m_EgoStartSpeed = StrToFloatValueWithPrecision(strInitSpeed);
            }
          }
          LOG_INFO << "XOSC Ego Init Info : " << TX_VARS_NAME(m_EgoStartPos, Utils::ToString(m_EgoStartPos))
                   << TX_VARS(m_EgoStartHeading) << TX_VARS(m_EgoStartSpeed);
          return true;
        }
      }
      LOG_WARN << "m_DataSource_XOSC do not have ego info.";
      return false;
    } else {
      LOG_WARN << "m_DataSource_XOSC->Storyboard is null.";
      return false;
    }
  } else {
    LOG_WARN << "m_DataSource_XOSC is null.";
    return false;
  }
}

bool OSC_SceneLoader::LoadObjects() TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_XOSC)) {
    ParseFileHeader(m_DataSource_XOSC->FileHeader);

    // parameter declaration
    auto& pdo = m_DataSource_XOSC->ParameterDeclarations;
    if (pdo.IsInited()) {
      ParseParameterDeclaration(pdo);
    }

    // entities
    auto& entities = m_DataSource_XOSC->Entities;
    if (CallFail(entities.empty())) {
      ParseEntities(entities);
    }

    // storyboard
    auto& sbo = m_DataSource_XOSC->Storyboard;
    if (sbo.IsInited()) {
      ParseStoryboard(sbo);
    }
    return true;
  } else {
    LOG_WARN << "loading xosc " << _source << " failure.";
    return false;
  }
}

bool OSC_SceneLoader::Load(const txString& str_xosc_file_path) TX_NOEXCEPT {
  LOG_INFO << TX_VARS(str_xosc_file_path);
  Clear();
  using namespace Utils;
  using namespace boost::filesystem;
  _source = str_xosc_file_path;
  if (exists(FilePath(_source))) {
    _status = EStatus::eLoading;
    Base::TimingCPU timer;
    timer.StartCounter();
    m_DataSource_XOSC = SceneLoader::OSC_TADSim::load_scene_tadsim_xosc(_source);
    LOG_INFO << "Loading TAD Scene File : " << _source << TX_VARS_NAME(Cost_Time, timer.GetElapsedMilliseconds());
    if (NonNull_Pointer(m_DataSource_XOSC)) {
      m_xodr_file_path = m_DataSource_XOSC->RoadNetwork.LogicFile.filepath;
      using namespace Utils;
      using namespace boost::filesystem;
      FilePath path_CurrentPath = FilePath(_source).parent_path();
      m_xodr_file_path = path_CurrentPath.string() + "/" + m_xodr_file_path;
      LOG_INFO << TX_VARS(m_xodr_file_path);
      m_proj_param = GetXodrProjParam(m_xodr_file_path);
      LOG(INFO) << TX_VARS_NAME(xodr_map_proj_param, m_proj_param.Str());
      return true;
    } else {
      LOG(WARNING) << "Loading TAD Scene File : " << _source << " Except.";
      _status = EStatus::eError;
      return false;
    }
  } else {
    LOG(FATAL) << "Scene File is not existing. File : " << _source;
    _status = EStatus::eError;
    return false;
  }
}

void OSC_SceneLoader::ReadLongitudinalAction_SpeedAction_Absolute(PrivateAction& pa, txString& strDynamicsShape,
                                                                  txString& strValue, txString& strDynamicsDimension,
                                                                  txString& strSpeedValue) TX_NOEXCEPT {
  strSpeedValue = pa.LongitudinalAction.SpeedAction.SpeedActionTarget.AbsoluteTargetSpeed.value;

  if (IsParameterDeclaration(strSpeedValue)) {
    strSpeedValue = FindParameterDeclarationsValudeRet(strSpeedValue);
  }

  strDynamicsShape = pa.LongitudinalAction.SpeedAction.SpeedActionDynamics.dynamicsShape;

  strValue = pa.LongitudinalAction.SpeedAction.SpeedActionDynamics.value;
  if (IsParameterDeclaration(strValue)) {
    strValue = FindParameterDeclarationsValudeRet(strValue);
  }
  strDynamicsDimension = pa.LongitudinalAction.SpeedAction.SpeedActionDynamics.dynamicsDimension;
}

void OSC_SceneLoader::AddIniSpeed(txString& strEntityRef, txString& strSpeedV) TX_NOEXCEPT {
  if (EntityIsVechile(strEntityRef)) {
    auto itr = m_mapVehicles.find(strEntityRef);
    if (itr != m_mapVehicles.end()) {
      itr->second.m_strStartVelocity = strSpeedV;
    } else {
      LOG_WARN << "vehicle " << strEntityRef << " not find in entities";
      return;
    }
  } else if (EntityIsPedestrian(strEntityRef)) {
    auto itr = m_mapPedestrians.find(strEntityRef);
    if (itr != m_mapPedestrians.end()) {
      itr->second.m_strStartVelocity = strSpeedV;
    } else {
      LOG_WARN << "pedestrian " << strEntityRef << " not find in entities";
      return;
    }
  } else {
    LOG_WARN << "unknown entity " << strEntityRef << " not find in entities";
    return;
  }
}

void OSC_SceneLoader::ReadTeleportAction_WorldPosition(PrivateAction& pa, txString& strX, txString& strY,
                                                       txString& strZ, txString& strH) TX_NOEXCEPT {
  strX = pa.TeleportAction.Position.WorldPosition.x;
  strY = pa.TeleportAction.Position.WorldPosition.y;
  if (CallFail(pa.TeleportAction.Position.WorldPosition.z.empty())) {
    strZ = pa.TeleportAction.Position.WorldPosition.z;
  }

  if (CallFail(pa.TeleportAction.Position.WorldPosition.h.empty())) {
    strH = pa.TeleportAction.Position.WorldPosition.h;
  }

  if (IsParameterDeclaration(strX)) {
    strX = FindParameterDeclarationsValudeRet(strX);
  }

  if (IsParameterDeclaration(strY)) {
    strY = FindParameterDeclarationsValudeRet(strY);
  }

  if (IsParameterDeclaration(strZ)) {
    strZ = FindParameterDeclarationsValudeRet(strZ);
  }

  if (IsParameterDeclaration(strH)) {
    strH = FindParameterDeclarationsValudeRet(strH);
  }
}

void OSC_SceneLoader::ConvertLonLat2SimPosition(const txFloat dLon, const txFloat dLat,
                                                sTagQueryInfoResult& result) TX_NOEXCEPT {
  result.lanelinkID = -1;
  result.laneUid.laneId = 0;
  result.laneUid.sectionId = 0;
  result.laneUid.roadId = 0;
  txFloat dLaneDist = 0;
  txFloat dLaneOffset = 0;
  txFloat dLanelinkDist = 0;
  txFloat dLanelinkOffset = 0;

  txFloat dist_unuse = 0.0;
  hadmap::txLanePtr pLane = HdMap::HadmapCacheConCurrent::GetLaneForInit(dLon, dLat, dLaneDist, dLaneOffset);

  hadmap::txLaneLinkPtr pLaneLink =
      HdMap::HadmapCacheConCurrent::GetLaneLinkForInit(dLon, dLat, dLanelinkDist, dLanelinkOffset);

  if (Null_Pointer(pLane) && Null_Pointer(pLaneLink)) {
    LOG_WARN << TX_VARS(dLon) << TX_VARS(dLat) << " no lane and lanelink!";
    return;
  } else if (NonNull_Pointer(pLane) && Null_Pointer(pLaneLink)) {
    result.eType = sTagQueryInfoResult::LocationType::onLane;
    result.laneUid = pLane->getTxLaneId();
    result.dStartPoint = pLane->getGeometry()->getStart();
    result.dShift = dLaneDist;
    result.dOffset = dLaneOffset;
  } else if (Null_Pointer(pLane) && NonNull_Pointer(pLaneLink)) {
    result.eType = sTagQueryInfoResult::LocationType::onLink;
    result.lanelinkID = pLaneLink->getId();
    result.dStartPoint = pLaneLink->getGeometry()->getStart();
    result.dShift = dLanelinkDist;
    result.dOffset = dLanelinkOffset;
  } else if (NonNull_Pointer(pLane) && NonNull_Pointer(pLaneLink)) {
    if (std::fabs(dLaneDist) < std::fabs(dLanelinkDist)) {
      result.eType = sTagQueryInfoResult::LocationType::onLane;
      result.laneUid = pLane->getTxLaneId();
      result.dStartPoint = pLane->getGeometry()->getStart();
      result.dShift = dLaneDist;
      result.dOffset = dLaneOffset;
    } else {
      result.eType = sTagQueryInfoResult::LocationType::onLink;
      result.lanelinkID = pLaneLink->getId();
      result.dStartPoint = pLaneLink->getGeometry()->getStart();
      result.dShift = dLanelinkDist;
      result.dOffset = dLanelinkOffset;
    }
  } else {
    LOG_WARN << TX_VARS(dLon) << TX_VARS(dLat) << " no lane and lanelink!";
    return;
  }
}

void OSC_SceneLoader::AddIniWordPos(txString& strEntityRef, txString& strX, txString& strY, txString& strZ,
                                    txString& strH) TX_NOEXCEPT {
  SimRoute TempRoute;
  TempRoute.m_strRouteId = Utils::IntToString(m_nRouteIDGenerator++);

  txFloat dLon = std::stod(strX);
  txFloat dLat = std::stod(strY);
  txFloat dAlt = std::stod(strZ);

  const hadmap::txPoint tmpP = m_proj_param.ConvertXODR2LonLat(dLon, dLat);
  dLon = tmpP.x;
  dLat = tmpP.y;

  /*TempRoute.m_strStartLon = Utils::FloatToString(dLon);
  TempRoute.m_strStartLat = Utils::FloatToString(dLat);
  TempRoute.m_strStartAlt = strZ;
  TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;*/

  if (EntityIsVechile(strEntityRef)) {
    sTagQueryInfoResult result;
    ConvertLonLat2SimPosition(dLon, dLat, result);

    TempRoute.m_strStartLon = Utils::FloatToString(result.dStartPoint.x);
    TempRoute.m_strStartLat = Utils::FloatToString(result.dStartPoint.y);
    TempRoute.m_strStartAlt = Utils::FloatToString(result.dStartPoint.z);
    TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;

    auto itr = m_mapVehicles.find(strEntityRef);
    if (itr != m_mapVehicles.end()) {
      itr->second.m_strLaneID = std::to_string((result.IsOnLane() ? (result.laneUid.laneId) : (0)));
      itr->second.m_strStartShift = Utils::FloatToString(result.dShift);
      itr->second.m_strOffset = Utils::FloatToString(result.dOffset);

      if (CallSucc(itr->second.m_strRouteID.empty())) {
        itr->second.m_strRouteID = TempRoute.m_strRouteId;
      } else {
        LOG_WARN << "vehicle route already exist!" << TX_VARS(strEntityRef)
                 << TX_VARS_NAME(routId, itr->second.m_strRouteID);
        return;
      }
    } else {
      LOG_WARN << strEntityRef << " vehicle not exist!";
      return;
    }
  } else if (EntityIsPedestrian(strEntityRef)) {
    sTagQueryInfoResult result;
    ConvertLonLat2SimPosition(dLon, dLat, result);

    TempRoute.m_strStartLon = Utils::FloatToString(result.dStartPoint.x);
    TempRoute.m_strStartLat = Utils::FloatToString(result.dStartPoint.y);
    TempRoute.m_strStartAlt = Utils::FloatToString(result.dStartPoint.z);
    TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;

    auto itr = m_mapPedestrians.find(strEntityRef);
    if (itr != m_mapPedestrians.end()) {
      itr->second.m_strLaneID = std::to_string((result.IsOnLane() ? (result.laneUid.laneId) : (0)));
      itr->second.m_strStartShift = Utils::FloatToString(result.dShift);
      itr->second.m_strOffset = Utils::FloatToString(result.dOffset);
      if (CallSucc(itr->second.m_strRouteID.empty())) {
        itr->second.m_strRouteID = TempRoute.m_strRouteId;
      } else {
        LOG_WARN << "pedestrian route already exist!" << TX_VARS(strEntityRef)
                 << TX_VARS_NAME(routId, itr->second.m_strRouteID);
        return;
      }
    } else {
      LOG_WARN << strEntityRef << " pedestrian not exist!";
      return;
    }
  } else if (EntityIsObstacle(strEntityRef)) {
    sTagQueryInfoResult result;
    ConvertLonLat2SimPosition(dLon, dLat, result);

    TempRoute.m_strStartLon = Utils::FloatToString(result.dStartPoint.x);
    TempRoute.m_strStartLat = Utils::FloatToString(result.dStartPoint.y);
    TempRoute.m_strStartAlt = Utils::FloatToString(result.dStartPoint.z);
    TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;

    auto itr = m_mapObstacles.find(strEntityRef);
    if (itr != m_mapObstacles.end()) {
      itr->second.m_strLaneID = std::to_string((result.IsOnLane() ? (result.laneUid.laneId) : (0)));
      itr->second.m_strStartShift = Utils::FloatToString(result.dShift);
      itr->second.m_strOffset = Utils::FloatToString(result.dOffset);
      itr->second.m_strDirection = strH;
      if (CallSucc(itr->second.m_strRouteID.empty())) {
        itr->second.m_strRouteID = TempRoute.m_strRouteId;
      } else {
        LOG_WARN << "obstacle route already exist!" << TX_VARS(strEntityRef)
                 << TX_VARS_NAME(routId, itr->second.m_strRouteID);
        return;
      }
    } else {
      LOG_WARN << strEntityRef << " obstacle not exist!";
      return;
    }
  } else {
    LOG_WARN << strEntityRef << " Wrong entity!";
    return;
  }

  m_mapRoutes.insert(std::make_pair(TempRoute.m_strRouteId, TempRoute));
}

void OSC_SceneLoader::ReadLateralAction_LaneChangeAction(PrivateAction& pa, txString& strDynamicsShape,
                                                         txString& strValue, txString& strDynamicsDimension,
                                                         txString& strTargetLane,
                                                         txString& strTargetOffset) TX_NOEXCEPT {
  auto& targetLaneOffset = pa.LateralAction.LaneChangeAction.targetLaneOffset;
  if (CallFail(targetLaneOffset.empty())) {
    strTargetOffset = targetLaneOffset;
  } else {
    strTargetOffset = "0";
  }

  strTargetLane = pa.LateralAction.LaneChangeAction.LaneChangeTarget.RelativeTargetLane.value;

  if (IsParameterDeclaration(strTargetLane)) {
    strTargetLane = FindParameterDeclarationsValudeRet(strTargetLane);
  }

  strDynamicsShape = pa.LateralAction.LaneChangeAction.LaneChangeActionDynamics.dynamicsShape;

  strValue = pa.LateralAction.LaneChangeAction.LaneChangeActionDynamics.value;

  if (IsParameterDeclaration(strValue)) {
    strValue = FindParameterDeclarationsValudeRet(strValue);
  }

  strDynamicsDimension = pa.LateralAction.LaneChangeAction.LaneChangeActionDynamics.dynamicsDimension;
}

void OSC_SceneLoader::ReadRoutingAction(PrivateAction& pa, const RoutingActionType type, sPathPoints& Vpoints,
                                        txString& Orientation) TX_NOEXCEPT {
  if (type == RoutingActionType::AssignRouteAction_Type) {
    const auto& wps = pa.RoutingAction.AssignRouteAction.Route.Waypoint;

    for (auto iter = wps.begin(); iter != wps.end(); iter++) {
      sPathPoint TempsPathPoint;
      TempsPathPoint.m_strLon = iter->Position.WorldPosition.x;
      TempsPathPoint.m_strLat = iter->Position.WorldPosition.y;

      if (CallFail(iter->Position.WorldPosition.z.empty())) {
        TempsPathPoint.m_strAlt = iter->Position.WorldPosition.z;
      }

      if (IsParameterDeclaration(TempsPathPoint.m_strLon)) {
        TempsPathPoint.m_strLon = FindParameterDeclarationsValudeRet(TempsPathPoint.m_strLon);
      }

      if (IsParameterDeclaration(TempsPathPoint.m_strLat)) {
        TempsPathPoint.m_strLat = FindParameterDeclarationsValudeRet(TempsPathPoint.m_strLat);
      }

      if (IsParameterDeclaration(TempsPathPoint.m_strAlt)) {
        TempsPathPoint.m_strAlt = FindParameterDeclarationsValudeRet(TempsPathPoint.m_strAlt);
      }

      Vpoints.push_back(TempsPathPoint);
    }
  }

  if (type == RoutingActionType::FollowTrajectoryAction_Type) {
    const auto& vs = pa.RoutingAction.FollowTrajectoryAction.Trajectory.Shape.Polyline.Vertexs;

    auto iter = vs.begin();
    if (iter->Position.WorldPosition.IsValid()) {
      for (; iter != vs.end(); iter++) {
        sPathPoint TempsPathPoint;

        TempsPathPoint.m_strLon = iter->Position.WorldPosition.x;
        TempsPathPoint.m_strLat = iter->Position.WorldPosition.y;

        if (CallFail(iter->Position.WorldPosition.z.empty())) {
          TempsPathPoint.m_strAlt = iter->Position.WorldPosition.z;
        }

        if (IsParameterDeclaration(TempsPathPoint.m_strLon)) {
          TempsPathPoint.m_strLon = FindParameterDeclarationsValudeRet(TempsPathPoint.m_strLon);
        }

        if (IsParameterDeclaration(TempsPathPoint.m_strLat)) {
          TempsPathPoint.m_strLat = FindParameterDeclarationsValudeRet(TempsPathPoint.m_strLat);
        }

        if (IsParameterDeclaration(TempsPathPoint.m_strAlt)) {
          TempsPathPoint.m_strAlt = FindParameterDeclarationsValudeRet(TempsPathPoint.m_strAlt);
        }

        Vpoints.push_back(TempsPathPoint);
      }

      // pedestrian velocity direction
    } else if (iter->Position.RelativeObjectPosition.IsValid()) {
      for (; iter != vs.end(); iter++) {
        if (CallFail(iter->Position.RelativeObjectPosition.Orientation.h.empty())) {
          Orientation = iter->Position.RelativeObjectPosition.Orientation.h;
        }
      }
    }
  }
}

void OSC_SceneLoader::AddRoute(txString& strEntityRef, sPathPoints& Vpoints) TX_NOEXCEPT {
  if (Vpoints.empty()) {
    LOG_WARN << "route point is zero!";
    return;
  }

  SimRoute r;
  for (auto& p : Vpoints) {
    TX_MARK("route start is invalid.");
    p.gps = m_proj_param.ConvertXODR2LonLat(std::stod(p.m_strLon), std::stod(p.m_strLat));
    r.m_path.push_back(p);
  }

  sTagQueryInfoResult route_start_result;
  if (CallFail(r.m_path.empty())) {
    auto& route_start_gps = r.m_path.front();

    ConvertLonLat2SimPosition(__Lon__(route_start_gps.gps), __Lat__(route_start_gps.gps), route_start_result);

    route_start_gps.m_strLon = Utils::FloatToString(route_start_result.dStartPoint.x);
    route_start_gps.m_strLat = Utils::FloatToString(route_start_result.dStartPoint.y);
    route_start_gps.m_strAlt = Utils::FloatToString(route_start_result.dStartPoint.z);
  }
  if (r.m_path.size() > 0) {
    r.m_strStartLon = r.m_path[0].m_strLon;
    r.m_strStartLat = r.m_path[0].m_strLat;
    r.m_strStartAlt = r.m_path[0].m_strAlt;
  }
  r.ConvertToStr();

  {
    if (EntityIsVechile(strEntityRef)) {
      auto itr = m_mapVehicles.find(strEntityRef);
      if (itr != m_mapVehicles.end()) {
        if (itr->second.m_strRouteID.size() < 1) {
          itr->second.m_strRouteID = Utils::IntToString(m_nRouteIDGenerator++);
        }

        r.m_strRouteId = itr->second.m_strRouteID;
        itr->second.m_strStartShift = Utils::FloatToString(route_start_result.dShift);
        itr->second.m_strOffset = Utils::FloatToString(route_start_result.dOffset);
        itr->second.m_strLaneID = std::to_string(route_start_result.laneUid.laneId);
      } else {
        LOG_WARN << "vehicle not exist " << strEntityRef;
        return;
      }
    } else if (EntityIsPedestrian(strEntityRef)) {
      auto itr = m_mapPedestrians.find(strEntityRef);
      if (itr != m_mapPedestrians.end()) {
        if (itr->second.m_strRouteID.size() < 1) {
          itr->second.m_strRouteID = Utils::IntToString(m_nRouteIDGenerator++);
        }

        r.m_strRouteId = itr->second.m_strRouteID;
        itr->second.m_strStartShift = Utils::FloatToString(route_start_result.dShift);
        itr->second.m_strOffset = Utils::FloatToString(route_start_result.dOffset);
        itr->second.m_strLaneID = std::to_string(route_start_result.laneUid.laneId);
      } else {
        LOG_WARN << "pedestrian not exist " << strEntityRef;
        return;
      }
    } else if (EntityIsObstacle(strEntityRef)) {
      auto itr = m_mapObstacles.find(strEntityRef);
      if (itr != m_mapObstacles.end()) {
        if (itr->second.m_strRouteID.size() < 1) {
          itr->second.m_strRouteID = Utils::IntToString(m_nRouteIDGenerator++);
        }

        r.m_strRouteId = itr->second.m_strRouteID;
        itr->second.m_strStartShift = Utils::FloatToString(route_start_result.dShift);
        itr->second.m_strOffset = Utils::FloatToString(route_start_result.dOffset);
        itr->second.m_strLaneID = std::to_string(route_start_result.laneUid.laneId);
      } else {
        LOG_WARN << "obstacle not exist " << strEntityRef;
        return;
      }
    } else if (EntityIsVechileEgo(strEntityRef)) {
      m_EgoRoute = r;
      return;
    } else {
      return;
    }
  }
  LOG_INFO << r.Str();
  m_mapRoutes[r.m_strRouteId] = r;
}

void OSC_SceneLoader::ParsePrivateAction(PrivateAction& pa, txString& strEntityRef, txString& strDynamicsValue,
                                         txString& strTargetValue, txString& strTargetOffset, txBool& isAcc,
                                         txBool& isMerge, txBool& isV, txString& Orientation,
                                         bool isInit /* = true*/) TX_NOEXCEPT {
  isAcc = false;
  isMerge = false;
  if (pa.LongitudinalAction.IsInited()) {
    if (pa.LongitudinalAction.SpeedAction.SpeedActionTarget.AbsoluteTargetSpeed.IsInited()) {
      txString strSpeedV, strV, strDynamicsShape, strDynamicsDimension;
      ReadLongitudinalAction_SpeedAction_Absolute(pa, strDynamicsShape, strV, strDynamicsDimension, strSpeedV);
      if (isInit) {
        AddIniSpeed(strEntityRef, strSpeedV);
      }
      if ("linear" == strDynamicsShape && "rate" == strDynamicsDimension) {
        strDynamicsValue = strV;
        strTargetValue = strSpeedV;
        isAcc = true;
      } else if (strDynamicsShape == "step") {
        strDynamicsValue = strV;
        strTargetValue = strSpeedV;
        isV = true;
      }
    } else { /*if (pa.LongitudinalAction.SpeedAction.SpeedActionTarget. .RelativeTargetSpeed)*/
      LOG_WARN << "Unsupport RelativeTargetSpeed.";
      return;  // 返回
    }
    return;
  }

  if (pa.TeleportAction.IsInited()) {
    if (pa.TeleportAction.Position.WorldPosition.IsInited()) {
      txString strH, strX, strY, strZ;
      ReadTeleportAction_WorldPosition(pa, strX, strY, strZ, strH);
      if (isInit) {
        AddIniWordPos(strEntityRef, strX, strY, strZ, strH);
      }
    } else { /*if (pa.TeleportAction.Position.RelativeLanePosition())*/
      LOG_WARN << "Unsupport RelativeLanePosition.";
      return;  // 返回
    }

    return;
  }

  if (pa.LateralAction.IsInited()) {
    txString strTargetLane, strV, strDynamicsShape, strDynamicsDimension;
    ReadLateralAction_LaneChangeAction(pa, strDynamicsShape, strV, strDynamicsDimension, strTargetLane,
                                       strTargetOffset);
    isMerge = true;

    strDynamicsValue = strV;
    strTargetValue = strTargetLane;
  }

  // Storyboard   Ini   AssignRouteAction     way point
  if (pa.RoutingAction.IsInited()) {
    // planner and vehicle
    if (pa.RoutingAction.AssignRouteAction.IsValid()) {
      sPathPoints TempPoints;
      ReadRoutingAction(pa, RoutingActionType::AssignRouteAction_Type, TempPoints, Orientation);
      AddRoute(strEntityRef, TempPoints);

      // pedestrian
    } else if (pa.RoutingAction.FollowTrajectoryAction.IsValid()) {
      sPathPoints TempPoints;
      ReadRoutingAction(pa, RoutingActionType::FollowTrajectoryAction_Type, TempPoints, Orientation);
      if (TempPoints.size() > 0) {
        AddRoute(strEntityRef, TempPoints);
      } else {
        // AddVelocity()
      }
    }
  }
}

void OSC_SceneLoader::ParseStartTrigger(Trigger& st, txString& strEntityRef, TrigConditionType& nType,
                                        txString& strTrigerValue, txString& strDistanceType,
                                        txString& strAlongRoute) TX_NOEXCEPT {
  const auto& cgs = st.ConditionGroup;
  for (const auto& cg : cgs) {
    const auto& cs = cg.Condition;
    for (const auto& c : cs) {
      const txString& strConditionName = c.name;
      const txString& strDelay = c.delay;
      const txString& strEdge = c.conditionEdge;

      // by value condition time absolute
      if (c.ByValueCondition.IsValid()) {
        strTrigerValue = c.ByValueCondition.SimulationTimeCondition.value;
        if (IsParameterDeclaration(strTrigerValue)) {
          strTrigerValue = FindParameterDeclarationsValudeRet(strTrigerValue);
        }
        nType = TrigConditionType::TCT_TIME_ABSOLUTE;
      }

      // by entity condition event
      if (c.ByEntityCondition.IsValid()) {
        // ttc time
        if (c.ByEntityCondition.EntityCondition.TimeToCollisionCondition.IsValid()) {
          strTrigerValue = c.ByEntityCondition.EntityCondition.TimeToCollisionCondition.value;
          if (IsParameterDeclaration(strTrigerValue)) {
            strTrigerValue = FindParameterDeclarationsValudeRet(strTrigerValue);
          }
          strAlongRoute = c.ByEntityCondition.EntityCondition.TimeToCollisionCondition.alongRoute;
          nType = TrigConditionType::TCT_TIME_RELATIVE;
        }

        // ttc distance
        if (c.ByEntityCondition.EntityCondition.RelativeDistanceCondition.IsInited()) {
          strTrigerValue = c.ByEntityCondition.EntityCondition.RelativeDistanceCondition.value;
          strDistanceType = c.ByEntityCondition.EntityCondition.RelativeDistanceCondition.relativeDistanceType;

          if (IsParameterDeclaration(strTrigerValue)) {
            strTrigerValue = FindParameterDeclarationsValudeRet(strTrigerValue);
          }
          if (IsParameterDeclaration(strDistanceType)) {
            strDistanceType = FindParameterDeclarationsValudeRet(strDistanceType);
          }
          nType = TrigConditionType::TCT_POSITION_RELATIVE;
        }
      }
    }
  }
}

void OSC_SceneLoader::AddAccOrMerge(txString& strEntityRef, txString& strDynamicsValue, txString& strTargetValue,
                                    txString& strOffset, const txBool isAcc, const txBool isMerge,
                                    const TrigConditionType nType, txString& strTrigerValue, txString& strDistanceType,
                                    txString& strAlongRoute) TX_NOEXCEPT {
  if (CallFail(EntityIsVechile(strEntityRef))) {
    return;
  }
  txString str = strEntityRef;

  auto vItr = m_mapVehicles.find(strEntityRef);
  if (vItr == m_mapVehicles.end()) {
    LOG_WARN << "vehicle not exist! " << strEntityRef;
    return;
  }

  DistanceModeType eDistanceMode = DistanceModeType::TDM_NONE;
  if (nType == TrigConditionType::TCT_POSITION_RELATIVE) {
    if (boost::iequals("longitudinal", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
    } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
    } else {
      LOG_WARN << "distance type error!";
      // return;
    }
  } else if (nType == TrigConditionType::TCT_TIME_RELATIVE) {
    if (boost::iequals("true", strAlongRoute)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
    } else if (boost::iequals("false", strAlongRoute)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
    } else {
      LOG_WARN << "distance type error!";
      // return;
    }
  } else {
    LOG_WARN << "event type error!";
    // return;
  }

  if (isAcc) {
    TimeNode TempTimeNode;
    TempTimeNode.dTriggerValue = atof(strTrigerValue.c_str());
    TempTimeNode.dAcc = atof(strDynamicsValue.c_str());
    TempTimeNode.eTriggerType = nType;
    TempTimeNode.eDistanceMode = eDistanceMode;
    TempTimeNode.nCount = 1;
    TempTimeNode.eStopType = AccEndType::AECT_TIME;
    TempTimeNode.dStopValue = 2;

    txBool isNew = true;

    txString strID = vItr->second.m_strAccID;
    if (strID.size() > 0) {
      auto aItr = m_mapAccs.find(strID);
      if (aItr == m_mapAccs.end()) {
        LOG_WARN << "acc is not exist " << strID << ", " << strEntityRef;
        return;
      }
      aItr->second.m_nodes.insert(TempTimeNode);
      isNew = false;
      aItr->second.ConvertToTimeStr();
      aItr->second.ConvertToEventStr();
    }

    if (isNew) {
      SimAcceleration TempAcceleration;
      TempAcceleration.m_ID = m_nAccIDGeneator++;
      TempAcceleration.m_strID = std::to_string(TempAcceleration.m_ID);
      TempAcceleration.m_nodes.insert(TempTimeNode);

      TempAcceleration.ConvertToTimeStr();
      TempAcceleration.ConvertToEventStr();

      m_mapAccs.insert(std::make_pair(TempAcceleration.m_strID, TempAcceleration));

      vItr->second.m_strAccID = TempAcceleration.m_strID;
    }
  }

  if (isMerge) {
    MergeNode TemptagMergeUnit;
    TemptagMergeUnit.dTrigValue = atof(strTrigerValue.c_str());
    TemptagMergeUnit.dDuration = atof(strDynamicsValue.c_str());
    TemptagMergeUnit.eDistanceMode = eDistanceMode;
    txInt nDir = atoi(strTargetValue.c_str());  // mergemodle
    TemptagMergeUnit.eDir = Int2MergeDirType(nDir);
    if (MergeDirType::Straight == TemptagMergeUnit.eDir) {
      txFloat dOffset = std::atof(strOffset.c_str());
      if (dOffset < 0) {
        TemptagMergeUnit.eDir = MergeDirType::LaneKeep_Right;
      } else {
        TemptagMergeUnit.eDir = MergeDirType::LaneKeep_Left;
      }

      TemptagMergeUnit.dOffset = ((dOffset > 0) ? (dOffset) : (-1 * dOffset));
    } else {
      TemptagMergeUnit.dOffset = 0;
    }

    TemptagMergeUnit.eTrigType = nType;
    TemptagMergeUnit.nCount = 1;

    bool isNew = true;
    txString strID = vItr->second.m_strMergeID;
    if (strID.size() > 0) {
      auto mItr = m_mapMerges.find(strID);
      if (mItr == m_mapMerges.end()) {
        LOG_WARN << "merge is not exist " << strID << ", " << strEntityRef;
        return;
      }
      mItr->second.m_merges.insert(TemptagMergeUnit);
      isNew = false;
      mItr->second.ConvertToTimeStr();
      mItr->second.ConvertToEventStr();
    }

    if (isNew) {
      SimMerge TempMerge;
      TempMerge.m_ID = m_nMergeIDGenerator++;
      TempMerge.m_strID = std::to_string(TempMerge.m_ID);
      TempMerge.m_merges.insert(TemptagMergeUnit);
      TempMerge.ConvertToTimeStr();
      TempMerge.ConvertToEventStr();

      // infos.m_traffic.m_mapMerges.insert(make_pair(TempMerge.m_strID, TempMerge));
      m_mapMerges.insert(std::make_pair(TempMerge.m_strID, TempMerge));
      vItr->second.m_strMergeID = TempMerge.m_strID;
    }
  }

  if (isAcc || isMerge) {
    vItr->second.m_eBehavior = VehicleBehavior::eUserDefined;
  }
}

void OSC_SceneLoader::AddPedestrianVelocity(txString& strEntityRef, txString& strDynamicsValue,
                                            txString& strTargetValue, const TrigConditionType nType,
                                            txString& strTrigerValue, txString& strDistanceType,
                                            txString& strOrientation, txString& strAlongRoute) TX_NOEXCEPT {
  if (CallFail(EntityIsPedestrian(strEntityRef))) {
    return;
  }

  if (strTargetValue.size() == 0) {
    return;
  }

  txString str = strEntityRef;

  auto pItr = m_mapPedestrians.find(strEntityRef);
  if (pItr == m_mapPedestrians.end()) {
    LOG_WARN << "Pedestrian not exist! " << strEntityRef;
    return;
  }

  DistanceModeType eDistanceMode = DistanceModeType::TDM_NONE;
  /*if (boost::iequals("longitudinal", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
  } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
  } else if (nType == TrigConditionType::TCT_TIME_RELATIVE) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
  }*/
  if (nType == TrigConditionType::TCT_POSITION_RELATIVE) {
    if (boost::iequals("longitudinal", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
    } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
    } else {
      LOG_WARN << "distance type error!";
      // return;
    }
  } else if (nType == TrigConditionType::TCT_TIME_RELATIVE) {
    if (boost::iequals("true", strAlongRoute)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
    } else if (boost::iequals("false", strAlongRoute)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
    } else {
      LOG_WARN << "distance type error!";
      // return;
    }
  } else {
    LOG_WARN << "event type error!";
    // return;
  }

  PedestrianVelocityNode TempPedestrianVelocityNode;
  TempPedestrianVelocityNode.dTriggerValue = atof(strTrigerValue.c_str());
  TempPedestrianVelocityNode.nDirection = atof(strOrientation.c_str());
  TempPedestrianVelocityNode.dVelocity = atof(strTargetValue.c_str());
  TempPedestrianVelocityNode.nCount = 1;
  TempPedestrianVelocityNode.eDistanceMode = eDistanceMode;
  TempPedestrianVelocityNode.eTriggerType = nType;
#  if 1
  auto vItr = m_mapPedestrianVelocitys.find(pItr->second.m_strID);
  if (vItr == m_mapPedestrianVelocitys.end()) {
    SimPedestrianVelocity TempCPedestrianVelocity;
    /*TempCPedestrianVelocity.m_ID = std::stoi(pItr->second.m_strID);*/
    TempCPedestrianVelocity.m_ID = std::stoi(pItr->second.m_strID);
    TempCPedestrianVelocity.m_nodes.push_back(TempPedestrianVelocityNode);

    TempCPedestrianVelocity.ConvertToEventStr();
    TempCPedestrianVelocity.ConvertToTimeStr();

    m_mapPedestrianVelocitys.insert(std::make_pair(TempCPedestrianVelocity.m_strID, TempCPedestrianVelocity));
  } else {
    SimPedestrianVelocity& pv = vItr->second;
    pv.m_nodes.push_back(TempPedestrianVelocityNode);

    pv.ConvertToEventStr();
    pv.ConvertToTimeStr();
  }

  pItr->second.m_eBehavior = SimPedestrain::PedestrainBehavior::UserDefine;
#  endif
}

void OSC_SceneLoader::AddVelocity(txString& strEntityRef, txString& strDynamicsValue, txString& strTargetValue,
                                  const TrigConditionType nType, txString& strTrigerValue, txString& strDistanceType,
                                  txString& strAlongRoute) TX_NOEXCEPT {
  if (CallFail(EntityIsVechile(strEntityRef))) {
    return;
  }

  if (strTargetValue.size() == 0) {
    return;
  }

  txString str = strEntityRef;

  auto pItr = m_mapVehicles.find(strEntityRef);
  if (pItr == m_mapVehicles.end()) {
    LOG_WARN << "Vehicle not exist! " << strEntityRef;
    return;
  }

  DistanceModeType eDistanceMode = DistanceModeType::TDM_NONE;
  /*if (boost::iequals("longitudinal", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
  } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
  } else if (nType == TrigConditionType::TCT_TIME_RELATIVE) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
  }*/
  if (nType == TrigConditionType::TCT_POSITION_RELATIVE) {
    if (boost::iequals("longitudinal", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
    } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
    } else {
      LOG_WARN << "distance type error!";
      // return;
    }
  } else if (nType == TrigConditionType::TCT_TIME_RELATIVE) {
    if (boost::iequals("true", strAlongRoute)) {
      eDistanceMode = DistanceModeType::TDM_LANE_PROJECTION;
    } else if (boost::iequals("false", strAlongRoute)) {
      eDistanceMode = DistanceModeType::TDM_EUCLIDEAN_DISTANCE;
    } else {
      LOG_WARN << "distance type error!";
      // return;
    }
  } else {
    LOG_WARN << "event type error!";
    // return;
  }

  VelocityTimeNode TempVelocityTimeNode;
  TempVelocityTimeNode.dTriggerValue = atof(strTrigerValue.c_str());
  TempVelocityTimeNode.dVelocity = atof(strTargetValue.c_str());
  TempVelocityTimeNode.nCount = 1;
  TempVelocityTimeNode.eDistanceMode = eDistanceMode;
  TempVelocityTimeNode.eTriggerType = nType;

  auto vItr = m_mapVelocitys.find(pItr->second.m_strID);
  if (vItr == m_mapVelocitys.end()) {
    SimVehicleVelocity TempCVelocity;
    TempCVelocity.m_ID = std::stoi(pItr->second.m_strID);
    TempCVelocity.m_nodes.push_back(TempVelocityTimeNode);
    TempCVelocity.ConvertToEventStr();
    TempCVelocity.ConvertToTimeStr();

    m_mapVelocitys.insert(std::make_pair(TempCVelocity.m_strID, TempCVelocity));
  } else {
    SimVehicleVelocity& pv = vItr->second;
    pv.m_nodes.push_back(TempVelocityTimeNode);

    pv.ConvertToEventStr();
    pv.ConvertToTimeStr();
  }

  pItr->second.m_eBehavior = VehicleBehavior::eUserDefined;
}

void OSC_SceneLoader::ParseStoryboard(xmlStoryboard& sb) TX_NOEXCEPT {
  bool isInit = true;
  // init
  auto& i = sb.Init;
  auto& ias = i.Actions;
  auto& ps = ias.Private;
  for (auto& p : ps) {
    txString strEntityRef = p.entityRef;
    txString str1 = "null", str2 = "null", str3 = "null";
    txString strOffset;
    bool is1, is2, is3;
    auto& pas = p.PrivateAction;
    for (auto& pa : pas) {
      ParsePrivateAction(pa, strEntityRef, str1, str2, strOffset, is1, is2, is3, str3, isInit);
    }
  }
  isInit = false;
  // storyboard
  auto& s = sb.Story;
  // for (auto s : ss) {
  auto& as = s.Act;
  for (auto& a : as) {
    txString strActName = a.name;
    auto& mgs = a.ManeuverGroup;
    for (auto& mg : mgs) {
      txString strManuverName = mg.name;
      txString strExeTimes = mg.maximumExecutionCount;
      auto& at = mg.Actors;
      txString strEntityCount = at.selectTriggeringEntities;

      txString strEntityName;
      auto& ers = at.EntityRef;
      for (auto& er : ers) {
        strEntityName = er.entityRef;
      }

      auto& ms = mg.Maneuver;
      for (auto& m : ms) {
        txString strManeuverName = m.name;
        auto& es = m.Event;
        for (auto& e : es) {
          std::string strEventName = e.name;
          std::string strEventPriority = e.priority;
          auto& as = e.Action;

          txString DynamicsValue = "", TargetValue = "", strOrientation = "";
          txString strTargetOffset;
          txBool isAcc = false, isMerge = false, isVmap = false;

          for (auto& a : as) {
            txString strActionName = a.name;
            auto& pao = a.PrivateAction;
            {
              ParsePrivateAction((pao), strEntityName, DynamicsValue, TargetValue, strTargetOffset, isAcc, isMerge,
                                 isVmap, strOrientation, false);
            }
          }
          auto& st = e.StartTrigger;
          TrigConditionType ntype = TrigConditionType::TCT_INVALID;
          txString TrigerValue = "", DistanceType = "", strAlongRoute = "";

          ParseStartTrigger(st, strEntityName, ntype, TrigerValue, DistanceType, strAlongRoute);

          if (isAcc || isMerge) {
            AddAccOrMerge(strEntityName, DynamicsValue, TargetValue, strTargetOffset, isAcc, isMerge, ntype,
                          TrigerValue, DistanceType, strAlongRoute);
          }
          if (isVmap) {
            AddVelocity(strEntityName, DynamicsValue, TargetValue, ntype, TrigerValue, DistanceType, strAlongRoute);

            AddPedestrianVelocity(strEntityName, DynamicsValue, TargetValue, ntype, TrigerValue, DistanceType,
                                  strOrientation, strAlongRoute);
          }
        }
      }
    }

    auto& st = a.StartTrigger;
    TrigConditionType ntype;
    txString TrigerValue, DistanceType, strAlongRoute;
    ParseStartTrigger(st, strActName, ntype, TrigerValue, DistanceType, strAlongRoute);
  }
}

// stop trigger
/*Storyboard::StopTrigger_type& st = sb.StopTrigger();*/
}

std::tuple<OSC_SceneLoader::ElemTypeInXOSC, OSC_SceneLoader::txSysId> OSC_SceneLoader::Parse_Type_Id_ByName(
    const Base::txString& strName) TX_NOEXCEPT {
  if (_NonEmpty_(strName)) {
    const char ch = strName.front();
    ElemTypeInXOSC type = ElemTypeInXOSC::xosc_undef;
    txSysId nId = 0;
    switch (ch) {
      case 'V': {
        type = ElemTypeInXOSC::xosc_vehicle;
        nId = ParseIdByName(strName);
        break;
      }
      case 'P': {
        type = ElemTypeInXOSC::xosc_pedestrian;
        nId = ParseIdByName(strName);
        break;
      }
      case 'B': {
        type = ElemTypeInXOSC::xosc_pedestrian;
        nId = ParseIdByName(strName);
        break;
      }
      case 'A': {
        type = ElemTypeInXOSC::xosc_pedestrian;
        nId = ParseIdByName(strName);
        break;
      }
      case 'M': {
        type = ElemTypeInXOSC::xosc_pedestrian;
        nId = ParseIdByName(strName);
        break;
      }
      case 'E': {
        type = ElemTypeInXOSC::xosc_ego;
        TX_MARK("ego default id 0");
        break;
      }
      case 'O': {
        type = ElemTypeInXOSC::xosc_obstacle;
        nId = ParseIdByName(strName);
        break;
      }
    }
    return std::make_tuple(type, nId);
  }
  LOG(FATAL) << "Support Name : " << strName;
  return std::make_tuple(ElemTypeInXOSC::xosc_undef, 0);
}

OSC_SceneLoader::txSysId OSC_SceneLoader::ParseIdByName(const Base::txString& strName) TX_NOEXCEPT {
  if (_NonEmpty_(strName)) {
    const Base::txString strId = strName.substr(2, strName.size() - 2);
    txSysId nId = 0;
    try {
      nId = std::stoll(strId);
    } catch (const std::invalid_argument& e) {
      LOG_WARN << TX_VARS(strName) << TX_VARS(strId) << e.what();
    } catch (const std::out_of_range& e) {
      LOG_WARN << TX_VARS(strName) << TX_VARS(strId) << e.what();
    } catch (...) {
      LOG_WARN << TX_VARS(strName) << TX_VARS(strId);
    }
    LOG_INFO << TX_VARS(strName) << TX_VARS(strId) << TX_VARS(nId);
    return nId;
  }
  LOG(FATAL) << "Error Id : " << strName;
  return 0;
}

void OSC_SceneLoader::ParseEntities(xmlEntities& entities) TX_NOEXCEPT {
  for (auto& scenarioObject : entities) {
    const txString scenarioObject_Name = scenarioObject.name;
    txString strEntryType = "null";
    txString strStartV = "0", strMaxV = "0", strBehavior = "UserDefine";
    ElemTypeInXOSC elemType = ElemTypeInXOSC::xosc_undef;
    txSysId sysId = 0;
    std::tie(elemType, sysId) = Parse_Type_Id_ByName(scenarioObject_Name);

    if (ElemTypeInXOSC::xosc_vehicle == elemType /*strName[0] == 'V'*/) {
      auto& objV = scenarioObject.Vehicle;
      if (objV.IsValid()) {
        strEntryType = objV.name;
        // Utils::StringReplace(strEntryType, "Truck_1", "Trunk_1");
        auto& pts = objV.Properties;
        for (const auto& itr : pts) {
          if (itr.name == "StartV") {
            strStartV = itr.value;
          }
          if (itr.name == "MaxV") {
            strMaxV = itr.value;
          }
          if (itr.name == "Behavior") {
            strBehavior = itr.value;
          }
        }
      } else {
        strEntryType = "Sedan";
      }

      SimVehicle v;
      v.m_strName = scenarioObject_Name;
      v.m_strID = Utils::Int64ToString(sysId);
      v.m_eType = __lpsz2enum__(VEHICLE_TYPE, strEntryType.c_str());
      v.m_strStartTime = "0";
      v.m_strStartVelocity = strStartV;
      v.m_strMaxVelocity = strMaxV;
      v.m_eBehavior = Str2Behavior(strBehavior); /*VehicleBehavior::_from_string_nocase(strBehavior.c_str());*/

      auto& vop = scenarioObject.Vehicle;
      if (vop.IsValid()) {
        v.m_strHeight = vop.BoundingBox.Dimensions.height;
        v.m_strWidth = vop.BoundingBox.Dimensions.width;
        v.m_strLength = vop.BoundingBox.Dimensions.length;
      } else {
        v.m_strHeight = "1.44";
        v.m_strWidth = "2.05";
        v.m_strLength = "4.76";
      }
      m_mapVehicles.insert(std::make_pair(v.m_strName, v));
    } else if (ElemTypeInXOSC::xosc_obstacle == elemType) {
      auto& objM = scenarioObject.Obstacle;
      if (objM.IsValid()) {
        strEntryType = objM.name;
      } else {
        strEntryType = "Box";
      }

      SimObstacle O;
      O.m_strName = scenarioObject_Name;
      O.m_strID = Utils::Int64ToString(sysId);
      O.m_eType = __lpsz2enum__(STATIC_ELEMENT_TYPE, strEntryType.c_str());
      auto& mop = scenarioObject.Obstacle;
      if (mop.IsValid()) {
        O.m_strHeight = mop.BoundingBox.Dimensions.height;
        O.m_strWidth = mop.BoundingBox.Dimensions.width;
        O.m_strLength = mop.BoundingBox.Dimensions.length;
      } else {
        O.m_strHeight = "0.75";
        O.m_strWidth = "0.75";
        O.m_strLength = "0.75";
      }
      m_mapObstacles.insert(make_pair(O.m_strName, O));
    } else if (ElemTypeInXOSC::xosc_pedestrian == elemType) {
      auto& objP = scenarioObject.Vehicle;
      if (objP.IsValid()) {
        strEntryType = objP.name;
        auto& pts = objP.Properties;
        for (const auto& itr : pts) {
          if (itr.name == "StartV") {
            strStartV = itr.value;
          }
          if (itr.name == "MaxV") {
            strMaxV = itr.value;
          }
        }
      } else {
        strEntryType = "human";
      }

      SimPedestrain pa;
      pa.m_strName = scenarioObject_Name;
      pa.m_strID = Utils::Int64ToString(sysId);
      pa.m_eBehavior = SimPedestrain::PedestrainBehavior::Trajectory;
      pa.m_eType = __lpsz2enum__(PEDESTRIAN_TYPE, strEntryType.c_str());
      pa.m_strStartTime = "0";
      pa.m_strEndTime = "100000";

      pa.m_strStartVelocity = strStartV;
      pa.m_strMaxVelocity = strMaxV;

      // infos.m_traffic.m_mapPedestriansV2.insert(make_pair(pa.m_strID, pa));
      m_mapPedestrians.insert(make_pair(pa.m_strName, pa));

      // animal
    } else {
      LOG_WARN << "Unsupport xosc element tyep." << scenarioObject_Name;
    }
  }
}

void OSC_SceneLoader::ParseParameterDeclaration(xmlParameterDeclarations& pas) TX_NOEXCEPT {
  m_ParameterDeclarations.clear();

  auto& pds = pas.ParameterDeclaration;
  for (auto& pd : pds) {
    Base::txString strName = pd.name;
    Base::txString strValue = pd.value;
    Base::txString strType = pd.parameterType;

    TypeValue temp;
    temp._parameterType = strType;
    temp._vaule = strValue;
    m_ParameterDeclarations.insert(std::make_pair(strName, temp));
    LOG_INFO << TX_VARS(strName) << TX_VARS(strValue) << TX_VARS(strType);
  }
}

Base::txString OSC_SceneLoader::FindParameterDeclarationsValudeRet(const Base::txString& strKey) const TX_NOEXCEPT {
  auto itr = m_ParameterDeclarations.find(strKey);
  if (std::end(m_ParameterDeclarations) != itr) {
    return (*itr).second._vaule;
  } else {
    LOG_WARN << "FindParameterDeclarationsValudeRet, un find key " << strKey;
    return "";
  }
}

void OSC_SceneLoader::ParseFileHeaderDesc(const Base::txString strDesc, Base::txString& refSimId,
                                          Base::txString& refDataVer) TX_NOEXCEPT {
  if (std::string::npos != strDesc.find(',')) {
    std::vector<txString> results_simId_dataVer;
    boost::algorithm::split(results_simId_dataVer, strDesc, boost::is_any_of(","));
    if (2 == results_simId_dataVer.size()) {
      Utils::trim(results_simId_dataVer[0]);
      Utils::trim(results_simId_dataVer[1]);
      refSimId = results_simId_dataVer[0];
      refDataVer = results_simId_dataVer[1];
    }
  }
}

void OSC_SceneLoader::ParseFileHeader(xmlFileHeader& fht) TX_NOEXCEPT {
  str_author = fht.author;
  str_date = fht.date;
  str_description = fht.description;
  ParseFileHeaderDesc(str_description, m_strSimId, m_strDataVersion);
  str_revMajor = fht.revMajor;
  srt_revMinor = fht.revMinor;
}

OSC_SceneLoader::MapProjParam OSC_SceneLoader::GetXodrProjParam(const Base::txString str_xodr_file_path) TX_NOEXCEPT {
  using namespace tinyxml2;
  tinyxml2::XMLDocument doc;
  MapProjParam default_param;
  doc.LoadFile(str_xodr_file_path.c_str());
  if (CallSucc(0 == doc.ErrorID())) {
    XMLElement* titleElement = doc.FirstChildElement("OpenDRIVE")->FirstChildElement("header");
    if (NonNull_Pointer(titleElement)) {
      if (titleElement->Attribute("north")) {
        default_param.m_north = std::atof(titleElement->Attribute("north"));
        LOG(INFO) << "Map Proj Success" << TX_VARS_NAME(north, default_param.m_north);
      } else {
        LOG(INFO) << "Map Proj Failure, " << TX_VARS_NAME(north, default_param.m_north);
      }

      if (titleElement->Attribute("south")) {
        default_param.m_south = std::atof(titleElement->Attribute("south"));
        LOG(INFO) << "Map Proj Success" << TX_VARS_NAME(south, default_param.m_south);
      } else {
        LOG(INFO) << "Map Proj Failure, " << TX_VARS_NAME(south, default_param.m_south);
      }

      if (titleElement->Attribute("west")) {
        default_param.m_west = std::atof(titleElement->Attribute("west"));
        LOG(INFO) << "Map Proj Success" << TX_VARS_NAME(west, default_param.m_west);
      } else {
        LOG(INFO) << "Map Proj Failure, " << TX_VARS_NAME(west, default_param.m_west);
      }

      if (titleElement->Attribute("east")) {
        default_param.m_east = std::atof(titleElement->Attribute("east"));
        LOG(INFO) << "Map Proj Success" << TX_VARS_NAME(east, default_param.m_east);
      } else {
        LOG(INFO) << "Map Proj Failure, " << TX_VARS_NAME(east, default_param.m_east);
      }

      XMLElement* geoElement = titleElement->FirstChildElement("geoReference");
      if (NonNull_Pointer(geoElement)) {
        default_param.m_proj = Base::txString(geoElement->Value());
        default_param.m_proj = Base::txString(geoElement->FirstChild()->ToText()->Value());
        LOG(INFO) << "Map Proj Success, " << TX_VARS_NAME(proj, default_param.m_proj);
      } else {
        LOG(WARNING) << "Map Proj Failure, " << TX_VARS_NAME(proj, default_param.m_proj);
      }
    }
  } else {
    LOG(WARNING) << "Load xodr file failure. " << str_xodr_file_path << TX_VARS(doc.ErrorID());
  }
  default_param.Initialize();
  return default_param;
}

#  if __TX_Mark__("OSC_SceneLoader::OSC_VehiclesViewer")

Base::txSysId OSC_SceneLoader::OSC_VehiclesViewer::id() const TX_NOEXCEPT {
  return StrToRetValue(m_vehicle.m_strID, stoll(m_vehicle.m_strID), -1);
}

Base::txBool OSC_SceneLoader::OSC_VehiclesViewer::IsIdValid() const TX_NOEXCEPT { return true; }
Base::txSysId OSC_SceneLoader::OSC_VehiclesViewer::routeID() const TX_NOEXCEPT {
  return StrToRetValue(m_vehicle.m_strRouteID, stoll(m_vehicle.m_strRouteID), -1);
}

Base::txLaneID OSC_SceneLoader::OSC_VehiclesViewer::laneID() const TX_NOEXCEPT {
  return StrToRetValue(m_vehicle.m_strLaneID, stoi(m_vehicle.m_strLaneID), -1);
}

Base::txBool OSC_SceneLoader::OSC_VehiclesViewer::IsLaneIdValid() const TX_NOEXCEPT { return true; }
Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::start_s() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strStartShift);
}

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::start_t() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strStartTime);
}

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::start_v() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strStartVelocity);
}

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::max_v() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strMaxVelocity);
}

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::l_offset() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strOffset);
}

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::length() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strLength);
}

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::width() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strWidth);
}
Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::height() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_vehicle.m_strHeight);
}
Base::txSysId OSC_SceneLoader::OSC_VehiclesViewer::accID() const TX_NOEXCEPT {
  return StrToRetValue(m_vehicle.m_strAccID, stoi(m_vehicle.m_strAccID), -1);
}
Base::txBool OSC_SceneLoader::OSC_VehiclesViewer::IsAccIdValid() const TX_NOEXCEPT { return true; }
Base::txSysId OSC_SceneLoader::OSC_VehiclesViewer::mergeID() const TX_NOEXCEPT {
  return StrToRetValue(m_vehicle.m_strMergeID, stoi(m_vehicle.m_strMergeID), -1);
}
Base::txBool OSC_SceneLoader::OSC_VehiclesViewer::IsMergeIdValid() const TX_NOEXCEPT { return true; }
Base::txString OSC_SceneLoader::OSC_VehiclesViewer::behavior() const TX_NOEXCEPT {
  if (_plus_(VehicleBehavior::eTadAI) == m_vehicle.m_eBehavior) {
    return txString("TrafficVehicle");
  } else if (_plus_(VehicleBehavior::eTadAI_Arterial) == m_vehicle.m_eBehavior) {
    return txString("TrafficVehicleArterial");
  } else {
    return txString("UserDefine");
  }
}
Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::aggress() const TX_NOEXCEPT { return 0.5; }

Base::txString OSC_SceneLoader::OSC_VehiclesViewer::vehicleType() const TX_NOEXCEPT {
  return Base::txString(__enum2lpsz__(VEHICLE_TYPE, m_vehicle.m_eType));
}

Base::txSysId OSC_SceneLoader::OSC_VehiclesViewer::follow() const TX_NOEXCEPT { return FLAGS_Topological_Root_Id; }

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::mergeTime() const TX_NOEXCEPT { return FLAGS_LaneChangeDuration; }

Base::txFloat OSC_SceneLoader::OSC_VehiclesViewer::offsetTime() const TX_NOEXCEPT { return FLAGS_OffsetChangeDuration; }

void OSC_SceneLoader::OSC_VehiclesViewer::Init(const SimVehicle& srcVehicle) TX_NOEXCEPT {
  m_vehicle = srcVehicle;
  m_inited = true;
}
#  endif /*__TX_Mark__("OSC_SceneLoader::OSC_VehiclesViewer")*/

#  if __TX_Mark__("OSC_SceneLoader::OSC_AccelerationViewer")

Base::txSysId OSC_SceneLoader::OSC_AccelerationViewer::id() const TX_NOEXCEPT { return m_acc.m_ID; }

Base::txString OSC_SceneLoader::OSC_AccelerationViewer::profile() const TX_NOEXCEPT {
  return (_plus_(EventActionType::TIME_TRIGGER) == m_type) ? (m_acc.m_strProfilesTime) : (m_acc.m_strProfilesEvent);
}

Base::txString OSC_SceneLoader::OSC_AccelerationViewer::endCondition() const TX_NOEXCEPT {
  return (_plus_(EventActionType::TIME_TRIGGER) == m_type) ? (m_acc.m_strEndConditionTime)
                                                           : (m_acc.m_strEndConditionEvent);
}

OSC_SceneLoader::EventActionType OSC_SceneLoader::OSC_AccelerationViewer::ActionType() const TX_NOEXCEPT {
  return m_type;
}

std::vector<std::pair<Base::txFloat, Base::txFloat> >
OSC_SceneLoader::OSC_AccelerationViewer::timestamp_acceleration_pair_vector() const TX_NOEXCEPT {
  std::vector<std::pair<Base::txFloat, Base::txFloat> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"0.0, 0.0; 5.0, -2.0; 9.1, 0.0\"");
    retVec = float_float_pair_parser(profile());
  }
  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(time, std::get<0>(refTuple)) << TX_VARS_NAME(acc, std::get<1>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

std::vector<OSC_SceneLoader::EventEndCondition_t> OSC_SceneLoader::OSC_AccelerationViewer::end_condition_vector() const
    TX_NOEXCEPT {
  std::vector<OSC_SceneLoader::EventEndCondition_t> retVec;
  const Base::txString strEndCondition = endCondition();
  if (!strEndCondition.empty()) {
    TX_MARK("endCondition=\"None, 0.0; Time, 2.3; Velocity, 8.0; None, 0.0\"");
    retVec = acc_invalid_type_threshold_parser(strEndCondition);
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_AccelerationViewer::ttc_acceleration_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1;egodistance 6.0, -1\"");
    retVec = float_float_pair_parser(profile(), FLAGS_EventTypeTTC);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0, 1;egodistance laneprojection 6.0, -1\"");
    retVec = float_float_pair_parser_with_projectionType(profile(), FLAGS_EventTypeTTC);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1 [3];egodistance laneprojection 6.0,-1 [1]\"");
    retVec = float_float_pair_parser_with_projectionType_triggerIndex(profile(), FLAGS_EventTypeTTC);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(acc, std::get<1>(refTuple))
                   << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_AccelerationViewer::egodistance_acceleration_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1;egodistance 6.0, -1\"");
    retVec = float_float_pair_parser(profile(), FLAGS_EventTypeEgoDistance);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0, 1;egodistance laneprojection 6.0, -1\"");
    retVec = float_float_pair_parser_with_projectionType(profile(), FLAGS_EventTypeEgoDistance);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1 [3];egodistance laneprojection 6.0,-1 [1]\"");
    retVec = float_float_pair_parser_with_projectionType_triggerIndex(profile(), FLAGS_EventTypeEgoDistance);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(acc, std::get<1>(refTuple))
                   << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }

  return retVec;
}

Base::txString OSC_SceneLoader::OSC_AccelerationViewer::Str() const TX_NOEXCEPT { return ""; }
void OSC_SceneLoader::OSC_AccelerationViewer::Init(const SimAcceleration& srcAcc,
                                                   const EventActionType srcType) TX_NOEXCEPT {
  using namespace boost::algorithm;
  m_acc = srcAcc;
  m_inited = true;
  // txAssert(m_acc.m_strProfilesTime.empty() && m_acc.m_strProfilesEvent.empty());
  // txAssert((!m_acc.m_strProfilesTime.empty()) && (!m_acc.m_strProfilesEvent.empty()));
  /*EvaluateActionType((m_acc.m_strProfilesTime.size() > 0) ? (m_acc.m_strProfilesTime) : (m_acc.m_strProfilesEvent))*/
  m_type = srcType;
}

#  endif /*__TX_Mark__("OSC_SceneLoader::OSC_AccelerationViewer")*/
std::unordered_map<Base::txSysId, OSC_SceneLoader::IVehiclesViewerPtr> OSC_SceneLoader::GetAllVehicleData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IVehiclesViewerPtr> retMap;
  for (auto& refVehiclePair : (m_mapVehicles)) {
    auto& refVehicle = refVehiclePair.second;
    auto retPtr = std::make_shared<OSC_VehiclesViewer>();
    retPtr->Init(refVehicle);
    retMap[retPtr->id()] = retPtr;
  }
  return retMap;
}

OSC_SceneLoader::IAccelerationViewerPtr OSC_SceneLoader::GetAccsData(Base::txSysId const _id) TX_NOEXCEPT {
  OSC_AccelerationViewerPtr retPtr;
  for (auto& refAccContainer : m_mapAccs) {
    const auto refAccId = stoll(refAccContainer.first);
    if (_id == refAccId && _NonEmpty_(refAccContainer.second.m_strProfilesTime)) {
      LOG(INFO) << TX_VARS(refAccContainer.second.m_strProfilesTime);
      retPtr = std::make_shared<OSC_AccelerationViewer>();
      if (retPtr) {
        retPtr->Init(refAccContainer.second, EventActionType::TIME_TRIGGER);
      }
      break;
    }
  }

  return retPtr;
}

OSC_SceneLoader::IAccelerationViewerPtr OSC_SceneLoader::GetAccsEventData(Base::txSysId const _id) TX_NOEXCEPT {
  OSC_AccelerationViewerPtr retPtr;
  for (auto& refAccContainer : m_mapAccs) {
    const auto refAccId = stoi(refAccContainer.first);
    if (_id == refAccId && _NonEmpty_(refAccContainer.second.m_strProfilesEvent)) {
      LOG(INFO) << TX_VARS(refAccContainer.second.m_strProfilesEvent);
      retPtr = std::make_shared<OSC_AccelerationViewer>();
      if (retPtr) {
        retPtr->Init(refAccContainer.second, EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex);
      }
      break;
    }
  }
  return retPtr;
}

#  if __TX_Mark__(OSC_SceneLoader::OSC_MergeViewer)

Base::txSysId OSC_SceneLoader::OSC_MergeViewer::id() const TX_NOEXCEPT { return m_merge.m_ID; }

Base::txString OSC_SceneLoader::OSC_MergeViewer::profile() const TX_NOEXCEPT {
  return (_plus_(EventActionType::TIME_TRIGGER) == m_type) ? (m_merge.m_strProfileTime) : (m_merge.m_strProfileEvent);
}

OSC_SceneLoader::EventActionType OSC_SceneLoader::OSC_MergeViewer::ActionType() const TX_NOEXCEPT { return m_type; }

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
OSC_SceneLoader::OSC_MergeViewer::timestamp_direction_pair_vector() const TX_NOEXCEPT {
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"0.0, +1, 4.5, v; 5.0, 0, 4.5, v; 9.1,-2, 3.0, 2.0\"");
    retVec = float_int_pair_parser(profile());
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(time, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
                   << TX_VARS_NAME(duration, std::get<2>(refTuple)) << TX_VARS_NAME(offset, std::get<3>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, OSC_SceneLoader::DistanceProjectionType, Base::txFloat /*Duration*/,
                       Base::txFloat /*offset*/, Base::txUInt> >
OSC_SceneLoader::OSC_MergeViewer::ttc_direction_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
      retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1;egodistance 6.0, -1\"");
    retVec = float_int_pair_parser(profile(), FLAGS_EventTypeTTC);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0, 1;egodistance laneprojection 6.0, -1\"");
    retVec = float_int_pair_parser_with_projectionType(profile(), FLAGS_EventTypeTTC);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1,4.5,v [4];egodistance laneprojection 6.0,-2,3.4,2.0 [1]\"");
    retVec = float_int_pair_parser_with_projectionType_with_triggerIndex(profile(), FLAGS_EventTypeTTC);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
                   << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(duration, std::get<3>(refTuple))
                   << TX_VARS_NAME(offset, std::get<4>(refTuple)) << TX_VARS_NAME(trigger, std::get<5>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, OSC_SceneLoader::DistanceProjectionType, Base::txFloat /*Duration*/,
                       Base::txFloat /*offset*/, Base::txUInt> >
OSC_SceneLoader::OSC_MergeViewer::egodistance_direction_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
      retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1;egodistance 6.0, -1\"");
    retVec = float_int_pair_parser(profile(), FLAGS_EventTypeEgoDistance);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0, 1;egodistance laneprojection 6.0, -1\"");
    retVec = float_int_pair_parser_with_projectionType(profile(), FLAGS_EventTypeEgoDistance);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1,4.5,v [4];egodistance laneprojection 6.0,-2,3.4,2.0 [1]\"");
    retVec = float_int_pair_parser_with_projectionType_with_triggerIndex(profile(), FLAGS_EventTypeEgoDistance);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
                   << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(duration, std::get<3>(refTuple))
                   << TX_VARS_NAME(offset, std::get<4>(refTuple)) << TX_VARS_NAME(trigger, std::get<5>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

Base::txString OSC_SceneLoader::OSC_MergeViewer::Str() const TX_NOEXCEPT { return ""; }
void OSC_SceneLoader::OSC_MergeViewer::Init(SimMerge const& srcMerge, const EventActionType srcType) TX_NOEXCEPT {
  m_merge = srcMerge;
  m_inited = true;
  m_type = srcType;
}
#  endif /*__TX_Mark__(OSC_SceneLoader::OSC_MergeViewer)*/

OSC_SceneLoader::IMergesViewerPtr OSC_SceneLoader::GetMergesData(Base::txSysId const _id) TX_NOEXCEPT {
  OSC_MergeViewerPtr retPtr;
  for (auto& refMergePair : m_mapMerges) {
    const auto refMergeId = stoll(refMergePair.first);
    if (_id == refMergeId && _NonEmpty_(refMergePair.second.m_strProfileTime)) {
      LOG(INFO) << TX_VARS(refMergePair.second.m_strProfileTime);
      retPtr = std::make_shared<OSC_MergeViewer>();
      if (retPtr) {
        retPtr->Init(refMergePair.second, EventActionType::TIME_TRIGGER);
      }
      break;
    }
  }

  return retPtr;
}

OSC_SceneLoader::IMergesViewerPtr OSC_SceneLoader::GetMergesEventData(Base::txSysId const _id) TX_NOEXCEPT {
  OSC_MergeViewerPtr retPtr;
  for (auto& refMergePair : m_mapMerges) {
    const auto refMergeId = stoll(refMergePair.first);
    if (_id == refMergeId && _NonEmpty_(refMergePair.second.m_strProfileEvent)) {
      LOG(INFO) << TX_VARS(refMergePair.second.m_strProfileEvent);
      retPtr = std::make_shared<OSC_MergeViewer>();
      if (retPtr) {
        retPtr->Init(refMergePair.second, EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex);
      }
      break;
    }
  }

  return retPtr;
}
#  if __TX_Mark__("OSC_SceneLoader::OSC_RouteViewer")
Base::txSysId OSC_SceneLoader::OSC_RouteViewer::id() const TX_NOEXCEPT {
  return StrToRetValue(m_route.m_strRouteId, stoll(m_route.m_strRouteId), -1);
}

OSC_SceneLoader::ROUTE_TYPE OSC_SceneLoader::OSC_RouteViewer::type() const TX_NOEXCEPT { return ROUTE_TYPE::ePos; }

Base::txFloat OSC_SceneLoader::OSC_RouteViewer::startLon() const TX_NOEXCEPT {
  return StrToRetValue(m_route.m_strStartLon, stod(m_route.m_strStartLon), 0.0);
}

Base::txFloat OSC_SceneLoader::OSC_RouteViewer::startLat() const TX_NOEXCEPT {
  return StrToRetValue(m_route.m_strStartLat, stod(m_route.m_strStartLat), 0.0);
}

Base::txFloat OSC_SceneLoader::OSC_RouteViewer::endLon() const TX_NOEXCEPT {
  if (CallFail(m_route.m_strEnd.empty())) {
    Base::txString strEnd = m_route.m_strEnd;
    Utils::trim(strEnd);
    const Base::txInt elemIdx = 0;
    if (!strEnd.empty()) {
      std::vector<Base::txString> results;
      boost::algorithm::split(results, strEnd, boost::is_any_of(","));
      if (results.size() > 1) {
        Utils::trim(results[elemIdx]);
        if (!results[elemIdx].empty()) {
          return std::stod(results[elemIdx]);
        } else {
          LOG(WARNING) << " Context's first elements is null.";
          return 0.0;
        }
      } else {
        LOG(WARNING) << " Context do not have two elements.";
        return 0.0;
      }
    } else {
      LOG(WARNING) << " Context is Null.";
      return 0.0;
    }
  } else {
    return 0.0;
  }
}
Base::txFloat OSC_SceneLoader::OSC_RouteViewer::endLat() const TX_NOEXCEPT {
  if (CallFail(m_route.m_strEnd.empty())) {
    Base::txString strEnd = m_route.m_strEnd;
    Utils::trim(strEnd);
    const Base::txInt elemIdx = 1;
    if (!strEnd.empty()) {
      std::vector<Base::txString> results;
      boost::algorithm::split(results, strEnd, boost::is_any_of(","));
      if (results.size() > 1) {
        Utils::trim(results[elemIdx]);
        if (!results[elemIdx].empty()) {
          return std::stod(results[elemIdx]);
        } else {
          LOG(WARNING) << " Context's second elements is null.";
          return 0.0;
        }
      } else {
        LOG(WARNING) << " Context do not have two elements.";
        return 0.0;
      }
    } else {
      LOG(WARNING) << " Context is Null.";
      return 0.0;
    }
  } else {
    return 0.0;
  }
}

std::vector<std::pair<Base::txFloat, Base::txFloat> > OSC_SceneLoader::OSC_RouteViewer::midPoints() const TX_NOEXCEPT {
  std::vector<std::pair<Base::txFloat, Base::txFloat> > res;
  if (!m_route.m_strMid.empty()) {
    res = float_float_pair_parser(m_route.m_strMid);
  }
  return res;
}

Base::txRoadID OSC_SceneLoader::OSC_RouteViewer::roidId() const TX_NOEXCEPT { return 0; }
Base::txSectionID OSC_SceneLoader::OSC_RouteViewer::sectionId() const TX_NOEXCEPT { return 0; }

void OSC_SceneLoader::OSC_RouteViewer::Init(SimRoute const& srcRoute) TX_NOEXCEPT {
  m_route = srcRoute;
  m_inited = true;
}

#  endif /*__TX_Mark__("OSC_SceneLoader::OSC_RouteViewer")*/

OSC_SceneLoader::IRouteViewerPtr OSC_SceneLoader::GetRouteData(Base::txSysId const _id) TX_NOEXCEPT {
  OSC_RouteViewerPtr retPtr;
  for (auto& refRoutePair : m_mapRoutes) {
    if (_NonEmpty_(refRoutePair.first)) {
      const auto refRouteId = stoi(refRoutePair.first);
      if (_id == refRouteId) {
        retPtr = std::make_shared<OSC_RouteViewer>();
        if (retPtr) {
          retPtr->Init(refRoutePair.second);
        }
        break;
      }
    }
  }

  return retPtr;
}

OSC_SceneLoader::IRouteViewerPtr OSC_SceneLoader::GetEgoRouteData() TX_NOEXCEPT {
  OSC_RouteViewerPtr retPtr;
  retPtr = std::make_shared<OSC_RouteViewer>();
  if (retPtr) {
    retPtr->Init(m_EgoRoute);
  }
  return retPtr;
}

#  if __TX_Mark__("OSC_SceneLoader::OSC_VelocityViewer")

Base::txSysId OSC_SceneLoader::OSC_VelocityViewer::id() const TX_NOEXCEPT { return m_velocity.m_ID; }

Base::txString OSC_SceneLoader::OSC_VelocityViewer::profile() const TX_NOEXCEPT {
  return (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) ? (m_velocity.m_strProfilesTime)
                                                                 : (m_velocity.m_strProfilesEvent);
}

OSC_SceneLoader::EventActionType OSC_SceneLoader::OSC_VelocityViewer::ActionType() const TX_NOEXCEPT { return m_type; }

std::vector<std::pair<Base::txFloat, Base::txFloat> > OSC_SceneLoader::OSC_VelocityViewer::timestamp_speed_pair_vector()
    const TX_NOEXCEPT {
  std::vector<std::pair<Base::txFloat, Base::txFloat> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"0.0, 3.3; 5.0, 4.4; 9.1, 5.5\"");
    retVec = float_float_pair_parser(profile());
  }
  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(time, std::get<0>(refTuple)) << TX_VARS_NAME(speed, std::get<1>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_VelocityViewer::ttc_speed_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1.2;egodistance 6.0, -1.6\"");
    retVec = float_float_pair_parser(profile(), FLAGS_EventTypeTTC);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0, 1.2;egodistance laneprojection 6.0, -1.6\"");
    retVec = float_float_pair_parser_with_projectionType(profile(), FLAGS_EventTypeTTC);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1.2 [4];egodistance laneprojection 6.0,-1.6 [1]\"");
    retVec = float_float_pair_parser_with_projectionType_triggerIndex(profile(), FLAGS_EventTypeTTC);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(velocity, std::get<1>(refTuple))
                   << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_VelocityViewer::egodistance_speed_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1.2;egodistance 6.0, -1.6\"");
    retVec = float_float_pair_parser(profile(), FLAGS_EventTypeEgoDistance);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0, 1.2;egodistance laneprojection 6.0, -1.6\"");
    retVec = float_float_pair_parser_with_projectionType(profile(), FLAGS_EventTypeEgoDistance);
  } else if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1.2 [4];egodistance laneprojection 6.0,-1.6 [1]\"");
    retVec = float_float_pair_parser_with_projectionType_triggerIndex(profile(), FLAGS_EventTypeEgoDistance);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(velocity, std::get<1>(refTuple))
                   << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }

  return retVec;
}

Base::txString OSC_SceneLoader::OSC_VelocityViewer::Str() const TX_NOEXCEPT { return ""; }
void OSC_SceneLoader::OSC_VelocityViewer::Init(const SimVehicleVelocity& srcVehicle,
                                               const EventActionType _type) TX_NOEXCEPT {
  m_type = _type;
  m_velocity = srcVehicle;
  m_inited = true;
}

#  endif /*__TX_Mark__("OSC_SceneLoader::OSC_VelocityViewer")*/

Base::ISceneLoader::IVelocityViewerPtr OSC_SceneLoader::GetVelocityData(Base::txSysId const _id) TX_NOEXCEPT {
  OSC_VelocityViewerPtr retPtr;
  for (auto& refVehiclePair : m_mapVelocitys) {
    const auto vehicleId = stoll(refVehiclePair.first);
    if (_id == vehicleId && _NonEmpty_(refVehiclePair.second.m_strProfilesTime)) {
      LOG(INFO) << TX_VARS(refVehiclePair.second.m_strProfilesTime);
      retPtr = std::make_shared<OSC_VelocityViewer>();
      if (retPtr) {
        retPtr->Init(refVehiclePair.second, EventActionType::TIME_TRIGGER);
      }
      break;
    }
  }

  return retPtr;
}

Base::ISceneLoader::IVelocityViewerPtr OSC_SceneLoader::GetVelocityEventData(Base::txSysId const _id) TX_NOEXCEPT {
  OSC_VelocityViewerPtr retPtr;
  for (auto& refVehiclePair : m_mapVelocitys) {
    const auto vehicleId = stoll(refVehiclePair.first);
    if (_id == vehicleId && _NonEmpty_(refVehiclePair.second.m_strProfilesEvent)) {
      LOG(INFO) << TX_VARS(refVehiclePair.second.m_strProfilesEvent);
      retPtr = std::make_shared<OSC_VelocityViewer>();
      if (retPtr) {
        retPtr->Init(refVehiclePair.second, EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex);
      }
      break;
    }
  }
  return retPtr;
}

Base::ISceneLoader::IPedestriansEventViewerPtr OSC_SceneLoader::GetPedestriansEventData_TimeEvent(
    Base::txSysId const _id) TX_NOEXCEPT {
  OSC_PedestriansEvent_time_velocity_ViewerPtr retPtr;
  for (auto& refPedeEventPair : m_mapPedestrianVelocitys) {
    const auto pedeId = stoll(refPedeEventPair.first);
    if (_id == pedeId && _NonEmpty_(refPedeEventPair.second.m_strProfileTime)) {
      LOG(INFO) << TX_VARS(refPedeEventPair.second.m_strProfileTime);
      retPtr = std::make_shared<OSC_PedestriansEvent_time_velocity_Viewer>();
      if (retPtr) {
        retPtr->Init(refPedeEventPair.second, EventActionType::TIME_TRIGGER);
      }
      break;
    }
  }
  return retPtr;
}

Base::ISceneLoader::IPedestriansEventViewerPtr OSC_SceneLoader::GetPedestriansEventData_VelocityEvent(
    Base::txSysId const _id) TX_NOEXCEPT {
  OSC_PedestriansEvent_event_velocity_ViewerPtr retPtr;
  for (auto& refPedeEventPair : m_mapPedestrianVelocitys) {
    const auto pedeId = stoll(refPedeEventPair.first);
    if (_id == pedeId && _NonEmpty_(refPedeEventPair.second.m_strProfileEvent)) {
      LOG(INFO) << TX_VARS(refPedeEventPair.second.m_strProfileEvent);
      retPtr = std::make_shared<OSC_PedestriansEvent_event_velocity_Viewer>();
      if (retPtr) {
        retPtr->Init(refPedeEventPair.second, EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex);
      }
      break;
    }
  }
  return retPtr;
}

#  if __TX_Mark__("OSC_SceneLoader::OSC_ObstacleViewer")

Base::txSysId OSC_SceneLoader::OSC_ObstacleViewer::id() const TX_NOEXCEPT { return stoll(m_obstacle.m_strID); }

Base::txSysId OSC_SceneLoader::OSC_ObstacleViewer::routeID() const TX_NOEXCEPT {
  return stoll(m_obstacle.m_strRouteID);
}

Base::txLaneID OSC_SceneLoader::OSC_ObstacleViewer::laneID() const TX_NOEXCEPT { return stoi(m_obstacle.m_strLaneID); }

Base::txFloat OSC_SceneLoader::OSC_ObstacleViewer::start_s() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_obstacle.m_strStartShift);
}

Base::txFloat OSC_SceneLoader::OSC_ObstacleViewer::l_offset() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_obstacle.m_strOffset);
}

Base::txFloat OSC_SceneLoader::OSC_ObstacleViewer::length() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_obstacle.m_strLength);
}

Base::txFloat OSC_SceneLoader::OSC_ObstacleViewer::width() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_obstacle.m_strWidth);
}

Base::txFloat OSC_SceneLoader::OSC_ObstacleViewer::height() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_obstacle.m_strHeight);
}

Base::txString OSC_SceneLoader::OSC_ObstacleViewer::vehicleType() const TX_NOEXCEPT {
  return Base::txString(__enum2lpsz__(STATIC_ELEMENT_TYPE, m_obstacle.m_eType));
}

Base::txFloat OSC_SceneLoader::OSC_ObstacleViewer::direction() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_obstacle.m_strDirection);
}

OSC_SceneLoader::OSC_ObstacleViewer::STATIC_ELEMENT_TYPE OSC_SceneLoader::OSC_ObstacleViewer::type() const TX_NOEXCEPT {
  return m_obstacle.m_eType;
}

void OSC_SceneLoader::OSC_ObstacleViewer::Init(SimObstacle const& srcObstacle) TX_NOEXCEPT {
  m_inited = true;
  m_obstacle = srcObstacle;
}

#  endif

#  if __TX_Mark__("OSC_SceneLoader::OSC_PedestriansViewer")

Base::txSysId OSC_SceneLoader::OSC_PedestriansViewer::id() const TX_NOEXCEPT {
  return StrToRetValue(m_pedestrian.m_strID, stoll(m_pedestrian.m_strID), -1);
}

Base::txSysId OSC_SceneLoader::OSC_PedestriansViewer::routeID() const TX_NOEXCEPT {
  return StrToRetValue(m_pedestrian.m_strRouteID, stoll(m_pedestrian.m_strRouteID), -1);
}
Base::txLaneID OSC_SceneLoader::OSC_PedestriansViewer::laneID() const TX_NOEXCEPT {
  return StrToRetValue(m_pedestrian.m_strLaneID, stoi(m_pedestrian.m_strLaneID), -1);
}

Base::txFloat OSC_SceneLoader::OSC_PedestriansViewer::start_s() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_pedestrian.m_strStartShift);
}

Base::txFloat OSC_SceneLoader::OSC_PedestriansViewer::start_t() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_pedestrian.m_strStartTime);
}

Base::txFloat OSC_SceneLoader::OSC_PedestriansViewer::end_t() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_pedestrian.m_strEndTime);
}

Base::txFloat OSC_SceneLoader::OSC_PedestriansViewer::l_offset() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_pedestrian.m_strOffset);
}

Base::txString OSC_SceneLoader::OSC_PedestriansViewer::type() const TX_NOEXCEPT {
  return Base::txString(__enum2lpsz__(PEDESTRIAN_TYPE, m_pedestrian.m_eType));
}

Base::txBool OSC_SceneLoader::OSC_PedestriansViewer::hadDirection() const TX_NOEXCEPT { return false; }
Base::txFloat OSC_SceneLoader::OSC_PedestriansViewer::start_v() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_pedestrian.m_strStartVelocity);
}

Base::txFloat OSC_SceneLoader::OSC_PedestriansViewer::max_v() const TX_NOEXCEPT {
  return StrToFloatValueWithPrecision(m_pedestrian.m_strMaxVelocity);
}

Base::txString OSC_SceneLoader::OSC_PedestriansViewer::behavior() const TX_NOEXCEPT { return "UserDefine"; }

void OSC_SceneLoader::OSC_PedestriansViewer::Init(SimPedestrain const& srcPedestrian) TX_NOEXCEPT {
  m_inited = true;
  m_pedestrian = srcPedestrian;
}

#  endif /*__TX_Mark__("OSC_SceneLoader::OSC_PedestriansViewer")*/
std::unordered_map<Base::txSysId, OSC_SceneLoader::IPedestriansViewerPtr> OSC_SceneLoader::GetAllPedestrianData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IPedestriansViewerPtr> retMap;
  for (auto& refPedestriansPair : m_mapPedestrians) {
    auto retPtr = std::make_shared<OSC_PedestriansViewer>();
    if (retPtr) {
      retPtr->Init(refPedestriansPair.second);
      retMap[retPtr->id()] = retPtr;
    }
  }
  return retMap;
}

Base::txSysId OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::id() const TX_NOEXCEPT {
  return m_time_velocity.m_ID;
}

Base::txString OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::profile() const TX_NOEXCEPT {
  return m_time_velocity.m_strProfileTime;
}

OSC_SceneLoader::EventActionType OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::ActionType() const
    TX_NOEXCEPT {
  return m_type;
}

std::vector<std::tuple<OSC_SceneLoader::txFloat, OSC_SceneLoader::txInt, OSC_SceneLoader::txFloat,
                       OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::ttc_threshold_direction_velocity_tuple_vector() const
    TX_NOEXCEPT {
  return std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::tuple<OSC_SceneLoader::txFloat, OSC_SceneLoader::txInt, OSC_SceneLoader::txFloat,
                       OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::egodistance_threshold_direction_velocity_tuple_vector()
    const TX_NOEXCEPT {
  return std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::pair<Base::txFloat, Base::txFloat> >
OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::timestamp_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::pair<Base::txFloat, Base::txFloat> >();
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::ttc_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::egodistance_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >
OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::timestamp_direction_speed_tuple_vector() const TX_NOEXCEPT {
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"time,direction,velocity;time,direction,velocity;time,direction,velocity\"");
    retVec = float_int_float_tuple_parser(profile());
  }
  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(time, std::get<0>(refTuple)) << TX_VARS_NAME(direction, std::get<1>(refTuple))
                   << TX_VARS_NAME(speed, std::get<2>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

Base::txString OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::Str() const TX_NOEXCEPT { return ""; }
void OSC_SceneLoader::OSC_PedestriansEvent_time_velocity_Viewer::Init(SimPedestrianVelocity const& srcTimeVelocity,
                                                                      const EventActionType _t) TX_NOEXCEPT {
  m_inited = true;
  m_time_velocity = srcTimeVelocity;
  m_type = EventActionType::TIME_TRIGGER;
}

Base::txSysId OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::id() const TX_NOEXCEPT {
  return m_event_velocity.m_ID;
}

Base::txString OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::profile() const TX_NOEXCEPT {
  return m_event_velocity.m_strProfileEvent;
}

OSC_SceneLoader::EventActionType OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::ActionType() const
    TX_NOEXCEPT {
  return m_type;
}

std::vector<std::tuple<OSC_SceneLoader::txFloat, OSC_SceneLoader::txInt, OSC_SceneLoader::txFloat,
                       OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::ttc_threshold_direction_velocity_tuple_vector() const
    TX_NOEXCEPT {
  std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1.2;egodistance 6.0, -1.6\"");
    retVec = shuttle_float_int_float_tuple_parser(profile(), FLAGS_EventTypeTTC);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
                   << TX_VARS_NAME(velocity, std::get<2>(refTuple)) << TX_VARS_NAME(proj, std::get<3>(refTuple))
                   << TX_VARS_NAME(trigger, std::get<4>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }

  return retVec;
}
std::vector<std::tuple<OSC_SceneLoader::txFloat, OSC_SceneLoader::txInt, OSC_SceneLoader::txFloat,
                       OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::egodistance_threshold_direction_velocity_tuple_vector()
    const TX_NOEXCEPT {
  std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_DefaultProjection) == ActionType()) {
    TX_MARK("profile=\"ttc 3.0, 1,2.0;egodistance 6.0, -1,6.5\"");
    retVec = shuttle_float_int_float_tuple_parser(profile(), FLAGS_EventTypeEgoDistance);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
                   << TX_VARS_NAME(velocity, std::get<2>(refTuple)) << TX_VARS_NAME(proj, std::get<3>(refTuple))
                   << TX_VARS_NAME(trigger, std::get<4>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }

  return retVec;
}

std::vector<std::pair<Base::txFloat, Base::txFloat> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::timestamp_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::pair<Base::txFloat, Base::txFloat> >();
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::ttc_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<
      std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::egodistance_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::timestamp_direction_speed_tuple_vector() const
    TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >();
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::ttc_direction_speed_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,direction,1.2 [4];egodistance laneprojection 6.0,direction,-1.6 [1]\"");
    retVec = float_int_float_tuple_parser_with_projectionType_with_triggerIndex(profile(), FLAGS_EventTypeTTC);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
                   << TX_VARS_NAME(velocity, std::get<2>(refTuple)) << TX_VARS_NAME(proj, std::get<3>(refTuple))
                   << TX_VARS_NAME(trigger, std::get<4>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }
  return retVec;
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, OSC_SceneLoader::DistanceProjectionType, Base::txUInt> >
OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::egodistance_direction_speed_pair_vector() const
    TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1.2 [4];egodistance laneprojection 6.0,-1.6 [1]\"");
    retVec = float_int_float_tuple_parser_with_projectionType_with_triggerIndex(profile(), FLAGS_EventTypeEgoDistance);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LOG(WARNING) << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LOG(WARNING) << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
                   << TX_VARS_NAME(velocity, std::get<2>(refTuple)) << TX_VARS_NAME(proj, std::get<3>(refTuple))
                   << TX_VARS_NAME(trigger, std::get<4>(refTuple));
    }
    LOG(WARNING) << "#############################################################";
  }

  return retVec;
}

Base::txString OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::Str() const TX_NOEXCEPT { return ""; }
void OSC_SceneLoader::OSC_PedestriansEvent_event_velocity_Viewer::Init(SimPedestrianVelocity const& srcEventVelocity,
                                                                       const EventActionType _t) TX_NOEXCEPT {
  m_inited = true;
  m_event_velocity = srcEventVelocity;
  m_type = EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex;
}

Base::txBool OSC_SceneLoader::GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT {
  refEgoData.Clear();
  if (NonNull_Pointer(m_DataSource_XOSC)) {
    sim_msg::Vec3* curPos = refEgoData.mutable_position();
    sim_msg::Vec3* curRpy = refEgoData.mutable_rpy();
    sim_msg::Vec3* curV = refEgoData.mutable_velocity();

    const double fAngle = m_EgoStartHeading; /*sim.simulation.planner.theta*/
    double lon = __Lon__(m_EgoStartPos);
    double lat = __Lat__(m_EgoStartPos);
    const double velocity = m_EgoStartSpeed; /*sim.simulation.planner.start_v*/
    curPos->set_x(lon);
    curPos->set_y(lat);
    curPos->set_z(0);
    curRpy->set_x(0);
    curRpy->set_y(0);
    curRpy->set_z(fAngle);
    curV->set_x(velocity * std::cos(fAngle));
    curV->set_y(velocity * std::sin(fAngle));
    curV->set_z(0);
    /*egoData.set_t(timeStamp);*/
    LOG(INFO) << "[XOSC_SCENE_LOADER] " << TX_VARS(Utils::ToString(m_EgoStartPos));

    return true;
  } else {
    LOG(WARNING) << " Data Source is Null.";
    return false;
  }
}

void OSC_SceneLoader::SetTADEgoData(sim_msg::Location const& _egoInfo) TX_NOEXCEPT { m_EgoData = _egoInfo; }

Base::ISceneLoader::IVehiclesViewerPtr OSC_SceneLoader::GetEgoData() TX_NOEXCEPT {
#  if Use_TruckEgo
  if (_plus_(EgoType::eVehicle) == GetEgoType()) {
    OSC_EgoVehiclesViewerPtr retPtr = std::make_shared<OSC_EgoVehiclesViewer>();
    retPtr->Init(m_EgoData);
    return retPtr;
  } else {
    OSC_TruckVehiclesViewerPtr retPtr = std::make_shared<OSC_TruckVehiclesViewer>();
    retPtr->Init(m_EgoData);
    return retPtr;
  }
#  else
  TAD_EgoVehiclesViewerPtr retPtr = std::make_shared<TAD_EgoVehiclesViewer>();
  retPtr->Init(m_EgoData);
  return retPtr;
#  endif
}

Base::ISceneLoader::IVehiclesViewerPtr OSC_SceneLoader::GetEgoTrailerData() TX_NOEXCEPT {
  if (_plus_(EgoType::eVehicle) == GetEgoType()) {
    return nullptr;
  } else {
    OSC_TrailerVehiclesViewerPtr retPtr = std::make_shared<OSC_TrailerVehiclesViewer>();
    retPtr->Init(m_EgoData);
    return retPtr;
  }
}

#  if __TX_Mark__("OSC_SceneLoader::OSC_EgoVehiclesViewer")
void OSC_SceneLoader::OSC_EgoVehiclesViewer::Init(sim_msg::Location const& srcEgo) TX_NOEXCEPT {
  m_ego = srcEgo;
  m_inited = true;
}

Base::txSysId OSC_SceneLoader::OSC_EgoVehiclesViewer::id() const TX_NOEXCEPT { return FLAGS_Default_EgoVehicle_Id; }
Base::txBool OSC_SceneLoader::OSC_EgoVehiclesViewer::IsIdValid() const TX_NOEXCEPT { return true; }
Base::txSysId OSC_SceneLoader::OSC_EgoVehiclesViewer::routeID() const TX_NOEXCEPT { return -1; }
Base::txBool OSC_SceneLoader::OSC_EgoVehiclesViewer::IsRouteIdValid() const TX_NOEXCEPT { return false; }
Base::txLaneID OSC_SceneLoader::OSC_EgoVehiclesViewer::laneID() const TX_NOEXCEPT { return 0; }
Base::txBool OSC_SceneLoader::OSC_EgoVehiclesViewer::IsLaneIdValid() const TX_NOEXCEPT { return true; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::start_s() const TX_NOEXCEPT { return 0.0; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::start_t() const TX_NOEXCEPT { return 0.0; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::start_v() const TX_NOEXCEPT { return 16.0; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::max_v() const TX_NOEXCEPT { return 16.0; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::l_offset() const TX_NOEXCEPT { return 0.0; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::lon() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().x()) : (0.0));
}
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::lat() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().y()) : (0.0));
}
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::alt() const TX_NOEXCEPT { return 0.0; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_EGO_Length; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_EGO_Width; }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_EGO_Height; }
Base::txSysId OSC_SceneLoader::OSC_EgoVehiclesViewer::accID() const TX_NOEXCEPT { return -1; }
Base::txBool OSC_SceneLoader::OSC_EgoVehiclesViewer::IsAccIdValid() const TX_NOEXCEPT { return false; }
Base::txSysId OSC_SceneLoader::OSC_EgoVehiclesViewer::mergeID() const TX_NOEXCEPT { return -1; }
Base::txBool OSC_SceneLoader::OSC_EgoVehiclesViewer::IsMergeIdValid() const TX_NOEXCEPT { return false; }
Base::txString OSC_SceneLoader::OSC_EgoVehiclesViewer::behavior() const TX_NOEXCEPT { return Base::txString("Ego"); }
Base::txFloat OSC_SceneLoader::OSC_EgoVehiclesViewer::aggress() const TX_NOEXCEPT { return 0.5; }

#  endif  // __TX_Mark__("TAD_SceneLoader::TAD_EgoVehiclesViewer")

#  if __TX_Mark__("TAD_TrailerVehiclesViewer")
Base::txSysId OSC_SceneLoader::OSC_TrailerVehiclesViewer::id() const TX_NOEXCEPT {
  return FLAGS_Default_TrailerVehicle_Id;
}
Base::txFloat OSC_SceneLoader::OSC_TrailerVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_Trailer_Length; }
Base::txFloat OSC_SceneLoader::OSC_TrailerVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_Trailer_Width; }
Base::txFloat OSC_SceneLoader::OSC_TrailerVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_Trailer_Height; }
#  endif /*__TX_Mark__("TAD_TrailerVehiclesViewer")*/

#  if __TX_Mark__("TAD_TruckVehiclesViewer")
Base::txSysId OSC_SceneLoader::OSC_TruckVehiclesViewer::id() const TX_NOEXCEPT { return FLAGS_Default_EgoVehicle_Id; }
Base::txFloat OSC_SceneLoader::OSC_TruckVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_Truck_Length; }
Base::txFloat OSC_SceneLoader::OSC_TruckVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_Truck_Width; }
Base::txFloat OSC_SceneLoader::OSC_TruckVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_Truck_Height; }
#  endif /*__TX_Mark__("TAD_TruckVehiclesViewer")*/

std::ostream& OSC_SceneLoader::PrintStream(std::ostream& os) TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_XOSC)) {
    for (auto& refPair : m_mapRoutes) {
      refPair.second.ConvertToStr();
      os << "[route]" << TX_VARS_NAME(id, refPair.second.m_strRouteId) << TX_VARS_NAME(type, refPair.second.m_strType)
         << TX_VARS_NAME(start, refPair.second.m_strStart) << TX_VARS_NAME(mid, refPair.second.m_strMid)
         << TX_VARS_NAME(end, refPair.second.m_strEnd) << std::endl;
    }

    for (auto& refPair : m_mapAccs) {
      refPair.second.ConvertToTimeStr();
      refPair.second.ConvertToEventStr();

      os << "[acc]" << TX_VARS_NAME(Id, refPair.second.m_strID)
         << TX_VARS_NAME(ProfilesTime, refPair.second.m_strProfilesTime)
         << TX_VARS_NAME(EndConditionTime, refPair.second.m_strEndConditionTime)
         << TX_VARS_NAME(ProfilesEvent, refPair.second.m_strProfilesEvent)
         << TX_VARS_NAME(EndConditionEvent, refPair.second.m_strEndConditionEvent) << std::endl;
    }

    for (auto& refPair : m_mapMerges) {
      refPair.second.ConvertToTimeStr();
      refPair.second.ConvertToEventStr();
      os << "[merge]" << TX_VARS_NAME(Id, refPair.second.m_strID)
         << TX_VARS_NAME(ProfilesTime, refPair.second.m_strProfileTime)
         << TX_VARS_NAME(ProfilesEvent, refPair.second.m_strProfileEvent) << std::endl;
    }

    for (auto& refPair : m_mapVelocitys) {
      refPair.second.ConvertToTimeStr();
      refPair.second.ConvertToEventStr();
      os << "[velocity]" << TX_VARS_NAME(Id, refPair.second.m_strID)
         << TX_VARS_NAME(ProfilesTime, refPair.second.m_strProfilesTime)
         << TX_VARS_NAME(ProfilesEvent, refPair.second.m_strProfilesEvent) << std::endl;
    }

    for (auto& refPair : m_mapPedestrianVelocitys) {
      refPair.second.ConvertToTimeStr();
      refPair.second.ConvertToEventStr();
      os << "[pedestrian]" << TX_VARS_NAME(Id, refPair.second.m_strID)
         << TX_VARS_NAME(ProfilesTime, refPair.second.m_strProfileTime)
         << TX_VARS_NAME(ProfilesEvent, refPair.second.m_strProfileEvent) << std::endl;
    }

    for (auto& refPair : m_mapVehicles) {
      /*
      <vehicle id="1" routeID="1" laneID="-2" start_s="225.465" start_t="0.000" start_v="5.000"
               max_v="12.000" l_offset="-0.103" length="4.700" width="1.850" height="1.360"
               accID="2" mergeID="2" vehicleType="Sedan" aggress="0.685" behavior="TrafficVehicle" />
      */
      os << "[vehicle]" << TX_VARS_NAME(Id, refPair.second.m_strID)
         << TX_VARS_NAME(routeID, refPair.second.m_strRouteID) << TX_VARS_NAME(laneID, refPair.second.m_strLaneID)
         << TX_VARS_NAME(start_s, refPair.second.m_strStartShift)
         << TX_VARS_NAME(start_t, refPair.second.m_strStartTime)
         << TX_VARS_NAME(start_v, refPair.second.m_strStartVelocity)
         << TX_VARS_NAME(max_v, refPair.second.m_strMaxVelocity) << TX_VARS_NAME(l_offset, refPair.second.m_strOffset)
         << TX_VARS_NAME(length, refPair.second.m_strLength) << TX_VARS_NAME(width, refPair.second.m_strWidth)
         << TX_VARS_NAME(height, refPair.second.m_strHeight) << TX_VARS_NAME(accID, refPair.second.m_strAccID)
         << TX_VARS_NAME(mergeID, refPair.second.m_strAccID) << TX_VARS_NAME(vehicleType, refPair.second.m_eType)
         << TX_VARS_NAME(behavior, refPair.second.m_eBehavior) << std::endl;
    }

    for (auto& refPair : m_mapPedestrians) {
      /*
      <pedestrian id="1" routeID="5" laneID="-1" start_s="101.103127" start_t="0.000000"
      type="moto_001" l_offset="-0.025624" start_v="0.000" max_v="1.000" behavior="UserDefine" />
      */
      os << "[pedestrian]" << TX_VARS_NAME(Id, refPair.second.m_strID)
         << TX_VARS_NAME(routeID, refPair.second.m_strRouteID) << TX_VARS_NAME(laneID, refPair.second.m_strLaneID)
         << TX_VARS_NAME(start_s, refPair.second.m_strStartShift)
         << TX_VARS_NAME(start_t, refPair.second.m_strStartTime) << TX_VARS_NAME(type, refPair.second.m_eType)
         << TX_VARS_NAME(l_offset, refPair.second.m_strOffset)
         << TX_VARS_NAME(start_v, refPair.second.m_strStartVelocity)
         << TX_VARS_NAME(max_v, refPair.second.m_strMaxVelocity) << std::endl;
    }

    for (auto& refPair : m_mapObstacles) {
      /*
      <obstacle id="1" routeID="6" laneID="-2" start_s="121.451" l_offset="-0.013"
      length="0.750" width="0.750" height="0.750" vehicleType="Box" direction="50.000" />
      */
      os << "[obstacle]" << TX_VARS_NAME(Id, refPair.second.m_strID)
         << TX_VARS_NAME(routeID, refPair.second.m_strRouteID) << TX_VARS_NAME(laneID, refPair.second.m_strLaneID)
         << TX_VARS_NAME(start_s, refPair.second.m_strStartShift) << TX_VARS_NAME(l_offset, refPair.second.m_strOffset)
         << TX_VARS_NAME(length, refPair.second.m_strLength) << TX_VARS_NAME(width, refPair.second.m_strWidth)
         << TX_VARS_NAME(height, refPair.second.m_strHeight) << TX_VARS_NAME(vehicleType, refPair.second.m_eType)
         << TX_VARS_NAME(direction, refPair.second.m_strDirection) << std::endl;
    }
  }
  return os;
}

TX_NAMESPACE_CLOSE(SceneLoader)

#  undef LOG_INFO
#  undef LOG_WARN
#  undef IsParameterDeclaration
#  undef StrToRetValue
#endif

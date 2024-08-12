// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "SceneLoader/tx_tadloader.h"
#include <boost/container/small_vector.hpp>
#include "tx_catalog_cache.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_path_utils.h"
#include "tx_profile_utils.h"
#include "tx_string_utils.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneReader)
#define LogWarn LOG(WARNING)
#define LogInfo1100 LOG_IF(INFO, FLAGS_LogLevel_SceneEvent_1100)
#define LogInfo1200 LOG_IF(INFO, FLAGS_LogLevel_SceneEvent_1200)

TX_NAMESPACE_OPEN(SceneLoader)

TAD_SceneLoader::TAD_SceneLoader() : _class_name(__func__) {}

Base::txBool TAD_SceneLoader::IsValid() TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  return (_plus_(EStatus::eReady) == Status()) && (exists(FilePath(_source)));
}
#if 1
Base::txBool TAD_SceneLoader::Sim2SceneFile(const Base::txString& _sim_path, Base::txString& _scene_data_file,
                                            Base::txString& _map_file) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  if (exists(FilePath(_sim_path))) {
    try {
      auto sim_file_ptr = SceneLoader::Sim::load_scene_sim(_sim_path);
      if (NonNull_Pointer(sim_file_ptr)) {
        LogInfo << "Loading TAD Scene File : " << _sim_path;
        LogInfo << (*sim_file_ptr);
        using namespace boost::filesystem;
        FilePath path_CurrentPath = FilePath(_sim_path).parent_path();
        path_CurrentPath.append((sim_file_ptr->traffic_file_path));
        _scene_data_file = path_CurrentPath.string();
        LogInfo << " TAD Scene Data File :" << _scene_data_file;

        path_CurrentPath = FilePath(_sim_path).parent_path();
        path_CurrentPath.append((sim_file_ptr->_mapfile.mapfile));
        _map_file = path_CurrentPath.string();
        LogInfo << " Hadmap File :" << _map_file;

        return true;
      } else {
        LogWarn << "load_scene_sim return nullptr. " << _sim_path;
        return false;
      }
    } catch (...) {
      LogWarn << "Loading TAD Scene File : " << _sim_path << " Except.";
      return false;
    }
  } else {
    LOG(WARNING) << "Scene File is not existing. File : " << _sim_path;
    return false;
  }
}
#endif
Base::txBool TAD_SceneLoader::Load(const txString& sim_path, const txString& scene_path,
                                   const txString& map_path) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  _source = sim_path;
  // 检查路径存在
  if (exists(FilePath(_source))) {
    _status = EStatus::eLoading;
    try {
      using namespace boost::filesystem;
      // 获取上级目录
      FilePath path_CurrentPath = FilePath(_source).parent_path();

      // 获取veh catalog路径
      FilePath veh_cata_path = path_CurrentPath;
      veh_cata_path.append(FLAGS_Catalog_VehicleCatalog);

      // 获取ped catalog路径
      FilePath ped_cata_path = path_CurrentPath;
      ped_cata_path.append(FLAGS_Catalog_PedestrianCatalog);

      FilePath obs_cata_path = path_CurrentPath;
      obs_cata_path.append(FLAGS_Catalog_MiscObjectCatalog);

#if USE_CustomModelImport
      FilePath veh_sys_cata_path = path_CurrentPath;
      veh_sys_cata_path.append(FLAGS_Catalog_Sys_VehicleCatalog);

      FilePath ped_sys_cata_path = path_CurrentPath;
      ped_sys_cata_path.append(FLAGS_Catalog_Sys_PedestrianCatalog);

      FilePath obs_sys_cata_path = path_CurrentPath;
      obs_sys_cata_path.append(FLAGS_Catalog_Sys_MiscObjectCatalog);

      if (CallSucc(Base::CatalogCache::Initialize(veh_cata_path.string(), ped_cata_path.string(),
                                                  obs_cata_path.string(), veh_sys_cata_path.string(),
                                                  ped_sys_cata_path.string(), obs_sys_cata_path.string()))) {
#else  /*USE_CustomModelImport*/
      if (CallSucc(
              Base::CatalogCache::Initialize(veh_cata_path.string(), ped_cata_path.string(), obs_cata_path.string()))) {
#endif /*USE_CustomModelImport*/
        LOG(INFO) << "Base::CatalogCache::Initialize() success.";
        Base::CatalogCache::ShowCacheInfo();
      } else {
        LOG(WARNING) << "Base::CatalogCache::Initialize() failure.";
      }
    } catch (...) {
      LogWarn << "Loading Catalog File : " << _source << " Except.";
    }

    try {
      // 加载场景文件
      m_DataSource_Scene = SceneLoader::Sim::load_scene_sim(_source);
      LogInfo << "Loading TAD Scene File : " << _source;
      LogInfo << (*m_DataSource_Scene);
      m_strSimId = m_DataSource_Scene->_sim_id;
      m_strDataVersion = m_DataSource_Scene->_date_version;
    } catch (...) {
      LogWarn << "Loading TAD Scene File : " << _source << " Except.";
      _status = EStatus::eError;
      return false;
    }

    try {
      if (NonNull_Pointer(m_DataSource_Scene)) {
        using namespace boost::filesystem;
        // 加载scene traffic 对象
        m_DataSource_Traffic = SceneLoader::Traffic::load_scene_traffic(scene_path);
        LogInfo << "Loading TAD Traffic File :" << scene_path;
        LogInfo << (*m_DataSource_Traffic);
        // 解析场景事件
        if (ParseSceneEvent()) {
          for (const auto& ref : _seceneEventVec) {
            LogInfo << (ref->Str());
          }
          LogInfo << "parse scene event success.";
        } else {
          LogWarn << "parse scene event failure.";
        }
      } else {
        LogWarn << "Loader TAD Scene File Return NullPtr.";
        _status = EStatus::eError;
        return false;
      }
    } catch (...) {
      LogWarn << "Loading TAD Traffic File : "
              << ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->traffic_file_path) : ("null string"))
              << " Except.";
      _status = EStatus::eError;
      return false;
    }

    LogInfo << "Scene File lon =" << (m_DataSource_Scene->_mapfile.lon);
    LogInfo << "Scene File lat =" << (m_DataSource_Scene->_mapfile.lat);
    LogInfo << "Scene File alt =" << (m_DataSource_Scene->_mapfile.alt);
    LogInfo << "Scene File mapfile =" << (m_DataSource_Scene->_mapfile.mapfile);
    return true;
  } else {
    LOG(FATAL) << "Scene File is not existing. File : " << _source;
    _status = EStatus::eError;
    return false;
  }
}

Base::txBool TAD_SceneLoader::Load(const txString& strFilePath) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  _source = strFilePath;
  // 检查source路径是否存在
  if (exists(FilePath(_source))) {
    _status = EStatus::eLoading;
    try {
      // 加载scene sim对象
      m_DataSource_Scene = SceneLoader::Sim::load_scene_sim(_source);
      LogInfo << "Loading TAD Scene File : " << _source;
      LogInfo << (*m_DataSource_Scene);
      m_strSimId = m_DataSource_Scene->_sim_id;
      m_strDataVersion = m_DataSource_Scene->_date_version;
    } catch (...) {
      LogWarn << "Loading TAD Scene File : " << _source << " Except.";
      _status = EStatus::eError;
      return false;
    }

    try {
      if (NonNull_Pointer(m_DataSource_Scene)) {
        using namespace boost::filesystem;
        FilePath path_CurrentPath = FilePath(_source).parent_path();
        path_CurrentPath.append((m_DataSource_Scene->traffic_file_path));
        // 加载 scene traffic 对象
        m_DataSource_Traffic = SceneLoader::Traffic::load_scene_traffic(path_CurrentPath.string());
        LogInfo << "Loading TAD Traffic File :" << (path_CurrentPath).string();
        LogInfo << (*m_DataSource_Traffic);

        // 解析场景事件
        if (ParseSceneEvent()) {
          for (const auto& ref : _seceneEventVec) {
            LogInfo << (ref->Str());
          }
          LogInfo << "parse scene event success.";
        } else {
          LogWarn << "parse scene event failure.";
        }
      } else {
        LogWarn << "Loader TAD Scene File Return NullPtr.";
        _status = EStatus::eError;
        return false;
      }
    } catch (...) {
      LogWarn << "Loading TAD Traffic File : "
              << ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->traffic_file_path) : ("null string"))
              << " Except.";
      _status = EStatus::eError;
      return false;
    }

    LogInfo << "Scene File lon =" << (m_DataSource_Scene->_mapfile.lon);
    LogInfo << "Scene File lat =" << (m_DataSource_Scene->_mapfile.lat);
    LogInfo << "Scene File alt =" << (m_DataSource_Scene->_mapfile.alt);
    LOG(INFO) << "Scene File mapfile =" << (m_DataSource_Scene->_mapfile.mapfile);
    return true;
  } else {
    LOG(FATAL) << "Scene File is not existing. File : " << _source;
    _status = EStatus::eError;
    return false;
  }
}

Base::txFloat TAD_SceneLoader::GetSimSimulationPlannerTheta() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_planner.theta) : (0.0));
}

Base::txFloat TAD_SceneLoader::GetSimSimulationMapfileLongitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.lon) : (0.0));
}

Base::txFloat TAD_SceneLoader::GetSimSimulationMapfileLatitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.lat) : (0.0));
}

Base::txFloat TAD_SceneLoader::GetSimSimulationMapfileAltitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.alt) : (FLAGS_default_altitude));
}

Base::txBool TAD_SceneLoader::GetSimSimulationPlannerRouteStart(std::tuple<Base::txFloat, Base::txFloat>& res) const
    TX_NOEXCEPT {
  Base::txString strStart = (m_DataSource_Scene->_planner.route.start);

  boost::container::small_vector<Base::txString, 2> results;
  boost::algorithm::split(results, strStart, boost::is_any_of(","));
  LOG(WARNING) << TX_VARS(strStart) << TX_VARS(results.size());
  if (results.size() > 1) {
    try {
      res = std::make_tuple<Base::txFloat, Base::txFloat>(std::stod(results[0]), std::stod(results[1]));
    } catch (const std::invalid_argument&) {
      FLAGS_error_info = (R"(ErrorContent:[)") + m_DataSource_Scene->_planner.route.start + (R"(])");
      LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                   << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
      return false;
    } catch (const std::out_of_range&) {
      FLAGS_error_info = (R"(ErrorContent:[)") + m_DataSource_Scene->_planner.route.start + (R"(])");
      LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                   << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
      return false;
    } catch (...) {
      FLAGS_error_info = (R"(ErrorContent:[)") + m_DataSource_Scene->_planner.route.start + (R"(])");
      LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                   << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
      return false;
    }
    return true;
  } else {
    LOG(WARNING) << "error _planner.route.start " << TX_VARS(results.size());
    return false;
  }
}

Base::txFloat TAD_SceneLoader::GetSimSimulationPlannerStartV() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_planner.start_v) : (0.0));
}

Base::txString TAD_SceneLoader::GetSimSimulationTraffic() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->traffic_file_path) : (""));
}

Base::txString TAD_SceneLoader::GetSimSimulationMapFile() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.mapfile) : (""));
}

Base::txString TAD_SceneLoader::GetSimSimulationPlannerEgoType() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_planner.ego.type) : (""));
}

TAD_SceneLoader::EgoType TAD_SceneLoader::GetEgoType() const TX_NOEXCEPT {
  if (FLAGS_ForcedTrailer) {
    return EgoType::eTruck;
  }

  const Base::txString strType = GetSimSimulationPlannerEgoType();
  if (Base::txString("truck") == strType) {
    return EgoType::eTruck;
  } else {
    return EgoType::eVehicle;
  }
}

Base::txBool TAD_SceneLoader::GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT {
  refEgoData.Clear();
  if (NonNull_Pointer(m_DataSource_Scene) && NonNull_Pointer(m_DataSource_Traffic)) {
    sim_msg::Vec3* curPos = refEgoData.mutable_position();
    sim_msg::Vec3* curRpy = refEgoData.mutable_rpy();
    sim_msg::Vec3* curV = refEgoData.mutable_velocity();

    const Base::txFloat fAngle = GetSimSimulationPlannerTheta(); /*sim.simulation.planner.theta*/
    Base::txFloat lon = 0.0;
    Base::txFloat lat = 0.0;
    std::tuple<Base::txFloat, Base::txFloat> start_tuple;
    if (GetSimSimulationPlannerRouteStart(start_tuple)) {
      std::tie(lon, lat) = start_tuple;                               /*sim.simulation.planner.route.start*/
      const Base::txFloat velocity = GetSimSimulationPlannerStartV(); /*sim.simulation.planner.start_v*/
      curPos->set_x(lon);
      curPos->set_y(lat);
      curPos->set_z(0);
      curRpy->set_x(0);
      curRpy->set_y(0);
      curRpy->set_z(fAngle);
      curV->set_x(velocity * std::cos(fAngle));
      curV->set_y(velocity * std::sin(fAngle));
      curV->set_z(0);
      return true;
    } else {
      LogWarn << "GetSimSimulationPlannerRouteStart failure.";
      return false;
    }
  } else {
    LogWarn << ", Data Source is Null.";
    return false;
  }
}

Base::txBool TAD_SceneLoader::GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
    TX_NOEXCEPT {
  refParams.strTrafficFilePath = GetSimSimulationTraffic(); /*simulation.traffic,*/
  refParams.strHdMapFilePath = GetSimSimulationMapFile();   /*simData.scene.c_str(),*/

  LogInfo << "Map File From .sim : " << refParams.strHdMapFilePath;
  using namespace Utils;
  using namespace boost::filesystem;
  FilePath path_CurrentPath = FilePath(_source).parent_path();
  path_CurrentPath.append(refParams.strHdMapFilePath);
  refParams.strHdMapFilePath = path_CurrentPath.string();
  LogInfo << "Map File Modify .sim : " << refParams.strHdMapFilePath;

  __Lon__(refParams.SceneOriginGPS) = GetSimSimulationMapfileLongitude();
  __Lat__(refParams.SceneOriginGPS) = GetSimSimulationMapfileLatitude();
  __Alt__(refParams.SceneOriginGPS) = GetSimSimulationMapfileAltitude();

  {
    Base::txString app_path;
    Utils::GetAppPath("", app_path);
    const Base::txString cfg_path = app_path + "/HadmapFilter.cfg";
    refParams.strHadmapFilter = cfg_path;
  }

  std::tuple<Base::txFloat, Base::txFloat> start_tuple;
  if (GetSimSimulationPlannerRouteStart(start_tuple)) {
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart Success. " << TX_VARS(std::get<0>(start_tuple))
                 << TX_VARS(std::get<1>(start_tuple));
    return true;
  } else {
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart Failure.";
    return false;
  }
  return true;
}

Base::ISceneLoader::IRouteViewerPtr TAD_SceneLoader::GetRouteData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_RouteViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refRoute : m_DataSource_Traffic->routes) {
      if (id == refRoute.id) {
        retPtr = std::make_shared<TAD_RouteViewer>();
        if (retPtr) {
          retPtr->Init(refRoute);
        }
        break;
      }
    }
  }

  return retPtr;
}

Base::ISceneLoader::IRouteViewerPtr TAD_SceneLoader::GetEgoRouteData() TX_NOEXCEPT { return GetRouteData(0); }

#if __TX_Mark__("TAD_SceneLoader::TAD_RouteViewer")

void TAD_SceneLoader::TAD_RouteViewer::Init(Base::txUInt _id, ROUTE_TYPE _type, Base::txFloat _startLon,
                                            Base::txFloat _startLat, Base::txFloat _endLon,
                                            Base::txFloat _endLat) TX_NOEXCEPT {
  m_route.id = _id;
  m_route.type = Base::txString("start_end");
  m_inited = true;
}
void TAD_SceneLoader::TAD_RouteViewer::Init(SceneLoader::Traffic::Route const& srcRoute) TX_NOEXCEPT {
  m_route = srcRoute;
  auto cp_vec = control_path();
  for (const auto cp : cp_vec) {
    LogInfo << cp;
  }
  m_inited = true;
}

Base::txSysId TAD_SceneLoader::TAD_RouteViewer::id() const TX_NOEXCEPT { return (m_inited) ? (m_route.id) : (0); }

TAD_SceneLoader::ROUTE_TYPE TAD_SceneLoader::TAD_RouteViewer::type() const TX_NOEXCEPT {
  if (Base::txString("roadID") == m_route.type) {
    return TAD_SceneLoader::ROUTE_TYPE::eRoad;
  } else {
    return TAD_SceneLoader::ROUTE_TYPE::ePos;
  }
}
Base::txFloat TAD_SceneLoader::TAD_RouteViewer::startLon() const TX_NOEXCEPT {
  if (_plus_(ROUTE_TYPE::ePos) == type()) {
    Base::txString strStart = m_route.start;
    Utils::trim(strStart);
    const Base::txInt elemIdx = 0;
    if (!strStart.empty()) {
      boost::container::small_vector<Base::txString, 2> results;
      boost::algorithm::split(results, strStart, boost::is_any_of(","));
      if (results.size() > 1) {
        Utils::trim(results[elemIdx]);
        if (!results[elemIdx].empty()) {
          return std::stod(results[elemIdx]);
        } else {
          LogWarn << ", Context's first elements is null.";
          return 0.0;
        }
      } else {
        LogWarn << ", Context do not have two elements.";
        return 0.0;
      }
    } else {
      LogWarn << ", Context is Null.";
      return 0.0;
    }
  } else {
    LogWarn << ", Type Error.";
    return 0.0;
  }
}
Base::txFloat TAD_SceneLoader::TAD_RouteViewer::startLat() const TX_NOEXCEPT {
  if (_plus_(ROUTE_TYPE::ePos) == type()) {
    Base::txString strStart = m_route.start;
    Utils::trim(strStart);
    const Base::txInt elemIdx = 1;
    if (!strStart.empty()) {
      boost::container::small_vector<Base::txString, 2> results;
      boost::algorithm::split(results, strStart, boost::is_any_of(","));
      if (results.size() > 1) {
        Utils::trim(results[elemIdx]);
        if (!results[elemIdx].empty()) {
          return std::stod(results[elemIdx]);
        } else {
          LogWarn << ", Context's second elements is null.";
          return 0.0;
        }
      } else {
        LogWarn << ", Context do not have two elements.";
        return 0.0;
      }
    } else {
      LogWarn << ", Context is Null.";
      return 0.0;
    }
  } else {
    LogWarn << ", Type Error.";
    return 0.0;
  }
}
Base::txFloat TAD_SceneLoader::TAD_RouteViewer::endLon() const TX_NOEXCEPT {
  if (_plus_(ROUTE_TYPE::ePos) == type()) {
    if (_NonEmpty_(m_route.end)) {
      Base::txString strEnd = m_route.end;
      Utils::trim(strEnd);
      const Base::txInt elemIdx = 0;
      if (_NonEmpty_(strEnd)) {
        boost::container::small_vector<Base::txString, 2> results;
        boost::algorithm::split(results, strEnd, boost::is_any_of(","));
        if (results.size() > 1) {
          Utils::trim(results[elemIdx]);
          if (_NonEmpty_(results[elemIdx])) {
            return std::stod(results[elemIdx]);
          } else {
            LogWarn << ", Context's first elements is null.";
            return 0.0;
          }
        } else {
          LogWarn << ", Context do not have two elements.";
          return 0.0;
        }
      } else {
        LogWarn << ", Context is Null.";
        return 0.0;
      }
    } else {
      LogInfo << ", do not have end point.";
      return 0.0;
    }
  } else {
    LogWarn << ", Type Error.";
    return 0.0;
  }
}

Base::txFloat TAD_SceneLoader::TAD_RouteViewer::endLat() const TX_NOEXCEPT {
  if (_plus_(ROUTE_TYPE::ePos) == type()) {
    if (_NonEmpty_(m_route.end)) {
      Base::txString strEnd = m_route.end;
      Utils::trim(strEnd);
      const Base::txInt elemIdx = 1;
      if (_NonEmpty_(strEnd)) {
        boost::container::small_vector<Base::txString, 2> results;
        boost::algorithm::split(results, strEnd, boost::is_any_of(","));
        if (results.size() > 1) {
          Utils::trim(results[elemIdx]);
          if (_NonEmpty_(results[elemIdx])) {
            return std::stod(results[elemIdx]);
          } else {
            LogWarn << ", Context's second elements is null.";
            return 0.0;
          }
        } else {
          LogWarn << ", Context do not have two elements.";
          return 0.0;
        }
      } else {
        LogWarn << ", Context is Null.";
        return 0.0;
      }
    } else {
      LogInfo << ", do not have end point.";
      return 0.0;
    }
  } else {
    LogWarn << ", Type Error.";
    return 0.0;
  }
}

std::vector<std::pair<Base::txFloat, Base::txFloat> > TAD_SceneLoader::TAD_RouteViewer::midPoints() const TX_NOEXCEPT {
  std::vector<std::pair<Base::txFloat, Base::txFloat> > res;
  if (_NonEmpty_(m_route.mid)) {
    res = float_float_pair_parser(m_route.mid);
  }
  return res;
}

Base::txRoadID TAD_SceneLoader::TAD_RouteViewer::roidId() const TX_NOEXCEPT { return 0; }
Base::txSectionID TAD_SceneLoader::TAD_RouteViewer::sectionId() const TX_NOEXCEPT { return 0; }

Base::txString TAD_SceneLoader::TAD_RouteViewer::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  ss << "Route : {" << TX_VARS(id()) << TX_VARS(type()) << TX_VARS(startLon()) << TX_VARS(startLat())
     << TX_VARS(endLon()) << TX_VARS(endLat()) << TX_VARS(roidId()) << TX_VARS(sectionId()) << "} ";
  return ss.str();
}

TAD_SceneLoader::TAD_RouteViewer::control_path_node_vec TAD_SceneLoader::TAD_RouteViewer::control_path() const
    TX_NOEXCEPT {
  control_path_node_vec ret_conrtol_path;
  if (_NonEmpty_(m_route.controlPath)) {
    Base::txString controlPath = m_route.controlPath;
    Utils::trim(controlPath);
    std::vector<txString> results;
    boost::algorithm::split(results, controlPath, boost::is_any_of(";"));
    for (txString controlPt : results) {
      Utils::trim(controlPt);
      std::vector<txString> results_props;
      boost::algorithm::split(results_props, controlPt, boost::is_any_of(","));
      control_path_node cp_node;
      for (txString prop : results_props) {
        Utils::trim(prop);
        std::vector<txString> results_kv;
        boost::algorithm::split(results_kv, prop, boost::is_any_of(":"));
        if (results_kv.size() > 1) {
          if (txString("lon") == results_kv[0]) {
            __Lon__(cp_node.waypoint) = std::atof(results_kv[1].c_str());
          } else if (txString("lat") == results_kv[0]) {
            __Lat__(cp_node.waypoint) = std::atof(results_kv[1].c_str());
          } else if (txString("alt") == results_kv[0]) {
            __Alt__(cp_node.waypoint) = std::atof(results_kv[1].c_str());
          } else if (txString("speed_m_s") == results_kv[0]) {
            cp_node.speed_m_s = std::atof(results_kv[1].c_str());
          } else if (txString("gear") == results_kv[0]) {
            cp_node.gear = __lpsz2enum__(ControlPathGear, results_kv[1].c_str());
          } else {
            LogWarn << "unknown key-value : " << prop;
            break;
          }
        } else {
          LogWarn << "unknown key-value : " << prop;
          break;
        }
      }
      ret_conrtol_path.emplace_back(cp_node);
    }
  }
  return ret_conrtol_path;
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_RouteViewer")

Base::ISceneLoader::IAccelerationViewerPtr TAD_SceneLoader::GetAccsData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_AccelerationViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refAcc : m_DataSource_Traffic->accs) {
      if (id == refAcc.id) {
        retPtr = std::make_shared<TAD_AccelerationViewer>();
        if (retPtr) {
          retPtr->Init(refAcc);
        }
        break;
      }
    }
  }

  return retPtr;
}

Base::ISceneLoader::IAccelerationViewerPtr TAD_SceneLoader::GetAccsEventData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_AccelerationViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refAccEvent : m_DataSource_Traffic->accs_event) {
      if (id == refAccEvent.id) {
        retPtr = std::make_shared<TAD_AccelerationViewer>();
        if (retPtr) {
          retPtr->Init(refAccEvent);
        }
        break;
      }
    }
  }
  return retPtr;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_AccelerationViewer")

void TAD_SceneLoader::TAD_AccelerationViewer::Init(SceneLoader::Traffic::Acc const& srcAccEvent) TX_NOEXCEPT {
  using namespace boost::algorithm;
  m_acc = srcAccEvent;
  m_inited = true;
  m_type = EvaluateActionType(m_acc.profile);
}

Base::txString TAD_SceneLoader::TAD_AccelerationViewer::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  ss << "Acc : {" << TX_VARS(m_inited) << TX_VARS(id()) << TX_VARS(ActionType()) << TX_VARS(profile()) << "} ";
  return ss.str();
}

Base::txSysId TAD_SceneLoader::TAD_AccelerationViewer::id() const TX_NOEXCEPT { return (m_inited ? (m_acc.id) : (0)); }

Base::txString TAD_SceneLoader::TAD_AccelerationViewer::profile() const TX_NOEXCEPT {
  return (m_inited ? (m_acc.profile) : (""));
}

Base::txString TAD_SceneLoader::TAD_AccelerationViewer::endCondition() const TX_NOEXCEPT {
  return (m_inited ? (m_acc.endCondition) : (""));
}

Base::ISceneLoader::EventActionType TAD_SceneLoader::TAD_AccelerationViewer::ActionType() const TX_NOEXCEPT {
  return m_type;
}

std::vector<std::pair<Base::txFloat, Base::txFloat> >
TAD_SceneLoader::TAD_AccelerationViewer::timestamp_acceleration_pair_vector() const TX_NOEXCEPT {
  std::vector<std::pair<Base::txFloat, Base::txFloat> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"0.0, 0.0; 5.0, -2.0; 9.1, 0.0\"");
    retVec = float_float_pair_parser(profile());
  }
  return retVec;
}

std::vector<TAD_SceneLoader::EventEndCondition_t> TAD_SceneLoader::TAD_AccelerationViewer::end_condition_vector() const
    TX_NOEXCEPT {
  std::vector<TAD_SceneLoader::EventEndCondition_t> retVec;
  const Base::txString strEndCondition = endCondition();
  if (_NonEmpty_(strEndCondition)) {
    TX_MARK("endCondition=\"None, 0.0; Time, 2.3; Velocity, 8.0; None, 0.0\"");
    retVec = acc_invalid_type_threshold_parser(strEndCondition);
  }
  return retVec;
}
std::vector<std::tuple<Base::txFloat, Base::txFloat, TAD_SceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_AccelerationViewer::ttc_acceleration_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txFloat, TAD_SceneLoader::DistanceProjectionType, Base::txUInt> > retVec;
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
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(acc, std::get<1>(refTuple))
              << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LogWarn << "#############################################################";
  }
  return retVec;
}
std::vector<std::tuple<Base::txFloat, Base::txFloat, TAD_SceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_AccelerationViewer::egodistance_acceleration_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txFloat, TAD_SceneLoader::DistanceProjectionType, Base::txUInt> > retVec;
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
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(acc, std::get<1>(refTuple))
              << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LogWarn << "#############################################################";
  }

  return retVec;
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_AccelerationViewer")

Base::ISceneLoader::IMergesViewerPtr TAD_SceneLoader::GetMergesData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_MergeViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refMerge : m_DataSource_Traffic->merges) {
      if (id == refMerge.id) {
        retPtr = std::make_shared<TAD_MergeViewer>();
        if (retPtr) {
          retPtr->Init(refMerge);
        }
        break;
      }
    }
  }

  return retPtr;
}

Base::ISceneLoader::IMergesViewerPtr TAD_SceneLoader::GetMergesEventData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_MergeViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refMergeEvent : m_DataSource_Traffic->merges_event) {
      if (id == refMergeEvent.id) {
        retPtr = std::make_shared<TAD_MergeViewer>();
        if (retPtr) {
          retPtr->Init(refMergeEvent);
        }
        break;
      }
    }
  }
  return retPtr;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_MergeViewer")

void TAD_SceneLoader::TAD_MergeViewer::Init(SceneLoader::Traffic::Merge const& srcMergeEvent) TX_NOEXCEPT {
  using namespace boost::algorithm;
  m_merge = srcMergeEvent;
  m_inited = true;
  m_type = EvaluateActionType(m_merge.profile);
}

Base::txString TAD_SceneLoader::TAD_MergeViewer::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  ss << "Merge : {" << TX_VARS(m_inited) << TX_VARS(id()) << TX_VARS(ActionType()) << TX_VARS(profile()) << "} ";
  return ss.str();
}

Base::txSysId TAD_SceneLoader::TAD_MergeViewer::id() const TX_NOEXCEPT { return (m_inited ? (m_merge.id) : (0)); }

Base::txString TAD_SceneLoader::TAD_MergeViewer::profile() const TX_NOEXCEPT {
  return (m_inited ? (m_merge.profile) : (""));
}

Base::ISceneLoader::EventActionType TAD_SceneLoader::TAD_MergeViewer::ActionType() const TX_NOEXCEPT { return m_type; }

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
TAD_SceneLoader::TAD_MergeViewer::timestamp_direction_pair_vector() const TX_NOEXCEPT {
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"0.0, +1, 4.5, v; 5.0, 0, 4.5, v; 9.1,-2, 3.0, 2.0\"");
    retVec = float_int_pair_parser(profile());
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(time, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
              << TX_VARS_NAME(duration, std::get<2>(refTuple)) << TX_VARS_NAME(offset, std::get<3>(refTuple));
    }
    LogWarn << "#############################################################";
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, TAD_SceneLoader::DistanceProjectionType, Base::txFloat /*Duration*/,
                       Base::txFloat /*offset*/, Base::txUInt> >
TAD_SceneLoader::TAD_MergeViewer::ttc_direction_pair_vector() const TX_NOEXCEPT {
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
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
              << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(duration, std::get<3>(refTuple))
              << TX_VARS_NAME(offset, std::get<4>(refTuple)) << TX_VARS_NAME(trigger, std::get<5>(refTuple));
    }
    LogWarn << "#############################################################";
  }
  return retVec;
}
std::vector<std::tuple<Base::txFloat, Base::txInt, TAD_SceneLoader::DistanceProjectionType, Base::txFloat /*Duration*/,
                       Base::txFloat /*offset*/, Base::txUInt> >
TAD_SceneLoader::TAD_MergeViewer::egodistance_direction_pair_vector() const TX_NOEXCEPT {
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
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
              << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(duration, std::get<3>(refTuple))
              << TX_VARS_NAME(offset, std::get<4>(refTuple)) << TX_VARS_NAME(trigger, std::get<5>(refTuple));
    }
    LogWarn << "#############################################################";
  }
  return retVec;
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_MergeViewer")

Base::ISceneLoader::IVelocityViewerPtr TAD_SceneLoader::GetVelocityData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_VelocityViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refVelocity : m_DataSource_Traffic->velocities) {
      if (id == refVelocity.id) {
        retPtr = std::make_shared<TAD_VelocityViewer>();
        if (retPtr) {
          retPtr->Init(refVelocity);
        }
        break;
      }
    }
  }

  return retPtr;
}

Base::ISceneLoader::IVelocityViewerPtr TAD_SceneLoader::GetVelocityEventData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_VelocityViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refVelocityEvent : m_DataSource_Traffic->velocities_event) {
      if (id == refVelocityEvent.id) {
        retPtr = std::make_shared<TAD_VelocityViewer>();
        if (retPtr) {
          retPtr->Init(refVelocityEvent);
        }
        break;
      }
    }
  }
  return retPtr;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_VelocityViewer")

void TAD_SceneLoader::TAD_VelocityViewer::Init(const SceneLoader::Traffic::Velocity& srcVelocityEvent) TX_NOEXCEPT {
  using namespace boost::algorithm;
  m_velocity = srcVelocityEvent;
  m_inited = true;
  m_type = EvaluateActionType(m_velocity.profile);
}

Base::txString TAD_SceneLoader::TAD_VelocityViewer::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  ss << "Velocity : {" << TX_VARS(m_inited) << TX_VARS(id()) << TX_VARS(ActionType()) << TX_VARS(profile()) << "} ";
  return ss.str();
}

Base::txSysId TAD_SceneLoader::TAD_VelocityViewer::id() const TX_NOEXCEPT { return (m_inited ? (m_velocity.id) : (0)); }

Base::txString TAD_SceneLoader::TAD_VelocityViewer::profile() const TX_NOEXCEPT {
  return (m_inited ? (m_velocity.profile) : (""));
}

Base::ISceneLoader::EventActionType TAD_SceneLoader::TAD_VelocityViewer::ActionType() const TX_NOEXCEPT {
  return m_type;
}

std::vector<std::pair<Base::txFloat, Base::txFloat> > TAD_SceneLoader::TAD_VelocityViewer::timestamp_speed_pair_vector()
    const TX_NOEXCEPT {
  std::vector<std::pair<Base::txFloat, Base::txFloat> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"0.0, 3.3; 5.0, 4.4; 9.1, 5.5\"");
    retVec = float_float_pair_parser(profile());
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_VelocityViewer::ttc_speed_pair_vector() const TX_NOEXCEPT {
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
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(velocity, std::get<1>(refTuple))
              << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LogWarn << "#############################################################";
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_VelocityViewer::egodistance_speed_pair_vector() const TX_NOEXCEPT {
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
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(velocity, std::get<1>(refTuple))
              << TX_VARS_NAME(proj, std::get<2>(refTuple)) << TX_VARS_NAME(trigger, std::get<3>(refTuple));
    }
    LogWarn << "#############################################################";
  }

  return retVec;
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_VelocityViewer")

#if __TX_Mark__("TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer")

Base::ISceneLoader::IPedestriansEventViewerPtr TAD_SceneLoader::GetPedestriansEventData_TimeEvent(
    Base::txSysId const Pedestrian_id) TX_NOEXCEPT {
  TAD_PedestriansEvent_time_velocity_ViewerPtr retPtr = nullptr;

  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refTimeVelocity : m_DataSource_Traffic->pedestrians_event_time_velocity) {
      if (Pedestrian_id == refTimeVelocity.id) {
        retPtr = std::make_shared<TAD_PedestriansEvent_time_velocity_Viewer>();
        if (retPtr) {
          retPtr->Init(refTimeVelocity);
        }
        break;
      }
    }
  }
  return retPtr;
}

void TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::Init(
    SceneLoader::Traffic::Time_velocity const& srcTimeVelocity) TX_NOEXCEPT {
  using namespace boost::algorithm;
  m_time_velocity = srcTimeVelocity;
  m_inited = true;
  m_type = EvaluateActionType(m_time_velocity.profile);
}

Base::txSysId TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_time_velocity.id) : (0));
}

Base::txString TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::profile() const TX_NOEXCEPT {
  return (m_inited ? (m_time_velocity.profile) : (""));
}

TAD_SceneLoader::EventActionType TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::ActionType() const
    TX_NOEXCEPT {
  return m_type;
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::ttc_threshold_direction_velocity_tuple_vector() const
    TX_NOEXCEPT {
  return std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::egodistance_threshold_direction_velocity_tuple_vector()
    const TX_NOEXCEPT {
  return std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >();
}

Base::txString TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  ss << "Pedestrians_time_velocity : {" << TX_VARS(m_inited) << TX_VARS(id()) << TX_VARS(ActionType())
     << TX_VARS(profile()) << "} ";
  return ss.str();
}

std::vector<std::pair<Base::txFloat, Base::txFloat> >
TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::timestamp_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::pair<Base::txFloat, Base::txFloat> >();
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >
TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::timestamp_direction_speed_tuple_vector() const TX_NOEXCEPT {
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > retVec;
  if (_plus_(EventActionType::TIME_TRIGGER) == ActionType()) {
    TX_MARK("profile=\"time,direction,velocity;time,direction,velocity;time,direction,velocity\"");
    retVec = float_int_float_tuple_parser(profile());
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::ttc_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer::egodistance_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
}
#endif /*__TX_Mark__("TAD_SceneLoader::TAD_PedestriansEvent_time_velocity_Viewer")*/

#if __TX_Mark__("TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer")
Base::ISceneLoader::IPedestriansEventViewerPtr TAD_SceneLoader::GetPedestriansEventData_VelocityEvent(
    Base::txSysId const Pedestrian_id) TX_NOEXCEPT {
  TAD_PedestriansEvent_event_velocity_ViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refEventVelocity : m_DataSource_Traffic->pedestrians_event_event_velocity) {
      if (Pedestrian_id == refEventVelocity.id) {
        retPtr = std::make_shared<TAD_PedestriansEvent_event_velocity_Viewer>();
        if (retPtr) {
          retPtr->Init(refEventVelocity);
        }
        break;
      }
    }
  }
  return retPtr;
}

void TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::Init(
    SceneLoader::Traffic::Event_velocity const& srcEventVelocity) TX_NOEXCEPT {
  using namespace boost::algorithm;
  m_event_velocity = srcEventVelocity;
  m_inited = true;
  m_type = EvaluateActionType(m_event_velocity.profile);
}

Base::txSysId TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_event_velocity.id) : (0));
}

Base::txString TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::profile() const TX_NOEXCEPT {
  return (m_inited ? (m_event_velocity.profile) : (""));
}

TAD_SceneLoader::EventActionType TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::ActionType() const
    TX_NOEXCEPT {
  return m_type;
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::ttc_threshold_direction_velocity_tuple_vector() const
    TX_NOEXCEPT {
  return std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::egodistance_threshold_direction_velocity_tuple_vector()
    const TX_NOEXCEPT {
  return std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >();
}

Base::txString TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::Str() const TX_NOEXCEPT {
  std::stringstream ss;
  ss << "Pedestrians_event_velocity : {" << TX_VARS(m_inited) << TX_VARS(id()) << TX_VARS(ActionType())
     << TX_VARS(profile()) << "} ";
  return ss.str();
}

std::vector<std::pair<Base::txFloat, Base::txFloat> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::timestamp_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::pair<Base::txFloat, Base::txFloat> >();
}

std::vector<
    std::tuple<Base::txFloat, Base::txFloat,
               TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::ttc_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<
      std::tuple<Base::txFloat, Base::txFloat,
                 TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::DistanceProjectionType, Base::txUInt> >();
}

std::vector<
    std::tuple<Base::txFloat, Base::txFloat,
               TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::egodistance_speed_pair_vector() const TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >();
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::timestamp_direction_speed_tuple_vector() const
    TX_NOEXCEPT {
  return std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> >();
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat,
               TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::ttc_direction_speed_pair_vector() const TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,direction,1.2 [4];egodistance laneprojection 6.0,direction,-1.6 [1]\"");
    retVec = float_int_float_tuple_parser_with_projectionType_with_triggerIndex(profile(), FLAGS_EventTypeTTC);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
              << TX_VARS_NAME(velocity, std::get<2>(refTuple)) << TX_VARS_NAME(proj, std::get<3>(refTuple))
              << TX_VARS_NAME(trigger, std::get<4>(refTuple));
    }
    LogWarn << "#############################################################";
  }
  return retVec;
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat,
               TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::DistanceProjectionType, Base::txUInt> >
TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer::egodistance_direction_speed_pair_vector() const
    TX_NOEXCEPT {
  using namespace boost::algorithm;
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> > retVec;
  if (_plus_(EventActionType::TTC_EgoDist_With_SpecialProjection_With_TriggerIndex) == ActionType()) {
    TX_MARK("profile=\"ttc laneprojection 3.0,1.2 [4];egodistance laneprojection 6.0,-1.6 [1]\"");
    retVec = float_int_float_tuple_parser_with_projectionType_with_triggerIndex(profile(), FLAGS_EventTypeEgoDistance);
  }

  if (FLAGS_LogLevel_SceneReader && _NonEmpty_(retVec)) {
    LogWarn << TX_VARS(profile()) << TX_VARS(retVec.size()) << TX_VARS(ActionType());

    for (const auto& refTuple : retVec) {
      LogWarn << TX_VARS_NAME(condition, std::get<0>(refTuple)) << TX_VARS_NAME(dir, std::get<1>(refTuple))
              << TX_VARS_NAME(velocity, std::get<2>(refTuple)) << TX_VARS_NAME(proj, std::get<3>(refTuple))
              << TX_VARS_NAME(trigger, std::get<4>(refTuple));
    }
    LogWarn << "#############################################################";
  }

  return retVec;
}
#endif /*__TX_Mark__("TAD_SceneLoader::TAD_PedestriansEvent_event_velocity_Viewer")*/

Base::ISceneLoader::IVehiclesViewerPtr TAD_SceneLoader::GetVehicleData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_VehiclesViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refVehicle : m_DataSource_Traffic->vehicles) {
      if (id == refVehicle.id) {
        retPtr = std::make_shared<TAD_VehiclesViewer>();
        if (retPtr) {
          retPtr->Init(refVehicle);
        }
        break;
      } else {
        // LOG(WARNING) << "[OOO] id = " << id << ", refVehicle.id() = " << refVehicle.id();
      }
    }
  }
  return retPtr;
}

std::unordered_map<Base::txSysId, Base::ISceneLoader::IVehiclesViewerPtr> TAD_SceneLoader::GetAllVehicleData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IVehiclesViewerPtr> retMap;

  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refVehicle : m_DataSource_Traffic->vehicles) {
      // LOG(INFO) << "[XXX] refVehicle.id() = " << refVehicle.id();
      auto retPtr = std::make_shared<TAD_VehiclesViewer>();
      if (retPtr) {
        retPtr->Init(refVehicle);
        retMap[refVehicle.id] = retPtr;
      } else {
        LOG(WARNING) << "[XXX] refVehicle.id() = " << refVehicle.id << ", not found.";
      }
    }
  }
  return retMap;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_VehiclesViewer")

void TAD_SceneLoader::TAD_VehiclesViewer::Init(SceneLoader::Traffic::Vehicle const& srcVehicle) TX_NOEXCEPT {
  m_vehicle = srcVehicle;
  m_inited = true;
}

Base::txBool TAD_SceneLoader::TAD_VehiclesViewer::IsMergeIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.mergeID >= 0);
}

Base::txBool TAD_SceneLoader::TAD_VehiclesViewer::IsAccIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.accID >= 0);
}

Base::txBool TAD_SceneLoader::TAD_VehiclesViewer::IsIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.id >= 0);
}

Base::txBool TAD_SceneLoader::TAD_VehiclesViewer::IsRouteIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.routeID >= 0);
}

Base::txBool TAD_SceneLoader::TAD_VehiclesViewer::IsLaneIdValid() const TX_NOEXCEPT { return m_inited; }

Base::txSysId TAD_SceneLoader::TAD_VehiclesViewer::id() const TX_NOEXCEPT {
  return (IsIdValid() ? (m_vehicle.id) : (-1));
}

Base::txSysId TAD_SceneLoader::TAD_VehiclesViewer::routeID() const TX_NOEXCEPT {
  return (IsRouteIdValid() ? (m_vehicle.routeID) : (-1));
}

Base::txLaneID TAD_SceneLoader::TAD_VehiclesViewer::laneID() const TX_NOEXCEPT {
  return (IsLaneIdValid() ? (m_vehicle.laneID) : (-1));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.start_s) : (10.0));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::start_t() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.start_t) : (0.0));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::start_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.start_v) : (8.0));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::max_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.max_v) : (30.0));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.l_offset) : (0.0));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::length() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.length) : (4.5));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::width() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.width) : (1.8));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::height() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.height) : (FLAGS_default_height_of_obstacle_pedestrian));
}

Base::txSysId TAD_SceneLoader::TAD_VehiclesViewer::accID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.accID) : (-1));
}

Base::txSysId TAD_SceneLoader::TAD_VehiclesViewer::mergeID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.mergeID) : (-1));
}

Base::txString TAD_SceneLoader::TAD_VehiclesViewer::behavior() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.behavior) : (Base::txString("UserDefine")));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::aggress() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.aggress) : (0.5));
}

Base::txString TAD_SceneLoader::TAD_VehiclesViewer::vehicleType() const TX_NOEXCEPT {
  if ("Trunk_1" == m_vehicle.vehicleType) {
    return Base::txString("Truck_1");
  } else {
    return ((m_inited) ? (m_vehicle.vehicleType) : ("UnDefined"));
  }
}

Base::txSysId TAD_SceneLoader::TAD_VehiclesViewer::follow() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_vehicle.follow)) {
    ISceneLoader::ISceneEventViewer::IConditionViewer::ElementType elementType;
    Base::txSysId elementId;
    std::tie(elementType, elementId) =
        ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(m_vehicle.follow);
    return elementId;
  }
  return FLAGS_Topological_Root_Id;
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::mergeTime() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.mergeTime) : (FLAGS_LaneChangeDuration));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::offsetTime() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.offsetTime) : (FLAGS_OffsetChangeDuration));
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::start_angle() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_vehicle.start_angle)) {
    return std::stod(m_vehicle.start_angle);
  }
  return IVehiclesViewer::start_angle();
}

std::vector<Base::txSysId> TAD_SceneLoader::TAD_VehiclesViewer::eventId() const TX_NOEXCEPT {
  std::vector<Base::txSysId> ret_event_id;
  if (m_inited) {
    Base::txString strEventId = m_vehicle.eventId;
    Utils::trim(strEventId);
    if (_NonEmpty_(strEventId)) {
      std::vector<Base::txString> results_event_id;
      boost::algorithm::split(results_event_id, strEventId, boost::is_any_of(","));
      for (Base::txString strId : results_event_id) {
        Utils::trim(strId);
        ret_event_id.emplace_back(std::atol(strId.c_str()));
      }
    }
  }
  return ret_event_id;
}

Base::txFloat TAD_SceneLoader::TAD_VehiclesViewer::angle() const TX_NOEXCEPT {
  if (m_inited) {
    return m_vehicle.angle;
  } else {
    return 0.0;
  }
}

Base::txString TAD_SceneLoader::TAD_VehiclesViewer::catalog() const TX_NOEXCEPT {
  if (m_inited) {
    return m_vehicle.catalog;
  } else {
    return "";
  }
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_VehiclesViewer")

#if __TX_Mark__("TAD_SceneLoader::TAD_EgoVehiclesViewer")
void TAD_SceneLoader::TAD_EgoVehiclesViewer::Init(sim_msg::Location const& srcEgo) TX_NOEXCEPT {
  m_ego = srcEgo;
  m_inited = true;
}

void TAD_SceneLoader::TAD_EgoVehiclesViewer::Init(sim_msg::Location const& srcEgo, Base::txString groupName,
                                                  Base::txInt ego_id) TX_NOEXCEPT {
  m_ego = srcEgo;
  m_inited = true;
  m_groupName = groupName;
  m_egoId = ego_id;
}

void TAD_SceneLoader::TAD_EgoVehiclesViewer::Init(sim_msg::Location const& srcEgo, Base::txString groupName,
                                                  Base::txInt ego_id, Base::txFloat start_v,
                                                  Base::txFloat max_v) TX_NOEXCEPT {
  m_ego = srcEgo;
  m_inited = true;
  m_groupName = groupName;
  m_egoId = ego_id;
  m_startV = start_v;
  m_maxV = max_v;
}

#  if USE_EgoGroup
Base::txSysId TAD_SceneLoader::TAD_EgoVehiclesViewer::id() const TX_NOEXCEPT { return m_egoId; }
#  else  /*USE_EgoGroup*/
Base::txSysId TAD_SceneLoader::TAD_EgoVehiclesViewer::id() const TX_NOEXCEPT { return FLAGS_Default_EgoVehicle_Id; }
#  endif /*USE_EgoGroup*/
Base::txBool TAD_SceneLoader::TAD_EgoVehiclesViewer::IsIdValid() const TX_NOEXCEPT { return true; }
Base::txSysId TAD_SceneLoader::TAD_EgoVehiclesViewer::routeID() const TX_NOEXCEPT { return -1; }
Base::txBool TAD_SceneLoader::TAD_EgoVehiclesViewer::IsRouteIdValid() const TX_NOEXCEPT { return false; }
Base::txLaneID TAD_SceneLoader::TAD_EgoVehiclesViewer::laneID() const TX_NOEXCEPT { return 0; }
Base::txBool TAD_SceneLoader::TAD_EgoVehiclesViewer::IsLaneIdValid() const TX_NOEXCEPT { return true; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::start_s() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::start_t() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::start_v() const TX_NOEXCEPT { return m_startV; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::max_v() const TX_NOEXCEPT { return m_maxV; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::l_offset() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::lon() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().x()) : (0.0));
}
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::lat() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().y()) : (0.0));
}
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::alt() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_EGO_Length; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_EGO_Width; }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_EGO_Height; }
Base::txSysId TAD_SceneLoader::TAD_EgoVehiclesViewer::accID() const TX_NOEXCEPT { return -1; }
Base::txBool TAD_SceneLoader::TAD_EgoVehiclesViewer::IsAccIdValid() const TX_NOEXCEPT { return false; }
Base::txSysId TAD_SceneLoader::TAD_EgoVehiclesViewer::mergeID() const TX_NOEXCEPT { return -1; }
Base::txBool TAD_SceneLoader::TAD_EgoVehiclesViewer::IsMergeIdValid() const TX_NOEXCEPT { return false; }
Base::txString TAD_SceneLoader::TAD_EgoVehiclesViewer::behavior() const TX_NOEXCEPT { return Base::txString("Ego"); }
Base::txFloat TAD_SceneLoader::TAD_EgoVehiclesViewer::aggress() const TX_NOEXCEPT { return 0.5; }

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_EgoVehiclesViewer")

#if __TX_Mark__("TAD_TrailerVehiclesViewer")
Base::txSysId TAD_SceneLoader::TAD_TrailerVehiclesViewer::id() const TX_NOEXCEPT {
  return FLAGS_Default_TrailerVehicle_Id;
}
Base::txFloat TAD_SceneLoader::TAD_TrailerVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_Trailer_Length; }
Base::txFloat TAD_SceneLoader::TAD_TrailerVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_Trailer_Width; }
Base::txFloat TAD_SceneLoader::TAD_TrailerVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_Trailer_Height; }
#endif /*__TX_Mark__("TAD_TrailerVehiclesViewer")*/

#if __TX_Mark__("TAD_TruckVehiclesViewer")
#  if USE_EgoGroup
Base::txSysId TAD_SceneLoader::TAD_TruckVehiclesViewer::id() const TX_NOEXCEPT { return m_egoId; }
#  else  /*USE_EgoGroup*/
Base::txSysId TAD_SceneLoader::TAD_TruckVehiclesViewer::id() const TX_NOEXCEPT { return FLAGS_Default_EgoVehicle_Id; }
#  endif /*USE_EgoGroup*/
Base::txFloat TAD_SceneLoader::TAD_TruckVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_Truck_Length; }
Base::txFloat TAD_SceneLoader::TAD_TruckVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_Truck_Width; }
Base::txFloat TAD_SceneLoader::TAD_TruckVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_Truck_Height; }
#endif /*__TX_Mark__("TAD_TruckVehiclesViewer")*/

Base::ISceneLoader::IPedestriansViewerPtr TAD_SceneLoader::GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_PedestriansViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refPedestrian : m_DataSource_Traffic->pedestrians) {
      if (id == refPedestrian.id) {
        retPtr = std::make_shared<TAD_PedestriansViewer>();
        if (retPtr) {
          retPtr->Init(refPedestrian);
        }
        break;
      }
    }
  }

  return retPtr;
}

std::unordered_map<Base::txSysId, Base::ISceneLoader::IPedestriansViewerPtr> TAD_SceneLoader::GetAllPedestrianData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IPedestriansViewerPtr> retMap;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refPedestrian : m_DataSource_Traffic->pedestrians) {
      auto retPtr = std::make_shared<TAD_PedestriansViewer>();
      if (retPtr) {
        retPtr->Init(refPedestrian);
        retMap[refPedestrian.id] = retPtr;
      }
    }
  }

  return retMap;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_PedestriansViewer")

void TAD_SceneLoader::TAD_PedestriansViewer::Init(SceneLoader::Traffic::Pedestrian const& srcPedestrian) TX_NOEXCEPT {
  m_pedestrian = srcPedestrian;
  m_inited = true;
}

Base::txSysId TAD_SceneLoader::TAD_PedestriansViewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_pedestrian.id) : (-1));
}

Base::txSysId TAD_SceneLoader::TAD_PedestriansViewer::routeID() const TX_NOEXCEPT {
  return (m_inited) ? (m_pedestrian.routeID) : (-1);
}

Base::txBool TAD_SceneLoader::TAD_PedestriansViewer::IsRouteIdValid() const TX_NOEXCEPT { return routeID() >= 0; }

Base::txLaneID TAD_SceneLoader::TAD_PedestriansViewer::laneID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.laneID) : (-1));
}

Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.start_s) : (0.0));
}

Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::start_t() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.start_t) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::end_t() const TX_NOEXCEPT {
  return (m_inited ? (m_pedestrian.end_t) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.l_offset) : (0.0));
}

Base::txString TAD_SceneLoader::TAD_PedestriansViewer::type() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.type) : ("human"));
}

Base::txBool TAD_SceneLoader::TAD_PedestriansViewer::hadDirection() const TX_NOEXCEPT { return false; }

Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::start_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.start_v) : (0.0));
}

Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::max_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.max_v) : (0.0));
}

Base::txString TAD_SceneLoader::TAD_PedestriansViewer::behavior() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.behavior) : ("UserDefine"));
}

std::vector<Base::txSysId> TAD_SceneLoader::TAD_PedestriansViewer::eventId() const TX_NOEXCEPT {
  std::vector<Base::txSysId> ret_event_id;
  if (m_inited) {
    Base::txString strEventId = m_pedestrian.eventId;
    Utils::trim(strEventId);
    if (_NonEmpty_(strEventId)) {
      std::vector<Base::txString> results_event_id;
      boost::algorithm::split(results_event_id, strEventId, boost::is_any_of(","));
      for (Base::txString strId : results_event_id) {
        Utils::trim(strId);
        ret_event_id.emplace_back(std::atol(strId.c_str()));
      }
    }
  }
  return ret_event_id;
}

Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::angle() const TX_NOEXCEPT {
  if (m_inited) {
    return m_pedestrian.angle;
  } else {
    return 0.0;
  }
}
Base::txFloat TAD_SceneLoader::TAD_PedestriansViewer::start_angle() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_pedestrian.start_angle)) {
    return std::stod(m_pedestrian.start_angle);
  }
  return IPedestriansViewer::start_angle();
}

Base::txString TAD_SceneLoader::TAD_PedestriansViewer::catalog() const TX_NOEXCEPT {
  if (m_inited) {
    return m_pedestrian.catalog;
  } else {
    return "";
  }
}

Base::ISceneLoader::IVehiclesViewerPtr TAD_SceneLoader::TAD_PedestriansViewer::vehicle_view_ptr() const TX_NOEXCEPT {
  TAD_VehiclesViewerPtr ret_ptr = std::make_shared<TAD_VehiclesViewer>();
  if (NonNull_Pointer(ret_ptr)) {
    SceneLoader::Traffic::Vehicle input_veh_param;
    input_veh_param.id = m_pedestrian.id * -1;
    input_veh_param.routeID = m_pedestrian.routeID;
    input_veh_param.laneID = m_pedestrian.laneID;
    input_veh_param.start_s = m_pedestrian.start_s;
    input_veh_param.start_t = m_pedestrian.start_t;
    input_veh_param.start_v = m_pedestrian.start_v;
    input_veh_param.max_v = m_pedestrian.max_v;
    input_veh_param.l_offset = m_pedestrian.l_offset;
    input_veh_param.vehicleType = m_pedestrian.type;
    input_veh_param.behavior = m_pedestrian.behavior;
    input_veh_param.aggress = 0.0;
    input_veh_param.follow = "";
    input_veh_param.mergeTime = FLAGS_LaneChangeDuration;
    input_veh_param.offsetTime = FLAGS_OffsetChangeDuration;
    input_veh_param.eventId = m_pedestrian.eventId;

    input_veh_param.length = 1;
    input_veh_param.width = 1;
    input_veh_param.height = 1;
    input_veh_param.accID = -1;
    input_veh_param.mergeID = -1;
    input_veh_param.angle = 0;
    input_veh_param.catalog = "";
    ret_ptr->Init(input_veh_param);
  }
  return ret_ptr;
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_PedestriansViewer")

Base::ISceneLoader::IObstacleViewerPtr TAD_SceneLoader::GetObstacleData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_ObstacleViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refObstacle : m_DataSource_Traffic->obstacles) {
      if (id == refObstacle.id) {
        retPtr = std::make_shared<TAD_ObstacleViewer>();
        if (retPtr) {
          retPtr->Init(refObstacle);
        }
        break;
      }
    }
  }
  return retPtr;
}

std::unordered_map<Base::txSysId, Base::ISceneLoader::IObstacleViewerPtr> TAD_SceneLoader::GetAllObstacleData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IObstacleViewerPtr> retMap;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refObstacle : m_DataSource_Traffic->obstacles) {
      auto retPtr = std::make_shared<TAD_ObstacleViewer>();
      if (retPtr) {
        retPtr->Init(refObstacle);
        retMap[refObstacle.id] = retPtr;
      }
    }
  }

  return retMap;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_ObstacleViewer")
TAD_SceneLoader::TAD_ObstacleViewer::STATIC_ELEMENT_TYPE TAD_SceneLoader::TAD_ObstacleViewer::str2type(
    Base::txString const& strType) const TX_NOEXCEPT {
  return __lpsz2enum__(STATIC_ELEMENT_TYPE, strType.c_str());
  /*return Utils::String2ElementType(strType, _plus_(STATIC_ELEMENT_TYPE::Box));*/
#  if 0
  TAD_SceneLoader::TAD_ObstacleViewer::STATIC_ELEMENT_TYPE retValue = STATIC_ELEMENT_TYPE::Box;
  auto opValue = TAD_SceneLoader::TAD_ObstacleViewer::STATIC_ELEMENT_TYPE::_from_string_nocase_nothrow(strType.c_str());
  if (opValue) {
    retValue = *opValue;
  }
  return retValue;
#  endif
}

void TAD_SceneLoader::TAD_ObstacleViewer::Init(SceneLoader::Traffic::Obstacle const& srcObstacle) TX_NOEXCEPT {
  m_obstacle = srcObstacle;
  m_inited = true;
}

Base::txSysId TAD_SceneLoader::TAD_ObstacleViewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_obstacle.id) : (-1));
}

Base::txSysId TAD_SceneLoader::TAD_ObstacleViewer::routeID() const TX_NOEXCEPT {
  return (m_inited ? (m_obstacle.routeID) : (-1));
}

Base::txBool TAD_SceneLoader::TAD_ObstacleViewer::IsRouteIdValid() const TX_NOEXCEPT {
  return m_inited && (m_obstacle.routeID >= 0);
}

Base::txLaneID TAD_SceneLoader::TAD_ObstacleViewer::laneID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.laneID) : (-1));
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.start_s) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.l_offset) : (0.0));
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::length() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.length) : (3.0));
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::width() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.width) : (3.0));
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::height() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.height) : (FLAGS_default_height_of_obstacle_pedestrian));
}

Base::txString TAD_SceneLoader::TAD_ObstacleViewer::vehicleType() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.vehicleType) : Base::txString("Sedan"));
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::start_angle() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_obstacle.start_angle)) {
    return std::stod(m_obstacle.start_angle);
  }
  return IObstacleViewer::start_angle();
}

Base::txString TAD_SceneLoader::TAD_ObstacleViewer::type() const TX_NOEXCEPT {
  // return str2type(vehicleType());
  return vehicleType();
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::direction() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.direction) : (0.0));
}

Base::txString TAD_SceneLoader::TAD_ObstacleViewer::behavior() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.behavior) : (""));
}

std::vector<Base::txSysId> TAD_SceneLoader::TAD_ObstacleViewer::eventId() const TX_NOEXCEPT {
  std::vector<Base::txSysId> ret_event_id;
  if (m_inited) {
    Base::txString strEventId = m_obstacle.eventId;
    Utils::trim(strEventId);
    if (_NonEmpty_(strEventId)) {
      std::vector<Base::txString> results_event_id;
      boost::algorithm::split(results_event_id, strEventId, boost::is_any_of(","));
      for (Base::txString strId : results_event_id) {
        Utils::trim(strId);
        ret_event_id.emplace_back(std::atol(strId.c_str()));
      }
    }
  }
  return ret_event_id;
}

Base::txFloat TAD_SceneLoader::TAD_ObstacleViewer::angle() const TX_NOEXCEPT {
  if (m_inited) {
    return m_obstacle.angle;
  } else {
    return 0.0;
  }
}

Base::txString TAD_SceneLoader::TAD_ObstacleViewer::catalog() const TX_NOEXCEPT {
  if (m_inited) {
    return m_obstacle.catalog;
  } else {
    return "";
  }
}
#endif  // #if  __TX_Mark__("TAD_SceneLoader::TAD_ObstacleViewer")

Base::ISceneLoader::ISignlightsViewerPtr TAD_SceneLoader::GetSignlightData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_SignlightsViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refSignlight : m_DataSource_Traffic->signlights) {
      if (id == refSignlight.id) {
        retPtr = std::make_shared<TAD_SignlightsViewer>();
        if (retPtr) {
          retPtr->Init(refSignlight);
        }
        break;
      }
    }
  }
  return retPtr;
}

std::unordered_map<Base::txSysId, Base::ISceneLoader::ISignlightsViewerPtr> TAD_SceneLoader::GetAllSignlightData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::ISignlightsViewerPtr> retMap;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (auto& refSignlight : m_DataSource_Traffic->signlights) {
      if (boost::equal(std::string("Activated"), refSignlight.status)) {
        auto retPtr = std::make_shared<TAD_SignlightsViewer>();
        if (NonNull_Pointer(retPtr)) {
          retPtr->Init(refSignlight);
          retMap[refSignlight.id] = retPtr;
        }
      } else {
        LOG(WARNING) << "Deactivated Signal :" << refSignlight;
      }
    }
  }
  return retMap;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_SignlightsViewer")

void TAD_SceneLoader::TAD_SignlightsViewer::Init(SceneLoader::Traffic::Signlight const& srcSignlight) TX_NOEXCEPT {
  m_signlight = srcSignlight;
  m_inited = true;
}

Base::txSysId TAD_SceneLoader::TAD_SignlightsViewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_signlight.id) : (-1));
}

Base::txSysId TAD_SceneLoader::TAD_SignlightsViewer::routeID() const TX_NOEXCEPT {
  return (m_inited ? (m_signlight.routeID) : (-1));
}

Base::txBool TAD_SceneLoader::TAD_SignlightsViewer::IsRouteIdValid() const TX_NOEXCEPT {
  return m_inited && (m_signlight.routeID >= 0);
}

Base::txLaneID TAD_SceneLoader::TAD_SignlightsViewer::laneID() const TX_NOEXCEPT {
  LOG(FATAL) << ", Unsupport Props.";
  return -1;
}

Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.start_s) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::start_t() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.start_t) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.l_offset) : (0.0));
}

Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::time_green() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.time_green) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::time_yellow() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.time_yellow) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::time_red() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.time_red) : (0));
}

Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::direction() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.direction) : (0));
}
#  if USE_SignalByLane
Base::txFloat TAD_SceneLoader::TAD_SignlightsViewer::compliance() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.compliance) : (1.0));
}

Base::txString TAD_SceneLoader::TAD_SignlightsViewer::lane() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.lane) : ("ALL"));
}

Base::txString TAD_SceneLoader::TAD_SignlightsViewer::phase() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.phase) : ("A"));
}

Base::txString TAD_SceneLoader::TAD_SignlightsViewer::status() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.status) : ("Activated"));
}
#  endif /*USE_SignalByLane*/

#  if __TX_Mark__("signal control configure")
Base::txString TAD_SceneLoader::TAD_SignlightsViewer::plan() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.plan) : (ISignlightsViewer::plan()));
}

Base::txString TAD_SceneLoader::TAD_SignlightsViewer::junction() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.junction) : (ISignlightsViewer::junction()));
}

Base::txString TAD_SceneLoader::TAD_SignlightsViewer::phaseNumber() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.phaseNumber) : (ISignlightsViewer::phaseNumber()));
}

Base::txString TAD_SceneLoader::TAD_SignlightsViewer::signalHead() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.signalHead) : (ISignlightsViewer::signalHead()));
}

Base::txString TAD_SceneLoader::TAD_SignlightsViewer::eventId() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.eventId) : (ISignlightsViewer::eventId()));
}

#  endif /*signal control configure*/

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_PedestriansViewer")

Base::txInt TAD_SceneLoader::GetRandomSeed() const TX_NOEXCEPT {
  if (IsSelfDefineRandomSeed()) {
    return GetSelfDefineRandomSeed();
  } else {
    if (NonNull_Pointer(m_DataSource_Traffic) && (m_DataSource_Traffic->data.randomseed > 0)) {
      return m_DataSource_Traffic->data.randomseed;
    } else {
      return FLAGS_default_random_seed;
    }
  }
}

Base::txFloat TAD_SceneLoader::GetRuleComplianceProportion() const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Traffic) && (m_DataSource_Traffic->data.ruleCompliance > 0)) {
    return m_DataSource_Traffic->data.ruleCompliance;
  } else {
    return FLAGS_DefaultRuleComplianceProportion;
  }
}

Base::ISceneLoader::IVehiclesViewerPtr TAD_SceneLoader::GetEgoData() TX_NOEXCEPT {
#if Use_TruckEgo
  if (_plus_(EgoType::eVehicle) == GetEgoType()) {
    TAD_EgoVehiclesViewerPtr retPtr = std::make_shared<TAD_EgoVehiclesViewer>();
    retPtr->Init(m_EgoData);
    return retPtr;
  } else {
    TAD_TruckVehiclesViewerPtr retPtr = std::make_shared<TAD_TruckVehiclesViewer>();
    retPtr->Init(m_EgoData);
    return retPtr;
  }
#else
  TAD_EgoVehiclesViewerPtr retPtr = std::make_shared<TAD_EgoVehiclesViewer>();
  retPtr->Init(m_EgoData);
  return retPtr;
#endif
}

Base::ISceneLoader::IVehiclesViewerPtr TAD_SceneLoader::GetEgoTrailerData() TX_NOEXCEPT {
  if (_plus_(EgoType::eVehicle) == GetEgoType()) {
    return nullptr;
  } else {
    TAD_TrailerVehiclesViewerPtr retPtr = std::make_shared<TAD_TrailerVehiclesViewer>();
    retPtr->Init(m_EgoData);
    return retPtr;
  }
}

TAD_SceneLoader::ILocationViewerPtr TAD_SceneLoader::GetLocationData(const Base::txUInt id) TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    for (const auto& refLocation : (m_DataSource_Traffic->Locations)) {
      if (id == std::stoi(refLocation.id)) {
        TAD_LocationViewerPtr retPtr = std::make_shared<TAD_LocationViewer>();
        retPtr->Init(refLocation);
        return retPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find location, id = " << id;
  return std::make_shared<TAD_SceneLoader::ILocationViewer>();
}

TAD_SceneLoader::ITrafficFlowViewerPtr TAD_SceneLoader::GetTrafficFlow() TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    const auto& refTrafficFlow = m_DataSource_Traffic->trafficFlow;
    TAD_TrafficFlowViewerPtr retPtr = std::make_shared<TAD_TrafficFlowViewer>();
    retPtr->Init(refTrafficFlow);
    return retPtr;
  }
  LOG(WARNING) << "TAD Cloud Traffic Flow Not Inited.";
  return std::make_shared<TAD_TrafficFlowViewer>();
}

std::unordered_map<Base::txInt, Base::ISceneLoader::ILocationViewerPtr> TAD_SceneLoader::GetAllLocationData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txInt, Base::ISceneLoader::ILocationViewerPtr> retV;
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    const auto& refLocations = m_DataSource_Traffic->Locations;
    for (const auto& refLoc : refLocations) {
      TAD_LocationViewerPtr tcLocationPtr = std::make_shared<TAD_LocationViewer>();
      tcLocationPtr->Init(refLoc);
      retV[tcLocationPtr->id()] = tcLocationPtr;
    }
  }
  LOG(INFO) << TX_VARS_NAME(LocationCount, retV.size());
  return retV;
}

#if __TX_Mark__("TAD_LocationViewer")

TAD_SceneLoader::TAD_LocationViewer::TAD_LocationViewer() : m_inited(false) {}

void TAD_SceneLoader::TAD_LocationViewer::Init(const Location& srcLocation) TX_NOEXCEPT {
  m_location = srcLocation;
  m_inited = true;
}

Base::txSysId TAD_SceneLoader::TAD_LocationViewer::id() const TX_NOEXCEPT {
  return (m_inited) ? (stoll(m_location.id)) : (InvalidId);
}

hadmap::txPoint TAD_SceneLoader::TAD_LocationViewer::PosGPS() const TX_NOEXCEPT {
  if (m_inited) {
    txString raw_str = m_location.Pos;
    Utils::trim(raw_str);
    if (CallFail(raw_str.empty())) {
      std::vector<Base::txString> results_lon_lat;
      boost::algorithm::split(results_lon_lat, raw_str, boost::is_any_of(","));
      if ((results_lon_lat.size() > 1)) {
        Utils::trim(results_lon_lat[0]);
        Utils::trim(results_lon_lat[1]);
        if (CallFail(results_lon_lat[0].empty()) && CallFail(results_lon_lat[1].empty())) {
          return hadmap::txPoint(std::stod(results_lon_lat[0]), std::stod(results_lon_lat[1]), FLAGS_default_altitude);
        }
      }
    }
  }
  LOG(WARNING) << "Get Location Pos Failure.";
  return hadmap::txPoint();
}

Base::txString TAD_SceneLoader::TAD_LocationViewer::info() const TX_NOEXCEPT {
  return (m_inited) ? (m_location.info) : ("uninited");
}

Base::txString TAD_SceneLoader::TAD_LocationViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, m_location.id) << TX_VARS_NAME(Pos, m_location.Pos)
      << TX_VARS_NAME(info, m_location.info);
  return oss.str();
}
#endif /*__TX_Mark__("TAD_LocationViewer")*/

#if __TX_Mark__("TTAD_TrafficFlowViewer")

#  if __TX_Mark__("TAD_VehType")

TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::TAD_VehType() : m_inited(false) {}

void TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::Init(const VehType& srcVehType) TX_NOEXCEPT {
  m_VehType = srcVehType;
  m_inited = true;
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::id() const TX_NOEXCEPT {
  return (m_inited) ? (stoll(m_VehType.id)) : (InvalidId);
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::vehicleType() const TX_NOEXCEPT {
  if (m_inited) {
    // return __lpsz2enum__(VEHICLE_TYPE, m_VehType.Type.c_str());
    return m_VehType.Type;
    /*return Utils::String2ElementType(m_VehType.Type, _plus_(Base::ISceneLoader::VEHICLE_TYPE::Sedan));*/
  }
  // return _plus_(Base::ISceneLoader::VEHICLE_TYPE::Sedan);
  return "Sedan";
}

Base::ISceneLoader::VEHICLE_BEHAVIOR TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::behavior() const TX_NOEXCEPT {
  if (m_inited) {
    return Base::ISceneLoader::Str2VehicleBehavior(m_VehType.behavior);
  }
  return _plus_(Base::ISceneLoader::VEHICLE_BEHAVIOR::undefined);
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::length() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehType.length) : (0.0);
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::width() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehType.width) : (0.0);
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::height() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehType.height) : (0.0);
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehType::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, m_VehType.id)
      << TX_VARS_NAME(behavior, m_VehType.behavior) << TX_VARS_NAME(length, m_VehType.length)
      << TX_VARS_NAME(width, m_VehType.width) << TX_VARS_NAME(height, m_VehType.height);
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_VehType")*/

#  if __TX_Mark__("TAD_VehComp")

TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::TAD_VehComp() : m_inited(false) {}

void TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::Init(const VehComp& srcVehComp) TX_NOEXCEPT {
  m_VehComp = srcVehComp;
  m_inited = true;
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.id) : (InvalidId);
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::Type(const txInt idx) const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.Type(idx)) : (IVehComp::Type(idx));
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::Percentage(const txInt idx) const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.Percentage(idx)) : (IVehComp::Percentage(idx));
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::Behavior(const txInt idx) const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.Behavior(idx)) : (IVehComp::Behavior(idx));
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::Aggress(const txInt idx) const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehComp.Aggress(idx)) : (IVehComp::Aggress(idx));
}

Base::txBool TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::IsValid(const txInt idx) const TX_NOEXCEPT {
  return (m_inited) && (_NonEmpty_(m_VehComp.Type(idx))) && (_NonEmpty_(m_VehComp.Percentage(idx))) &&
         (_NonEmpty_(m_VehComp.Behavior(idx))) && (_NonEmpty_(m_VehComp.Aggress(idx)));
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::Info() const TX_NOEXCEPT {
  return (m_inited) ? (m_VehComp.Info) : ("uninited");
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id());
  for (Base::txInt idx = 0; idx < m_VehComp.CompNodeArray.size(); ++idx) {
    oss << TX_VARS(idx) << TX_VARS_NAME(Type, Type(idx)) << TX_VARS_NAME(Percentage, Percentage(idx))
        << TX_VARS_NAME(Behavior, Behavior(idx)) << TX_VARS_NAME(Aggress, Aggress(idx));
  }
  oss << TX_VARS_NAME(Info, Info());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_VehComp")*/

#  if __TX_Mark__("TAD_VehInput")

TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::TAD_VehInput() : m_inited(false) {}

void TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::Init(const VehInput& srcVehInput) TX_NOEXCEPT {
  m_VehInput = srcVehInput;
  m_inited = true;
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.id) : (InvalidId);
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::location() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.Location) : (IVehInput::location());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::composition() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.Composition) : (IVehInput::composition());
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::start_v() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehInput.start_v) : (IVehInput::start_v());
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::max_v() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehInput.max_v) : (IVehInput::max_v());
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::halfRange_v() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_VehInput.halfRange_v)) {
    return stod(m_VehInput.halfRange_v);
  }
  return IVehInput::halfRange_v();
  /*return (m_inited) ? stod(m_VehInput.halfRange_v) : (IVehInput::halfRange_v());*/
}

TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::Distribution
TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::distribution() const TX_NOEXCEPT {
  return (m_inited) ? (IVehInput::Str2Distribution(m_VehInput.Distribution)) : (IVehInput::distribution());
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::timeHeadway() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehInput.TimeHeadway) : (IVehInput::timeHeadway());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::duration() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.Duration) : (IVehInput::duration());
}

std::set<Base::txLaneID> TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::cover() const TX_NOEXCEPT {
  if (m_inited) {
    std::set<Base::txLaneID> res;
    if (m_VehInput.Cover.empty() || "100" == m_VehInput.Cover) {
      for (int i = 1; i < 20; ++i) {
        res.insert(-1 * i);
      }
    } else {
      const auto refResult = Utils::SpliteStringVector(m_VehInput.Cover, ",");
      for (const auto& refStr : refResult) {
        res.insert(std::atoi(refStr.c_str()));
      }
    }
    return res;
  } else {
    return IVehInput::cover();
  }
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::cover_str() const TX_NOEXCEPT {
  return (m_inited) ? (m_VehInput.Cover) : (IVehInput::cover_str());
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::Info() const TX_NOEXCEPT { return ""; }

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehInput::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(location, location())
      << TX_VARS_NAME(composition, composition()) << TX_VARS_NAME(start_v, start_v()) << TX_VARS_NAME(max_v, max_v())
      << TX_VARS_NAME(halfRange_v, halfRange_v()) << TX_VARS_NAME(distribution, distribution())
      << TX_VARS_NAME(timeHeadway, timeHeadway()) << TX_VARS_NAME(duration, duration())
      << TX_VARS_NAME(cover, cover_str())

      << TX_VARS_NAME(Info, Info());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_VehInput")*/

#  if __TX_Mark__("TAD_Beh")
TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::TAD_Beh() : m_inited(false) {}

void TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::Init(const Beh& srcBeh) TX_NOEXCEPT {
  m_Beh = srcBeh;
  m_inited = true;
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_Beh.id) : (IBeh::id());
}

TAD_SceneLoader::TAD_TrafficFlowViewer::IBeh::BehType TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::type() const
    TX_NOEXCEPT {
  return (m_inited) ? (IBeh::Str2BehType(m_Beh.Type)) : (IBeh::type());
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::cc0() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.cc0)) {
    return stod(m_Beh.cc0);
  }
  return IBeh::cc0();
}
Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::cc1() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.cc1)) {
    return stod(m_Beh.cc1);
  }
  return IBeh::cc1();
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::cc2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.cc2)) {
    return stod(m_Beh.cc2);
  }
  return IBeh::cc2();
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::AX() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.AX)) {
    return stod(m_Beh.AX);
  }
  return IBeh::AX();
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::BX_Add() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.BX_Add)) {
    return stod(m_Beh.BX_Add);
  }
  return IBeh::BX_Add();
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::LCduration() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.LCduration)) {
    return stod(m_Beh.LCduration);
  }
  return IBeh::LCduration();
}

Base::txFloat TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::BX_Mult() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.BX_Mult)) {
    return stod(m_Beh.BX_Mult);
  }
  return IBeh::BX_Mult();
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_Beh::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(type, type())
      << TX_VARS_NAME(cc0, cc0()) << TX_VARS_NAME(cc1, cc1()) << TX_VARS_NAME(cc2, cc2())

      << TX_VARS_NAME(AX, AX()) << TX_VARS_NAME(BX_Add, BX_Add()) << TX_VARS_NAME(BX_Mult, BX_Mult())
      << TX_VARS_NAME(LCduration, LCduration());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Beh")*/

#  if __TX_Mark__("TAD_VehExit")
TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::TAD_VehExit() : m_inited(false) {}

void TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::Init(const VehExit& srcVehExit) TX_NOEXCEPT {
  m_VehExit = srcVehExit;
  m_inited = true;
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehExit.id) : (IVehExit::id());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::location() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehExit.Location) : (IVehExit::location());
}

std::set<Base::txLaneID> TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::cover() const TX_NOEXCEPT {
  if (m_inited) {
    std::set<Base::txLaneID> res;
    if (m_VehExit.Cover.empty() || "100" == m_VehExit.Cover) {
      for (int i = 1; i < 20; ++i) {
        res.insert(-1 * i);
      }
    } else {
      const auto refResult = Utils::SpliteStringVector(m_VehExit.Cover, ",");
      for (const auto& refStr : refResult) {
        res.insert(std::atoi(refStr.c_str()));
      }
    }
    return res;
  } else {
    return IVehExit::cover();
  }
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::cover_str() const TX_NOEXCEPT {
  return (m_inited) ? (m_VehExit.Cover) : (IVehExit::cover_str());
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::Info() const TX_NOEXCEPT { return ""; }

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehExit::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(location, location())
      << TX_VARS_NAME(cover, cover_str()) << TX_VARS_NAME(Info, Info());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_VehExit")*/

#  if __TX_Mark__("TAD_RouteGroup")

void TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::Init(const RouteGroup& srcRouteGroup) TX_NOEXCEPT {
  m_RouteGroup = srcRouteGroup;
  m_inited = true;
}

TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::TAD_RouteGroup() : m_inited(false) {}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.id) : (IRouteGroup::id());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::start() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.Start) : (IRouteGroup::start());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::mid1() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.Mid1) : (IRouteGroup::mid1());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::end1() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.End1) : (IRouteGroup::end1());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::percentage1() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.Percentage1) : (IRouteGroup::percentage1());
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::mid2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Mid2)) {
    return stoi(m_RouteGroup.Mid2);
  }
  return IRouteGroup::mid2();
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::end2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.End2)) {
    return stoi(m_RouteGroup.End2);
  }
  return IRouteGroup::end2();
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::percentage2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Percentage2)) {
    return stoi(m_RouteGroup.Percentage2);
  }
  return IRouteGroup::percentage2();
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::mid3() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Mid3)) {
    return stoi(m_RouteGroup.Mid3);
  }
  return IRouteGroup::mid3();
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::end3() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.End3)) {
    return stoi(m_RouteGroup.End3);
  }
  return IRouteGroup::end3();
}

Base::txInt TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::percentage3() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Percentage3)) {
    return stoi(m_RouteGroup.Percentage3);
  }
  return IRouteGroup::percentage3();
}

Base::txString TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_RouteGroup::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(start, start())
      << TX_VARS_NAME(mid1, mid1()) << TX_VARS_NAME(end1, end1()) << TX_VARS_NAME(percentage1, percentage1())

      << TX_VARS_NAME(mid2, mid2()) << TX_VARS_NAME(end2, end2()) << TX_VARS_NAME(percentage2, percentage2())

      << TX_VARS_NAME(mid3, mid3()) << TX_VARS_NAME(end3, end3()) << TX_VARS_NAME(percentage3, percentage3());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_RouteGroup")*/

void TAD_SceneLoader::TAD_TrafficFlowViewer::Init(const TrafficFlowXML& srcTrafficFlow) TX_NOEXCEPT {
  m_trafficflow = srcTrafficFlow;
  m_inited = true;
}

std::unordered_map<Base::txInt, TAD_SceneLoader::TAD_TrafficFlowViewer::VehInputPtr>
TAD_SceneLoader::TAD_TrafficFlowViewer::GetAllVehInputData() const TX_NOEXCEPT {
  std::unordered_map<Base::txInt, VehInputPtr> retV;
  if (IsInited()) {
    for (const auto& refVehInput : m_trafficflow.VehicleInput) {
      TAD_VehInputPtr curPtr = std::make_shared<TAD_VehInput>();
      curPtr->Init(refVehInput);
      retV[curPtr->id()] = curPtr;
    }
  }
  return retV;
}

std::unordered_map<Base::txInt, TAD_SceneLoader::TAD_TrafficFlowViewer::RouteGroupPtr>
TAD_SceneLoader::TAD_TrafficFlowViewer::GetAllRouteGroupData() const TX_NOEXCEPT {
  std::unordered_map<Base::txInt, RouteGroupPtr> retV;
  if (IsInited()) {
    for (const auto& refRouteGroup : m_trafficflow.RouteGroups) {
      TAD_RouteGroupPtr curPtr = std::make_shared<TAD_RouteGroup>();
      curPtr->Init(refRouteGroup);
      retV[curPtr->id()] = curPtr;
    }
  }
  return retV;
}

std::unordered_map<Base::txInt, TAD_SceneLoader::TAD_TrafficFlowViewer::VehExitPtr>
TAD_SceneLoader::TAD_TrafficFlowViewer::GetAllVehExitData() const TX_NOEXCEPT {
  std::unordered_map<Base::txInt, VehExitPtr> retV;
  if (IsInited()) {
    for (const auto& refVehExit : m_trafficflow.VehicleExit) {
      TAD_VehExitPtr curPtr = std::make_shared<TAD_VehExit>();
      curPtr->Init(refVehExit);
      retV[curPtr->id()] = curPtr;
    }
  }
  return retV;
}

TAD_SceneLoader::TAD_TrafficFlowViewer::VehCompPtr TAD_SceneLoader::TAD_TrafficFlowViewer::GetVehComp(
    const txInt id) const TX_NOEXCEPT {
  if (m_inited) {
    const txString strObjId = std::to_string(id);
    for (const auto& refComp : m_trafficflow.VehicleComposition) {
      if (strObjId == refComp.id) {
        TAD_VehCompPtr curPtr = std::make_shared<TAD_VehComp>();
        curPtr->Init(refComp);
        return curPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find VehComp, id = " << id;
  return std::make_shared<TAD_SceneLoader::TAD_TrafficFlowViewer::TAD_VehComp>();
}

TAD_SceneLoader::TAD_TrafficFlowViewer::VehTypePtr TAD_SceneLoader::TAD_TrafficFlowViewer::GetVehType(
    const Base::txInt id) const TX_NOEXCEPT {
  if (m_inited) {
    const txString strObjId = std::to_string(id);
    for (const auto& refVehType : m_trafficflow.VehicleType) {
      if (strObjId == refVehType.id) {
        TAD_VehTypePtr curPtr = std::make_shared<TAD_VehType>();
        curPtr->Init(refVehType);
        return curPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find VehType, id = " << id;
  return std::make_shared<TAD_VehType>();
}

TAD_SceneLoader::TAD_TrafficFlowViewer::BehPtr TAD_SceneLoader::TAD_TrafficFlowViewer::GetBeh(const txInt id) const
    TX_NOEXCEPT {
  if (m_inited) {
    const txString strObjId = std::to_string(id);
    for (const auto& refBeh : m_trafficflow.Behavior) {
      if (strObjId == refBeh.id) {
        TAD_BehPtr curPtr = std::make_shared<TAD_Beh>();
        curPtr->Init(refBeh);
        return curPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find Beh, id = " << id;
  return std::make_shared<TAD_Beh>();
}

#endif /*__TX_Mark__("TTAD_TrafficFlowViewer")*/

#if __TX_Mark__("Scene Event")
Base::txBool TAD_SceneLoader::ParseSceneEvent() TX_NOEXCEPT {
  // 设置场景事件版本
  SetSceneEventVersion(ParentClass::eSceneEventVersion::V_0_0_0_0);
  _seceneEventVec.clear();
  // scene traffic对象不为空
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    // 获取场景事件集合
    const SceneLoader::Traffic::scene_event& ref_scene_event = m_DataSource_Traffic->scene_event_array;
    // 若场景事件版本是1.0.0.0
    if (boost::equal(txString("1.0.0.0"), ref_scene_event.version)) {
      // 设置版本
      SetSceneEventVersion(ParentClass::eSceneEventVersion::V_1_0_0_0);
      // 遍历场景事件集合
      for (const auto& ref_event : ref_scene_event.event_array) {
        LogInfo << "parse scene event id : " << ref_event.id;
        ISceneEventViewerPtr ptr = std::make_shared<ISceneEventViewer>();
        ptr->set_version(ParentClass::eSceneEventVersion::V_1_0_0_0);
        try {
          const Base::txSysId evId = std::atol(ref_event.id.c_str());
          // 生成事件condition的kv
          kvMap conditionKVMap = ISceneEventViewer::generateKVMap(ref_event.condition);
          conditionKVMap["type"] = ref_event.type;
          // 生成场景事件结束condtion的集合
          kvMapVec endConditionKVMapVec = ISceneEventViewer::generateKVMapVec(ref_event.endCondition);
          // 生成场景事件的行为集合
          kvMapVec actionKVMapVec = ISceneEventViewer::generateKVMapVec(ref_event.action);
          if (endConditionKVMapVec.size() == actionKVMapVec.size() && 3 == actionKVMapVec.size()) {
            std::vector<txInt> uncheckedIdxVec;
            // 遍历动作
            for (txInt i = 0; i < actionKVMapVec.size(); ++i) {
              if (_Contain_(actionKVMapVec[i], "checked")) {
                if ("true" != actionKVMapVec[i].at("checked")) {
                  // 添加到check的集合中
                  uncheckedIdxVec.emplace_back(i);
                }
              } else {
                uncheckedIdxVec.emplace_back(i);
              }
            }
            for (txInt i = (uncheckedIdxVec.size() - 1); i >= 0; --i) {
              TX_MARK("reverse iterator");
              const txInt deleteIdx = uncheckedIdxVec[i];
              endConditionKVMapVec.erase(endConditionKVMapVec.begin() + deleteIdx);
              actionKVMapVec.erase(actionKVMapVec.begin() + deleteIdx);
              LogWarn << TX_VARS(evId) << " ignore action index " << deleteIdx;
            }
            if (_NonEmpty_(conditionKVMap) && _NonEmpty_(endConditionKVMapVec) && _NonEmpty_(actionKVMapVec) &&
                endConditionKVMapVec.size() == actionKVMapVec.size()) {
              if (CallSucc(ptr->initialize(evId, conditionKVMap, endConditionKVMapVec, actionKVMapVec))) {
                LogInfo << "parse scene event id : " << ref_event.id << " success.";
                // 到此解析成功添加事件视图
                _seceneEventVec.push_back(ptr);
              } else {
                LogWarn << "parse scene event id : " << ref_event.id << " failure." << TX_VARS(ref_event);
                /*_seceneEventVec.clear();
                return false;*/
              }
            } else {
              LogWarn << TX_VARS_NAME(ignore_event_id, evId) << TX_VARS(conditionKVMap.size())
                      << TX_VARS(endConditionKVMapVec.size()) << TX_VARS(actionKVMapVec.size())
                      << TX_COND(endConditionKVMapVec.size() == actionKVMapVec.size());
              /*_seceneEventVec.clear();
return false;*/
            }
          } else {
            LogWarn << "scene event error. " << TX_VARS(endConditionKVMapVec.size()) << TX_VARS(actionKVMapVec.size());
          }
        } catch (const std::exception& e) {
          LOG(WARNING) << e.what();
          _seceneEventVec.clear();
          return false;
        }
      }
      return true;
    } else if (boost::equal(txString("1.1.0.0"), ref_scene_event.version)) {
      SetSceneEventVersion(ParentClass::eSceneEventVersion::V_1_1_0_0);
      LogInfo1100 << "ParseSceneEvent 1.1.0.0";
      for (const auto& ref_event : ref_scene_event.event_array) {
        LogInfo << "parse scene event id : " << ref_event.id;
        ISceneEventViewerPtr ptr = std::make_shared<ISceneEventViewer>();
        ptr->set_version(ParentClass::eSceneEventVersion::V_1_1_0_0);
        try {
          const Base::txSysId evId = std::atol(ref_event.id.c_str());
          kvMap conditionKVMap = ISceneEventViewer::generateKVMap(ref_event.condition);
          conditionKVMap["type"] = ref_event.type;
          kvMapVec endConditionKVMapVec = ISceneEventViewer::generateKVMapVec(ref_event.endCondition);
          kvMapVec actionKVMapVec = ISceneEventViewer::generateKVMapVec_Json(ref_event.action);
          if (endConditionKVMapVec.size() == actionKVMapVec.size()) {
            if (_NonEmpty_(conditionKVMap) && _NonEmpty_(endConditionKVMapVec) && _NonEmpty_(actionKVMapVec) &&
                endConditionKVMapVec.size() == actionKVMapVec.size()) {
              if (CallSucc(ptr->initialize(evId, conditionKVMap, endConditionKVMapVec, actionKVMapVec))) {
                LogInfo << "parse scene event id : " << ref_event.id << " success.";
                _seceneEventVec.push_back(ptr);
              } else {
                LogWarn << "parse scene event id : " << ref_event.id << " failure." << TX_VARS(ref_event);
                /*_seceneEventVec.clear();
                return false;*/
              }
            } else {
              LogWarn << TX_VARS_NAME(ignore_event_id, evId) << TX_VARS(conditionKVMap.size())
                      << TX_VARS(endConditionKVMapVec.size()) << TX_VARS(actionKVMapVec.size())
                      << TX_COND(endConditionKVMapVec.size() == actionKVMapVec.size());
              /*_seceneEventVec.clear();
              return false;*/
            }
          } else {
            LogWarn << "scene event error. " << TX_VARS(endConditionKVMapVec.size()) << TX_VARS(actionKVMapVec.size());
          }
        } catch (const std::exception& e) {
          LOG(WARNING) << e.what();
          _seceneEventVec.clear();
          return false;
        }
      }

      return true;
    } else if (boost::equal(txString("1.2.0.0"), ref_scene_event.version)) {
#  if __SecenEventVersion_1_2_0_0__
      SetSceneEventVersion(ParentClass::eSceneEventVersion::V_1_2_0_0);
      LogInfo1200 << "ParseSceneEvent 1.2.0.0";
      for (const auto& ref_event : ref_scene_event.event_array) {
        LogInfo << "parse scene event id : " << ref_event.id;
        ISceneEventViewerPtr ptr = std::make_shared<ISceneEventViewer>();
        ptr->set_version(ParentClass::eSceneEventVersion::V_1_2_0_0);
        try {
          const Base::txSysId evId = std::atol(ref_event.id.c_str());
          if (CallSucc(ref_event.IsConditionGroup())) {
            EventGroupParam_t evGroup;
            evGroup.conditionNumber = std::atoi(ref_event.conditionNumber.c_str());
            for (Base::txInt idx = 0; idx < evGroup.conditionNumber; ++idx) {
              kvMap conditionKVMap = ISceneEventViewer::generateKVMap(ref_event.groupCondition(idx));
              conditionKVMap["type"] = ref_event.groupType(idx);
              evGroup.groupConditionVec.emplace_back(conditionKVMap);
            }
            evGroup.endConditionKVMapVec = ISceneEventViewer::generateKVMapVec(ref_event.endCondition);
            evGroup.actionKVMapVec = ISceneEventViewer::generateKVMapVec_Json(ref_event.action);

            if (CallSucc(evGroup.IsValid())) {
              if (ptr->initialize(evId, evGroup)) {
                LogInfo << "parse scene event id : " << ref_event.id << " success.";
                _seceneEventVec.push_back(ptr);
              } else {
                LogWarn << "parse scene group condition event id : " << ref_event.id << " failure."
                        << TX_VARS(ref_event);
              }
            } else {
              LogWarn << "scene event error. " << TX_VARS(ref_event) << TX_VARS(evGroup.groupConditionVec.size())
                      << TX_VARS(evGroup.endConditionKVMapVec.size()) << TX_VARS(evGroup.actionKVMapVec.size());
            }
          } else {
            LOG(WARNING) << TX_VARS(ref_event.conditionNumber) << "  error.";
          }
        } catch (const std::exception& e) {
          LOG(WARNING) << e.what();
          _seceneEventVec.clear();
          return false;
        }
      }

      return true;
    } else {
      LogWarn << "un support scene event version " << ref_scene_event.version << ", support version 1.0.0.0";
      return false;
    }
#  endif /*__SecenEventVersion_1_2_0_0__*/
  } else {
    return false;
  }
}
#endif /*__TX_Mark__("Scene Event")*/

Base::txFloat TAD_SceneLoader::PlannerStartV() const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    return m_DataSource_Scene->_planner.start_v;
  } else {
    return ParentClass::PlannerStartV();
  }
}

Base::txFloat TAD_SceneLoader::PlannerTheta() const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    return m_DataSource_Scene->_planner.theta;
  } else {
    return ParentClass::PlannerTheta();
  }
}

Base::txFloat TAD_SceneLoader::PlannerVelocityMax() const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    return m_DataSource_Scene->_planner.Velocity_Max;
  } else {
    return ParentClass::PlannerVelocityMax();
  }
}

Base::txString TAD_SceneLoader::activePlan() const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Traffic)) {
    return m_DataSource_Traffic->signlights_activePlan;
  } else {
    return ParentClass::activePlan();
  }
}

Base::txBool TAD_SceneLoader::Planner_Trajectory_Enabled() const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    return ("true" == (m_DataSource_Scene->_planner.trajectory_enabled));
  } else {
    return ParentClass::Planner_Trajectory_Enabled();
  }
}

TAD_SceneLoader::IRouteViewer::control_path_node_vec TAD_SceneLoader::ControlPath() const TX_NOEXCEPT {
  using txString = Base::txString;
  IRouteViewer::control_path_node_vec ret_conrtol_path;
  txString str_control_path = m_DataSource_Scene->_planner.control_path.points;
  LOG(INFO) << TX_VARS(str_control_path);

  const auto res_waypoints = Utils::SpliteStringVector(str_control_path, ";");
  for (auto waypt : res_waypoints) {
    LOG(INFO) << TX_VARS(waypt);
    const auto res_kinect_params = Utils::SpliteStringVector(waypt, ",");
    if (res_kinect_params.size() > 5) {
      LOG(INFO) << TX_VARS_NAME(lon, res_kinect_params[0]) << TX_VARS_NAME(lat, res_kinect_params[1])
                << TX_VARS_NAME(alt, res_kinect_params[2]) << TX_VARS_NAME(speed, res_kinect_params[3])
                << TX_VARS_NAME(gear, res_kinect_params[4]);
      IRouteViewer::control_path_node cp_node;
      __Lon__(cp_node.waypoint) = std::stod(res_kinect_params[0]);
      __Lat__(cp_node.waypoint) = std::stod(res_kinect_params[1]);
      __Alt__(cp_node.waypoint) = std::stod(res_kinect_params[2]);
      cp_node.speed_m_s = std::stod(res_kinect_params[3]);
      cp_node.gear = ("reverse" == res_kinect_params[4]) ? (_plus_(Base::Enums::ControlPathGear::reverse))
                                                         : (_plus_(Base::Enums::ControlPathGear::drive));

      ret_conrtol_path.emplace_back(std::move(cp_node));
    } else {
      LogWarn << "[Error]" << TX_VARS(res_kinect_params.size()) << TX_VARS(waypt);
    }
  }
  return ret_conrtol_path;
}

#if USE_EgoGroup

std::vector<Base::txString> TAD_SceneLoader::GetAllEgoGroups() const TX_NOEXCEPT {
  std::vector<Base::txString> all_groups;
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        all_groups.push_back(egoInfo.ego.group);
      }
    } else {
      all_groups.push_back("Ego_001");
    }
  }
  return all_groups;
}

Base::txFloat TAD_SceneLoader::GetSimSimulationPlannerTheta(Base::txString egoGroup) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return egoInfo.theta;
        }
      }
      return 0.0;
    } else {
      return m_DataSource_Scene->_planner.theta;
    }
  } else {
    return 0.0;
  }
}

Base::txBool TAD_SceneLoader::GetSimSimulationPlannerRouteStart(
    Base::txString egoGroup, std::tuple<Base::txFloat, Base::txFloat>& res) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          Base::txString strStart = (egoInfo.route.start);

          boost::container::small_vector<Base::txString, 2> results;
          boost::algorithm::split(results, strStart, boost::is_any_of(","));
          // LOG(WARNING) << TX_VARS(egoGroup) << TX_VARS(strStart) << TX_VARS(results.size());
          if (results.size() > 1) {
            try {
              res = std::make_tuple<Base::txFloat, Base::txFloat>(std::stod(results[0]), std::stod(results[1]));
            } catch (const std::invalid_argument&) {
              FLAGS_error_info = (R"(ErrorContent:[)") + egoInfo.route.start + (R"(])");
              LOG(WARNING) << "GetSimSimulationPlannerRouteStart " << TX_VARS_NAME(ErrorContent, egoInfo.route.start);
              return false;
            } catch (const std::out_of_range&) {
              FLAGS_error_info = (R"(ErrorContent:[)") + egoInfo.route.start + (R"(])");
              LOG(WARNING) << "GetSimSimulationPlannerRouteStart " << TX_VARS_NAME(ErrorContent, egoInfo.route.start);
              return false;
            } catch (...) {
              FLAGS_error_info = (R"(ErrorContent:[)") + egoInfo.route.start + (R"(])");
              LOG(WARNING) << "GetSimSimulationPlannerRouteStart " << TX_VARS_NAME(ErrorContent, egoInfo.route.start);
              return false;
            }
            return true;
          } else {
            LOG(WARNING) << "error egoInfo.start " << TX_VARS(results.size());
            return false;
          }
        }
      }
      LOG(WARNING) << "GetSimSimulationPlannerRouteStart cannot find ego " << TX_VARS(egoGroup);
      return false;
    } else {
      Base::txString strStart = (m_DataSource_Scene->_planner.route.start);

      boost::container::small_vector<Base::txString, 2> results;
      boost::algorithm::split(results, strStart, boost::is_any_of(","));
      LOG(WARNING) << TX_VARS(strStart) << TX_VARS(results.size());
      if (results.size() > 1) {
        try {
          res = std::make_tuple<Base::txFloat, Base::txFloat>(std::stod(results[0]), std::stod(results[1]));
        } catch (const std::invalid_argument&) {
          FLAGS_error_info = (R"(ErrorContent:[)") + m_DataSource_Scene->_planner.route.start + (R"(])");
          LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                       << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
          return false;
        } catch (const std::out_of_range&) {
          FLAGS_error_info = (R"(ErrorContent:[)") + m_DataSource_Scene->_planner.route.start + (R"(])");
          LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                       << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
          return false;
        } catch (...) {
          FLAGS_error_info = (R"(ErrorContent:[)") + m_DataSource_Scene->_planner.route.start + (R"(])");
          LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                       << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
          return false;
        }
        return true;
      } else {
        LOG(WARNING) << "error _planner.route.start " << TX_VARS(results.size());
        return false;
      }
    }
  }
  return false;
}

Base::txFloat TAD_SceneLoader::GetSimSimulationPlannerStartV(Base::txString egoGroup) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return egoInfo.start_v;
        }
      }
      return 0.0;
    } else {
      return m_DataSource_Scene->_planner.start_v;
    }
  } else {
    return 0.0;
  }
}

Base::txString TAD_SceneLoader::GetSimSimulationPlannerEgoType(Base::txString egoGroup) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return egoInfo.ego.type;
        }
      }
      return "";
    } else {
      return m_DataSource_Scene->_planner.ego.type;
    }
  } else {
    return "";
  }
}

TAD_SceneLoader::EgoType TAD_SceneLoader::GetEgoType(Base::txString egoGroup) const TX_NOEXCEPT {
  if (FLAGS_ForcedTrailer) {
    return EgoType::eTruck;
  }

  const Base::txString strType = GetSimSimulationPlannerEgoType(egoGroup);
  if (Base::txString("truck") == strType) {
    return EgoType::eTruck;
  } else {
    return EgoType::eVehicle;
  }
}

Base::txInt TAD_SceneLoader::GetEgoId(Base::txString egoGroup) const TX_NOEXCEPT {
  Base::txString egoIdStr = "-1";
  size_t pos = egoGroup.find('_');
  if (pos != std::string::npos) {
    egoIdStr = egoGroup.substr(pos + 1).c_str();
  } else {
    egoIdStr = egoGroup.substr(3, egoGroup.size() - 3);
  }

  try {
    Base::txInt egoId = std::stoi(egoIdStr.c_str());
    return egoId;
  } catch (...) {
    return -1;
  }
}

Base::txBool TAD_SceneLoader::GetRoutingInfo(Base::txString egoGroup, sim_msg::Location& refEgoData) TX_NOEXCEPT {
  refEgoData.Clear();
  if (NonNull_Pointer(m_DataSource_Scene) && NonNull_Pointer(m_DataSource_Traffic)) {
    sim_msg::Vec3* curPos = refEgoData.mutable_position();
    sim_msg::Vec3* curRpy = refEgoData.mutable_rpy();
    sim_msg::Vec3* curV = refEgoData.mutable_velocity();

    const Base::txFloat fAngle = GetSimSimulationPlannerTheta(egoGroup); /*sim.simulation.planner.theta*/
    Base::txFloat lon = 0.0;
    Base::txFloat lat = 0.0;
    std::tuple<Base::txFloat, Base::txFloat> start_tuple;
    if (GetSimSimulationPlannerRouteStart(egoGroup, start_tuple)) {
      std::tie(lon, lat) = start_tuple;                                       /*sim.simulation.planner.route.start*/
      const Base::txFloat velocity = GetSimSimulationPlannerStartV(egoGroup); /*sim.simulation.planner.start_v*/
      curPos->set_x(lon);
      curPos->set_y(lat);
      curPos->set_z(0);
      curRpy->set_x(0);
      curRpy->set_y(0);
      curRpy->set_z(fAngle);
      curV->set_x(velocity * std::cos(fAngle));
      curV->set_y(velocity * std::sin(fAngle));
      curV->set_z(0);
      return true;
    } else {
      LogWarn << "GetSimSimulationPlannerRouteStart failure.";
      return false;
    }
  } else {
    LogWarn << ", Data Source is Null.";
    return false;
  }
}

Base::ISceneLoader::IVehiclesViewerPtr TAD_SceneLoader::GetEgoData(Base::txString egoGroup) TX_NOEXCEPT {
  sim_msg::Location pbEgoInfo;
  Base::txInt egoId = GetEgoId(egoGroup);
  GetRoutingInfo(egoGroup, pbEgoInfo);
  const Base::txFloat velocity = PlannerStartV(egoGroup);
  const Base::txFloat maxVelocity = PlannerVelocityMax(egoGroup);
#  if Use_TruckEgo
  if (_plus_(EgoType::eVehicle) == GetEgoType(egoGroup)) {
    TAD_EgoVehiclesViewerPtr retPtr = std::make_shared<TAD_EgoVehiclesViewer>();
    retPtr->Init(pbEgoInfo, egoGroup, (egoId + FLAGS_Default_EgoIdPrefix), velocity, maxVelocity);
    return retPtr;
  } else {
    TAD_TruckVehiclesViewerPtr retPtr = std::make_shared<TAD_TruckVehiclesViewer>();
    retPtr->Init(pbEgoInfo, egoGroup, (egoId + 2 * FLAGS_Default_EgoIdPrefix), velocity, maxVelocity);
    return retPtr;
  }
#  else
    TAD_EgoVehiclesViewerPtr retPtr = std::make_shared<TAD_EgoVehiclesViewer>();
    retPtr->Init(pbEgoInfo, egoGroup, (egoId + FLAGS_Default_EgoIdPrefix), velocity, maxVelocity);
    return retPtr;
#  endif
}

Base::ISceneLoader::IVehiclesViewerPtr TAD_SceneLoader::GetEgoTrailerData(Base::txString egoGroup) TX_NOEXCEPT {
  sim_msg::Location pbEgoInfo;
  Base::txInt egoId = GetEgoId(egoGroup);
  GetRoutingInfo(egoGroup, pbEgoInfo);
  const Base::txFloat velocity = PlannerStartV(egoGroup);
  const Base::txFloat maxVelocity = PlannerVelocityMax(egoGroup);
  if (_plus_(EgoType::eVehicle) == GetEgoType()) {
    return nullptr;
  } else {
    TAD_TrailerVehiclesViewerPtr retPtr = std::make_shared<TAD_TrailerVehiclesViewer>();
    retPtr->Init(pbEgoInfo, egoGroup, (egoId + 2 * FLAGS_Default_EgoIdPrefix), velocity, maxVelocity);
    return retPtr;
  }
}

Base::ISceneLoader::IRouteViewerPtr TAD_SceneLoader::GetEgoRouteData(Base::txString egoGroup) TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return GetRouteData(egoInfo.route.id);
        }
      }
      return nullptr;
    } else {
      return GetEgoRouteData();
    }
  } else {
    return nullptr;
  }
}

Base::txFloat TAD_SceneLoader::PlannerStartV(Base::txString egoGroup) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return egoInfo.start_v;
        }
      }
      return ParentClass::PlannerStartV();
    } else {
      return m_DataSource_Scene->_planner.start_v;
    }
  } else {
    return ParentClass::PlannerStartV();
  }
}

Base::txFloat TAD_SceneLoader::PlannerTheta(Base::txString egoGroup) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return egoInfo.theta;
        }
      }
      return ParentClass::PlannerTheta();
    } else {
      return m_DataSource_Scene->_planner.theta;
    }
  } else {
    return ParentClass::PlannerTheta();
  }
}

Base::txFloat TAD_SceneLoader::PlannerVelocityMax(Base::txString egoGroup) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return egoInfo.Velocity_Max;
        }
      }
      return ParentClass::PlannerVelocityMax();
    } else {
      return m_DataSource_Scene->_planner.Velocity_Max;
    }
  } else {
    return ParentClass::PlannerVelocityMax();
  }
}

Base::txBool TAD_SceneLoader::Planner_Trajectory_Enabled(Base::txString egoGroup) const TX_NOEXCEPT {
  if (NonNull_Pointer(m_DataSource_Scene)) {
    if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
      for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
        if (egoGroup == egoInfo.ego.group) {
          return ("true" == (egoInfo.trajectory_enabled));
        }
      }
      return ParentClass::Planner_Trajectory_Enabled();
    } else {
      return ("true" == (m_DataSource_Scene->_planner.trajectory_enabled));
    }
  } else {
    return ParentClass::Planner_Trajectory_Enabled();
  }
}

TAD_SceneLoader::IRouteViewer::control_path_node_vec TAD_SceneLoader::ControlPath(Base::txString egoGroup) const
    TX_NOEXCEPT {
  using txString = Base::txString;
  IRouteViewer::control_path_node_vec ret_conrtol_path;
  if (m_DataSource_Scene->_planner.ego_list.size() > 0) {
    for (auto& egoInfo : m_DataSource_Scene->_planner.ego_list) {
      if (egoGroup == egoInfo.ego.group) {
        txString str_control_path = egoInfo.control_path.points;
        LOG(INFO) << TX_VARS(str_control_path);

        const auto res_waypoints = Utils::SpliteStringVector(str_control_path, ";");
        for (auto waypt : res_waypoints) {
          LOG(INFO) << TX_VARS(waypt);
          const auto res_kinect_params = Utils::SpliteStringVector(waypt, ",");
          if (res_kinect_params.size() > 5) {
            LOG(INFO) << TX_VARS_NAME(lon, res_kinect_params[0]) << TX_VARS_NAME(lat, res_kinect_params[1])
                      << TX_VARS_NAME(alt, res_kinect_params[2]) << TX_VARS_NAME(speed, res_kinect_params[3])
                      << TX_VARS_NAME(gear, res_kinect_params[4]);
            IRouteViewer::control_path_node cp_node;
            __Lon__(cp_node.waypoint) = std::stod(res_kinect_params[0]);
            __Lat__(cp_node.waypoint) = std::stod(res_kinect_params[1]);
            __Alt__(cp_node.waypoint) = std::stod(res_kinect_params[2]);
            cp_node.speed_m_s = std::stod(res_kinect_params[3]);
            cp_node.gear = ("reverse" == res_kinect_params[4]) ? (_plus_(Base::Enums::ControlPathGear::reverse))
                                                               : (_plus_(Base::Enums::ControlPathGear::drive));

            ret_conrtol_path.emplace_back(std::move(cp_node));
          } else {
            LogWarn << "[Error]" << TX_VARS(res_kinect_params.size()) << TX_VARS(waypt);
          }
        }
        break;
      }
    }
  } else {
    txString str_control_path = m_DataSource_Scene->_planner.control_path.points;
    LOG(INFO) << TX_VARS(str_control_path);

    const auto res_waypoints = Utils::SpliteStringVector(str_control_path, ";");
    for (auto waypt : res_waypoints) {
      LOG(INFO) << TX_VARS(waypt);
      const auto res_kinect_params = Utils::SpliteStringVector(waypt, ",");
      if (res_kinect_params.size() > 5) {
        LOG(INFO) << TX_VARS_NAME(lon, res_kinect_params[0]) << TX_VARS_NAME(lat, res_kinect_params[1])
                  << TX_VARS_NAME(alt, res_kinect_params[2]) << TX_VARS_NAME(speed, res_kinect_params[3])
                  << TX_VARS_NAME(gear, res_kinect_params[4]);
        IRouteViewer::control_path_node cp_node;
        __Lon__(cp_node.waypoint) = std::stod(res_kinect_params[0]);
        __Lat__(cp_node.waypoint) = std::stod(res_kinect_params[1]);
        __Alt__(cp_node.waypoint) = std::stod(res_kinect_params[2]);
        cp_node.speed_m_s = std::stod(res_kinect_params[3]);
        cp_node.gear = ("reverse" == res_kinect_params[4]) ? (_plus_(Base::Enums::ControlPathGear::reverse))
                                                           : (_plus_(Base::Enums::ControlPathGear::drive));

        ret_conrtol_path.emplace_back(std::move(cp_node));
      } else {
        LogWarn << "[Error]" << TX_VARS(res_kinect_params.size()) << TX_VARS(waypt);
      }
    }
  }
  return ret_conrtol_path;
}
#endif

TX_NAMESPACE_CLOSE(SceneLoader)

#undef LogInfo
#undef LogWarn

// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txcar_interface.h"

#include "coord_trans.h"

#include "car.pb.h"
#include "control_v2.pb.h"
#include "location.pb.h"
#include "settings.pb.h"

#include "boost/filesystem.hpp"
#include "glog/logging.h"

#include "os_utils.h"

namespace fs = boost::filesystem;

#pragma comment(lib, "glog.lib")
#pragma comment(lib, "gflags.lib")

#define _LOG_SEPERATOR_ "===========================\n"
#define ENABLE_CATALOG_VEHICLE 1

namespace tx_car {

// global dll handler
VehicleModel_DLL_Handle g_vehicle_dll;

TxCarInterface::TxCarInterface() {
  m_subtopics.clear();
  m_pubtopics.clear();

  m_pubtopics.emplace(tx_car::topic::LOCATION);
  m_pubtopics.emplace(tx_car::topic::VEHICLE_STATE);
  m_subtopics.emplace(tx_car::topic::CONTROL_V2);
  m_subtopics.emplace(tx_car::topic::CONTROL);

  FLAGS_v = 0;
  bCloudEnv = false;
  bUseCatalog = false;
  mCloudDataDir = "";
}

TxCarInterface::~TxCarInterface() { g_vehicle_dll.freeVehicleLibrary(); }

void TxCarInterface::calVehicleDllFullPath() {
#ifdef _WIN32
  m_ICE_DLL_FullPath = m_dllPath + "\\" + Constants::const_mdl_ICE_Name;
  m_EV_DLL_FullPath = m_dllPath + "\\" + Constants::const_mdl_EV_Name;
  m_Hybrid_DLL_FullPath = m_dllPath + "\\" + Constants::const_mdl_Hybrid_Name;
#else
  m_ICE_DLL_FullPath = m_dllPath + "/" + Constants::const_mdl_ICE_Name;
  m_EV_DLL_FullPath = m_dllPath + "/" + Constants::const_mdl_EV_Name;
  m_Hybrid_DLL_FullPath = m_dllPath + "/" + Constants::const_mdl_Hybrid_Name;
#endif  // _WIN32

  fs::path ICE_Path(m_ICE_DLL_FullPath);
  fs::path EV_Path(m_EV_DLL_FullPath);
  fs::path Hybrid_Path(m_Hybrid_DLL_FullPath);
  if (!fs::exists(ICE_Path)) {
    std::string errorInfo = m_ICE_DLL_FullPath + " does not exist!";
    throw std::runtime_error(errorInfo.c_str());
  }
  if (!fs::exists(EV_Path)) {
    std::string errorInfo = m_EV_DLL_FullPath + " does not exist!";
    throw std::runtime_error(errorInfo.c_str());
  }
  if (!fs::exists(Hybrid_Path)) {
    std::string errorInfo = m_Hybrid_DLL_FullPath + " does not exist!";
    throw std::runtime_error(errorInfo.c_str());
  }

  VLOG(0) << "ICE dll path:" << m_ICE_DLL_FullPath << "\n";
  VLOG(0) << "EV dll path:" << m_EV_DLL_FullPath << "\n";
  VLOG(0) << "Hybrid dll path:" << m_Hybrid_DLL_FullPath << "\n";
}

TxCarBasePtr TxCarInterface::getVehicleModel(const TxCarInit& carInit, int flags_v) {
  // free loaded dll
  g_vehicle_dll.freeVehicleLibrary();

  // load vehicle parameter
  tx_car::car vehicleParam;
  std::string content;
  if (tx_car::loadDataFromFile(content, m_carParam.param_path)) {
    if (tx_car::jsonToProto(content, vehicleParam)) {
      auto propulsionType = static_cast<uint32_t>(vehicleParam.car_type().propulsion_type());
      VLOG(0) << "propulsionType:" << propulsionType << "\n";
      if (propulsionType == 0) {
        // ICE vehicle model
        if (!g_vehicle_dll.loadVehicleLibrary(m_ICE_DLL_FullPath)) {
          std::string errorInfo = "fail to load library -> " + m_ICE_DLL_FullPath;
          throw std::runtime_error(errorInfo.c_str());
        }
        VLOG(0) << "vd | " << m_ICE_DLL_FullPath << " loaded.\n";
      } else if (propulsionType == 1) {
        // ICE vehicle model
        if (!g_vehicle_dll.loadVehicleLibrary(m_EV_DLL_FullPath)) {
          std::string errorInfo = "fail to load library -> " + m_EV_DLL_FullPath;
          throw std::runtime_error(errorInfo.c_str());
        }
        VLOG(0) << "vd | " << m_EV_DLL_FullPath << " loaded.\n";
      } else if (propulsionType > 1 && propulsionType <= 5) {
        // Hybrid vehicle model
        if (!g_vehicle_dll.loadVehicleLibrary(m_Hybrid_DLL_FullPath)) {
          std::string errorInfo = "fail to load library -> " + m_Hybrid_DLL_FullPath;
          throw std::runtime_error(errorInfo.c_str());
        }
        VLOG(0) << "vd | " << m_Hybrid_DLL_FullPath << " loaded.\n";
      }

      // get funtion interface from dll
      GetVehicleModelFunc getVehicleModelFuncPtr =
          (GetVehicleModelFunc)g_vehicle_dll.getFunctionPtr(Hook_Get_VehicleModel);
      TxCarBasePtr vehicleModel(reinterpret_cast<TxCarBase*>(getVehicleModelFuncPtr()));

      // set log level
      vehicleModel->setLogLevel(flags_v);

      return vehicleModel;
    } else {
      std::string errorInfo = m_carParam.param_path + " data content does not match data structure.";
      throw std::runtime_error(errorInfo.c_str());
    }
  } else {
    std::string errorInfo = m_carParam.param_path + " fail to open.";
    throw std::runtime_error(errorInfo.c_str());
  }

  // default is nullptr
  return std::shared_ptr<TxCarBase>(nullptr);
}

void TxCarInterface::Init(tx_sim::InitHelper& helper) {
  VLOG(0) << _LOG_SEPERATOR_;

  // LOG(INFO) << "group name:" << helper.GetGroupName() << "\n";

  // set vlog level
  std::string glog_level = helper.GetParameter("_log_level");
  if (!glog_level.empty()) {
    FLAGS_v = std::atoi(glog_level.c_str());
    VLOG(0) << "log level is " << FLAGS_v << "\n";
  }

  // get dll path
  m_dllPath = helper.GetParameter(tx_sim::constant::kInitKeyModuleSharedLibDirectory);
  calVehicleDllFullPath();

  std::string par_path = helper.GetParameter("ParaFile");

  // get switches
  std::string cloudEnv = helper.GetParameter("cloudEnv");
  std::string useCatalog = helper.GetParameter("useCatalog");
  mCloudDataDir = helper.GetParameter(tx_sim::constant::kInitKeyModuleSharedLibDirectory);

  if (cloudEnv.size() > 0) {
    bCloudEnv = std::atoi(cloudEnv.c_str());
  }
  if (useCatalog.size() > 0) {
    bUseCatalog = std::atoi(useCatalog.c_str());
  }
  VLOG(0) << "bCloudEnv:" << bCloudEnv << ", bUseCatalog:" << bUseCatalog << ".\n";

  // mdl init state
  bool mdl_init_state = true;

  std::string enable_terrain = helper.GetParameter("enableTerrain");
  std::string step_time_ms = helper.GetParameter("stepTime");

  // param path
  m_carParam.param_path = par_path;

  // step time
  if (step_time_ms.size() > 0) {
    m_carParam.loops_per_step = 10;
    try {
      m_carParam.loops_per_step = std::atoi(step_time_ms.c_str());
      m_carParam.loops_per_step = m_carParam.loops_per_step <= 0 ? 10 : m_carParam.loops_per_step;
    } catch (const char* msg) {
      LOG(ERROR) << "get step time failed, use default[10] instead, " << msg << ".\n";
    }
  }

  // if enable terrain
  {
    m_carParam.enable_terrain = true;
    if (enable_terrain.size() > 0) {
      try {
        m_carParam.enable_terrain = static_cast<bool>(std::atoi(enable_terrain.c_str()));
      } catch (const char* msg) {
        LOG(ERROR) << "enable terrain parameter failed, " << msg << ".\n";
      }
    }
  }

  // sub/pub topics
  {
    for (const std::string& topic : m_subtopics) {
      helper.Subscribe(topic);
      VLOG(0) << "sub topic " << topic << "\n";
    }

    for (const std::string& topic : m_pubtopics) {
      helper.Publish(topic);
      VLOG(0) << "pub topic " << topic << "\n";
    }
  }
}

void TxCarInterface::Reset(tx_sim::ResetHelper& helper) {
  VLOG(0) << _LOG_SEPERATOR_;
  LOG(INFO) << "group name:" << helper.group_name() << "\n";
  // release mdl
  m_car.reset();

  // get vehicle geometory
  {
    std::vector<std::pair<int64_t, std::string>> measurements;
    m_carParam.vehicle_geometory_payload.clear();
    helper.vehicle_measurements(measurements);
    if (measurements.size() > 0) m_carParam.vehicle_geometory_payload = measurements.at(0).second;
  }

  // get map origin and start location
  {
    m_start_loc_payload = helper.ego_start_location();
    m_carParam.start_loc_payload = m_start_loc_payload;
    sim_msg::Location start_loc;
    start_loc.ParseFromString(m_start_loc_payload);
    mMapOrigin.x = start_loc.position().x();
    mMapOrigin.y = start_loc.position().y();
    mMapOrigin.z = start_loc.position().z();
  }

  // get map path
  m_carParam.hadmap_path = helper.map_file_path();

  // switches
  if (bCloudEnv) {
    m_carParam.param_path = mCloudDataDir + "/" + m_carParam.param_path;
  }

#ifdef ENABLE_CATALOG_VEHICLE
  if (bUseCatalog) {
    sim_msg::Setting setting;
    setting.ParseFromString(helper.setting_pb());
    m_carParam.param_path = setting.vehicle_dynamic_cfg_path();
  }
#endif  // ENABLE_CATALOG_VEHICLE

  VLOG(0) << "parameter path:" << m_carParam.param_path << "\n";

  if (!fs::exists(fs::path(m_carParam.param_path))) {
    std::string errorInfo = m_carParam.param_path + " misssing.";
    throw std::runtime_error(errorInfo.c_str());
  }

  // create TxCar object
  m_car = getVehicleModel(m_carParam, FLAGS_v);
  if (m_car.get() == nullptr)
    throw std::runtime_error("fail to make TxCarImp object.");
  else
    VLOG(0) << "vehicle model created, ptr -> " << m_car.get() << "\n";

  m_car->init(m_carParam);
}

void TxCarInterface::Step(tx_sim::StepHelper& helper) {
  VLOG(2) << _LOG_SEPERATOR_;

  double t_ms = helper.timestamp();

  if (m_car.get() != nullptr) {
    // 0. convert to TxCarImp pointer
    auto car_ptr = m_car.get();
    if (car_ptr == nullptr) helper.StopScenario("fail to convert to TxCarImp object.");

    std::string ctrl_payload, loc_payload, chassis_payload;

    // 1. get control msg
    helper.GetSubscribedMessage(tx_car::topic::CONTROL, ctrl_payload);
    car_ptr->setMsgByTopic(tx_car::topic::CONTROL, ctrl_payload);

    helper.GetSubscribedMessage(tx_car::topic::CONTROL_V2, ctrl_payload);
    car_ptr->setMsgByTopic(tx_car::topic::CONTROL_V2, ctrl_payload);

    // 3. upadte vehicle model
    m_car->step(t_ms);

    // 5. get location and chassis from vehicle model
    chassis_payload = car_ptr->getMsgByTopic(tx_car::topic::VEHICLE_STATE);
    loc_payload = car_ptr->getMsgByTopic(tx_car::topic::LOCATION);

    // 6. publish
    helper.PublishMessage(tx_car::topic::VEHICLE_STATE, chassis_payload);
    helper.PublishMessage(tx_car::topic::LOCATION, loc_payload);

    // 7. check if mdl valid
    if (!car_ptr->isModelValid()) {
      helper.StopScenario("Vehicle mdl is not valid.\n");
    }
  }
}

void TxCarInterface::Stop(tx_sim::StopHelper& helper) {
  VLOG(0) << _LOG_SEPERATOR_;

  if (m_car.get() != nullptr) {
    m_car->stop();
  }
  m_car.reset();

  g_vehicle_dll.freeVehicleLibrary();
}
}  // namespace tx_car

TXSIM_MODULE(tx_car::TxCarInterface)

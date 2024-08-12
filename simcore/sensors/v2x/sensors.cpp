/**
 * @file sensors.cpp
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "sensors.h"
#include <glog/logging.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "common/coord_trans.h"
#include "hmap.h"
#include "scene.pb.h"

// this ego id
std::int64_t ego_id = 0;
// rsu sensors
std::map<std::int64_t, std::shared_ptr<RsuSensor>> rsu_sensors;
// traffic obu sensor
std::map<std::int64_t, std::shared_ptr<ObuSensor>> traffic_obu_sensors;
// ego obu sensor
std::map<std::int64_t, std::shared_ptr<ObuSensor>> ego_obu_sensors;
std::vector<ObuMountConfig> obu_mount_config_list;
std::vector<RsuLocConfig> rsu_loc_config_list;

// #define TMap google::protobuf::Map<std::string, std::string>

/**
 * @brief Get the Prop Value object
 *
 * @tparam TMap Type of map
 * @param params parameters
 * @param para input string
 * @param value value of para
 */
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, double &value) {
  if (params.find(para) != params.end()) {
    value = std::atof(params.find(para)->second.c_str());
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, float &value) {
  if (params.find(para) != params.end()) {
    value = std::atof(params.find(para)->second.c_str());
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, int &value) {
  if (params.find(para) != params.end()) {
    value = std::atoi(params.find(para)->second.c_str());
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, std::string &value) {
  if (params.find(para) != params.end()) {
    value = params.find(para)->second;
  }
}
template <typename TMap>
void GetPropValue(const TMap &params, const std::string &para, bool &value) {
  if (params.find(para) != params.end()) {
    value = true;
    auto v = params.find(para)->second;
    if (v == ("false") || v == ("0") || v == ("close") || v == ("disable")) value = false;
  }
}

/**
 * @brief download a file and save it into local disk
 *
 * @param fpath path of link
 * @param fdest path of local disk
 * @return true on success
 * @return false
 */
bool Download(const std::string &fpath, const std::string &fdest) {
  char *envvar = 0;
  envvar = getenv("TADSIM_ENV");
  if (!envvar) {
    LOG(ERROR) << "connt read env. " << std::endl;
    return false;
  }
  mINI::INIFile file(std::string("/home/work/sim-agent/etc/sim_agent_") + envvar + ".conf");

  mINI::INIStructure ini;
  if (!file.read(ini)) {
    LOG(ERROR) << "connt read conf: " << envvar << std::endl;
    return false;
  }
  std::string access_key = ini["s3"]["access_key"];
  std::string secret_key = ini["s3"]["secret_key"];
  std::string bucket = ini["s3"]["bucket"];
  std::string endpoint_url = ini["s3"]["endpoint_url"];

  std::string args = "s3cmd --access_key=";
  args += access_key;
  args += " --secret_key=";
  args += secret_key;
  args += " --host=";
  args += endpoint_url;
  args += " --host-bucket=";
  args += bucket + "." + endpoint_url.substr(endpoint_url.find_first_of('/') + 2);
  args += " get ";
  args += fpath;
  args += " ";
  args += fdest;
  LOG(INFO) << "exec: " << args << std::endl;
  return 0 != system(args.c_str());
}

/**
 * @brief load custom fault data
 *
 * @param fpath path of data
 * @return std::string of file
 */
std::string LoadCustomFault(const std::string &fpath) {
  std::string fault_string;
  if (fpath.empty()) return fault_string;

  std::string srcpath = fpath;
  if (!boost::filesystem::exists(fpath)) {
    srcpath = "fault.tmp";
    if (!Download(fpath, srcpath)) std::cout << "download faild: " << fpath << std::endl;
  }
  if (boost::filesystem::exists(srcpath)) {
    std::ifstream in(srcpath);
    std::istreambuf_iterator<char> begin(in);
    std::istreambuf_iterator<char> end;
    fault_string = std::string(begin, end);
  } else {
    std::cout << "read custom file not exists " << srcpath << std::endl;
  }
  std::cout << "read custom fault size is " << fault_string.size() << std::endl;
  return fault_string;
}

/**
 * @brief load rsu
 *
 * @param fpath path of sensor catalog
 * @param oriLL originial location
 * @return true on success
 * @return false
 */
bool LoadRSU(const std::string &fpath, const tx_sim::Vector3d &oriLL) {
  std::map<std::pair<std::string, std::string>, std::map<std::string, std::string>> intValue;

  try {
    boost::property_tree::ptree pt;
    std::locale::global(std::locale(""));
    boost::property_tree::xml_parser::read_xml(fpath, pt);
    for (auto &Catalog : pt.get_child("TADSim")) {
      if (Catalog.first == "Catalog" && Catalog.second.get<std::string>("<xmlattr>.name") == "SensorDefine") {
        for (auto &Sensor : Catalog.second) {
          for (auto &v1 : Sensor.second) {
            if (v1.first == "<xmlattr>") {
              std::map<std::string, std::string> attr;
              for (auto &vAttr : v1.second) {
                attr[vAttr.first] = vAttr.second.data();
              }
              if (attr.find("idx") != attr.end()) {
                intValue[std::make_pair(Sensor.first, attr["idx"])] = attr;
              }
            }
          }
        }
      }
    }
  } catch (const std::exception &e) {
    LOG(WARNING) << e.what();
  }

  std::map<std::string, std::string> nameMap;
  nameMap["Camera"] = "V2xCamera";
  nameMap["Lidar"] = "V2xTraditionalLidar";
  nameMap["Millimeter_Wave_Radar"] = "V2xRadar";
  nameMap["RSU"] = "RSU";

  std::map<std::int64_t, std::shared_ptr<CameraSensor>> cameras;
  std::map<std::int64_t, std::shared_ptr<LidarSensor>> lidars;
  std::map<std::int64_t, std::shared_ptr<RadarSensor>> radars;
  std::map<std::int64_t, std::int64_t> belongRSU;
  for (const auto &object : mapObjects) {
    std::string type, subtype, name;
    object->getRawTypeString(type, subtype);
    name = nameMap[object->getName()];
    auto pos = object->getPos();
    coord_trans_api::lonlat2enu(pos.x, pos.y, pos.z, oriLL.x, oriLL.y, oriLL.z);
    double roll = 0, pitch = 0, yaw = 0;
    object->getRPY(roll, pitch, yaw);
    if (type == "none") {
      std::map<std::string, std::string> usrData;
      object->getUserData(usrData);
      if (usrData.find("V2X_idx") != usrData.end()) {
        auto &idx = usrData.at("V2X_idx");
        if (intValue.find(std::make_pair(name, idx)) != intValue.end()) {
          auto &attr = intValue.at(std::make_pair(name, idx));
          usrData.insert(attr.begin(), attr.end());
        }
      }
      // load rsu
      if (name == "RSU") {
        auto rsu = std::make_shared<RsuSensor>();
        rsu->map_ori = Eigen::Vector3d(oriLL.x, oriLL.y, oriLL.z);
        rsu->position = Eigen::Vector3d(object->getPos().x, object->getPos().y, object->getPos().z);
        GetPropValue(usrData, "FrequencySPAT", rsu->config.FrequencySPAT);
        GetPropValue(usrData, "FrequencyRSM", rsu->config.FrequencyRSM);
        GetPropValue(usrData, "FrequencyRSI", rsu->config.FrequencyRSI);
        GetPropValue(usrData, "FrequencyMAP", rsu->config.FrequencyMAP);
        GetPropValue(usrData, "DistanceCommu", rsu->config.DistanceCommu);
        GetPropValue(usrData, "Mbps", rsu->config.Mbps);
        GetPropValue(usrData, "MaxDelay", rsu->config.MaxDelay);
        GetPropValue(usrData, "MapRadius", rsu->config.MapRadius);
        GetPropValue(usrData, "TriggerImmediately", rsu->config.TriggerImmediately);
        GetPropValue(usrData, "CongestionRegulation", rsu->config.CongestionRegulation);
        GetPropValue(usrData, "V2N", rsu->config.V2N);
        GetPropValue(usrData, "V2I", rsu->config.V2I);
        GetPropValue(usrData, "PreMSG", rsu->config.PreRsu);
        std::string band;
        GetPropValue(usrData, "Band", band);
        if (band == "5905-5925") {
          rsu->config.Band = 2;
        } else if (band == "5905-5915") {
          rsu->config.Band = 0;
        } else if (band == "5915-5925") {
          rsu->config.Band = 1;
        }
        std::string JunctionIDs;
        GetPropValue(usrData, "JunctionIDs", JunctionIDs);
        std::string s;
        std::stringstream ss(JunctionIDs);
        while (std::getline(ss, s, ',')) {
          rsu->config.JunctionIDs.push_back(std::atoi(s.c_str()));
        }
        rsu->config.PreRsu = LoadCustomFault(rsu->config.PreRsu);
        if (!rsu->Init()) {
          LOG(WARNING) << "rus " << object->getId() << " init faild";
          continue;
        }
        rsu_sensors[object->getId()] = rsu;
      } else if (name == "V2xCamera") {
        // load camera
        auto cam = std::make_shared<CameraSensor>(object->getId());
        cam->setCarPosition(Eigen::Vector3d(pos.x, pos.y, pos.z));
        cam->setCarRoatation(roll, pitch, yaw);
        int ResHorizontal = 0, ResVertical = 0, Frequency;
        GetPropValue(usrData, "ResHorizontal", ResHorizontal);
        GetPropValue(usrData, "ResVertical", ResVertical);
        GetPropValue(usrData, "Frequency", Frequency);
        cam->setFPS(Frequency);
        cam->setResolution(ResHorizontal, ResVertical);

        Eigen::Matrix3d insmat = Eigen::Matrix3d::Identity();
        const double a2r = EIGEN_PI / 180.;
        std::string Distortion, IntrinsicType;
        GetPropValue(usrData, "IntrinsicType", IntrinsicType);
        if (IntrinsicType == "0") {
          std::string IntrinsicMat;
          GetPropValue(usrData, "IntrinsicMat", IntrinsicMat);
          std::replace(IntrinsicMat.begin(), IntrinsicMat.end(), ',', ' ');
          double dis[5] = {0};
          auto iss = std::istringstream(IntrinsicMat);
          for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
              iss >> insmat(i, j);
            }
          }
        } else if (IntrinsicType == "1") {
          double FovHorizontal = 0, FovVertical = 0;
          GetPropValue(usrData, "FovHorizontal", FovHorizontal);
          GetPropValue(usrData, "FovVertical", FovVertical);
          if (FovHorizontal > 0 && FovVertical > 0) {
            insmat(0, 2) = ResHorizontal * 0.5;
            insmat(1, 2) = ResVertical * 0.5;
            insmat(0, 0) = ResHorizontal * 0.5 / std::tan(FovHorizontal * a2r * 0.5);
            insmat(1, 1) = ResVertical * 0.5 / std::tan(FovVertical * a2r * 0.5);
          }
        } else if (IntrinsicType == "2") {
          double CcdWidth = 0, CcdHeight = 0, CcdFocal = 0;
          GetPropValue(usrData, "CcdWidth", CcdWidth);
          GetPropValue(usrData, "CcdHeight", CcdHeight);
          GetPropValue(usrData, "CcdFocal", CcdFocal);
          if (CcdWidth > 0 && CcdHeight > 0) {
            insmat(0, 2) = ResHorizontal * 0.5;
            insmat(1, 2) = ResVertical * 0.5;
            insmat(0, 0) = ResHorizontal * CcdFocal / CcdWidth;
            insmat(1, 1) = ResVertical * CcdFocal / CcdHeight;
          }
        }

        cam->setIntrinsic(insmat);
        GetPropValue(usrData, "Distortion", Distortion);
        std::replace(Distortion.begin(), Distortion.end(), ',', ' ');
        double dis[5] = {0};
        auto iss = std::istringstream(Distortion);
        for (int i = 0; i < 5; i++) {
          iss >> dis[i];
        }
        cam->setDistortion(dis[0], dis[1], dis[2], dis[3], dis[4]);
        GetPropValue(usrData, "MaxDistance", cam->MaxDistance);
        GetPropValue(usrData, "Completeness", cam->Completeness);
        GetPropValue(usrData, "MinArea", cam->MinArea);

        cameras[object->getId()] = cam;
        int BelongRSU = 0;
        GetPropValue(usrData, "BelongRSU", BelongRSU);
        belongRSU[object->getId()] = BelongRSU;
      } else if (name == "V2xTraditionalLidar") {
        // load lidar
        auto lid = std::make_shared<LidarSensor>(object->getId());
        lid->setCarPosition(Eigen::Vector3d(pos.x, pos.y, pos.z));
        lid->setCarRoatation(roll, pitch, yaw);

        GetPropValue(usrData, "FovUp", lid->upFOV);
        GetPropValue(usrData, "FovDown", lid->downFOV);
        GetPropValue(usrData, "FovStart", lid->leftFOV);
        GetPropValue(usrData, "FovEnd", lid->rightFOV);
        GetPropValue(usrData, "Radius", lid->dRange);
        GetPropValue(usrData, "HorzionalRes", lid->resHorizonal);
        GetPropValue(usrData, "RayNum", lid->rayNum);
        GetPropValue(usrData, "Completeness", lid->Completeness);
        GetPropValue(usrData, "MinHitNum", lid->MinHitNum);

        lidars[object->getId()] = lid;
        int BelongRSU = 0;
        GetPropValue(usrData, "BelongRSU", BelongRSU);
        belongRSU[object->getId()] = BelongRSU;
      } else if (name == "V2xRadar") {
        // load radar
        auto rad = std::make_shared<RadarSensor>(object->getId());
        rad->setCarPosition(Eigen::Vector3d(pos.x, pos.y, pos.z));
        rad->setCarRoatation(roll, pitch, yaw);
        radar_bit::radar_parameter radar_para;
        radar_para.rcs = 7.9f;
        radar_para.weather = 0;
        radar_para.anne_tag = 1;
        radar_para.radar_angle = 0;
        radar_para.tag = 0;
        GetPropValue(usrData, "F0_GHz", radar_para.F0_GHz);
        GetPropValue(usrData, "Pt_dBm", radar_para.Pt_dBm);
        GetPropValue(usrData, "Gt_dBi", radar_para.Gt_dBi);
        GetPropValue(usrData, "Gr_dBi", radar_para.Gr_dBi);
        GetPropValue(usrData, "Ts_K", radar_para.Ts_K);
        GetPropValue(usrData, "Fn_dB", radar_para.Fn_dB);
        GetPropValue(usrData, "L0_dB", radar_para.L0_dB);
        GetPropValue(usrData, "FovHorizontal", radar_para.hfov);
        GetPropValue(usrData, "FovVertical", radar_para.vfov);
        GetPropValue(usrData, "ResHorizontal", radar_para.hwidth);
        GetPropValue(usrData, "ResVertical", radar_para.vwidth);
        rad->setRadar(radar_para);
        double Delay = 0;
        GetPropValue(usrData, "Delay", Delay);
        rad->Delay() = Delay;

        radars[object->getId()] = rad;
        int BelongRSU = 0;
        GetPropValue(usrData, "BelongRSU", BelongRSU);
        belongRSU[object->getId()] = BelongRSU;
      }
    }
  }

  for (const auto &sen : cameras) {
    if (rsu_sensors.find(belongRSU.at(sen.first)) == rsu_sensors.end()) {
      continue;
    }
    rsu_sensors[belongRSU.at(sen.first)]->cameras.insert(sen);
  }
  for (const auto &sen : lidars) {
    if (rsu_sensors.find(belongRSU.at(sen.first)) == rsu_sensors.end()) {
      continue;
    }
    rsu_sensors[belongRSU.at(sen.first)]->lidars.insert(sen);
  }
  for (const auto &sen : radars) {
    if (rsu_sensors.find(belongRSU.at(sen.first)) == rsu_sensors.end()) {
      continue;
    }
    rsu_sensors[belongRSU.at(sen.first)]->radars.insert(sen);
  }

  return true;
}

/**
 * @brief load sensor from scene protobuf
 *
 * @param buffer scene protobuf
 * @param oriLL origin of local coordinate system in latitude and longitude
 * @return true
 * @return false
 */
bool LoadSensor(const sim_msg::Scene &scene, const tx_sim::Vector3d &oriLL, const std::string &groupname) {
  auto parseObu = [](const google::protobuf::Map<std::string, std::string> &config) -> std::shared_ptr<ObuSensor> {
    auto obu = std::make_shared<ObuSensor>();
    GetPropValue(config, "FrequencyBSM", obu->config.FrequencyBSM);
    GetPropValue(config, "DistanceOpen", obu->config.DistanceOpen);
    GetPropValue(config, "DistanceCity", obu->config.DistanceCity);
    GetPropValue(config, "Mbps", obu->config.Mbps);
    GetPropValue(config, "SystemDelay", obu->config.SystemDelay);
    GetPropValue(config, "CommuDelay", obu->config.CommuDelay);
    GetPropValue(config, "TriggerImmediately", obu->config.TriggerImmediately);
    GetPropValue(config, "PosAccuracy", obu->config.PosAccuracy);
    GetPropValue(config, "NoTeam", obu->config.NoTeam);
    GetPropValue(config, "DisableRSU", obu->config.DisableRSU);
    GetPropValue(config, "PreBSM", obu->config.PreBSM);
    std::string band;
    GetPropValue(config, "Band", band);
    if (band == "5905-5925") {
      obu->config.Band = 2;
    } else if (band == "5905-5915") {
      obu->config.Band = 0;
    } else if (band == "5915-5925") {
      obu->config.Band = 1;
    }
    obu->config.PreBSM = LoadCustomFault(obu->config.PreBSM);
    return obu;
  };

  ego_id = std::atoi(groupname.substr(groupname.length() - 3).c_str());
  for (const auto &ego : scene.egos()) {
    int64_t eid = std::atoi(ego.group().substr(ego.group().length() - 3).c_str());
    for (const auto &sensor : ego.sensor_group().sensors()) {
      if (sensor.type() == sim_msg::SENSOR_TYPE_OBU) {
        auto obu = parseObu(sensor.intrinsic().params());
        if (!obu->Init()) {
          LOG(WARNING) << "ego obu " << ego.id() << " init faild.";
          continue;
        }
        ego_obu_sensors[eid] = obu;
        traffic_obu_sensors[-eid] = obu;
      }
    }
  }

  for (const auto &vehicle : scene.vehicles()) {
    for (const auto &sensor : vehicle.sensor_group().sensors()) {
      if (sensor.type() == sim_msg::SENSOR_TYPE_OBU) {
        auto obu = parseObu(sensor.intrinsic().params());
        if (!obu->Init()) {
          LOG(WARNING) << "traffic obu " << vehicle.id() << " init faild.";
          continue;
        }
        traffic_obu_sensors[vehicle.id()] = obu;
        break;
      }
    }
  }

  return LoadRSU(scene.setting().sensor_cfg_path(), oriLL);
}

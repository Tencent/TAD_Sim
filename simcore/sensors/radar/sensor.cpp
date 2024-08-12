/**
 * @file sensor.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "sensor.h"
#include <Eigen/Geometry>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <iostream>
#include <limits>
#include <sstream>
#include "ini.h"
#include "scene.pb.h"

/**
 * @brief Construct a new Sensor object
 *
 * @param _id : sensor id
 */
Sensor::Sensor(std::int64_t _id) : id(_id) {
  R = Eigen::Matrix3d::Identity();
  Rl = Eigen::Matrix3d::Identity();
  Ri = Eigen::Matrix3d::Identity();
  Rli = Eigen::Matrix3d::Identity();
}

/**
 * @brief Destroy the Sensor:: Sensor object
 *
 */
Sensor::~Sensor() {}

/**
 * @brief Set sensor position
 *
 * @param p position of installation
 */
void Sensor::setPosition(const Eigen::Vector3d &p) { Tl = -p; }

/**
 * @brief Set sensor rotation
 *
 * @param roll roll angle of installation
 * @param pitch pitch angle of installation
 * @param yaw yaw angle of installation
 */
void Sensor::setRotation(double roll, double pitch, double yaw) {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Rli = q.toRotationMatrix();
  Rl = q.toRotationMatrix().inverse();
}

/**
 * @brief Set radar parameters
 *
 * @param para parameters of radar
 */
void Sensor::setRadar(const radar_bit::radar_parameter &para) { radar_para = para; }

/**
 * @brief Set car position
 *
 * @param p position of car
 */
void Sensor::setCarPosition(const Eigen::Vector3d &p) { T = -p; }

/**
 * @brief Set car rotation
 *
 * @param roll roll angle of car
 * @param pitch pitch angle of car
 * @param yaw yaw angle of car
 */
void Sensor::setCarRoatation(double roll, double pitch, double yaw) {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Ri = q.toRotationMatrix();
  R = Ri.inverse();
}

/**
 * @brief Calculate the local direction vector and radar normal vector
 *
 * @param nor direction vector of object
 * @return dot the local direction vector and radar normal vector
 */
double Sensor::faceto(const Eigen::Vector3d &nor) {
  Eigen::Vector3d n = Rli * Ri * Eigen::Vector3d(1, 0, 0);
  return -n.dot(nor.normalized());
}

/**
 * @brief calculate the points is in fov
 *
 * @param wps world position of points
 * @return true if there is in fov
 * @return false if there is not in fov
 */
bool Sensor::inFov(const std::vector<Eigen::Vector3d> &wps) const {
  double minH = std::numeric_limits<double>::max(), minV = std::numeric_limits<double>::max(),
         maxH = -std::numeric_limits<double>::max(), maxV = -std::numeric_limits<double>::max();
  for (const auto &wp : wps) {
    Eigen::Vector3d c = Vector(wp);
    if (c.norm() >= maxDistance) {
      // out of radar range
      continue;
    }
    double h = -std::atan2(c.y(), c.x());
    double v = std::atan2(c.z(), std::sqrt(c.x() * c.x() + c.y() * c.y()));
    minH = std::min(minH, h);
    minV = std::min(minV, v);
    maxH = std::max(maxH, h);
    maxV = std::max(maxV, v);
  }
  if (minH > maxH || minV > maxV) {
    return false;
  }

  return !(minH > radar_para.hfov * 0.5 || maxH < -radar_para.hfov * 0.5 || minV > radar_para.vfov * 0.5 ||
           maxV < -radar_para.vfov * 0.5);
}

/**
 * @brief Calculate the local vector
 *
 * @param wp world position of point
 * @return local vector
 */
Eigen::Vector3d Sensor::Vector(const Eigen::Vector3d &wp) const { return Rl * (R * (wp + T) + Tl); }

/**
 * @brief Calculate the local vector in car coordinate
 *
 * @param wp point in world coordinate
 * @return local vector in car coordinate
 */
Eigen::Vector3d Sensor::VectorWithoutSelfRot(const Eigen::Vector3d &wp) const { return R * (wp + T) + Tl; }

/**
 * @brief Rotate the euler angle into local coordinate
 *
 * @param roll
 * @param pitch
 * @param yaw
 */
void Sensor::Rotator(double &roll, double &pitch, double &yaw) const {
  Eigen::Quaterniond q = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX());
  Eigen::Matrix3d m = Rl * R * (q.toRotationMatrix());
  Eigen::Vector3d a = m * Eigen::Vector3d(1, 0, 0);
  Eigen::Vector3d b = m * Eigen::Vector3d(0, 1, 0);
  yaw = std::atan2(a.y(), a.x());
  a = Eigen::AngleAxisd(-yaw, Eigen::Vector3d::UnitZ()) * a;
  b = Eigen::AngleAxisd(-yaw, Eigen::Vector3d::UnitZ()) * b;
  pitch = std::atan2(-a.z(), a.x());
  b = Eigen::AngleAxisd(-pitch, Eigen::Vector3d::UnitY()) * b;
  roll = std::atan2(b.z(), b.y());
}

/**
 * @brief Rotate the euler angle into local coordinate
 *
 * @param np direction vector in world coordinate
 * @return direction vector in local coordinate
 */
Eigen::Vector3d Sensor::Rotator(const Eigen::Vector3d &np) const { return Rl * R * np; }

/**
 * @brief Rotate the euler angle into car coordinate
 *
 * @param np direction vector in world coordinate
 * @return direction vector in car coordinate
 */
Eigen::Vector3d Sensor::RotatorWithoutSelfRot(const Eigen::Vector3d &np) const { return R * np; }

/**
 * @brief local coordinate to world coordinate
 *
 * @param local vector in local coordinate
 * @return vector in world coordinate
 */
Eigen::Vector3d Sensor::LocalVector(const Eigen::Vector3d &local) const { return Ri * (Rli * local - Tl) - T; }

/**
 * @brief rotate the vector into world coordinate
 *
 * @param np direction in local coordinate
 * @return direction vector in world coordinate
 */
Eigen::Vector3d Sensor::LocalRotator(const Eigen::Vector3d &np) const {
  Eigen::Quaterniond q = Eigen::AngleAxisd(np.z(), Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(np.y(), Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(np.x(), Eigen::Vector3d::UnitX());
  Eigen::Matrix3d m = Rli * Ri * q.matrix();
  Eigen::Vector3d rpy = m.eulerAngles(2, 1, 0);
  std::swap(rpy.x(), rpy.z());
  return rpy;
}

std::int64_t ego_id = 0;
/// @brief weather
std::vector<std::pair<double, int>> weathers;
/// @brief all radar sensor
std::map<std::int64_t, std::shared_ptr<Sensor>> radarSensors;

#define TMap google::protobuf::Map<std::string, std::string>

/**
 * @brief Get the Prop Value object
 *
 * @param params
 * @param para
 * @param value
 */
void GetPropValue(const TMap &params, const std::string &para, double &value) {
  if (params.find(para) != params.end()) {
    value = std::atof(params.find(para)->second.c_str());
  }
}

/**
 * @brief Get the Prop Value object
 *
 * @param params
 * @param para
 * @param value
 */
void GetPropValue(const TMap &params, const std::string &para, float &value) {
  if (params.find(para) != params.end()) {
    value = static_cast<float>(std::atof(params.find(para)->second.c_str()));
  }
}

/**
 * @brief Get the Prop Value object
 *
 * @param params
 * @param para
 * @param value
 */
void GetPropValue(const TMap &params, const std::string &para, int &value) {
  if (params.find(para) != params.end()) {
    value = std::atoi(params.find(para)->second.c_str());
  }
}

/**
 * @brief Get the Prop Value object
 *
 * @param params
 * @param para
 * @param value
 */
void GetPropValue(const TMap &params, const std::string &para, std::string &value) {
  if (params.find(para) != params.end()) {
    value = params.find(para)->second;
  }
}

/**
 * @brief Get the Prop Value object
 *
 * @param params
 * @param para
 * @param value
 */
void GetPropValue(const TMap &params, const std::string &para, bool &value) {
  if (params.find(para) != params.end()) {
    value = true;
    auto v = params.find(para)->second;
    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
    if (v == ("false") || v == ("0") || v == ("close") || v == ("disable")) value = false;
  }
}

/**
 * @brief load custom fault
 *
 * @param fpath path to be loaded
 * @return fault string
 */
std::string LoadCustomFault(const std::string &fpath) {
  std::string fault_string;
  if (fpath.empty()) return fault_string;

  // download fault data
  std::string srcpath = fpath;
  // read the file to string
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
 * @brief load radar sensor
 *
 * @param buffer message string
 * @return true on success
 * @return false on failure
 */
bool LoadRadarSensors(const std::string &buffer, const std::string &groupname) {
  radarSensors.clear();
  sim_msg::Scene scene;
  if (!scene.ParseFromString(buffer)) {
    std::cout << "parse scene faild.";
    return false;
  }
  ego_id = std::atoi(groupname.substr(groupname.length() - 3).c_str());
  const double a2r = EIGEN_PI / 180.;
  for (const auto &ego : scene.egos()) {
    int64_t eid = std::atoi(ego.group().substr(ego.group().length() - 3).c_str());
    if (eid != ego_id) continue;
    for (const auto &sensor : ego.sensor_group().sensors()) {
      if (sensor.type() == sim_msg::SENSOR_TYPE_RADAR) {
        auto radar = std::make_shared<Sensor>(sensor.extrinsic().id());
        // set position and rotation
        radar->setPosition(Eigen::Vector3d(sensor.extrinsic().locationx() * 0.01, sensor.extrinsic().locationy() * 0.01,
                                           sensor.extrinsic().locationz() * 0.01));
        radar->setRotation(a2r * sensor.extrinsic().rotationx(), a2r * sensor.extrinsic().rotationy(),
                           a2r * sensor.extrinsic().rotationz());

        // get radar param
        radar_bit::radar_parameter para;
        para.rcs = 7.9f;
        para.weather = 0;
        para.anne_tag = 1;
        para.radar_angle = 0;
        para.tag = 0;

        GetPropValue(sensor.intrinsic().params(), "F0_GHz", para.F0_GHz);
        GetPropValue(sensor.intrinsic().params(), "Pt_dBm", para.Pt_dBm);
        GetPropValue(sensor.intrinsic().params(), "Gt_dBi", para.Gt_dBi);
        GetPropValue(sensor.intrinsic().params(), "Gr_dBi", para.Gr_dBi);
        GetPropValue(sensor.intrinsic().params(), "Ts_K", para.Ts_K);
        GetPropValue(sensor.intrinsic().params(), "Fn_dB", para.Fn_dB);
        GetPropValue(sensor.intrinsic().params(), "L0_dB", para.L0_dB);
        GetPropValue(sensor.intrinsic().params(), "SNR_min_dB", para.SNR_min_dB);
        GetPropValue(sensor.intrinsic().params(), "hfov", para.hfov);
        GetPropValue(sensor.intrinsic().params(), "vfov", para.vfov);
        GetPropValue(sensor.intrinsic().params(), "hwidth", para.hwidth);
        GetPropValue(sensor.intrinsic().params(), "vwidth", para.vwidth);

        radar->setRadar(para);
        double Frequency = 0, delay = 0;
        GetPropValue(sensor.intrinsic().params(), "Frequency", Frequency);
        GetPropValue(sensor.intrinsic().params(), "delay", delay);
        radar->Frequency() = Frequency;
        radar->Delay() = delay;

        std::string customFault;
        GetPropValue(sensor.intrinsic().params(), "CustomFault", customFault);

        // load custom fault
        auto fault_string = LoadCustomFault(customFault);
        // parse custom fault
        try {
          std::stringstream sss(fault_string);
          std::string line;
          std::getline(sss, line);  // skip first line
          while (std::getline(sss, line)) {
            std::string s;
            std::stringstream ss(line);
            std::vector<std::string> data;
            while (std::getline(ss, s, ',')) data.push_back(s);
            if (data.size() == 5) {
              data.push_back("");
            }
            if (data.size() != 6) {
              continue;
            }
            if (data[0].empty() || data[1].empty()) {
              continue;
            }
            std::int64_t time = (std::int64_t)std::floor(std::atof(data[0].c_str()) * 1000);
            std::int64_t id = std::atoll(data[1].c_str());
            CustomFault &cf = radar->getCustomFault()[time][id];
            if (!data[2].empty()) cf.distance = std::atof(data[2].c_str());
            if (!data[3].empty()) cf.speed = std::atof(data[3].c_str());
            if (!data[4].empty()) cf.angle = std::atof(data[4].c_str());
            if (!data[5].empty()) cf.rcs = std::atof(data[5].c_str());
          }
          std::cout << "load " << radar->getCustomFault().size() << " custom fault in radar " << radar->ID() << "\n";
        } catch (const std::exception &) {
        }

        radarSensors[radar->ID()] = radar;
      }
    }
  }

  std::cout << "load " << radarSensors.size() << " radar\n";
  // read weater
  weathers.clear();
  for (const auto &env : scene.environment()) {
    double tm = env.first;
    int wea = 0;
    // 0 晴天
    // 1 小雨
    // 2 中雨
    // 3 大雨
    // 4 暴雨
    // 5 小雾
    // 6 中雾
    // 7 大雾
    // 8 小雪
    // 9 中雪
    // 10 大雪
    // 11 多云
    auto intst = env.second.precipitation().intensity();
    auto vibly = env.second.fog().visibility();
    auto cl = env.second.clouds().fractional_cloud_cover();
    switch (env.second.precipitation().type()) {
      case sim_msg::EnvironmentalConditions_Precipitation_Type_RAIN:
        if (intst < 0.5) {
          wea = 1;
        } else if (intst < 1) {
          wea = 2;
        } else if (intst < 2) {
          wea = 3;
        } else {
          wea = 4;
        }
        break;
      case sim_msg::EnvironmentalConditions_Precipitation_Type_SNOW:
        if (intst < 0.1) {
          wea = 8;
        } else if (intst < 0.3) {
          wea = 9;
        } else {
          wea = 10;
        }
        break;
      default:
        if (vibly < 500) {
          wea = 7;
        } else if (vibly < 1000) {
          wea = 6;
        } else if (vibly < 10000) {
          wea = 5;
        } else {
          if (static_cast<int>(cl) > 5) {
            wea = 11;
          }
        }
        break;
    }

    weathers.push_back(std::make_pair(tm, wea));
  }
  std::cout << "load " << weathers.size() << " env\n";
  std::sort(weathers.begin(), weathers.end(),
            [](const std::pair<double, int> &rh1, const std::pair<double, int> &rh2) { return rh1.first < rh2.first; });

  return true;
}

/**
 * @brief Get the Weather
 *
 * @param timestamp current timestamp
 * @return weather
 */
int getWeather(double timestamp) {
  if (weathers.empty()) {
    return 0;
  }
  // get weather
  auto pos = std::upper_bound(
      weathers.begin(), weathers.end(), std::make_pair(timestamp, 0),
      [](const std::pair<double, int> &rh1, const std::pair<double, int> &rh2) { return rh1.first < rh2.first; });
  std::int64_t n = std::distance(weathers.begin(), pos);
  n = std::min((std::int64_t)(weathers.size() - 1), std::max((std::int64_t)0, n - 1));
  return weathers.at(n).second;
}

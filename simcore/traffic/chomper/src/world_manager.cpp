// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "world_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "traffic_ga.h"

#ifdef _WIN32
#  include <windows.h>
#elif __linux__
#  include <unistd.h>
#endif  // _WIN32

#include "ibeo_data.h"
#include "boost/dll/import.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"

#include <sstream>
#include "defines.h"
#include <iostream>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <fstream>
// defalut driving parameters
#define DEFAULT_TRAFFIC_CC0 1.50  //
#define DEFAULT_TRAFFIC_CC1 1.30  //
#define DEFAULT_TRAFFIC_CC2 4.00  //

float g_Driving_Parameters[10] = {
    DEFAULT_TRAFFIC_CC0,
    DEFAULT_TRAFFIC_CC1,
    DEFAULT_TRAFFIC_CC2,
    -12.00,  // 进入跟车状态的阈值 - 秒
    -0.25,   // 消极跟车状态的阈值 - 米/秒
    1.35,    // 积极跟车状态的阈值 - 米/秒
    0.0006,  // 车速振动 - 1/米秒
    0.25,    // 加速度波动幅度 - 米/秒2
    2.00,    // 停车时加速度 - 米/秒2
    1.50     // 速度为80km/h的加速度 - 米/秒2
};
TT_Feature tt_feature_;

TrafficGA *g_pTrafficGA = nullptr;
bool g_bFinishedState = true;

namespace boostfs = boost::filesystem;
namespace boostdll = boost::dll;
void WorldManager::InitManager(const std::string &sceneFile, double time_step, int max_step) {
  sceneFile_ = sceneFile;
  _max_step = max_step;
  _time_step = time_step;
  // init txSimTrafficPlugin
  boostfs::path pluginPath = boostfs::current_path() / boostfs::path("txSimTrafficPlugin");
  // boostfs::path pluginPath = boostfs::path("txSimTrafficPlugin");

  try {
    pluginCreator =
        boostdll::import_alias<PluginCreate>(pluginPath, "create_plugin", boostdll::load_mode::append_decorations);
  } catch (const boost::system::system_error &err) {
    LOG(ERROR) << "Cannot load Plugin from " << pluginPath << std::endl;
    LOG(ERROR) << err.what() << std::endl;
    return;
  }

  txSimTraffic = pluginCreator();
  txSimTraffic->init(std::string(""));
  LOG(INFO) << "plugin class name : " << typeid(txSimTraffic).name() << std::endl;
  LOG(INFO) << "Plugin Name: " << txSimTraffic->module_name() << std::endl;
  LOG(INFO) << "Plugin Version = " << txSimTraffic->module_version() << std::endl;
  // init GA
  if (!g_pTrafficGA) g_pTrafficGA = new TrafficGA();
}

void ReleaseTrafficGA() {
  // init GA
  if (g_pTrafficGA) delete g_pTrafficGA;
}

bool UpdateTrafficGA(const float *V_Obs, unsigned int V_Obs_Count, float TT_Obs, float TT_ObsDistance, float &cc0,
                     float &cc1, float &cc2) {
  if (!g_pTrafficGA) return false;

  if (g_pTrafficGA->Update(V_Obs, V_Obs_Count, TT_Obs, TT_ObsDistance, &(WorldManager::instance()))) {
    g_pTrafficGA->GetTrainningData(cc0, cc1, cc2);
    return true;
  }

  return false;
}

void WorldManager::ReleaseTrafficGA() { ReleaseTrafficGA(); }

void WorldManager::LoadTrafficInputData(const std::string &datadir) {
  static int flag = 1;
  if (flag) {
    flag = 0;
    std::string filename = datadir + "/TrafficInputData.txt";
    tt_feature_.fromfile(filename);

    std::string drivingfile = datadir + "/TrafficData.txt";
    DrivingData drivingData;
    drivingData.fromfile(drivingfile);

    g_Driving_Parameters[0] = drivingData.cc0_;
    g_Driving_Parameters[1] = drivingData.cc1_;
    g_Driving_Parameters[2] = drivingData.cc2_;

    LOG(INFO) << "[Model Params Input]  "
              << "cc0 = " << g_Driving_Parameters[0] << "; cc1 = " << g_Driving_Parameters[1]
              << "; cc2 = " << g_Driving_Parameters[2] << std::endl;
  } else {
    LOG(INFO) << "TrafficInputData loaded!" << std::endl;
  }
}

void WorldManager::UpdateTrafficGATraining(const std::string &datadir) {
  char szFileName[1024];
  float cc[3] = {DEFAULT_TRAFFIC_CC0, DEFAULT_TRAFFIC_CC1, DEFAULT_TRAFFIC_CC2};
  LoadTrafficInputData(datadir);

  while (!UpdateTrafficGA(tt_feature_.v_obs_.data(), tt_feature_.v_obs_.size(), tt_feature_.tt_obs_,
                          tt_feature_.tt_obs_distance, cc[0], cc[1], cc[2])) {
    LOG(WARNING) << "UpdateTrafficGA execution error\n";
// ::sleep(10);
#ifdef _WIN32
    Sleep(10);
#elif __linux__
    usleep(1000);
#endif
  }

  for (int i = 0; i < 3; i++) g_Driving_Parameters[i] = cc[i];

  DrivingData drivingData;
  drivingData.cc0_ = g_Driving_Parameters[0];
  drivingData.cc1_ = g_Driving_Parameters[1];
  drivingData.cc2_ = g_Driving_Parameters[2];

  std::string drivingfile = datadir + "/TrafficData.txt";
  drivingData.tofile(drivingfile);

  // VehicleTestOnCurrentMapFromTrafficModule(false, -1.0, false, false);
}

void WorldManager::OutputIbeoDataToGA(const std::string &datadir) {
  IbeoOutputDataForGA(tt_feature_, trafficga::FLAGS_OBS_DISTANCE, datadir);
}

void WorldManager::save_ga_args(const std::string &data, const std::string &name) {
  std::string datafile = datadir_ + "/" + name;
  std::ofstream ofs((datafile).c_str());
  ofs << data;
  ofs.close();
}
namespace IbeoDataManager {
#include <map>
struct IbeoData_Interval {
  std::map<int, std::vector<Ibeo_BBox>> bboxs_;

  std::vector<Ibeo_BBox> *getbox(int id) {
    auto it = bboxs_.find(id);
    if (it != bboxs_.end()) {
      return &(it->second);
    }
    return nullptr;
  }
  void addbox(const Ibeo_BBox &bbox) {
    auto it = bboxs_.find(bbox.id);
    if (it != bboxs_.end()) {
      it->second.emplace_back(bbox);
    } else {
      std::vector<Ibeo_BBox> b;
      b.emplace_back(bbox);
      bboxs_[bbox.id] = b;
    }
  }
  double calc_velociy() {
    double vel = 0;
    for (const auto &it : bboxs_) {
      double v = 0;
      int count = 0;
      for (const auto &box : it.second) {
        // if (box.velocity < 20) {
        v += box.velocity;
        count += 1;
        // }
      }
      vel += v / count;
    }
    return vel / bboxs_.size();
  }
};

TT_Feature calc_tt_feature_interval(const Ibeo_BBox &base_box, const Ibeo_BBox &final_box, int interval_num,
                                    const std::vector<Ibeo_Data> &raw_datas) {
  LOG(INFO) << "[calc_tt_feature_interval] start\n";
  LOG(INFO) << "calc_tt" << std::endl;
  std::vector<IbeoData_Interval> datas_;
  datas_.resize(interval_num);
  TT_Feature tt_feature;
  tt_feature.tt_obs_ = final_box.duration - base_box.duration;

  auto total_distance = GetDistance(final_box.lat, final_box.lon, base_box.lat, base_box.lon);
  auto interval_dis = total_distance / interval_num;
  tt_feature.tt_obs_distance = interval_dis;
  LOG(INFO) << "interval_dis: " << interval_dis << "\n";
  // LOG(INFO) << "[calc_tt_feature_interval] end\n";
  auto calc_interval_idx = [&](const Ibeo_BBox &bbox) {
    auto dis = GetDistance(bbox.lat, bbox.lon, base_box.lat, base_box.lon);
    return static_cast<int>(dis / interval_dis);
  };
  // LOG(INFO) << "[calc_tt_feature_interval] end\n";
  auto addbox = [&](int interval_idx, const Ibeo_BBox &bbox) {
    // assert(interval_idx >=0 && interval_idx <= interval_num);
    if (interval_idx >= interval_num) interval_idx = interval_num - 1;
    if (interval_idx < 0) interval_idx = 0;
    datas_[interval_idx].addbox(bbox);
  };
  // LOG(INFO) << "[calc_tt_feature_interval] end\n";
  for (size_t i = 0; i < raw_datas.size(); i++) {
    for (auto &bbox : raw_datas[i].bboxList) {
      auto idx = calc_interval_idx(bbox);
      addbox(idx, bbox);
    }
  }
  LOG(INFO) << "size: " << datas_.size() << "\n";
  for (auto &i : datas_) {
    tt_feature.v_obs_.emplace_back(i.calc_velociy());
  }
  LOG(INFO) << "[calc_tt_feature_interval] end\n";
  return tt_feature;
}

TT_Feature calc_tt_feature(int interval_num, const std::vector<Ibeo_Data> &raw_datas) {
  std::vector<IbeoData_Interval> datas_;
  auto addinterval_func = [&](size_t beg, size_t end) {
    IbeoData_Interval stIbeoData_Interval;
    for (auto i = beg; i < end; i++) {
      for (auto &bbox : raw_datas[i].bboxList) {
        stIbeoData_Interval.addbox(bbox);
      }
    }
    datas_.emplace_back(stIbeoData_Interval);
  };

  auto size = raw_datas.size();
  auto num = size / interval_num;
  for (size_t i = 0; i < interval_num - 1; i++) {
    addinterval_func(i * num, (i + 1) * num);
  }
  addinterval_func((interval_num - 1) * num, size);
  auto &beg = datas_.front();
  auto &end = datas_.back();
  auto b = beg.getbox(0);
  auto e = end.getbox(0);
  // LOG(INFO) << "[calc_tt_feature] end\n";
  return calc_tt_feature_interval(b->front(), e->back(), interval_num, raw_datas);

  TT_Feature tt_feature;
  tt_feature.tt_obs_ = e->back().duration - b->front().duration;
  auto total_distance = GetDistance(e->back().lat, e->back().lon, b->front().lat, b->front().lon);
  auto interval_dis = total_distance / interval_num;

  for (auto &i : datas_) {
    tt_feature.v_obs_.emplace_back(i.calc_velociy());
  }
  return tt_feature;
}

};  // namespace IbeoDataManager

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, false);
  if (!trafficga::FLAGS_flagsfile.empty()) {
    google::ReadFromFlagsFile(trafficga::FLAGS_flagsfile, argv[0], true);
  }
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;
  // FLAGS_log_dir设置日志输出目录。
  FLAGS_log_dir = "./log/";
  // FLAGS_stderrthreshold = 0;

  if (trafficga::FLAGS_stage == 1) {
    LOG(WARNING) << "######## model trainning start.... #########" << std::endl;

    std::string scenefile = trafficga::FLAGS_scene;
    std::string datadir = trafficga::FLAGS_data;
    int _max_step = trafficga::FLAGS_MAX_STEP;
    double _time_step = trafficga::FLAGS_TIME_STEP;
    WorldManager::instance().InitManager(scenefile, _time_step, _max_step);
    WorldManager::instance().LoadTrafficInputData(datadir);

    WorldManager::instance().UpdateTrafficGATraining(datadir);
    LOG(WARNING) << "######## model trainning finished.... #########" << std::endl;
  } else if (trafficga::FLAGS_stage == 0) {
    LOG(WARNING) << "######## data process start.... #########" << std::endl;
    LoadIbeoData(trafficga::FLAGS_data.c_str(), trafficga::FLAGS_start_idx, trafficga::FLAGS_end_idx,
                 trafficga::FLAGS_islonlat);
    //  for (auto& i : g_ibeoDataList) {
    //    LOG(INFO)  << i.tostring() << std::endl;
    //  }
    LOG(INFO) << "raw data size=" << g_ibeoDataList.size() << std::endl;
    tt_feature_ = IbeoDataManager::calc_tt_feature(trafficga::FLAGS_interval_num, g_ibeoDataList);
    std::string filename = trafficga::FLAGS_data + "/TrafficInputData.txt";
    tt_feature_.tofile(filename);
    // IbeoOutputDataForGA(tt_feature_, trafficga::FLAGS_OBS_DISTANCE, trafficga::FLAGS_data);

    LOG(INFO) << "g_TT_Obs:" << tt_feature_.tt_obs_ << ",g_TT_ObsCount:" << tt_feature_.v_obs_.size()
              << ",content:" << tt_feature_.tostring() << std::endl;
    LOG(WARNING) << "######## data process finished.... #########" << std::endl;
  }
  return 0;
}

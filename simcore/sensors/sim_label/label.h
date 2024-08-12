// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include "data_queue.h"
#include "osi_datarecording.pb.h"
#include "sensor_raw.pb.h"
#include "thread_pool.h"
#include "txsim_module.h"

class sim_label final : public tx_sim::SimModule {
 public:
  sim_label();
  virtual ~sim_label();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  std::string savePathBase = "/home/sim/data/display_pic_dir";
  std::string savePath;

  std::string device;

  double minArea = 5;
  double completeness = 0.2;
  double maxDistance = 200;
  int disNum = 30;
  bool saveScenarioDir = true;
  bool debugFiles = false;
  bool fullBox = false;
  std::shared_ptr<ThreadPool> threads;
  std::shared_ptr<DataQueue> queues;

  std::set<std::string> saved_files;
  std::mutex save_mutex;
  std::string config_dir;
  int ego_id = -1;

  bool parseImage(const std::string &buf, ImageInfo &info);
  bool parseLidar(const std::string &buf, PcInfo &info);
  bool saveFile(const std::string &fname, const std::string &buf);
  void saveImageLabel(const ImagePackage &info);
  void savePcdLabel(const PcdPackage &info);
  std::string getUTC();
  bool getSensorConfig(const std::string &buffer, const std::string &groupname);
  bool saveMaskJson(const std::string &fpath);
};

// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "tad_chomper_interface.h"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "traffic.pb.h"

class WorldManager {
 public:
  static WorldManager &instance() {
    static WorldManager worldManager;
    return worldManager;
  }

  void InitManager(const std::string &sceneFile, double time_step, int max_step);

  void UpdateTrafficGATraining(const std::string &datadir);

  void OutputIbeoDataToGA(const std::string &datadir);

  void LoadTrafficInputData(const std::string &datadir);

  void ReleaseTrafficGA();

  void save_ga_args(const std::string &data, const std::string &name);

 public:
  typedef boost::shared_ptr<ChomperGAInterface>(PluginCreate)();
  boost::function<PluginCreate> pluginCreator;
  boost::shared_ptr<ChomperGAInterface> txSimTraffic;
  std::string datadir_;
  std::string sceneFile_;
  int _time_step;
  int _max_step;

 protected:
  WorldManager() {}
};

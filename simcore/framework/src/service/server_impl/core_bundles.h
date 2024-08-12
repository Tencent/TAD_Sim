// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

#include "zmq.h"

#include "coordinator/config_mgr.h"
#include "coordinator/coordinator.h"
#include "coordinator/module_mgr.h"
#include "coordinator/scenario_parser.h"
#include "utils/proc_service.h"

namespace tx_sim {
namespace service {

struct CoreComponents {
  std::shared_ptr<tx_sim::coordinator::Coordinator> codr;
  std::shared_ptr<tx_sim::coordinator::ConfigManager> cfgr;
  std::shared_ptr<tx_sim::coordinator::ModuleManager> mdlr;
  std::shared_ptr<tx_sim::coordinator::IScenarioParser> snpr;
};

inline CoreComponents CreateCoreComponents(const std::string& app_root_path, const std::string& default_cfg_path,
                                           const std::string& config_db_path, std::shared_ptr<zmq::context_t>& zmq_ctx,
                                           uint16_t instance_id = 0) {
  CoreComponents c;
  c.cfgr = std::make_shared<tx_sim::coordinator::ConfigManager>(config_db_path);
  c.cfgr->InitConfigData(default_cfg_path);
  c.snpr = std::make_shared<tx_sim::coordinator::ScenarioXmlParser>();
  c.mdlr = std::make_shared<tx_sim::coordinator::ModuleManagerImpl>(
      std::make_shared<tx_sim::coordinator::ModuleRegistryImpl>(zmq_ctx, instance_id),
      std::make_shared<tx_sim::coordinator::ZmqConnectionFactory>(zmq_ctx),
      std::make_shared<tx_sim::utils::ModuleProcessServiceImpl>(zmq_ctx), default_cfg_path);
  c.codr = std::make_shared<tx_sim::coordinator::Coordinator>(
      app_root_path, std::dynamic_pointer_cast<tx_sim::coordinator::ConfigLoader>(c.cfgr), c.mdlr, c.snpr);
  std::vector<tx_sim::coordinator::ModuleConfig> defaultLaunchModuleConfigs = c.cfgr->GetDefaultLaunchModuleConfigs();
  c.codr->AddLocalServiceDefaultSetupModule(defaultLaunchModuleConfigs);
  return c;
}

}  // namespace service
}  // namespace tx_sim

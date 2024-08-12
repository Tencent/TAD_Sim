// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "service_context.h"

#include "boost/filesystem/path.hpp"

#include "utils/constant.h"


using namespace tx_sim::impl;
using namespace tx_sim::coordinator;


namespace tx_sim {
namespace service {

LocalServiceContext::LocalServiceContext(const std::string& app_root_path, uint16_t instance_id) {
  boost::filesystem::path cfg_file = app_root_path, cfg_db = app_root_path;
  cfg_file = cfg_file / kPathUserDirSys / kServiceConfigFileName;
  cfg_db = cfg_db / kPathUserDirData / kServiceDatabaseName;
  zmq_ctx = std::make_shared<zmq::context_t>(2);
  core_ = CreateCoreComponents(app_root_path, cfg_file.lexically_normal().string(), cfg_db.lexically_normal().string(),
                               zmq_ctx, instance_id);
}

}  // namespace service
}  // namespace tx_sim

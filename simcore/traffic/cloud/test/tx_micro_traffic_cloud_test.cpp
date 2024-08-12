// Copyright 2024 Tencent Inc. All rights reserved.
//

#if 1
#  include <glog/logging.h>
#  include <memory>
#  include "tx_tc_Application.h"
int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  txApplication::Initialize(argc, argv);

  int nExitCode = txApplication::Run(argc, argv);
  txApplication::Terminate();
  return nExitCode;
}
#else

#  include <iostream>
#  include <string>
#  include <typeinfo>
#  include "boost/dll/import.hpp"
#  include "boost/function.hpp"
#  include "boost/shared_ptr.hpp"
#  include "tx_tc_TypeDef.h"
#  include "tx_tc_marco.h"
#  include "tx_tc_plugin_micro.h"
#  include "txsim_module.h"

namespace boostfs = boost::filesystem;
namespace boostdll = boost::dll;

int main(int argc, char** argv) {
  namespace boostfs = boost::filesystem;
  namespace boostdll = boost::dll;

  std::cout << "txSimTrafficCloud DLL/so testing ..." << std::endl;

  boostfs::path pluginPath;

  typedef boost::shared_ptr<TrafficCloud::TrafficManager>(PluginCreate)();
  boost::function<PluginCreate> pluginCreator;
  boost::shared_ptr<TrafficCloud::TrafficManager> mPlugin = nullptr;

  pluginPath = boostfs::current_path() / boostfs::path("txMicroTrafficCloud");
  std::cout << "Load Plugin from " << pluginPath << std::endl;

  try {
    pluginCreator =
        boostdll::import_alias<PluginCreate>(pluginPath, "create_plugin", boostdll::load_mode::append_decorations);
    /* create the plugin */
    mPlugin = pluginCreator();
    if (mPlugin) {
      std::cout << "plugin class name : " << typeid(mPlugin).name() << std::endl;
      std::cout << "Plugin Name: " << mPlugin->module_name() << std::endl;
      std::cout << "Plugin Version = " << mPlugin->module_version() << std::endl;
    } else {
      std::cout << "Plugin loading error. " << pluginPath;
    }
  } catch (const boost::system::system_error& err) {
    std::cout << "Cannot load Plugin from " << pluginPath << std::endl;
    std::cout << err.what() << std::endl;
    return -1;
  }

  // mPlugin->test();

  return 0;
}
#endif

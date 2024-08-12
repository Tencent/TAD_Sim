// Copyright 2024 Tencent Inc. All rights reserved.
//

#if 0
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
#  include "tx_tc_plugin_micro.h"
namespace boostfs = boost::filesystem;
namespace boostdll = boost::dll;

int main(int argc, char **argv) {
  std::cout << "txSimTrafficCloud DLL/so testing ..." << std::endl;

  boostfs::path pluginPath = boostfs::current_path() / boostfs::path("txMicroTrafficCloud");
  std::cout << "Load Plugin from " << pluginPath << std::endl;

  typedef boost::shared_ptr<TrafficCloud::TrafficManager>(PluginCreate)();
  boost::function<PluginCreate> pluginCreator;
  try {
    pluginCreator =
        boostdll::import_alias<PluginCreate>(pluginPath, "create_plugin", boostdll::load_mode::append_decorations);
  } catch (const boost::system::system_error &err) {
    std::cerr << "Cannot load Plugin from " << pluginPath << std::endl;
    std::cerr << err.what() << std::endl;
    return -1;
  }

  /* create the plugin */
  boost::shared_ptr<TrafficCloud::TrafficManager> plugin = pluginCreator();
  std::cout << "plugin class name : " << typeid(plugin).name() << std::endl;
  std::cout << "Plugin Name: " << plugin->module_name() << std::endl;
  std::cout << "Plugin Version = " << plugin->module_version() << std::endl;

  return 0;
}
#endif

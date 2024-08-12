// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "common/log/log.h"
#include "common/utils/misc.h"
#include "common/xml_parser/xosc_replay/xosc_replay_converter.h"
#ifdef __CLOUD_HADMAP_SERVER__
#  include "common/utils/xsd_validator.h"
#endif
#include "database/entity/scenario.h"
#include "engine/config.h"
#include "server_scene/scene_data.h"
#include "xml_parser/entity/parser_json.h"

#include "asam.h"
#include "boost/filesystem/path.hpp"
#include "gflags/gflags.h"

DEFINE_string(usage, R"(
  Usage: xosc_convert_tool COMMAND --input_xosc=<xosc filename> --input_map=<map filename> --param=<json string>

  Commands:
    automatic             auto select `xosc2sim` or `xosc2simrec` by param
    xosc2sim              world xosc to sim
    serialize2points      logsim xosc to points
    xosc2simrec           logsim xosc to simrec
    frames2xosc           pblog to xosc

)",
              "");
DEFINE_string(op_serialize2points, "serialize2points", "");
DEFINE_string(op_frames2xosc, "frames2xosc", "");
DEFINE_string(op_xosc2sim, "xosc2sim", "");
DEFINE_string(op_xosc2simrec, "xosc2simrec", "");
DEFINE_string(op_automatic, "automatic", "");
DEFINE_string(input_frames, "", "");
DEFINE_string(input_xosc, "", "");
DEFINE_string(input_map, "", "");
DEFINE_string(root_dir, "/home/sim/data", "default data dir of sim-agent");
DEFINE_string(sim_log_dir, "/home/work/sim-agent/log", "default log dir of sim-agent");
DEFINE_string(param, "", "");

void GenericInit() {
  CEngineConfig::Instance().Init(FLAGS_root_dir.c_str(), FLAGS_root_dir.c_str());
  xercesc::XMLPlatformUtils::Initialize();
  CLog::Initialize(FLAGS_sim_log_dir.c_str(), true);
#ifdef __CLOUD_HADMAP_SERVER__
  utils::XsdValidator::Instance().Initialize(FLAGS_root_dir + "/OpenX");
#endif
}

void Serialize2PointsHandler() {
  xercesc::XMLPlatformUtils::Initialize();

  auto status = XOSCReplayConverter{}.SerializeToPoints(FLAGS_input_xosc);
  std::cout << status.ok() << " " << status.ToString() << std::endl;
}

void Frames2XoscHandler() {
  GenericInit();

  XOSCReplayConverter::Options opts;
  opts.map_filename = FLAGS_input_map;

  XOSCReplayConverter{}.ParseFromSimlog(boost::filesystem::exists(FLAGS_input_frames)
                                            ? FLAGS_input_frames
                                            : FLAGS_root_dir + "/scene/" + FLAGS_input_frames,
                                        opts);
}

void Xosc2SimHandler() {
#ifdef __CLOUD_HADMAP_SERVER__
  GenericInit();

  sTagEntityScenario scenario;
  scenario.m_strName = FLAGS_input_xosc;
  scenario.m_strMapName = FLAGS_input_map;
  scenario.m_strMapVersion = "";
  CParserJson{}.XoscToSim(CEngineConfig::Instance().ResourceDir(), "scene", scenario);
#endif
}

void Xosc2SimrecHandler() {
  GenericInit();

  XOSCReplayConverter::Options opts;
  opts.map_filename = FLAGS_input_map;

  XOSCReplayConverter{}.SerializeToSimrec(
      boost::filesystem::exists(FLAGS_input_xosc) ? FLAGS_input_xosc : FLAGS_root_dir + "/scene/" + FLAGS_input_xosc,
      opts);
}

void Automatic() {
#ifdef __CLOUD_HADMAP_SERVER__
  GenericInit();

  std::cout << "automatic param: " << FLAGS_param << std::endl;
  Json::Value param;
  param << FLAGS_param;
  std::string category = param.get("category", "").asString();
  std::string xosc = param.get("name", "").asString();
  std::string map_name = boost::filesystem::path(param.get("mapUrl", "").asString()).filename().string();
  if (category.empty() || xosc.empty() || map_name.empty()) {
    std::cerr << "invalid param: " << FLAGS_param << std::endl;
    return;
  }
  if (category == "WORLD_SIM") {
    sTagEntityScenario scenario;
    scenario.m_strName = xosc;
    scenario.m_strMapName = map_name;
    scenario.m_strMapVersion = "";
    CParserJson{}.XoscToSim(CEngineConfig::Instance().ResourceDir(), "scene", scenario);
  } else if (category == "LOG_SIM") {
    XOSCReplayConverter::Options opts;
    opts.map_filename = map_name;
    XOSCReplayConverter{}.SerializeToSimrec(FLAGS_root_dir + "/scene/" + xosc, opts);
  }
#endif
}

#define CHECK_ARG(arg)                                                    \
  {                                                                       \
    google::CommandLineFlagInfo info;                                     \
    if (google::GetCommandLineFlagInfo(#arg, &info) && info.is_default) { \
      std::cerr << FLAGS_usage << std::endl;                              \
      exit(-1);                                                           \
    }                                                                     \
  }

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cerr << FLAGS_usage << std::endl;
    exit(-1);
  }

  std::map<std::string, std::function<void()>> handlers = {
      {FLAGS_op_serialize2points, Serialize2PointsHandler},
      {FLAGS_op_frames2xosc, Frames2XoscHandler},
      {FLAGS_op_xosc2sim, Xosc2SimHandler},
      {FLAGS_op_xosc2simrec, Xosc2SimrecHandler},
      {FLAGS_op_automatic, Automatic},
  };

  std::string op = argv[1];
  if (handlers.count(op) <= 0) {
    std::cerr << FLAGS_usage << std::endl;
    exit(-1);
  }

  google::ParseCommandLineFlags(&argc, &argv, false);
  if (op == FLAGS_op_frames2xosc) {
    CHECK_ARG(input_frames);
  } else if (op != FLAGS_op_automatic) {
    CHECK_ARG(input_xosc);
  } else {
    CHECK_ARG(input_param);
  }

  handlers.at(op)();

  return 0;
}

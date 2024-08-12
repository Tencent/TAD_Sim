/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc_replay/xosc_replay_converter.h"

#ifdef __linux__
#  include <malloc.h>
#endif
#include <gflags/gflags.h>
#include <google/protobuf/util/json_util.h>
#include <json/json.h>
#include <tinyxml.h>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <locale>
#include <map>
#include <sstream>
#include "common/coord_trans.h"
#include "common/map_util.h"
#include "common/utils/misc.h"
#include "intros_ptree.hpp"
#include "util_traits.hpp"

#include "../xml_parser/entity/parser.h"
#include "common/engine/math/vec3.h"
#include "common/map_data/map_data_predef.h"
#include "common/utils/oid_generator.h"
#include "common/xml_parser/xosc/xosc_reader_1_0_v4.h"
#include "common/xml_parser/xosc_replay/xosc_replay.h"
#include "engine/config.h"
#include "server_map_cache/map_data_cache.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

DEFINE_string(sim_tpl, R"(<?xml version="1.0" encoding="utf-8" standalone="yes" ?>
<simulation version="1.0">
  <mapfile lon="ego_start_lon" lat="ego_start_lat" alt="ego_start_alt" unrealLevelIndex="0">map_filepath</mapfile>
  <traffic>traffic_file_name</traffic>
  <planner>
    <ego type="suv" />
    <route id="0" type="start_end" start="ego_start_lon,ego_start_lat" end="ego_end_lon,ego_end_lat" />
    <start_v>0</start_v>
    <theta>ego_start_heading</theta>
    <Velocity_Max>20</Velocity_Max>
    <altitude start="ego_start_alt" end="ego_end_alt" />
    <InputPath points="ego_start_lon,ego_start_lat,0.000;ego_end_lon,ego_end_lat,ego_end_alt" />
  </planner>
</simulation>
)",
              "sim file template");
DEFINE_string(pj_latlong_str, "+proj=latlong +datum=WGS84 +no_defs", "");
DEFINE_string(author_simcity, "TAD SimCity", "");
DEFINE_string(author_logsim, "logsim", "");
DEFINE_string(no_transform_xyz_author, "TAD SimCity;logsim", "");
DEFINE_string(l2w_event_trigger_prefix, "$Ego_Log2WorldEventTrigger_", "");
DEFINE_int32(route_sample_rate, 10, "");
DEFINE_bool(simrec_enable_load_hadmap, true, "");
namespace internal {

constexpr double kEps = 1e-6;
constexpr double kDefaultVelocity = 0.3;
// string literal
constexpr char kFrameKeyLoc[] = "loc";
constexpr char kFrameKeyTfc[] = "tfc";
constexpr char kFrameKeyExtInfo[] = "extInfo";
constexpr char kEgoEntityName[] = "ego";
constexpr char kSceneTypeXosc[] = "xosc";
constexpr char kDefaultVehicleName[] = "default";
constexpr char kDefaultVehicleCategory[] = "car";
constexpr char kPropertyKeyControl[] = "control";
constexpr char kPropertyValInternal[] = "internal";
constexpr char kPropertyValExternal[] = "external";
constexpr char kProPertyKeyModelId[] = "model_id";
/////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __CLOUD_HADMAP_SERVER__
DEFINE_string(suffix_of_sim, ".sim", "suffix of sim file");
DEFINE_string(suffix_of_simrec, ".simrec", "suffix of simrec file");
#else
DEFINE_string(suffix_of_sim, "_convert.sim", "suffix of sim file");
DEFINE_string(suffix_of_simrec, "_convert.simrec", "suffix of simrec file");
#endif

DEFINE_string(suffix_of_xosc, ".xosc", "suffix of xosc file");

DEFINE_string(suffix_of_json, ".json", "suffix of json file");

DEFINE_string(prefix_of_logic_file, "../hadmap/", "prefix of logic file");

DEFINE_int32(default_precision, 14, "float/double default precision");

std::string D2S(double d, int precision = FLAGS_default_precision) {
  thread_local std::ostringstream oss;
  oss.clear();
  oss.str("");
  oss << std::setprecision(precision) << d;
  return std::move(oss.str());
}

double S2D(const std::string& s) {
  try {
    return std::stod(s);
  } catch (...) {
    return 0.0;
  }
}

bool PbToJson(const google::protobuf::Message& message, std::string* output) {
  // 清空输出参数,否则内容会附加在已有内容之后
  output->clear();

  google::protobuf::util::JsonPrintOptions opts;

  // 是否把枚举值当作字符整形数,缺省是字符串
  opts.always_print_enums_as_ints = true;
  // 是否把下划线字段更改为驼峰格式,缺省时更改
  opts.preserve_proto_field_names = true;
  // 是否输出仅有默认值的原始字段,缺省时忽略
  opts.always_print_primitive_fields = true;

  google::protobuf::util::MessageToJsonString(message, output, opts);

  return true;
}

std::string ReplaceSuffix(const std::string& file, const std::string& s) {
  boost::filesystem::path pwd{file};
  return (pwd.parent_path() / (pwd.stem().string() + s)).string();
}

bool SuffixIsValid(const std::string& file) {
  boost::filesystem::path p{file};
  std::string ext = p.extension().string();
  if (kHadmapTypeDict.count(ext) <= 0) {
    return false;
  }
  auto type = kHadmapTypeDict.at(ext);
  return type == hadmap::MAP_DATA_TYPE::OPENDRIVE;
}

template <typename ct>
auto FindByName(ct& c, const std::string& name) {
  auto finder = [&](const auto& e) { return e.name == name; };
  return std::find_if(c.begin(), c.end(), finder);
}

template <typename ct>
auto FindByEntityRef(const ct& c, const std::string& name) {
  auto finder = [&](const auto& e) { return e.entityRef == name; };
  return std::find_if(c.begin(), c.end(), finder);
}

std::string GetEgoEntityName(const SIM::OSC::OpenScenarioPtr& osc) {
  for (auto& entity : osc->Entities) {
    for (auto& kv : entity.Vehicle.Properties.Property) {
      if (kv.name == internal::kPropertyKeyControl && kv.value == internal::kPropertyValExternal) {
        return entity.name;
      }
    }
    std::string name = entity.name;
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
    if (name.find(internal::kEgoEntityName) != std::string::npos) {
      return entity.name;
    }
  }
  return internal::kEgoEntityName;
}

void FillSimulationByDefaultSettings(const SIM::OSC::ScenarioObjectPtr& scenario_object, CSimulation& sim) {
  auto setter = [](std::string& field, const char* val) { field = val; };

  if (scenario_object) {
    scenario_object->Decode();
    if (scenario_object->type.second == SIM::OSC::VehicleType::kTruck) {
      sim.m_planner.m_strType = "combination";
      sim.m_planner.m_strName = "truck";
    }
  }
  if (sim.m_planner.m_strType.empty() || sim.m_planner.m_strName.empty()) {
    setter(sim.m_planner.m_strType, "car");
    setter(sim.m_planner.m_strName, "suv");
  }
  setter(sim.m_planner.m_strVelocityMax, "80");
  setter(sim.m_planner.m_accelerationMax, "0");
  setter(sim.m_planner.m_decelerationMax, "10");
  setter(sim.m_planner.m_route.m_strType, "start_end");
  setter(sim.m_planner.m_route.m_strID, "0");
}

}  // namespace internal

std::atomic_int XOSCReplayConverter::catalog_init_flag_;

boost::shared_mutex XOSCReplayConverter::catalog_rw_mutex_;

// 2-red;1-yellow;0-green
std::unordered_map<std::string, int> XOSCReplayConverter::signal_state_map_ = {
    {"true;false;false", 2}, {"false;true;false", 1}, {"false;false;true", 0}};

std::unordered_map<std::string, SIM::OSC::OpenCatalogPtr> XOSCReplayConverter::catalog_map_;

constexpr const char* XOSCReplayConverter::catalog_name_[];

XOSCReplayConverter::XOSCReplayConverter() {
#ifdef PROJ_NEW_API
  m_C = proj_context_create();
#else
  pj_map_[FLAGS_pj_latlong_str] = pj_init_plus(FLAGS_pj_latlong_str.c_str());
#endif
  if (++catalog_init_flag_ <= 1) {
    boost::unique_lock<boost::shared_mutex> _(catalog_rw_mutex_);
    auto& engine_config = CEngineConfig::Instance();
    catalog_map_[catalog_name_[kMiscObjectCatalog]] = SIM::OSC::LoadCatalog(engine_config.getMisobject());
    catalog_map_[catalog_name_[kVehicleCatalog]] = SIM::OSC::LoadCatalog(engine_config.getVehicle());
    catalog_map_[catalog_name_[kPedestrianCatalog]] = SIM::OSC::LoadCatalog(engine_config.getPedestrian());
    catalog_map_[catalog_name_[kDefaultCatalog]] = SIM::OSC::LoadCatalog(engine_config.getCataLogDir());
  } else {
    --catalog_init_flag_;
  }
}

XOSCReplayConverter::~XOSCReplayConverter() {
#ifdef PROJ_NEW_API
  if (m_PJ) {
    proj_destroy(m_PJ);
  }
  if (m_PJ2) {
    proj_destroy(m_PJ2);
  }
  if (m_C) {
    proj_context_destroy(m_C);
  }
#else
  for (auto& kv : pj_map_) {
    if (kv.second) {
      pj_free(kv.second);
      kv.second = nullptr;
    }
  }
#endif
#ifdef __linux__
  malloc_trim(0);
#endif
}

void XOSCReplayConverter::ExpandCatalogReference() {
  boost::shared_lock<boost::shared_mutex> _(catalog_rw_mutex_);
  if (!osc_ || catalog_map_.empty()) {
    return;
  }
  for (auto& so : osc_->Entities) {
    auto& ref = so.CatalogReference;

    if (ref.catalogName.empty() && ego_entity_name_ != so.name) {
      if (!so.Vehicle.vehicleCategory.empty()) {
        ref.catalogName = catalog_name_[kVehicleCatalog];
        ref.entryName = so.Vehicle.name;
      } else if (!so.MiscObject.miscObjectCategory.empty()) {
        ref.catalogName = catalog_name_[kMiscObjectCatalog];
        ref.entryName = so.MiscObject.name;
      } else if (!so.Pedestrian.pedestrianCategory.empty()) {
        ref.catalogName = catalog_name_[kPedestrianCatalog];
        ref.entryName = so.Pedestrian.name;
      }
    }

    if (ref.catalogName.empty() || ref.entryName.empty() || catalog_map_.count(ref.catalogName) <= 0 ||
        !catalog_map_.at(ref.catalogName)) {
      continue;
    }

    std::string catalog_name = (ego_entity_name_ == so.name ? catalog_name_[kDefaultCatalog] : ref.catalogName);
    auto& catalog = catalog_map_[catalog_name]->Catalog;

    auto expand_impl = [](const std::string& name, const auto& all, auto& the) {
      auto it = std::find_if(all.begin(), all.end(), [&](const auto& one) { return one.name == name; });
      if (it != all.end()) {
        the = *it;
      }
    };
    expand_impl(ref.entryName, catalog.MiscObject, so.MiscObject);
    expand_impl(ref.entryName, catalog.Vehicle, so.Vehicle);
    expand_impl(ref.entryName, catalog.Pedestrian, so.Pedestrian);
  }
}

void XOSCReplayConverter::ContractCatalogReference() {
  boost::shared_lock<boost::shared_mutex> _(catalog_rw_mutex_);
  if (!osc_ || catalog_map_.empty()) {
    return;
  }

  auto update_ref_impl = [&](auto& so) {
    auto& ref = so.CatalogReference;
    if (!so.Vehicle.empty() || so.type.first == SIM::OSC::ObjectType::kVehicle) {
      ref.catalogName = catalog_name_[so.name != ego_entity_name_ ? kVehicleCatalog : kDefaultCatalog];
      ref.entryName = so.Vehicle.name;
    } else if (!so.MiscObject.empty() || so.type.first == SIM::OSC::ObjectType::kStaticObstacle) {
      ref.catalogName = catalog_name_[kMiscObjectCatalog];
      ref.entryName = so.MiscObject.name;
    } else if (!so.Pedestrian.empty() || so.type.first == SIM::OSC::ObjectType::kDynamicObstacle) {
      ref.catalogName = catalog_name_[kPedestrianCatalog];
      ref.entryName = so.Pedestrian.name;
    }
  };

  auto expand_ref_by_name_impl = [](const std::string& name, const auto& all, auto& the) {
    auto it = std::find_if(all.begin(), all.end(), [&](auto&& one) { return one.name == name; });
    if (it != all.end()) {
      the = *it;
      return true;
    }
    return false;
  };

  auto expand_ref_by_mid_impl = [](const std::string& id, const auto& all, auto& the) {
    auto it = std::find_if(all.begin(), all.end(), [&](const auto& one) {
      auto& property = one.Properties.Property;
      auto mid = std::find_if(property.begin(), property.end(),
                              [](auto&& p) { return p.name == internal::kProPertyKeyModelId; });
      return mid != property.end() && mid->value == id;
    });
    if (it != all.end()) {
      the = *it;
      return true;
    }
    return false;
  };

  for (auto& so : osc_->Entities) {
    if (so.CatalogReference.empty()) {
      update_ref_impl(so);
    }

    auto& ref = so.CatalogReference;
    if (ref.catalogName.empty() || catalog_map_.count(ref.catalogName) <= 0 || !catalog_map_.at(ref.catalogName)) {
      continue;
    }

    bool expand_ret = false;
    auto& catalog = catalog_map_[ref.catalogName]->Catalog;
    if (so.type.second != -1) {
      std::string model_id = std::to_string(so.type.second);
      expand_ret |= expand_ref_by_mid_impl(model_id, catalog.MiscObject, so.MiscObject);
      expand_ret |= expand_ref_by_mid_impl(model_id, catalog.Vehicle, so.Vehicle);
      expand_ret |= expand_ref_by_mid_impl(model_id, catalog.Pedestrian, so.Pedestrian);
    } else if (!ref.entryName.empty()) {
      expand_ret |= expand_ref_by_name_impl(ref.entryName, catalog.MiscObject, so.MiscObject);
      expand_ret |= expand_ref_by_name_impl(ref.entryName, catalog.Vehicle, so.Vehicle);
      expand_ret |= expand_ref_by_name_impl(ref.entryName, catalog.Pedestrian, so.Pedestrian);
    }
    if (expand_ret) {
      so.CatalogReference.clear();
    }
  }
}

utils::Status XOSCReplayConverter::SerializeToSimrec(const std::string& xosc) {
  Options opts;
  return SerializeToSimrec(xosc, opts);
}

utils::Status XOSCReplayConverter::SerializeToSimrec(const std::string& xosc, const Options& opts) {
  namespace bfs = boost::filesystem;
  if (!bfs::exists(bfs::path(xosc))) {
    return utils::Status::NotFound(utils::Slice(xosc));
  }
  try {
    // file to osc
    osc_ = SIM::OSC::Load(xosc);
    cur_time_ms_ = CurTime();
  } catch (const std::runtime_error& ex) {
    return utils::Status::Corruption(utils::Slice("load xosc"), utils::Slice(ex.what()));
  }

  ResetMapFilename(xosc, opts);
  if (opts.map_filename.empty()) {
    return utils::Status::InvalidArgument(utils::Slice("opts.map_filename and xosc.LogicFile cannot be both empty"));
  }
  if (!bfs::exists(bfs::path(opts.map_filename))) {
    return utils::Status::NotFound(utils::Slice(opts.map_filename));
  }
  try {
    if (FLAGS_simrec_enable_load_hadmap) {
      std::string pure_map_filename = bfs::path(opts.map_filename).filename().string();
      auto pos = pure_map_filename.find(VERSION_NAME_DELIM);
      auto strMapName = computeMapCompleteName(pure_map_filename.substr(pos + 1),
                                               pos != std::string::npos ? pure_map_filename.substr(0, pos) : "");
      auto wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName.c_str());
      CHadmap* hadmap = CMapDataCache::Instance().HadmapCache(wstrMapName.c_str());
      if (!hadmap) {
        CMapDataCache::Instance().LoadHadMap(wstrMapName.c_str());
        hadmap = CMapDataCache::Instance().HadmapCache(wstrMapName.c_str());
      }
      if (hadmap && hadmap->MapQuery().GettxMapHandle()) {
        hadmap::txOdHeaderPtr header;
        hadmap::getHeader(hadmap->MapQuery().GettxMapHandle(), header);
        if (header) {
          odr_ = std::make_shared<SIM::ODR::xmlOpenDRIVE>();
          odr_->header.north = header->getNorth();
          odr_->header.south = header->getSouth();
          odr_->header.east = header->getEast();
          odr_->header.west = header->getWest();
          odr_->header.geoReference = header->getGeoReference();
        }
      }
    } else {
      // init odr, use geo reference
      odr_ = internal::SuffixIsValid(opts.map_filename) ? SIM::ODR::Load(opts.map_filename) : nullptr;
    }
  } catch (const std::runtime_error& ex) {
    return utils::Status::Corruption(utils::Slice("load xodr"), utils::Slice(ex.what()));
  }

  std::string simrec_file = internal::ReplaceSuffix(xosc, internal::FLAGS_suffix_of_simrec);
  // init ego car name
  ego_entity_name_ = internal::GetEgoEntityName(osc_);

  ExpandCatalogReference();
  sTagSimuTraffic scene;
  XOSCReader_1_0_v4 xosc_reader;
  {
    scene.m_strType = internal::kSceneTypeXosc;
    std::string pure_map_filename = bfs::path(opts.map_filename).filename().string();
    auto pos = pure_map_filename.find(VERSION_NAME_DELIM);
    auto& sim = scene.m_simulation;
    sim.m_mapFile.m_strMapfile = pure_map_filename.substr(pos + 1);
    if (pos != std::string::npos) {
      sim.m_mapFile.m_strVersion = pure_map_filename.substr(0, pos);
    }
#ifdef XOSC2SIMREC_GEN_SIM_WITH_XOSC_READER
    xosc_reader.ParseXOSC(xosc.c_str(), scene);
#else
    xosc_reader.InitMapQuery(sim, FLAGS_simrec_enable_load_hadmap);
#endif
    if (sim.m_mapFile.m_strMapfile.size() > 0) {
      sim.m_mapFile.m_strMapfile = internal::FLAGS_prefix_of_logic_file + sim.m_mapFile.m_strMapfile;
    }
  }

  // get traffic lights xyz
  hadmap::txObjects objects;
  std::unordered_map<std::string, hadmap::txObjectPtr> traffic_lights_table;
  if (xosc_reader.m_query && xosc_reader.m_query->GettxMapHandle()) {
    hadmap::getObjects(xosc_reader.m_query->GettxMapHandle(), std::vector<hadmap::txLaneId>{},
                       {hadmap::OBJECT_TYPE_TrafficLights}, objects);
  }
  for (auto object : objects) {
    if (!object) {
      continue;
    }
    traffic_lights_table[std::to_string(object->getId())] = object;
  }

  // update traffic lights
  for (auto& controller : osc_->RoadNetwork.TrafficSignals.TrafficSignalController) {
    for (auto i = 0; i < controller.Phase.size(); i++) {
      if (controller.Phase[i].TrafficSignalState.empty()) {
        continue;
      }
      for (auto& traffic_signal_state : controller.Phase[i].TrafficSignalState) {
        auto id = traffic_signal_state.trafficSignalId;
        if (traffic_lights_table.count(id) <= 0) {
          continue;
        }
        auto pos = traffic_lights_table.at(id)->getPos();
        traffic_signal_state.x = pos.x;
        traffic_signal_state.y = pos.y;
        if (signal_state_map_.count(traffic_signal_state.state) > 0) {
          traffic_signal_state.color = signal_state_map_.at(traffic_signal_state.state);
        }
      }
      controller.Phase[i].time =
          (i == 0 ? controller.delay : controller.Phase[i - 1].time) + controller.Phase[i].duration;
    }
  }

  sim_msg::TrafficRecords4Logsim simrec;
  uint64_t duration_ms = 0;
  // fill location
  PartialSerialize(simrec.mutable_ego_location(), duration_ms, opts.frame_interval);
  // fill traffic
  PartialSerialize(simrec.mutable_traffic_record(), duration_ms, opts.frame_interval);
  if (simrec.ego_location_size() + simrec.traffic_record_size() == 0) {
    return utils::Status::Corruption(utils::Slice("no ego_location and traffic_record"));
  }
  simrec.set_duration_ms(duration_ms);
  // init disengage_events
  for (auto& pds : osc_->ParameterDeclarations) {
    for (auto& pd : pds.ParameterDeclaration) {
      if (pd.name.find(FLAGS_l2w_event_trigger_prefix) == std::string::npos) {
        continue;
      }
      auto disengage_events = simrec.add_disengage_events();
      disengage_events->set_type(sim_msg::DisengageEventType::Disengage_Sim);
      disengage_events->set_timestamp_ms(static_cast<int64_t>(std::atof(pd.value.c_str()) * 1000));
    }
  }
  {
    // write *.simrec
    std::ofstream ofs(simrec_file, std::ios::binary);
    simrec.SerializeToOstream(&ofs);
    ofs.close();
  }

  double ego_start_velocity = internal::kDefaultVelocity;
  if (simrec.ego_location_size() > 0) {
    ego_start_velocity = simrec.ego_location(0).velocity().x() /
                         (std::cos(simrec.ego_location(0).rpy().z() * M_PI / 180.0) + internal::kEps);
  }

  {
    // write *.sim
    auto& sim = scene.m_simulation;
    sim.SetTrafficFile(bfs::path(simrec_file).filename().string().c_str());
    sim.m_nID = scene.m_nID;
    sim.m_planner.m_fStartVelocity = ego_start_velocity;
    sim.m_planner.m_strStartVelocity = std::to_string(ego_start_velocity);

#ifdef XOSC2SIMREC_GEN_SIM_WITH_XOSC_READER
    sim.m_planner.m_route.m_strMids.clear();

    std::vector<std::string> inputs;
    boost::algorithm::split(inputs, sim.m_planner.m_inputPath.m_strPoints, boost::algorithm::is_any_of(";"));
    sim.m_planner.m_inputPath.m_strPoints.clear();
    for (size_t i = 0; i < inputs.size(); i++) {
      if (i != 0 && i != inputs.size() - 1 && (i % FLAGS_route_sample_rate) != 0) {
        continue;
      }
      if (!sim.m_planner.m_inputPath.m_strPoints.empty()) {
        sim.m_planner.m_inputPath.m_strPoints += ";";
      }
      sim.m_planner.m_inputPath.m_strPoints += inputs[i];
    }
#else
    for (int i = 0; i < simrec.ego_location_size(); i++) {
      if (i != 0 && i != simrec.ego_location_size() - 1 && (i % FLAGS_route_sample_rate) != 0) {
        continue;
      }
      CPoint3D point(simrec.ego_location(i).position().x(), simrec.ego_location(i).position().y(), 0.0);
      if (i == 0) {
        sim.m_planner.m_route.m_strStart = point.ToString(true);
        sim.m_planner.m_strTheta = to_string_with_precision(simrec.ego_location(i).rpy().z());
      }
      if (i == simrec.ego_location_size() - 1) {
        sim.m_planner.m_route.m_strEnd = point.ToString(true);
      }
      if (!sim.m_planner.m_inputPath.m_strPoints.empty()) {
        sim.m_planner.m_inputPath.m_strPoints += ";";
      }
      sim.m_planner.m_inputPath.m_strPoints += point.ToString();
    }
#endif
#ifdef XOSC2SIMREC_GEN_SIM_WITH_XOSC_READER
#else
    internal::FillSimulationByDefaultSettings(SIM::OSC::GetScenarioObjectPtrByName(osc_, ego_entity_name_), sim);
    for (auto& pds : osc_->ParameterDeclarations) {
      for (auto& pd : pds.ParameterDeclaration) {
        CSimulation::ParameterDeclaration one;
        one.name = pd.name;
        one.parameterType = pd.parameterType;
        one.value = pd.value;
        sim.m_parameterDeclarations.emplace_back(one);
      }
    }
#endif
    sim.Save(internal::ReplaceSuffix(xosc, internal::FLAGS_suffix_of_sim).c_str());
  }
  if (opts.write_debug_json) {
    std::ofstream ofs(internal::ReplaceSuffix(xosc, internal::FLAGS_suffix_of_json));
    std::string s;
    internal::PbToJson(simrec, &s);
    ofs << s;
    ofs.close();
  }

  return utils::Status::OK();
}

utils::Status XOSCReplayConverter::SerializeToPoints(const std::string& xosc) {
  namespace bfs = boost::filesystem;
  if (!bfs::exists(bfs::path(xosc))) {
    return utils::Status::NotFound(utils::Slice(xosc));
  }
  try {
    osc_ = SIM::OSC::Load(xosc);
  } catch (const std::runtime_error& ex) {
    return utils::Status::Corruption(utils::Slice("load xosc"), utils::Slice(ex.what()));
  }

  ego_entity_name_ = internal::GetEgoEntityName(osc_);

  std::ofstream ofs(internal::ReplaceSuffix(xosc, ".txt"));
  for (auto& scenario_object : osc_->Entities) {
    auto& name = scenario_object.name;
    scenario_object.Decode();
    auto trajectory = SIM::OSC::GetTrajectoryPtrByName(osc_, name);
    if (!trajectory || trajectory->Shape.Polyline.Vertexs.empty()) {
      continue;
    }

    ofs << name << ":" << std::endl;
    for (auto& vertex : trajectory->Shape.Polyline.Vertexs) {
      auto& world_position = vertex.Position.WorldPosition;
      ofs << world_position.x << " " << world_position.y << " " << world_position.z << std::endl;
    }
  }
  ofs.close();

  return utils::Status::OK();
}

utils::Status XOSCReplayConverter::ParseFromSimlog(const std::string& simlog, const std::string& map_filename) {
  Options opts;
  opts.map_filename = map_filename;
  return ParseFromSimlog(simlog, opts);
}

utils::Status XOSCReplayConverter::ParseFromSimlog(const std::string& simlog, const Options& opts) {
  std::ifstream ifs(simlog);
  if (!ifs.is_open()) {
    return utils::Status::NotFound(utils::Slice(simlog));
  }

  ResetMapFilename(simlog, opts);
  if (opts.map_filename.empty()) {
    return utils::Status::InvalidArgument(utils::Slice("map_filename in opts must be non-empty"));
  }

  try {
    // init odr, use geo reference
    odr_ = internal::SuffixIsValid(opts.map_filename) ? SIM::ODR::Load(opts.map_filename) : nullptr;
  } catch (const std::runtime_error& ex) {
    return utils::Status::Corruption(utils::Slice("load xodr"), utils::Slice(ex.what()));
  }

  osc_ = std::make_shared<SIM::OSC::xmlOpenSCENARIO>();

  // parse header, set logic file path
  ParseHeader(opts.map_filename);

  // init ego car name
  SetEgoEntityName(internal::kEgoEntityName);

  // [{frame},{frame}] or {frame}\n{frame}
  int fid = 0;
  for (std::string line; std::getline(ifs, line);) {
    if (line.empty()) {
      continue;
    }
    Json::Value one;
    if (!Json::Reader{}.parse(line, one)) {
      return utils::Status::Corruption(utils::Slice("json parse failed or json invalid"));
    }

    Json::Value root(Json::arrayValue);
    one.isObject() ? root.append(one) : (root = one);

    for (auto&& record : root) {
      if (!record.isObject() || !record.isMember(internal::kFrameKeyLoc) ||
          !record[internal::kFrameKeyLoc].isString() || !record.isMember(internal::kFrameKeyTfc) ||
          !record[internal::kFrameKeyTfc].isString()) {
        continue;
      }
      // 当前帧时刻，单位（s）
      double frame_time = fid * opts.frame_interval;
      std::string ext_info = record[internal::kFrameKeyExtInfo].asString();
      ext_info = boost::regex_replace(ext_info, boost::regex(R"([\w\-.]+)"), R"("$&")");

      PartialParse<sim_msg::Location>(record[internal::kFrameKeyLoc].asString(), ext_info.c_str(), frame_time);

      PartialParse<sim_msg::Traffic>(record[internal::kFrameKeyTfc].asString(), ext_info.c_str(), frame_time);
      ++fid;
    }
  }

  ContractCatalogReference();

  std::ofstream ofs(internal::ReplaceSuffix(simlog, internal::FLAGS_suffix_of_xosc), std::ios::out);
  ofs << SIM::OSC::Dump(osc_);
  ofs.close();

  return utils::Status::OK();
}

void XOSCReplayConverter::ResetMapFilename(const std::string& xosc_filename, const Options& opts) {
  namespace bfs = boost::filesystem;
  bfs::path p = opts.map_filename.empty() && osc_ ? osc_->RoadNetwork.LogicFile.filepath : opts.map_filename;
  if (bfs::exists(p)) {
    std::cout << "use map file: " << p.string() << std::endl;
    const_cast<Options&>(opts).map_filename = p.string();
    return;
  }
  bfs::path p1 = bfs::path(xosc_filename).parent_path() / p.filename();
  if (bfs::exists(p1)) {
    std::cout << "use map file: " << p1.string() << std::endl;
    const_cast<Options&>(opts).map_filename = p1.string();
    return;
  }
  bfs::path p2 = bfs::path(xosc_filename).parent_path() / internal::FLAGS_prefix_of_logic_file / p.filename();
  if (bfs::exists(p2)) {
    std::cout << "use map file: " << p2.string() << std::endl;
    const_cast<Options&>(opts).map_filename = p2.string();
  }
}

void XOSCReplayConverter::PartialParse(const sim_msg::Location& loc, const char* ext_info_s, double t) {
  std::string name = internal::kEgoEntityName;
  if (internal::FindByName(osc_->Entities, name) == osc_->Entities.end()) {
    // ScenarioObject/Vehicle
    SIM::OSC::xmlScenarioObject so;
    so.name = name;
    osc_->Entities.emplace_back(so);

    auto& vehicle = osc_->Entities.back().Vehicle;
    Json::Value ext_info(Json::objectValue);
    ext_info << ext_info_s;
    vehicle.name = ext_info.get("name", internal::kDefaultVehicleName).asString();
    vehicle.vehicleCategory = ext_info.get("category", internal::kDefaultVehicleCategory).asString();
    vehicle.Properties.Property.resize(1);
    vehicle.Properties.Property.back().name = internal::kPropertyKeyControl;
    vehicle.Properties.Property.back().value = internal::kPropertyValExternal;

    // Storyboard/Init
    osc_->Storyboard.Init.Actions.Private.emplace_back(SIM::OSC::xmlPrivate());
    osc_->Storyboard.Init.Actions.Private.back().entityRef = name;
    auto& world_position =
        osc_->Storyboard.Init.Actions.Private.back().PrivateAction.TeleportAction.Position.WorldPosition;
    world_position.h = loc.rpy().z();
    world_position.p = loc.rpy().y();
    world_position.r = loc.rpy().x();
    world_position.x = loc.position().x();
    world_position.y = loc.position().y();
    world_position.z = loc.position().z();
  }
  ParseSpeedAndTrajectory(name, t, loc.rpy().z(), loc.position().x(), loc.position().y(), loc.position().z(),
                          loc.velocity().y());
}

void XOSCReplayConverter::PartialParse(const sim_msg::Traffic& tfc, const char* ext_info_s, double t) {
  for (int i = 0; i < tfc.cars_size(); i++) {
    ParseVehicle(tfc.cars(i), t);
  }
}

void XOSCReplayConverter::ParseHeader(const std::string& map_filename) {
  std::string pure_map_filename = map_filename.substr(map_filename.find(VERSION_NAME_DELIM) + 1);
  char buf[64] = {0};
  std::time_t now = std::time(nullptr);
  std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));
  osc_->FileHeader.description;
  osc_->FileHeader.revMajor = "1";
  osc_->FileHeader.revMinor = "0";
  osc_->FileHeader.author = FLAGS_author_logsim;
  osc_->FileHeader.date = std::string(buf);

  if (osc_->ParameterDeclarations.empty()) {
    osc_->ParameterDeclarations.emplace_back(SIM::OSC::xmlParameterDeclarations());
  }
  osc_->ParameterDeclarations.back().ParameterDeclaration.emplace_back(
      SIM::OSC::xmlParameterDeclaration("string", "ActualMap", FLAGS_pj_latlong_str));

  namespace bfs = boost::filesystem;
  auto logic_file = bfs::path(internal::FLAGS_prefix_of_logic_file) / bfs::path(pure_map_filename).filename();
  osc_->RoadNetwork.LogicFile.filepath = logic_file.string();
}

void XOSCReplayConverter::ParseVehicle(const sim_msg::Car& car, double t) {
  auto so = SIM::OSC::xmlScenarioObject();
  so.id = car.id();
  so.type = SIM::OSC::osc_type_t(SIM::OSC::ObjectType::kVehicle, car.type());
  so.Encode();

  std::string name = so.name;

  if (internal::FindByName(osc_->Entities, name) == osc_->Entities.end()) {
    // ScenarioObject/Vehicle
    osc_->Entities.emplace_back(so);

    auto& vehicle = osc_->Entities.back().Vehicle;
    vehicle.BoundingBox.Dimensions.length = car.length();
    vehicle.BoundingBox.Dimensions.width = car.width();
    vehicle.BoundingBox.Dimensions.height = car.height();
    vehicle.Properties.Property.resize(1);
    vehicle.Properties.Property.back().name = internal::kPropertyKeyControl;
    vehicle.Properties.Property.back().value = internal::kPropertyValInternal;

    // Storyboard/Init
    osc_->Storyboard.Init.Actions.Private.emplace_back(SIM::OSC::xmlPrivate());
    osc_->Storyboard.Init.Actions.Private.back().entityRef = name;
    auto& world_position =
        osc_->Storyboard.Init.Actions.Private.back().PrivateAction.TeleportAction.Position.WorldPosition;
    world_position.h = car.heading();
    world_position.x = car.x();
    world_position.y = car.y();
  }

  ParseSpeedAndTrajectory(name, t, car.heading(), car.x(), car.y(), 0, car.v());
}

void XOSCReplayConverter::ParseSpeedAndTrajectory(const std::string& name, double t, double h, double x, double y,
                                                  double z, double v) {
  auto mg = SIM::OSC::GetManeuverGroupPtrByName(osc_, name);
  if (!mg) {
    auto mgs = GetManeuverGroupsPtr(osc_);
    mg = &(*(mgs->insert(mgs->end(), SIM::OSC::xmlManeuverGroup())));
  }
  // assert(mg != nullptr);

  auto& maneuver_group = *mg;
  maneuver_group.name = name;
  maneuver_group.Actors.EntityRef.entityRef = name;

  auto& maneuver = maneuver_group.Maneuver;
  if (maneuver.Event.empty()) {
    maneuver.Event.emplace_back(SIM::OSC::xmlEvent());
    maneuver.Event[0].Action.emplace_back(SIM::OSC::xmlAction(tad::sim::ObjectIdGenerator{}.Next()));
    maneuver.Event[0]
        .Action.back()
        .PrivateAction.LongitudinalAction.SpeedAction.SpeedActionTarget.AbsoluteTargetSpeed.value = v;
    maneuver.Event[0].Action.emplace_back(SIM::OSC::xmlAction(tad::sim::ObjectIdGenerator{}.Next()));
  }

  auto& follow_trajectory_action = maneuver.Event[0].Action.back().PrivateAction.RoutingAction.FollowTrajectoryAction;
  auto& polyline = follow_trajectory_action.Trajectory.Shape.Polyline;
  polyline.Vertexs.emplace_back(SIM::OSC::xmlVertex());
  polyline.Vertexs.back().time = internal::D2S(t);
  polyline.Vertexs.back().Position.WorldPosition.h = h;
  polyline.Vertexs.back().Position.WorldPosition.x = x;
  polyline.Vertexs.back().Position.WorldPosition.y = y;
  polyline.Vertexs.back().Position.WorldPosition.z = z;
}

utils::Status XOSCReplayConverter::GenSimContent(const char* traffic_file_name, std::string& sim) {
  SIM::OSC::xmlWorldPosition head, tail;
  auto mg = SIM::OSC::GetManeuverGroupPtrByName(osc_, ego_entity_name_);
  if (mg) {
    auto& vertexs = SIM::OSC::GetTrajectoryPtr(mg)->Shape.Polyline.Vertexs;
    if (!vertexs.empty()) {
      vertexs.front().Position.WorldPosition;
      vertexs.back().Position.WorldPosition;
    }
  } else {
    auto ta = SIM::OSC::GetTeleportActionPtrByName(osc_, ego_entity_name_);
    if (!ta) {
      return utils::Status::NotFound(utils::Slice("there is no start pos, whose entryRef is " + ego_entity_name_));
    }
    // head = ta->Position.WorldPosition;
  }

  // transform xy
  TransformXY(head);
  TransformXY(tail);
  // do replace
  sim = FLAGS_sim_tpl;
  boost::replace_all(sim, "ego_start_heading", internal::D2S(head.h));
  boost::replace_all(sim, "ego_start_lon", internal::D2S(head.x));
  boost::replace_all(sim, "ego_start_lat", internal::D2S(head.y));
  boost::replace_all(sim, "ego_start_alt", internal::D2S(head.z));
  boost::replace_all(sim, "ego_end_lon", internal::D2S(tail.x));
  boost::replace_all(sim, "ego_end_lat", internal::D2S(tail.y));
  boost::replace_all(sim, "ego_end_alt", internal::D2S(tail.z));
  boost::replace_all(sim, "traffic_file_name", traffic_file_name);
  auto map_filepath = boost::filesystem::path(internal::FLAGS_prefix_of_logic_file) /
                      boost::filesystem::path(osc_->RoadNetwork.LogicFile.filepath).filename();
  boost::replace_all(sim, "map_filepath", map_filepath.string());

  return utils::Status::OK();
}

void XOSCReplayConverter::PartialSerialize(RepeatedPtrLocation* locs, uint64_t& duration_ms, double frame_interval) {
  auto trajectory = SIM::OSC::GetTrajectoryPtrByName(osc_, ego_entity_name_);
  if (!trajectory) {
    return;
  }

  CalcSpeed(ego_entity_name_, trajectory);

  auto calc_velocity_and_acc_components = [&](const SIM::OSC::xmlWorldPosition& wp, sim_msg::Location* loc) {
    double rad_heading = wp.h * M_PI / 180.0;
    double cos_heading = cos(rad_heading);
    double sin_heading = sin(rad_heading);
    loc->mutable_velocity()->set_x(wp.v * cos_heading);
    loc->mutable_velocity()->set_y(wp.v * sin_heading);
    loc->mutable_acceleration()->set_x(wp.acc * cos_heading);
    loc->mutable_acceleration()->set_y(wp.acc * sin_heading);
  };

  double ref_time = RefTime(ego_entity_name_);

  for (auto& vertex : trajectory->Shape.Polyline.Vertexs) {
    uint64_t time = static_cast<uint64_t>((internal::S2D(vertex.time) + ref_time) * 1000);
    if (!locs->empty() && static_cast<uint64_t>(locs->Get(locs->size() - 1).t()) >= time + cur_time_ms_) {
      continue;
    }
    sim_msg::Location* loc = locs->Add();
    auto& world_position = vertex.Position.WorldPosition;
    loc->mutable_position()->set_x(world_position.x);
    loc->mutable_position()->set_y(world_position.y);
    loc->mutable_position()->set_z(world_position.z);
    loc->set_t(time + cur_time_ms_);
    loc->mutable_rpy()->set_z(world_position.h);
    calc_velocity_and_acc_components(world_position, loc);
    TransformXY(*loc);
    duration_ms = std::max(time, duration_ms);
  }
}

void XOSCReplayConverter::PartialSerialize(RepeatedPtrTraffic* tfcs, uint64_t& duration_ms, double frame_interval) {
  double f_time = -1.0;
  double t_time = -1.0;

  std::map<std::string, std::pair<SIM::OSC::TrajectoryPtr, SIM::OSC::ScenarioObjectPtr>> sc_table;

  // 1st, calc time range
  for (auto& scenario_object : osc_->Entities) {
    // ignore ego car
    const auto& name = scenario_object.name;
    if (name == ego_entity_name_) {
      continue;
    }
    scenario_object.Decode();
    if (scenario_object.type.first == SIM::OSC::ObjectType::kNone) {
      continue;
    }
    // no trajectory
    auto trajectory = SIM::OSC::GetTrajectoryPtrByName(osc_, name);
    if (!trajectory) {
      continue;
    }
    // trajectory vertexs empty
    auto& polyline = trajectory->Shape.Polyline;
    if (polyline.Vertexs.empty()) {
      continue;
    }

    CalcSpeed(name, trajectory);

    sc_table[name].first = trajectory;
    sc_table[name].second = &scenario_object;

    auto t0 = RefTime(name);
    auto t1 = internal::S2D(polyline.Vertexs.front().time) + t0;
    auto t2 = internal::S2D(polyline.Vertexs.back().time) + t0;
    f_time = (f_time < 0 || f_time > t1) ? t1 : f_time;
    t_time = (t_time < t2) ? t2 : t_time;

    scenario_object.age = static_cast<int32_t>((t2 - t1) * 1000);
  }
  if (sc_table.empty()) {
    std::cout << "no traffic vehicle" << std::endl;
    return;
  }
  duration_ms = std::max(static_cast<uint64_t>(t_time * 1000), duration_ms);

  // 时间非法
  if (f_time < 0 || f_time > t_time) {
    return;
  }

  // 2nd, for each valid frames, default 0.01s per frame
  for (double t = 0; t < t_time; t += frame_interval) {
    if (t < f_time && std::fabs(t - f_time) * 10 >= frame_interval) {
      continue;
    }
    // 每帧对应一个Traffic，一个Traffic可包含多辆车
    sim_msg::Traffic* tfc = nullptr;
    for (auto& kv : sc_table) {
      const auto& name = kv.first;
      const auto& trajectory = kv.second.first;
      const auto& polyline = trajectory->Shape.Polyline;
      if (polyline.Vertexs.empty()) {
        continue;
      }
      double _time = internal::S2D(polyline.Vertexs.front().time) + RefTime(name);
      if (_time > t && std::fabs(_time - t) * 10 >= frame_interval) {
        continue;
      }
      if (!tfc) {
        tfc = tfcs->Add();
      }

      const auto sc = kv.second.second;
      if (sc->type.first == SIM::OSC::ObjectType::kVehicle) {
        auto car = tfc->add_cars();
        SerializeCommon(sc_table.at(name).second, car);
        SerializeTrajectory(name, t * 1000, trajectory, car);
      } else if (sc->type.first == SIM::OSC::ObjectType::kDynamicObstacle) {
        auto obstacle = tfc->add_dynamicobstacles();
        SerializeCommon(sc_table.at(name).second, obstacle);
        SerializeTrajectory(name, t * 1000, trajectory, obstacle);
      } else if (sc->type.first == SIM::OSC::ObjectType::kStaticObstacle) {
        auto obstacle = tfc->add_staticobstacles();
        SerializeCommon(sc_table.at(name).second, obstacle);
        SerializeTrajectory(name, t * 1000, trajectory, obstacle);
      } else {
        // std::cout << "ignore object, name: " << name << ", category: " << sc->category << ", time: " << t <<
        // std::endl;
      }
    }
    // lights
    for (auto& controller : osc_->RoadNetwork.TrafficSignals.TrafficSignalController) {
      if (controller.Phase.empty() || controller.delay > t) {
        continue;
      }
      for (auto iter = controller.Phase.begin(); iter != controller.Phase.end() && iter->time < t;
           iter = controller.Phase.erase(iter)) {
      }
      if (controller.Phase.empty() || controller.Phase.front().TrafficSignalState.empty()) {
        continue;
      }
      if (!tfc) {
        tfc = tfcs->Add();
      }
      auto& traffic_signal_state = controller.Phase.front().TrafficSignalState[0];
      auto traffic_light = tfc->add_trafficlights();
      traffic_light->set_id(std::atoi(traffic_signal_state.trafficSignalId.c_str()));
      traffic_light->set_x(traffic_signal_state.x);
      traffic_light->set_y(traffic_signal_state.y);
      traffic_light->set_color(traffic_signal_state.color);
      tfc->set_time(t * 1000 + cur_time_ms_);
    }
  }
}

void XOSCReplayConverter::CalcSpeed(const std::string& name, SIM::OSC::TrajectoryPtr trajectory) {
  if (!trajectory || trajectory->Shape.Polyline.Vertexs.empty()) {
    return;
  }
#ifdef PROJ_NEW_API
  bool WGS84 = true;
#else
  if (odr_ && !odr_->header.geoReference.empty() && pj_map_.count(odr_->header.geoReference) <= 0) {
    pj_map_[odr_->header.geoReference] = pj_init_plus(odr_->header.geoReference.c_str());
  }
  bool WGS84 = true;
  if (odr_) {
    WGS84 = !!pj_map_.count(odr_->header.geoReference) && !!pj_is_latlong(pj_map_.at(odr_->header.geoReference));
  }
#endif

  bool no_start_velocity = false;
  auto& vertexs = trajectory->Shape.Polyline.Vertexs;
  for (auto i = 0; i < vertexs.size(); i++) {
    if (i == 0) {
      auto speed_action = SIM::OSC::GetSpeedActionPtrByName(osc_, name);
      if (speed_action) {
        vertexs[i].Position.WorldPosition.v = speed_action->SpeedActionTarget.AbsoluteTargetSpeed.value;
      } else {
        no_start_velocity = true;
      }
      continue;
    }

    auto& wp = vertexs[i].Position.WorldPosition;
    auto& last_wp = vertexs[i - 1].Position.WorldPosition;
    auto delta_t = internal::S2D(vertexs[i].time) - internal::S2D(vertexs[i - 1].time);
    if (delta_t < 0.001) {
      wp.v = last_wp.v;
      wp.acc = last_wp.acc;
      continue;
    }
    hadmap::Point3d a(last_wp.x, last_wp.y, 0);
    hadmap::Point3d b(wp.x, wp.y, 0);
    wp.v = map_util::distanceBetweenPoints(a, b, WGS84) / (delta_t + internal::kEps);
    for (auto j = i - 1; j >= 0 && no_start_velocity; vertexs[j--].Position.WorldPosition.v = wp.v) {
      // cal speed
    }
    no_start_velocity = false;
    wp.acc = (wp.v - last_wp.v) / (delta_t + internal::kEps);
  }
}

template <typename T>
void XOSCReplayConverter::SerializeTrajectory(const std::string& name, double t, SIM::OSC::TrajectoryPtr trajectory,
                                              T* object) {
  // 动态信息
  auto& polyline = trajectory->Shape.Polyline;
  auto& world_position = polyline.Vertexs.front().Position.WorldPosition;
  object->set_t(t + cur_time_ms_);
  object->set_x(world_position.x);
  object->set_y(world_position.y);
  object->set_z(world_position.z);
  object->set_v(world_position.v);
  object->set_acc(world_position.acc);
  object->set_heading(world_position.h);
  // 处理队头
  polyline.Vertexs.erase(polyline.Vertexs.begin());
  // 转换坐标
  TransformXY(*object);
}

void XOSCReplayConverter::SerializeTrajectory(const std::string& name, double t, SIM::OSC::TrajectoryPtr trajectory,
                                              sim_msg::StaticObstacle* object) {
  // 动态信息
  auto& polyline = trajectory->Shape.Polyline;
  auto& world_position = polyline.Vertexs.front().Position.WorldPosition;
  object->set_t(t + cur_time_ms_);
  object->set_x(world_position.x);
  object->set_y(world_position.y);
  object->set_z(world_position.z);
  object->set_heading(world_position.h);
  // 处理队头
  polyline.Vertexs.erase(polyline.Vertexs.begin());
  // 转换坐标
  TransformXY(*object);
}

template <typename T>
void XOSCReplayConverter::SerializeCommon(const SIM::OSC::ScenarioObjectPtr input, T* object) {
  if (input->name.empty()) {
    return;
  }
  object->set_id(input->id);
  object->set_age(input->age);
  object->set_type(input->type.second);
  const auto& dimensions = input->Dimensions();
}

template <typename T>
void XOSCReplayConverter::TransformXY(T& object) {
  double x = object.x();
  double y = object.y();
  TransformXYImpl(x, y);
  object.set_x(x);
  object.set_y(y);
}

void XOSCReplayConverter::TransformXY(sim_msg::Location& loc) {
  if (!loc.has_position()) {
    return;
  }
  double x = loc.mutable_position()->x();
  double y = loc.mutable_position()->y();
  TransformXYImpl(x, y);
  loc.mutable_position()->set_x(x);
  loc.mutable_position()->set_y(y);
}

void XOSCReplayConverter::TransformXY(SIM::OSC::xmlWorldPosition& pos) { TransformXYImpl(pos.x, pos.y); }

void XOSCReplayConverter::TransformXYImpl(double& x, double& y) {
  if (FLAGS_no_transform_xyz_author.find(osc_->FileHeader.author) != std::string::npos) {
    return;
  }
  // not xodr or no geoReference
  if (!odr_ || odr_->header.geoReference.empty()) {
    coord_trans_api::mercator2lonlat(x, y);
    return;
  }
#ifdef PROJ_NEW_API

#else
  // has xodr file, has geoReference
  if (pj_map_.count(odr_->header.geoReference) <= 0) {
    pj_map_[odr_->header.geoReference] = pj_init_plus(odr_->header.geoReference.c_str());
  }
  if (!pj_map_.at(odr_->header.geoReference)) {
    return;
  }
  if (pj_is_latlong(pj_map_.at(odr_->header.geoReference))) {
    return;
  }
  x += odr_->header.west;
  y += odr_->header.south;
  pj_transform(pj_map_.at(odr_->header.geoReference), pj_map_.at(FLAGS_pj_latlong_str), 1, 1, &x, &y, NULL);
  x *= RAD_TO_DEG;
  y *= RAD_TO_DEG;
#endif
}

int64_t XOSCReplayConverter::CurTime() {
  if (!osc_ || osc_->FileHeader.date.empty()) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
  }
  std::tm t = {};
  std::istringstream iss(osc_->FileHeader.date);
  iss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");
  return mktime(&t) * 1000;
}

double XOSCReplayConverter::RefTime(const std::string& name) {
  double ref_time = 0.0;
  auto sim_time_cond = SIM::OSC::GetSimulationTimeConditionPtrByName(osc_, name);
  if (sim_time_cond) {
    ref_time = sim_time_cond->value * sim_time_cond->sign;
  }
  return ref_time;
}

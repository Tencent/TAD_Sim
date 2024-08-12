// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include "boost/filesystem/path.hpp"
#include "tinyxml2.h"

#include "config.h"
#include "scene.pb.h"
#include "utils/msgs.h"

const std::string kDefaultCatalogFileRelativePath = "Catalogs/Vehicles/default.xosc";
const std::string kVehicleCatalogFileRelativePath = "Catalogs/Vehicles/VehicleCatalog.xosc";
const std::string kMiscObjectCatalogFileRelativePath = "Catalogs/MiscObjects/MiscObjectCatalog.xosc";
const std::string kPedestrianCatalogFileRelativePath = "Catalogs/Pedestrians/PedestrianCatalog.xosc";
const std::string kModel3dCatalogFileRelativePath = "Catalogs/MapModels/mapmodels.xml";

const std::string kDefaultCatalogName = "CarCatalog";
const std::string kDefaultCatalogEntryName = "ego";

using namespace tinyxml2;

namespace tx_sim {

namespace utils {
//! @brief 函数名：ParseEgoMeasurement
//! @details 函数功能：解析Ego测量数据
//!
//! @param[in] root 一个包含tinyxml2::XMLElement对象的常量指针
//! @param[in] catalog_name 一个包含目录名称的字符串
//! @param[in] entry_name 一个包含条目名称的字符串
//! @param[in] ego_type 一个包含Ego类型的字符串
//! @param[in] ego_measurement_msg 一个包含zmq::message_t对象的引用
bool ParseEgoMeasurement(const std::string& catalogpath, const char* catalog_name, const char* entry_name,
                         const char* ego_type, zmq::message_t& ego_measurement_msg);

//! @brief 函数名：ParseSimCloudCityVehicle
//! @details 函数功能：解析模拟CloudCity车辆
//!
//! @param[in] scene_pb 一个包含zmq::message_t对象的引用
//! @param[in] catalogPath 一个包含目录路径的字符串
void ParseSimCloudCityVehicle(zmq::message_t& scene_pb, const std::string& catalogSysPath,
                              const std::string& catalogDataPath);
}  // namespace utils
namespace coordinator {

class IScenarioParser {
 public:
  //! @brief 函数名：CheckIsOpenScenario
  //! @details 函数功能：检查是否为OpenScenario
  //!
  //! @param[in] path 一个包含路径的字符串
  //!
  //! @return 返回一个字符串，表示是否为OpenScenario
  virtual std::string CheckIsOpenScenario(const std::string& path) = 0;

  //! @brief 函数名：ParseScenario
  //! @details 函数功能：解析场景
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
  //!
  //! @return 返回一个布尔值，表示解析是否成功
  virtual bool ParseScenario(const std::string& path, tx_sim::impl::ModuleResetRequest& req) = 0;

  // only support .sim + .simrec format.
  /*
   * <simulation version="1.0">
   *     ...
   *     <traffic>xxx.simrec</traffic>
   *     <log2world ego_switch="true" traffic_switch="false" switch_time="23100" switch_type="SCENE"/>
   * </simulation>
   */
  // TODO(nemo): these 2 methods need RW-lock sync mechanism based on each scenario granularity.
  //! @brief 函数名：ParseLogsimEvents
  //! @details 函数功能：解析Logsim事件
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] event_info 一个包含LogsimEventInfo对象的引用
  virtual void ParseLogsimEvents(const std::string& path, LogsimEventInfo& event_info) = 0;

  //! @brief 函数名：ParseLog2WorldConfig
  //! @details 函数功能：解析Log2World配置
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] config 一个包含Log2WorldPlayConfig对象的引用
  virtual void ParseLog2WorldConfig(const std::string& path, Log2WorldPlayConfig& config) = 0;

  //! @brief 函数名：CacheLog2WorldConfig
  //! @details 函数功能：缓存Log2World配置
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] config 一个包含Log2WorldPlayConfig对象的引用
  virtual void CacheLog2WorldConfig(const std::string& path, Log2WorldPlayConfig& config) = 0;

  //! @brief 函数名：GetEgoType
  //! @details 函数功能：获取Ego类型
  //!
  //! @param[in] path 一个包含路径的字符串
  //!
  //! @return 返回一个字符串，表示Ego类型
  virtual std::string GetEgoType(const std::string& path) = 0;

  //! @brief 函数名：ExistSimRec
  //! @details 函数功能：检查是否存在模拟记录
  //!
  //! @param[in] path 一个包含路径的字符串
  //!
  //! @return 返回一个布尔值，表示是否存在模拟记录
  virtual bool ExistSimRec(const std::string& path) = 0;
};

class ScenarioXmlParser final : public IScenarioParser {
 public:
  //! @brief 函数名：CheckIsOpenScenario
  //! @details 函数功能：检查是否为OpenScenario
  //!
  //! @param[in] path 一个包含路径的字符串
  //!
  //! @return 返回一个字符串，表示是否为OpenScenario
  std::string CheckIsOpenScenario(const std::string& path) override;

  //! @brief 函数名：ParseScenario
  //! @details 函数功能：解析场景
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
  //!
  //! @return 返回一个布尔值，表示解析是否成功
  bool ParseScenario(const std::string& path, tx_sim::impl::ModuleResetRequest& req) override;

  //! @brief 函数名：ParseLogsimEvents
  //! @details 函数功能：解析Logsim事件
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] event_info 一个包含LogsimEventInfo对象的引用
  void ParseLogsimEvents(const std::string& path, LogsimEventInfo& event_info) override;

  //! @brief 函数名：ParseLog2WorldConfig
  //! @details 函数功能：解析Log2World配置
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] config 一个包含Log2WorldPlayConfig对象的引用
  void ParseLog2WorldConfig(const std::string& path, Log2WorldPlayConfig& config) override;

  //! @brief 函数名：CacheLog2WorldConfig
  //! @details 函数功能：缓存Log2World配置
  //!
  //! @param[in] path 一个包含路径的字符串
  //! @param[in] config 一个包含Log2WorldPlayConfig对象的引用
  void CacheLog2WorldConfig(const std::string& path, Log2WorldPlayConfig& config) override;

  //! @brief 函数名：GetEgoType
  //! @details 函数功能：获取Ego类型
  //!
  //! @param[in] path 一个包含路径的字符串
  //!
  //! @return 返回一个字符串，表示Ego类型
  std::string GetEgoType(const std::string& path) override;

  //! @brief 函数名：ExistSimRec
  //! @details 函数功能：检查是否存在模拟记录
  //!
  //! @param[in] path 一个包含路径的字符串
  //!
  //! @return 返回一个布尔值，表示是否存在模拟记录
  bool ExistSimRec(const std::string& path) override;

 private:
  bool ParseSimEgo(const std::string& scenario, const XMLElement* ego_node, tx_sim::impl::ModuleEgoInfo& egoinfo);

  //! @brief 函数名：ParseSimScenario
  //! @details 函数功能：解析模拟场景
  //!
  //! @param[in] scenarioPath 一个包含场景路径的字符串
  //! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
  //!
  //! @return 返回一个布尔值，表示解析是否成功
  bool ParseSimScenario(const std::string& scenarioPath, tx_sim::impl::ModuleResetRequest& req);

  // the parsing logic for Open Scenario format is not supported for further updates.
  // it is the responsibility of scenario editor to converting Open scenario file to .sim file.
  //! @brief 函数名：ParseOpenScenario
  //! @details 函数功能：解析OpenScenario
  //!
  //! @param[in] root 一个包含tinyxml2::XMLElement对象的指针
  //! @param[in] scenario_dir 一个包含boost::filesystem::path对象的引用
  //! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
  void ParseOpenScenario(const tinyxml2::XMLElement* root, boost::filesystem::path scenario_dir,
                         tx_sim::impl::ModuleResetRequest& req);
};

//! @brief 函数名：AssembleInitialLocation
//! @details 函数功能：组装初始位置
//!
//! @param[in] req 一个包含tx_sim::impl::ModuleResetRequest对象的引用
void AssembleInitialLocation(tx_sim::impl::ModuleResetRequest& req);

}  // namespace coordinator
}  // namespace tx_sim

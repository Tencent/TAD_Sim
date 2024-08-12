// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "enviroment_parser.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "xml_util.h"

using namespace boost::gregorian;
using namespace boost::posix_time;

namespace tx_sim {
namespace coordinator {

//! @brief 函数名：strToTime
//! @details 函数功能：字符串时间转时间戳
//! @param[in] datetime 字符串时间
//! @return 整形时间戳
int64_t strToTime(const char* datetime) {
  std::string sDateTime(datetime);
  ptime time_obj = from_iso_extended_string(sDateTime);
  time_duration duration = time_obj.time_of_day();
  int64_t t1 = duration.total_milliseconds();

  ptime epoch(boost::gregorian::date(1970, 1, 1));
  time_duration diff = time_obj - epoch;
  return diff.total_milliseconds();
}

//!
//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[in]
//!
int GetTimeStamp(const tinyxml2::XMLElement* pEnvNodeItem) {
  const tinyxml2::XMLElement* pParameterDeclarationsItemItem = pEnvNodeItem->FirstChildElement();
  while (pParameterDeclarationsItemItem) {
    const char* pAttrName = pParameterDeclarationsItemItem->Attribute("name");
    if (pAttrName) {
      std::string sAttrName(pAttrName);
      const char* pValue = pParameterDeclarationsItemItem->Attribute("value");
      if (!pValue) {
        pParameterDeclarationsItemItem = pParameterDeclarationsItemItem->NextSiblingElement();
        continue;
      }
      if ("TimeStamp" == sAttrName) { return std::stod(pValue); }
    }
    pParameterDeclarationsItemItem = pParameterDeclarationsItemItem->NextSiblingElement();
  }
}

//!
//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[in]
//!
void BuildParameterDeclarationsItemNode(const tinyxml2::XMLElement* pEnvNodeItem,
                                        sim_msg::EnvironmentalConditions& envConditions) {
  const tinyxml2::XMLElement* pParameterDeclarationsItemItem = pEnvNodeItem->FirstChildElement();
  while (pParameterDeclarationsItemItem) {
    const char* pAttrName = pParameterDeclarationsItemItem->Attribute("name");
    if (pAttrName) {
      std::string sAttrName(pAttrName);
      const char* pParameterType = pParameterDeclarationsItemItem->Attribute("parameterType");
      const char* pValue = pParameterDeclarationsItemItem->Attribute("value");
      if (pValue == nullptr || pParameterType == nullptr) {
        pParameterDeclarationsItemItem = pParameterDeclarationsItemItem->NextSiblingElement();
        continue;
      }
      if ("TimeStamp" == sAttrName) {
        // envConditions.set_unix_timestamp(std::stod(pValue));
      } else if ("UsingSunByUser" == sAttrName) {
      } else if ("wind_speed" == sAttrName) {
        envConditions.mutable_wind()->set_speed(std::stod(pValue));
      } else if ("tmperature" == sAttrName) {
        envConditions.set_temperature(std::stod(pValue));
      }
    }
    pParameterDeclarationsItemItem = pParameterDeclarationsItemItem->NextSiblingElement();
  }
}

//!
//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[in]
//!
void BuildWeatherItemNode(const tinyxml2::XMLElement* pEnvNodeItem, sim_msg::EnvironmentalConditions& envConditions) {
  const tinyxml2::XMLElement* pWeatherItem = pEnvNodeItem->FirstChildElement();
  while (pWeatherItem) {
    const char* pName = pWeatherItem->Name();
    std::string sName(pName);
    if ("Sun" == sName) {
      std::string azimuth = tx_sim::utils::GetNodeAttributeWithDefault(pWeatherItem, "azimuth", "0.0");
      std::string elevation = tx_sim::utils::GetNodeAttributeWithDefault(pWeatherItem, "elevation", "0.0");
      std::string intensity = tx_sim::utils::GetNodeAttributeWithDefault(pWeatherItem, "intensity", "0.0");
      envConditions.mutable_sun()->set_azimuth(std::stod(azimuth));
      envConditions.mutable_sun()->set_elevation(std::stod(elevation));
      envConditions.mutable_sun()->set_intensity(std::stod(intensity));
    } else if ("Fog" == sName) {
      std::string visualRange = tx_sim::utils::GetNodeAttributeWithDefault(pWeatherItem, "visualRange", "0.0");
      envConditions.mutable_fog()->set_visibility(std::stod(visualRange));
    } else if ("Precipitation" == sName) {
      std::string precipitationType = tx_sim::utils::GetNodeAttributeWithDefault(pWeatherItem, "precipitationType", "");
      std::string intensity = tx_sim::utils::GetNodeAttributeWithDefault(pWeatherItem, "intensity", "0.00");

      if ("rain" == precipitationType) {
        envConditions.mutable_precipitation()->set_type(sim_msg::EnvironmentalConditions_Precipitation_Type_RAIN);
      } else if ("dry" == precipitationType) {
        envConditions.mutable_precipitation()->set_type(sim_msg::EnvironmentalConditions_Precipitation_Type_DRY);
      } else if ("snow" == precipitationType) {
        envConditions.mutable_precipitation()->set_type(sim_msg::EnvironmentalConditions_Precipitation_Type_SNOW);
      } else {
        envConditions.mutable_precipitation()->set_type(sim_msg::EnvironmentalConditions_Precipitation_Type_OTHER);
      }
      envConditions.mutable_precipitation()->set_intensity(std::stod(intensity));
    }
    pWeatherItem = pWeatherItem->NextSiblingElement();
  }
}
//!
//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[in]
//!
void BuildEnvNode(const tinyxml2::XMLElement* pEnvironmentNode, sim_msg::EnvironmentalConditions& envConditions) {
  const tinyxml2::XMLElement* pEnvNodeItem = pEnvironmentNode->FirstChildElement();
  while (pEnvNodeItem) {
    const char* pName = pEnvNodeItem->Name();
    if (pName) {
      std::string sName(pName);
      if ("ParameterDeclarations" == sName) {
        BuildParameterDeclarationsItemNode(pEnvNodeItem, envConditions);
      } else if ("Weather" == sName) {
        std::string cloudState = tx_sim::utils::GetNodeAttributeWithDefault(pEnvNodeItem, "cloudState", "");
        if ("rainy" == cloudState) {
          envConditions.mutable_clouds()->set_fractional_cloud_cover(
              sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_SEVEN_OKTAS);
        } else if ("free" == cloudState) {
          envConditions.mutable_clouds()->set_fractional_cloud_cover(
              sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_ZERO_OKTAS);
        } else if ("cloudy" == cloudState) {
          envConditions.mutable_clouds()->set_fractional_cloud_cover(
              sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_TWO_OKTAS);
        } else if ("overcast" == cloudState) {
          envConditions.mutable_clouds()->set_fractional_cloud_cover(
              sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_FIVE_OKTAS);
        } else if ("skyOff" == cloudState) {
          envConditions.mutable_clouds()->set_fractional_cloud_cover(
              sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_OTHER);
        } else {
          envConditions.mutable_clouds()->set_fractional_cloud_cover(
              sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover_FRACTIONAL_CLOUD_COVER_UNKNOWN);
        }

        BuildWeatherItemNode(pEnvNodeItem, envConditions);
      }
    }
    pEnvNodeItem = pEnvNodeItem->NextSiblingElement();
  }
}

//!
//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[in]
//!
CEnviromentParser::CEnviromentParser(std::string envPath) : m_envPath(envPath) {}

//!
//! @brief 函数名：
//! @details 函数功能：
//!
//! @param[in]
//! @param[in]
//!
CEnviromentParser::~CEnviromentParser() {}

//! @brief 函数名：BuildEnviroment
//! @details 函数功能：解析环境配置文件，提取环境信息
//! @param[in] mapEnv 指向一个整数到EnvironmentalConditions的映射的指针，用于存储解析出的环境信息
//! @return 布尔值，表示解析是否成功
bool CEnviromentParser::BuildEnviroment(google::protobuf::Map<int64_t, sim_msg::EnvironmentalConditions>* mapEnv) {
  tinyxml2::XMLDocument m_doc;
  tinyxml2::XMLElement* pEnvXMLRoot = tx_sim::utils::GetXMLRoot(m_envPath, m_doc);
  if (!pEnvXMLRoot) return false;
  const tinyxml2::XMLElement* nodeCatalog = tx_sim::utils::GetChildNode(pEnvXMLRoot, "Catalog");
  const tinyxml2::XMLElement* environmentNode = nodeCatalog->FirstChildElement();
  while (environmentNode) {
    const char* pName = environmentNode->Name();
    if (std::string("Environment") != std::string(pName)) {
      environmentNode = environmentNode->NextSiblingElement();
      continue;
    }
    const tinyxml2::XMLElement* pDateNode = environmentNode->FirstChildElement("TimeOfDay");
    const char* pDataName = pDateNode->Name();
    const char* pDateTime = pDateNode->Attribute("dateTime");
    int64_t unixTime = strToTime(pDateTime);

    const tinyxml2::XMLElement* pParameterDeclarations = environmentNode->FirstChildElement("ParameterDeclarations");
    int64_t timeStamp = GetTimeStamp(pParameterDeclarations);
    sim_msg::EnvironmentalConditions& env = (*mapEnv)[timeStamp];
    env.set_unix_timestamp(unixTime);
    env.mutable_time_of_day()->set_seconds_since_midnight(unixTime / 1000 % 86400);

    BuildEnvNode(environmentNode, env);
    environmentNode = environmentNode->NextSiblingElement();
  }
  return true;
}

}  // namespace coordinator
}  // namespace tx_sim

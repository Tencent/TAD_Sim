/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scenario_set_data.h"
#include <json/value.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include "../database/db.h"
#include "../database/entity/scenario_set.h"
#include "../engine/config.h"
#include "../engine/util/scene_util.h"
#include "../xml_parser/entity/parser_json.h"
#include "../xml_parser/entity/scenario_set_params.h"
#include "../xml_parser/entity/scene_params.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/parser_tools.h"
// #include <regex>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/regex.hpp>
#include <exception>

CScenarioSetData& CScenarioSetData::Instance() {
  static CScenarioSetData instance;

  return instance;
}

const wchar_t* CScenarioSetData::GetScenarioSetList(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("create scenario set %s ", strParams.c_str());

  CParserJson jParser;
  sTagGetScenarioSetListParam param;
  int ret = jParser.ParseScenarioSetListParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  CScenarioSets sets;
  CDatabase::Instance().ScenarioSetList(param, sets);

  Json::Value data;
  ScenarioSets& sss = sets.Sets();
  if (sss.size() > 0) {
    for (ScenarioSets::iterator itr = sss.begin(); itr != sss.end(); ++itr) {
      Json::Value ss;
      ss["id"] = Json::Value::Int64(itr->m_nID);
      ss["name"] = itr->m_strName;
      ss["label"] = itr->m_strLabel;
      ss["scenes"] = itr->m_strScenes;
      data.append(ss);
    }
  } else {
    data.resize(0);
  }

  Json::Value root;
  root["data"] = data;
  root["code"] = 0;

  std::string strResult = root.toStyledString();

  m_wstrScenarioSetList.clear();
  m_wstrScenarioSetList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_wstrScenarioSetList.c_str();
}

const wchar_t* CScenarioSetData::CreateScenarioSet(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("create scenario set %s ", strParams.c_str());

  CParserJson jParser;
  sTagCreateScenarioSetParam param;
  int ret = jParser.ParseScenarioSetCreateParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  // int nRet = CDatabase::Instance().InsertScenarioSet(param);
  sTagEntityScenarioSet scenarioSet;
  scenarioSet.m_nID = param.m_nID;
  scenarioSet.m_strName = param.m_strName;
  scenarioSet.m_strLabel = param.m_strLabel;
  scenarioSet.m_strScenes = param.m_strScenes;
  int nRet = CDatabase::Instance().CreateOneScenarioSet(scenarioSet);
  if (nRet != 0) {
    return L"{'code':-1}";
  }

  return L"{'code': 0}";
}

const wchar_t* CScenarioSetData::UpdateScenarioSet(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("update scenario set %s ", strParams.c_str());

  CParserJson jParser;
  sTagUpdateScenarioSetParam param;
  int ret = jParser.ParseScenarioSetUpdateParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  // int nRet = CDatabase::Instance().UpdateScenarioSet(param);
  sTagEntityScenarioSet scenarioSet;
  scenarioSet.m_nID = param.m_nID;
  scenarioSet.m_strName = param.m_strName;
  scenarioSet.m_strLabel = param.m_strLabel;
  scenarioSet.m_strScenes = param.m_strScenes;
  int nRet = CDatabase::Instance().UpdateOneScenarioSet(scenarioSet);
  if (nRet != 0) {
    return L"{'code':-1}";
  }

  return L"{'code': 0}";
}

const wchar_t* CScenarioSetData::DeleteScenarioSet(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("delete scenario set %s ", strParams.c_str());

  CParserJson jParser;
  sTagDeleteScenarioSetParam param;
  int ret = jParser.ParseScenarioSetDeleteParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  // int nRet = CDatabase::Instance().DeleteScenarioSet(param);
  int nRet = CDatabase::Instance().DeleteOneScenarioSet(param.m_nID);
  if (nRet != 0) {
    return L"{'code':-1}";
  }

  return L"{'code': 0}";
}

/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scenario.h"
#include <json/value.h>
#include <json/writer.h>

void sTagEntityScenario::Reset() {
  m_nID = -1;
  m_strName = "";
  m_strMap = "";
  m_strPath = "";
  m_strInfo = "";
  m_strLabel = "";
  m_strSets = "";
  m_strContent = "";
  m_strDataSource = "";
}
Json::Value ToJson(sTagEntityScenario& scenario) {
  Json::Value root;
  root["id"] = (Json::Value::Int64)(scenario.m_nID);
  root["from"] = 2;
  root["editable"] = true;
  root["deletable"] = true;
  root["name"] = scenario.m_strName;
  root["filename"] = scenario.m_strName + "." + scenario.m_strType;
  root["type"] = scenario.m_strType;
  root["map"] = scenario.m_strMap;
  root["info"] = scenario.m_strInfo;
  root["set"] = scenario.m_strSets;
  root["path"] = scenario.m_strPath;
  root["traffictype"] = scenario.m_strTrafficType;
  root["dataSource"] = scenario.m_strLabel;
  scenario.m_strContent == "0" ? root["preset"] = scenario.m_strContent : root["preset"] = "";
  return root;
}

std::string sTagEntityScenario::ToJson() {
  Json::Value root;
  root = ::ToJson(*this);
  root["err"] = 0;
  root["message"] = "ok";
  return root.toStyledString();
}

std::string CScenarioEntities::ToJson() {
  Json::Value root;
  ScenarioEntities& entities = Entities();
  if (entities.size() > 0) {
    Json::Value maps;
    root["count"] = Json::Value::Int64(entities.size());
    int nCount = 0;

    if (entities.size() == 0) {
      root["list"].resize(0);

    } else {
      ScenarioEntities::iterator it = entities.begin();
      for (; it != entities.end(); ++it) {
        Json::Value d = ::ToJson((*it));
        maps.append(d);
      }
      root["list"] = maps;
    }
  } else {
    root["list"].resize(0);
    root["count"] = 0;
  }

  return root.toStyledString();
}

/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include "scene_data.h"

#include <exception>

#include <json/reader.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <sstream>
#include "../database/db.h"
#include "../database/entity/scenario.h"
#include "../database/entity/scenario_set.h"
#include "../database/synchronizer.h"
#include "../engine/config.h"
#include "../engine/error_code.h"
#include "../engine/util/scene_util.h"
#include "../server_map_cache/map_data_cache.h"
#include "../xml_parser/entity/batch_scene_param.h"
#include "../xml_parser/entity/generator.h"
#include "../xml_parser/entity/generator_v2.h"
#include "../xml_parser/entity/parser_json.h"
#include "../xml_parser/entity/scenario_set_params.h"
#include "../xml_parser/entity/scene_params.h"
#include "../xml_parser/entity/simulation.h"
#include "common/log/system_logger.h"
#include "common/utils/misc.h"
#include "common/xml_parser/entity/mapmodel.h"
#include "common/xml_parser/entity/parser_tools.h"
#include "common/xml_parser/xosc/xosc_converter.h"
#include "common/xml_parser/xosc/xosc_reader_1_0_v4.h"
#include "common/xml_parser/xosc/xosc_writer_1_0_v4.h"

#include "common/xml_parser/entity/SensorV2.h"
#include "common/xml_parser/entity/catalog.h"
namespace fs = boost::filesystem;

CSceneData& CSceneData::Instance() {
  static CSceneData instance;
  return instance;
}

const wchar_t* CSceneData::GetSceneData(const wchar_t* wstrSceneName) {
  if (!wstrSceneName) {
    return L"{\"err\":-1, \"message\": \"Scene name null\"}";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;

  SystemLogger::Info("load scene %s ", strSceneName.c_str());

  sTagEntityScenario scenario;
  scenario.m_strName = strSceneName;
  std::string strJson = jParser.XmlToJson(strResourceDir.c_str(), "scene", scenario);
  if (strJson.empty()) {
    return L"{\"err\":-1, \"message\": \"Scene is error\"}";
  }
  m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());

  return m_wstrSceneContent.c_str();
}

const wchar_t* CSceneData::GetTemplateData(const wchar_t* wstrTemplateName) {
  if (!wstrTemplateName) {
    return L"{\"err\":-1, \"message\": \"Template name null\"}";
  }

  //
  std::string strTemplateName = CEngineConfig::Instance().WStr2MBStr(wstrTemplateName);
  // std::string strImageDir = CEngineConfig::Instance().ImageFileDir();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;

  sTagEntityScenario scenario;
  scenario.m_strName = strTemplateName;
  SYSTEM_LOGGER_INFO("load template %s ", strTemplateName.c_str());

  std::string strJson = jParser.XmlToJson(strResourceDir.c_str(), "template", scenario);

  m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());

  return m_wstrSceneContent.c_str();
}

const wchar_t* CSceneData::SaveSceneData(const wchar_t* wstrSceneName, const wchar_t* wstrSceneContent) {
  if (!wstrSceneName) {
    return L"{\"err\":-1, \"message\": \"Scenen name null\"}";
  }

  if (!wstrSceneContent) {
    return L"{\"err\":-1, \"message\": \"Scenen content null\"}";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);

  std::string strSceneContent = CEngineConfig::Instance().WStr2MBStr(wstrSceneContent);

  // std::string strImageDir = CEngineConfig::Instance().ImageFileDir();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("start save scene file %s", strSceneName.c_str());
  // SYSTEM_LOGGER_DEBUG("save scene content %s", strSceneContent.c_str());

  try {
    std::string strSimPath = "";
    sTagSimuTraffic scene;
    int nRet = jParser.JsonToXml(strSceneContent.c_str(), scene);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Parse save scene data error!\"}";
    }

    nRet = jParser.SaveToFile(strResourceDir.c_str(), strSceneName.c_str(), scene, strSimPath);
    // int nRet = jParser.JsonToXml(strResourceDir.c_str(), strSceneName.c_str(), strSceneContent.c_str(), strSimPath);

    SYSTEM_LOGGER_INFO("scene file %s saved.", strSceneName.c_str());

    if (nRet == 0) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveSceneData exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Scene save error\"}";
}

const wchar_t* CSceneData::ParamScene(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  if (!wstrSceneName) {
    return LR"({"err":-1, "message": "Scene name null"})";
  }

  if (!wstrParam) {
    return LR"({"err":-1, "message": "Scene param null"})";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("parse scene param  %s", strParam.c_str());

  try {
    sTagBatchSceneParam param;

    int nRet = jParser.ParseSceneParam(strParam.c_str(), param);

    if (nRet != 0) {
      return LR"({"err":-1, "message": "Parse param error"})";
    }

    sTagEntityScenario scenario;
    scenario.m_nID = atoi(strSceneName.c_str());
    nRet = CDatabase::Instance().Scenario(scenario);
    if (nRet != 0) {
      return LR"({"err":-1, "message": "Get scene name error"})";
    }

    SYSTEM_LOGGER_INFO("batch generate scene file id: %s name: %s, param parsed.", strSceneName.c_str(),
                       scenario.m_strName.c_str());
    CGenerator gen;

    // nRet = gen.Generate(strSceneName.c_str(), param, strParam.c_str());
    param.m_strInfo = scenario.m_strInfo;
    std::string strGeneratedDirName = "";
    nRet = gen.Generate(scenario.m_strName.c_str(), param, strParam.c_str(), strGeneratedDirName);
    if (nRet == 0) {
      Json::Value root;
      root["err"] = 0;
      root["message"] = "ok";
      root["dir"] = strGeneratedDirName;
      std::string strResp = root.toStyledString();
      m_wstrParamSceneResp = CEngineConfig::Instance().MBStr2WStr(strResp.c_str());
    }
    if (param.m_export_flag == NOT_EXPORT_TO_TADSIM) {
      if (nRet == 0) {
        return m_wstrParamSceneResp.c_str();
      }

    } else if (param.m_export_flag == EXPORT_TO_TADSIM) {
      int64_t lScenarioCount = CDatabase::Instance().ScenarioCount();
      int64_t lScenarioSetCount = CDatabase::Instance().ScenarioSetCount();
      if (lScenarioCount > 2000 || lScenarioSetCount > 2000) {
        return LR"({"err":-1, "message": "Scenario count exceed max number!"})";
      }

      std::string strFailedFiles = "";
      std::string strExistFiles = "";
      nRet = ExportInBatchGeneratedScenes(strGeneratedDirName.c_str(), strExistFiles, strFailedFiles);
      if (strExistFiles.size() > 0 || strFailedFiles.size() > 0) {
        std::string strMessage = R"({"code": -1, )";
        strMessage.append(R"("message":"scene: ")" + strFailedFiles + R"(" Export failed!, scene: ")" + strExistFiles +
                          R"(" })");
        m_wstrParamSceneResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());

        return m_wstrParamSceneResp.c_str();
      }
      return m_wstrParamSceneResp.c_str();
    } else {
      SYSTEM_LOGGER_ERROR("batch generation scenario export flag error!");
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("param scene exception: %s", e.what());
  }

  return LR"({"err":-1, "message": "Parse param error"})";
}

const wchar_t* CSceneData::ParamSceneCount(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  if (!wstrSceneName) {
    return L"{\"err\":-1, \"message\": \"Scene name null\"}";
  }

  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"Scene param null\"}";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("parse scene param  %s", strParam.c_str());

  try {
    sTagBatchSceneParam param;

    int nRet = jParser.ParseSceneParam(strParam.c_str(), param);

    SYSTEM_LOGGER_INFO("batch generate scene file %s , param parsed.", strSceneName.c_str());

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Parse param error\"}";
    }

    CGenerator gen;

    uint64_t ullCount = 0, ullSize = 0, ullTime = 0;
    nRet = gen.GenerateCount(strSceneName.c_str(), param, ullCount, ullSize, ullTime);
    if (nRet == 0) {
      return GetParamSceneCount(ullCount, ullSize, ullTime);
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("param scene exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Parse param error\"}";
}

const wchar_t* CSceneData::ParamSceneV2(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  if (!wstrSceneName) {
    return LR"({"err":-1, "message": "Scene name null"})";
  }

  if (!wstrParam) {
    return LR"({"err":-1, "message": "Scene param null"})";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("parse scene param  %s", strParam.c_str());

  try {
    sTagBatchSceneParamV2 param;

    int nRet = jParser.ParseSceneParamV2(strParam.c_str(), param);

    if (nRet != 0) {
      return LR"({"err":-1, "message": "Parse param error"})";
    }
    // 1) check scenario is exist
    sTagEntityScenario scenario;
    scenario.m_nID = atoi(strSceneName.c_str());
    nRet = CDatabase::Instance().Scenario(scenario);
    if (nRet != 0) {
      return LR"({"err":-1, "message": "Get scene name error"})";
    }
    SYSTEM_LOGGER_INFO("batch generate scene file id: %s name: %s, param parsed.", strSceneName.c_str(),
                       scenario.m_strName.c_str());
    CGeneratorV2 gen;
    gen.setFilePathName(scenario.m_strPath);
    param.m_strInfo = scenario.m_strInfo;
    std::string strGeneratedDirName = "";
    nRet = gen.Generate(scenario.m_strName.c_str(), scenario.m_strType.c_str(), param, strParam.c_str(),
                        strGeneratedDirName);
    // 获取源目录
    boost::filesystem::path p = CEngineConfig::Instance().getSceneDataDir();
    // 批量场景的生成目录
    p.append("/batch_scene");
    p /= strGeneratedDirName;
    boost::filesystem::path srcPath = p;
    if (!boost::filesystem::exists(srcPath)) {
      SYSTEM_LOGGER_ERROR("src dir %s not exist", srcPath.string().c_str());
      return LR"({"err":-1, "message": "Make scene dir error;"})";
    }
    // 获取场景集名称
    std::string strScenarioSetName = srcPath.filename().string();
    // 创建用源目录名称创建场景集
    sTagEntityScenarioSet scenarioSet;
    scenarioSet.m_strName = strScenarioSetName;
    scenarioSet.m_strLabel = strScenarioSetName;
    scenarioSet.m_strScenes = "";
    CDatabase::Instance().CreateOneScenarioSet(scenarioSet);
    std::string strSetID = std::to_string(scenarioSet.m_nID);
    // 开始创建场景
    if (nRet == 0) {
      auto iter = gen.m_simTraffics.begin();
      int index = 0;
      std::string strFailedFiles = "";
      std::string strExistFiles = "";
      while (iter != gen.m_simTraffics.end()) {
        int nSaveRet = gen.BatchSaveOne(param, strGeneratedDirName, index);
        if (nSaveRet == -2) break;
        if (nSaveRet == 0 && param.m_export_flag == EXPORT_TO_TADSIM) {
          int64_t lScenarioCount = CDatabase::Instance().ScenarioCount();
          int64_t lScenarioSetCount = CDatabase::Instance().ScenarioSetCount();
          if (lScenarioCount > 2000 || lScenarioSetCount > 2000) {
            return LR"({"err":-1, "message": "Scenario count exceed max number!"})";
          }
          boost::filesystem::path scenepath = iter->m_path;
          int nRet = ExportInOneScene(iter->m_path, strSetID);
          if (nRet == HSEC_COPY_SCENE_FILE_EXIST) {
            strExistFiles += scenepath.filename().string();
            strExistFiles += " ";
          } else if (nRet == HSEC_COPY_SCENE_FAILED) {
            strFailedFiles += scenepath.filename().string();
            strFailedFiles += " ";
          }
        }
        iter++;
        index++;
      }
      if (strExistFiles.size() > 0 || strFailedFiles.size() > 0) {
        std::string strMessage = R"({"code": -1, )";
        strMessage.append(R"("message":"scene: ")" + strFailedFiles + R"(" Export failed!, scene: ")" + strExistFiles +
                          R"(" })");
        m_wstrParamSceneResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());
        return m_wstrParamSceneResp.c_str();
      }
      Json::Value root;
      root["err"] = 0;
      root["message"] = "ok";
      root["dir"] = strGeneratedDirName;
      std::string strResp = root.toStyledString();
      m_wstrParamSceneResp = CEngineConfig::Instance().MBStr2WStr(strResp.c_str());
      return m_wstrParamSceneResp.c_str();
    } else {
      SYSTEM_LOGGER_ERROR("batch generation scenario export flag error!");
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("param scene exception: %s", e.what());
  }
  return LR"({"err":-1, "message": "Parse param error"})";
}

const wchar_t* CSceneData::ParamSceneCountV2(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  if (!wstrSceneName) {
    return L"{\"err\":-1, \"message\": \"Scene name null\"}";
  }

  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"Scene param null\"}";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("parse scene param  %s", strParam.c_str());

  try {
    sTagBatchSceneParamV2 param;

    int nRet = jParser.ParseSceneParamV2(strParam.c_str(), param);

    SYSTEM_LOGGER_INFO("batch generate scene file %s , param parsed.", strSceneName.c_str());

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"1111Parse param error\"}";
    }

    CGeneratorV2 gen;

    boost::filesystem::path projectName = strSceneName;
    std::string strProjName = projectName.stem().string();

    sTagEntityScenario scenario;
    scenario.m_strName = strProjName;
    nRet = CDatabase::Instance().ScenarioByName(scenario);

    gen.setFilePathName(scenario.m_strPath);

    // SYSTEM_LOGGER_INFO("real path=%s,filename=%s",scenario.m_strPath.c_str(),scenario.m_strName.c_str());
    uint64_t ullCount = 0, ullSize = 0, ullTime = 0;
    nRet = gen.GenerateCount(strSceneName.c_str(), param, ullCount, ullSize, ullTime);
    SYSTEM_LOGGER_INFO("real path=%s,filename=%s,nRet=%d", scenario.m_strPath.c_str(), scenario.m_strName.c_str(),
                       nRet);
    if (nRet == 0) {
      return GetParamSceneCount(ullCount, ullSize, ullTime);
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("param scene exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Parse param error\"}";
}

const wchar_t* CSceneData::GetSceneDataByParam(const wchar_t* wstrParam) {
  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"Scenen param is null\"}";
  }
  // Get json data
  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;
  sTagEntityScenario scenario;

  try {
    int nRet = jParser.ParseScenarioGetParams(strParam.c_str(), scenario);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Scene param parse error!\"}";
    }
    if (scenario.m_nID < 0) {
      return L"{\"err\":-1, \"message\": \"Scenen param is null\"}";
    }

    if (CDatabase::Instance().ScenarioExist(scenario.m_nID)) {
      CDatabase::Instance().Scenario(scenario);
      boost::filesystem::path parentPath = scenario.m_strPath;
      parentPath = parentPath.parent_path().parent_path();
      strResourceDir = parentPath.string();
      SYSTEM_LOGGER_INFO("Load scene %s, path = %s", scenario.m_strName.c_str(), scenario.m_strPath.c_str());
      std::string strJson = jParser.XmlToJson(strResourceDir.c_str(), "scene", scenario);
      if (strJson.empty()) {
        SYSTEM_LOGGER_ERROR("Get scene data is empty");
        return L"{\"err\":-1, \"message\": \"Scene param parse error!\"}";
      }
      m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());
      return m_wstrSceneContent.c_str();
    }
  } catch (std::exception const& e) {
    SYSTEM_LOGGER_FATAL("GetSceneDataByParam exception: %s", e.what());
  }
  return L"{\"err\":-1, \"message\": \"Get scene by id error!\"}";
}

const wchar_t* CSceneData::SaveSceneDataByParam(const wchar_t* wstrParam) {
  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"Scene param is null\"}";
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  boost::filesystem::path parentPath = CEngineConfig::Instance().getSceneDataDir();
  std::string strResourceDir = parentPath.parent_path().parent_path().string();

  CParserJson jParser;

  try {
    sTagEntityScenario scenario;
    int nRet = jParser.ParseScenarioCreateParams(strParam.c_str(), scenario);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Scene param parse error!\"}";
    }

    sTagSimuTraffic scene;
    scene.m_defaultXosc = CEngineConfig::Instance().getCataLogDir();
    nRet = jParser.JsonToXml(scenario.m_strContent.c_str(), scene);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Parse save scene data error!\"}";
    }
    // sensor
    boost::filesystem::path sensorPath = CEngineConfig::Instance().getCataLogDir();
    CCataLog v2;
    v2.parseDefaultXosc(CEngineConfig::Instance().getDataCatalogsDir(), CEngineConfig::Instance().getSysCatalogsDir());
    for (auto it : v2.getDriveCatalogList()) {
      if (it.m_strName == scene.m_simulation.m_planner.m_strName) {
        scene.m_simulation.m_planner.m_sensorId = it.m_SensorGroup;
      }
    }
    scene.m_simulation.m_strInfo = scenario.m_strInfo;
    scenario.m_strType = scene.m_strType;
    scene.m_nID = scenario.m_nID;
    // string temp = jParser.json_to_xml(scene).c_str();
    nRet = jParser.SaveToFile(strResourceDir.c_str(), scenario.m_strName.c_str(), scene, scenario.m_strPath);

    if (nRet == Error_MapFormat) {
      return L"{\"err\":-1, \"message\": \"Save scene to file failed! Map formate is not surported\"}";
    }
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Save scene to file failed!\"}";
    }

    if (boost::iequals(scene.m_strType, "xosc")) {
      std::string strName = scenario.m_strName;
      strName.append("_convert");
      jParser.SaveToSim(strResourceDir.c_str(), strName.c_str(), scene, scenario.m_strPath);
    }

    SYSTEM_LOGGER_INFO("scene file %s saved.path=%s", scenario.m_strName.c_str(), scenario.m_strPath.c_str());
    scenario.m_strContent = "0";
    if (scenario.m_nID < 0) {
      bool bExist = CDatabase::Instance().ScenarioExist(scenario.m_strName.c_str(), scenario.m_strType.c_str());
      if (bExist) {
        SYSTEM_LOGGER_WARN("SaveSceneDataByParam. %s : no scenario id transmited but scenario has been saved!",
                           scenario.m_strName.c_str());
        sTagEntityScenario scenarioInDB = scenario;
        CDatabase::Instance().ScenarioByName(scenarioInDB);
        scenario.m_nID = scenarioInDB.m_nID;
        nRet = CDatabase::Instance().UpdateOneScenario(scenario);
      } else {
        nRet = CDatabase::Instance().CreateOneScenario(scenario);
      }

    } else {
      bool bExist = CDatabase::Instance().ScenarioExist(scenario.m_nID);
      if (bExist) {
        nRet = CDatabase::Instance().UpdateOneScenario(scenario);
      } else {
        nRet = CDatabase::Instance().CreateOneScenario(scenario);
      }
      // nRet = CDatabase::Instance().UpdateScenario(scenario);
    }
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Save scene record to db failed! Try to reopen the scene! \"}";
    }

    // no need to do check here.
    // refact it later.
    if (nRet == 0) {
      std::string strResp = scenario.ToJson();
      m_wstrSaveSceneResp = CEngineConfig::Instance().MBStr2WStr(strResp.c_str());
      // m_wstrSaveSceneResp = L"{\"err\":0, \"message\": \"ok\", \"sceneid\":" + std::to_wstring(scenario.m_nID) +
      // L"}";
      return m_wstrSaveSceneResp.c_str();
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveSceneDataByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Scene save error\"}";
}

const wchar_t* CSceneData::SaveSceneDataByParam(const char* wstrParam) {
  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"Scene param is null\"}";
  }

  std::string strParam = wstrParam;

  boost::filesystem::path parentPath = CEngineConfig::Instance().getSceneDataDir();
  std::string strResourceDir = parentPath.parent_path().parent_path().string();

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("resourcedir=%s,datadir=%s,start save scene file %s", strResourceDir.c_str(),
                     CEngineConfig::Instance().getSceneDataDir(), strParam.c_str());

  try {
    sTagEntityScenario scenario;
    int nRet = jParser.ParseScenarioCreateParams(strParam.c_str(), scenario);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Scene param parse error!\"}";
    }

    sTagSimuTraffic scene;
    scene.m_defaultXosc = CEngineConfig::Instance().getCataLogDir();
    nRet = jParser.JsonToXml(scenario.m_strContent.c_str(), scene);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Parse save scene data error!\"}";
    }

    scene.m_simulation.m_strInfo = scenario.m_strInfo;
    scenario.m_strType = scene.m_strType;

    scene.m_nID = scenario.m_nID;
    // string temp = jParser.json_to_xml(scene).c_str();

    nRet = jParser.SaveToFile(strResourceDir.c_str(), scenario.m_strName.c_str(), scene, scenario.m_strPath);

    if (nRet == Error_MapFormat) {
      return L"{\"err\":-1, \"message\": \"Save scene to file failed! Map formate is not surported\"}";
    }
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Save scene to file failed!\"}";
    }

    if (boost::iequals(scene.m_strType, "xosc")) {
      std::string strName = scenario.m_strName;
      strName.append("_convert");
      jParser.SaveToSim(strResourceDir.c_str(), strName.c_str(), scene, scenario.m_strPath);
    }

    SYSTEM_LOGGER_INFO("scene file %s saved.path=%s", scenario.m_strName.c_str(), scenario.m_strPath.c_str());
    scenario.m_strContent = "0";
    if (scenario.m_nID < 0) {
      bool bExist = CDatabase::Instance().ScenarioExist(scenario.m_strName.c_str(), scenario.m_strType.c_str());
      if (bExist) {
        SYSTEM_LOGGER_WARN("SaveSceneDataByParam. %s : no scenario id transmited but scenario has been saved!",
                           scenario.m_strName.c_str());
        sTagEntityScenario scenarioInDB = scenario;
        CDatabase::Instance().ScenarioByName(scenarioInDB);
        scenario.m_nID = scenarioInDB.m_nID;
        nRet = CDatabase::Instance().UpdateOneScenario(scenario);
      } else {
        nRet = CDatabase::Instance().CreateOneScenario(scenario);
      }

    } else {
      bool bExist = CDatabase::Instance().ScenarioExist(scenario.m_nID);
      if (bExist) {
        nRet = CDatabase::Instance().UpdateOneScenario(scenario);
      } else {
        nRet = CDatabase::Instance().CreateOneScenario(scenario);
      }
    }
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Save scene record to db failed! Try to reopen the scene! \"}";
    }

    // no need to do check here.
    // refact it later.
    if (nRet == 0) {
      std::string strResp = scenario.ToJson();
      m_wstrSaveSceneResp = CEngineConfig::Instance().MBStr2WStr(strResp.c_str());
      // m_wstrSaveSceneResp = L"{\"err\":0, \"message\": \"ok\", \"sceneid\":" + std::to_wstring(scenario.m_nID) +
      // L"}";
      return m_wstrSaveSceneResp.c_str();
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveSceneDataByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Scene save error\"}";
}

int CSceneData::DeleteSingleScene(const char* strSceneName, const char* strExt) {
  boost::filesystem::path scenePath = CEngineConfig::Instance().getSceneDataDir();

  std::string strFullName = CSceneUtil::SceneFullName(strSceneName, strExt, false);

  boost::filesystem::path simPath = CEngineConfig::Instance().getSceneDataDir() + strFullName;

  SYSTEM_LOGGER_INFO("DeleteSingleScene=%s", simPath.string().c_str());
  bool bRet = false;
  if (boost::filesystem::exists(simPath) && boost::filesystem::is_regular_file(simPath)) {
    if (boost::iequals(strExt, "xosc")) {
      bRet = boost::filesystem::remove(simPath);

    } else {
      CSimulation sim;
      int nRet = GetRelatedFiles(simPath.string().c_str(), sim);
      if (nRet) {
        SYSTEM_LOGGER_ERROR("get scene traffic file name error!");
        return nRet;
      }

      // delete traffic file
      boost::filesystem::path trafficPath = scenePath;
      std::string strTrafficFile = sim.m_strTraffic;
      trafficPath.append(strTrafficFile);

      if (strTrafficFile.length() > 0 && boost::filesystem::exists(trafficPath) &&
          boost::filesystem::is_regular_file(trafficPath)) {
        bRet = boost::filesystem::remove(trafficPath);

        if (!bRet) {
          SYSTEM_LOGGER_ERROR("remove traffic file %s error!", strTrafficFile.c_str());
          return -1;
        }
      }

      // delete sensor file
      if (sim.m_strSensor.size() > 0) {
        boost::filesystem::path sensorPath = scenePath;
        std::string strSensorFile = sim.m_strSensor;
        sensorPath.append(strSensorFile);

        if (strSensorFile.length() > 0 && boost::filesystem::exists(sensorPath) &&
            boost::filesystem::is_regular_file(sensorPath)) {
          bRet = boost::filesystem::remove(sensorPath);

          if (!bRet) {
            SYSTEM_LOGGER_ERROR("remove sensor file %s error!", strSensorFile.c_str());
            return -1;
          }
        }
      }

      // delete environment file
      if (sim.m_strEnvironment.size() > 0) {
        boost::filesystem::path environmentPath = scenePath;
        std::string strEnvironmentFile = sim.m_strEnvironment;
        environmentPath.append(strEnvironmentFile);

        if (strEnvironmentFile.length() > 0 && boost::filesystem::exists(environmentPath) &&
            boost::filesystem::is_regular_file(environmentPath)) {
          bRet = boost::filesystem::remove(environmentPath);

          if (!bRet) {
            SYSTEM_LOGGER_ERROR("remove environment file %s error!", strEnvironmentFile.c_str());
            return -1;
          }
        }
      }

      // delete grading file
      if (sim.m_strGrading.size() > 0) {
        boost::filesystem::path gradingPath = scenePath;
        std::string strGradingFile = sim.m_strGrading;
        gradingPath.append(strGradingFile);

        if (strGradingFile.length() > 0 && boost::filesystem::exists(gradingPath) &&
            boost::filesystem::is_regular_file(gradingPath)) {
          bRet = boost::filesystem::remove(gradingPath);

          if (!bRet) {
            SYSTEM_LOGGER_ERROR("remove grading file %s error!", strGradingFile.c_str());
            return -1;
          }
        }
      }
      // delete prj file
      boost::filesystem::path prjPath = scenePath;
      std::string strPrjFileName = simPath.stem().string() + ".prj";
      prjPath.append(strPrjFileName);
      if (strPrjFileName.length() > 5 && boost::filesystem::exists(prjPath) &&
          boost::filesystem::is_regular_file(prjPath)) {
        bRet = boost::filesystem::remove(prjPath);
        if (!bRet) {
          SYSTEM_LOGGER_ERROR("remove prj file error!");
        }
      }

      // delete sim file
      bRet = boost::filesystem::remove(simPath);
    }
  }

  if (bRet) {
    return 0;
  }

  return -1;
}

int CSceneData::CopySingleScene(const char* strSceneName, const char* strSrcDir, const char* strDestDir,
                                std::string& strDestPath, std::string strExportype, std::string strExt /*=""*/) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;
  strDestPath.clear();

  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single scene dest dir not exist");
    return HSEC_DestDirNotExist;
  }

  std::string TempName = strSceneName;
  boost::filesystem::path scenePath = srcPath;
  std::string strFullName = CSceneUtil::SceneFullName(strSceneName, strExt.c_str(), false);
  boost::filesystem::path simPath = CSceneUtil::ScenePath(strSrcDir, strFullName.c_str());

  if (boost::filesystem::exists(simPath) && boost::filesystem::is_regular_file(simPath)) {
    if (strExt == "xosc") {
      // copy xosc file
      boost::filesystem::path xoscDestPath = destPath;
      xoscDestPath /= strFullName;
      try {
        boost::filesystem::copy_file(simPath, xoscDestPath, BOOST_COPY_OPTION);
      } catch (std::exception e) {
        SYSTEM_LOGGER_ERROR("copy xosc file %s error!", strFullName.c_str());
        return HSEC_XOSCFileCopyError;
      }

      strDestPath = xoscDestPath.string();

      // if (0)
      // {
      // 解析 xosc文件   转换成sim 和 traffic.xml  文件
      sTagSimuTraffic st;
      XOSCReader_1_0_v4 reader;

      int nRet = reader.ParseXOSC(strDestPath.c_str(), st);
      if (nRet != 0) {
        return HSEC_Error;
      }

      strMapName = st.m_simulation.m_mapFile.m_strMapfile;
      // boost::filesystem::path simDestPath = destPath;
      // std::string  simTempName = TempName + ".sim";
      // simDestPath /= simTempName;
      // st.m_simulation.Save(simDestPath.string().c_str());

      // boost::filesystem::path trafficDestPath = destPath;
      // std::string  trafficTempName = TempName + "_traffic.xml";
      // trafficDestPath /= trafficTempName;
      // st.m_traffic.Save(trafficDestPath.string().c_str());

      //

    } else {
      if (strExportype == "sim") {
        CSimulation sim;
        int nRet = GetRelatedFiles(simPath.string().c_str(), sim);
        if (nRet) {
          SYSTEM_LOGGER_ERROR("get scene traffic file name error!");
          return HSEC_SimFileFormatError;
        }
        strMapName = sim.m_mapFile.m_strMapfile;
        // copy traffic file
        boost::filesystem::path trafficPath = scenePath;
        boost::filesystem::path trafficDestPath = destPath;
        std::string strTrafficFile = sim.m_strTraffic;
        trafficPath.append(strTrafficFile);
        trafficDestPath /= strTrafficFile;

        // traffic文件存在
        if (strTrafficFile.length() > 0 && boost::filesystem::exists(trafficPath) &&
            boost::filesystem::is_regular_file(trafficPath)) {
          // 1, 判断sensor文件是否存在
          // copy sensor file
          if (sim.m_strSensor.size() > 0) {
            boost::filesystem::path sensorPath = scenePath;
            boost::filesystem::path sensorDestPath = destPath;
            std::string strSensorFile = sim.m_strSensor;
            sensorPath.append(strSensorFile);
            sensorDestPath /= strSensorFile;

            // 1.1，sensor文件存在则进行拷贝
            if (strSensorFile.length() > 0 && boost::filesystem::exists(sensorPath) &&
                boost::filesystem::is_regular_file(sensorPath)) {
              try {
                boost::filesystem::copy_file(sensorPath, sensorDestPath, BOOST_COPY_OPTION);
              } catch (std::exception e) {
                SYSTEM_LOGGER_ERROR("copy sensor file %s error!", strSensorFile.c_str());
                return HSEC_SensorFileCopyError;
              }
            } else {
              // 1.2，不存在则记录并返回
              SYSTEM_LOGGER_ERROR("copy sensor file %s error! scene file incomplete", strSensorFile.c_str());
              return HSEC_SensorFileNotExist;
            }
          }

          // 2, 判断environment文件是否存在
          // copy environment file
          if (sim.m_strEnvironment.size() > 0) {
            boost::filesystem::path environmentPath = scenePath;
            boost::filesystem::path environmentDestPath = destPath;
            std::string strEnvironmentFile = sim.m_strEnvironment;
            environmentPath.append(strEnvironmentFile);
            environmentDestPath /= strEnvironmentFile;

            // 2.1，environment文件存在则进行拷贝
            if (strEnvironmentFile.length() > 0 && boost::filesystem::exists(environmentPath) &&
                boost::filesystem::is_regular_file(environmentPath)) {
              try {
                boost::filesystem::copy_file(environmentPath, environmentDestPath, BOOST_COPY_OPTION);
              } catch (std::exception e) {
                SYSTEM_LOGGER_ERROR("copy environment file %s error!", strEnvironmentFile.c_str());
                return HSEC_EnvironmentFileCopyError;
              }
            } else {
              // 2.2，不存在则记录并返回
              SYSTEM_LOGGER_ERROR("copy environment file %s error! scene file incomplete", strEnvironmentFile.c_str());
              return HSEC_EnvironmentFileNotExist;
            }
          }

          // 3, 判断grading文件是否存在
          // copy grading file
          if (sim.m_strGrading.size() > 0) {
            boost::filesystem::path gradingPath = scenePath;
            boost::filesystem::path gradingDestPath = destPath;
            std::string strGradingFile = sim.m_strGrading;
            gradingPath.append(strGradingFile);
            gradingDestPath /= strGradingFile;

            // 3.1，grading文件存在则进行拷贝
            if (strGradingFile.length() > 0 && boost::filesystem::exists(gradingPath) &&
                boost::filesystem::is_regular_file(gradingPath)) {
              try {
                boost::filesystem::copy_file(gradingPath, gradingDestPath, BOOST_COPY_OPTION);
              } catch (std::exception e) {
                SYSTEM_LOGGER_ERROR("copy grading file %s error!", strGradingFile.c_str());
                return HSEC_GradingFileCopyError;
              }
            } else {
              // 3.2，不存在则记录并返回
              SYSTEM_LOGGER_ERROR("copy grading file %s error! scene file incomplete", strGradingFile.c_str());
              return HSEC_GradingFileNotExist;
            }
          }
          // 4.1 traffic文件拷贝
          try {
            boost::filesystem::copy_file(trafficPath, trafficDestPath, BOOST_COPY_OPTION);
          } catch (boost::filesystem::filesystem_error& e) {
            SYSTEM_LOGGER_ERROR("copy traffic file %s error: %s", strTrafficFile.c_str(), e.what());
            return HSEC_TrafficFileCopyError;
          }
        } else {
          // 4.2 traffic文件不存在则报错
          SYSTEM_LOGGER_ERROR("traffic file %s not exist! scene file incomplete", strTrafficFile.c_str());
          return HSEC_TrafficFileNotExist;
        }

        // 5 copy sim file
        boost::filesystem::path simDestPath = destPath;
        simDestPath /= strFullName;
        try {
          boost::filesystem::copy_file(simPath, simDestPath, BOOST_COPY_OPTION);
        } catch (std::exception e) {
          SYSTEM_LOGGER_ERROR("copy sim file %s error!", strFullName.c_str());
          return HSEC_SimFileCopyError;
        }

        strDestPath = simDestPath.string();
      } else if (strExportype == "xosc") {
        sTagSimuTraffic st1;
        CParser parse;
        boost::filesystem::path prjPath = scenePath;
        int nRet = parse.Parse(st1, simPath.string().c_str(), prjPath.string().c_str(), false);
        SYSTEM_LOGGER_DEBUG("**********************%s", st1.m_simulation.m_planner.m_strName);
        if (nRet != 0) {
          return HSEC_Error;
        }
        CSimulation& sim1 = st1.m_simulation;
        CTraffic& traffic1 = st1.m_traffic;
        CEnvironment& environment1 = st1.m_environment;
        XOSCWriter_1_0_v4 writer4;
        SYSTEM_LOGGER_DEBUG("********************** v%s", sim1.m_planner.m_strName);

        boost::filesystem::path xoscDestPath = destPath;
        std::string xoscTempName = TempName + ".xosc";
        xoscDestPath /= xoscTempName;
        nRet = writer4.Save2XOSC(xoscDestPath.string().c_str(), st1);
        if (nRet != 0) {
          if (nRet == Error_MapFormat) return HSEC_MapFormatError;
          if (nRet == Error_MapLost) return HSEC_MapLostError;
          return HSEC_Error;
        }
      }
    }
  }
  return HSEC_OK;
}

int CSceneData::CopySingleScene(const char* strSceneName, const char* strSrcDir, const char* strDestDir,
                                std::string& strDestPath, std::string strFlag, std::string strExt, std::string xoscType,
                                std::string& newname) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;
  strDestPath.clear();

  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single scene dest dir not exist");
    return HSEC_DestDirNotExist;
  }

  std::string TempName = strSceneName;
  boost::filesystem::path scenePath = srcPath;
  std::string strFullName = CSceneUtil::SceneFullName(strSceneName, strExt.c_str(), false);
  boost::filesystem::path simPath = CSceneUtil::ScenePath(strSrcDir, strFullName.c_str());

  if (boost::filesystem::exists(simPath) && boost::filesystem::is_regular_file(simPath)) {
    if (strExt == "xosc") {
      // copy xosc file
      boost::filesystem::path xoscDestPath = destPath;
      std::string strstemsrc = simPath.stem().string();

      xoscDestPath /= strFullName;
      std::string strstem;
      newname = strstemsrc;
      if (strFlag == "false") {
        // strFullName = strstem + "(1)." + strExt;
        // newname = strstem + "(1)";
        int n = 1;
        std::string strindex = "(" + std::to_string(n) + ")";
        strstem = strstemsrc + strindex;

        // strstem = strstem + "(1)";
        strFullName = strstem + "." + strExt;
        xoscDestPath = destPath;
        xoscDestPath /= strFullName;
        while (boost::filesystem::exists(xoscDestPath) && boost::filesystem::is_regular_file(xoscDestPath)) {
          n++;
          strindex = "(" + std::to_string(n) + ")";
          strstem = strstemsrc + strindex;
          strFullName = strstem + "." + strExt;
          xoscDestPath = destPath;
          xoscDestPath /= strFullName;
        }

        newname = strstem;
      }
      sTagSimuTraffic st;
      XOSCReader_1_0_v4 reader;
      int nRet = 0;
      st.m_oscXsdPath = CEngineConfig::Instance().getOscXsdPath();
      if (xoscType != "LOG_SIM") {
        nRet = reader.ParseXOSC(simPath.string().c_str(), st);
        if (nRet != 0) {
          return HSEC_Error;
        }
      }
      try {
        reader.CopyXOSC(simPath.string().c_str(), xoscDestPath.string().c_str());
      } catch (std::exception e) {
        std::string str = e.what();
        SYSTEM_LOGGER_ERROR("copy xosc file %s error!", strFullName.c_str());
        return HSEC_XOSCFileCopyError;
      }
      strDestPath = xoscDestPath.string();
      // if (0)  //   暂不加
      // {
      // 解析 xosc文件   转换成sim 和 traffic.xml  文件

      strMapName = st.m_simulation.m_mapFile.m_strMapfile;
    } else if (strExt == "sim") {
      CSimulation sim;
      int nRet = GetRelatedFiles(simPath.string().c_str(), sim);
      if (nRet) {
        SYSTEM_LOGGER_ERROR("get scene traffic file name error = %d!", nRet);
        return HSEC_SimFileFormatError;
      }
      strMapName = sim.m_mapFile.m_strMapfile;

      boost::filesystem::path simDestPath = destPath;
      simDestPath /= strFullName;

      std::string strstemsrc = simPath.stem().string();
      std::string strstem;
      newname = strstemsrc;
      if (strFlag == "false") {
        int n = 1;
        std::string strindex = "(" + std::to_string(n) + ")";
        strstem = strstemsrc + strindex;
        strFullName = strstem + "." + strExt;
        simDestPath = destPath;
        simDestPath /= strFullName;
        while (boost::filesystem::exists(simDestPath) && boost::filesystem::is_regular_file(simDestPath)) {
          n++;
          strindex = "(" + std::to_string(n) + ")";
          strstem = strstemsrc + strindex;
          strFullName = strstem + "." + strExt;
          simDestPath = destPath;
          simDestPath /= strFullName;
        }

        newname = strstem;
      }

      // copy traffic file
      boost::filesystem::path trafficPath = scenePath;
      boost::filesystem::path trafficDestPath = destPath;
      std::string strTrafficFile = sim.m_strTraffic;

      trafficPath.append(strTrafficFile);

      if (strFlag == "false") {
        strTrafficFile = strstem + "_traffic.xml";
      }
      trafficDestPath /= strTrafficFile;

      // traffic文件存在
      if (strTrafficFile.length() > 0 && boost::filesystem::exists(trafficPath) &&
          boost::filesystem::is_regular_file(trafficPath)) {
        // 1, 判断sensor文件是否存在
        // copy sensor file
        if (sim.m_strSensor.size() > 0) {
          boost::filesystem::path sensorPath = scenePath;
          boost::filesystem::path sensorDestPath = destPath;
          std::string strSensorFile = sim.m_strSensor;
          sensorPath.append(strSensorFile);
          sensorDestPath /= strSensorFile;

          // 1.1，sensor文件存在则进行拷贝
          if (strSensorFile.length() > 0 && boost::filesystem::exists(sensorPath) &&
              boost::filesystem::is_regular_file(sensorPath)) {
            try {
              boost::filesystem::copy_file(sensorPath, sensorDestPath, BOOST_COPY_OPTION);
            } catch (std::exception e) {
              SYSTEM_LOGGER_ERROR("copy sensor file %s error!", strSensorFile.c_str());
              return HSEC_SensorFileCopyError;
            }
          } else {
            // 1.2，不存在则记录并返回
            SYSTEM_LOGGER_ERROR("copy sensor file %s error! scene file incomplete", strSensorFile.c_str());
            return HSEC_SensorFileNotExist;
          }
        }

        // 2, 判断environment文件是否存在
        // copy environment file
        if (sim.m_strEnvironment.size() > 0) {
          boost::filesystem::path environmentPath = scenePath;
          boost::filesystem::path environmentDestPath = destPath;
          std::string strEnvironmentFile = sim.m_strEnvironment;
          environmentPath.append(strEnvironmentFile);
          environmentDestPath /= strEnvironmentFile;

          // 2.1，environment文件存在则进行拷贝
          if (strEnvironmentFile.length() > 0 && boost::filesystem::exists(environmentPath) &&
              boost::filesystem::is_regular_file(environmentPath)) {
            try {
              boost::filesystem::copy_file(environmentPath, environmentDestPath, BOOST_COPY_OPTION);
            } catch (std::exception e) {
              SYSTEM_LOGGER_ERROR("copy environment file %s error!", strEnvironmentFile.c_str());
              return HSEC_EnvironmentFileCopyError;
            }
          } else {
            // 2.2，不存在则记录并返回
            SYSTEM_LOGGER_ERROR("copy environment file %s error! scene file incomplete", strEnvironmentFile.c_str());
            return HSEC_EnvironmentFileNotExist;
          }
        }

        // 3, 判断grading文件是否存在
        // copy grading file
        if (sim.m_strGrading.size() > 0) {
          boost::filesystem::path gradingPath = scenePath;
          boost::filesystem::path gradingDestPath = destPath;
          std::string strGradingFile = sim.m_strGrading;
          gradingPath.append(strGradingFile);
          gradingDestPath /= strGradingFile;

          // 3.1，grading文件存在则进行拷贝
          if (strGradingFile.length() > 0 && boost::filesystem::exists(gradingPath) &&
              boost::filesystem::is_regular_file(gradingPath)) {
            try {
              boost::filesystem::copy_file(gradingPath, gradingDestPath, BOOST_COPY_OPTION);
            } catch (std::exception e) {
              SYSTEM_LOGGER_ERROR("copy grading file %s error!", strGradingFile.c_str());
              return HSEC_GradingFileCopyError;
            }
          } else {
            // 3.2，不存在则记录并返回
            SYSTEM_LOGGER_ERROR("copy grading file %s error! scene file incomplete", strGradingFile.c_str());
            return HSEC_GradingFileNotExist;
          }
        }
        // 4.1 traffic文件拷贝
        try {
          boost::filesystem::copy_file(trafficPath, trafficDestPath, BOOST_COPY_OPTION);
        } catch (boost::filesystem::filesystem_error& e) {
          SYSTEM_LOGGER_ERROR("copy traffic file %s error: %s", strTrafficFile.c_str(), e.what());
          return HSEC_TrafficFileCopyError;
        }
      } else {
        // 4.2 traffic文件不存在则报错
        SYSTEM_LOGGER_ERROR("traffic file %s not exist! scene file incomplete", strTrafficFile.c_str());
        return HSEC_TrafficFileNotExist;
      }

      // 5 copy sim file

      try {
        boost::filesystem::copy_file(simPath, simDestPath, BOOST_COPY_OPTION);

        if (strFlag == "false") {
          // 修改sim文件里面的traffic
          SYSTEM_LOGGER_INFO(" rename scene sim_traffic");
          sTagSimuTraffic scene;
          int ret = sim.ChangeTrafficName(simDestPath.string().c_str(), strTrafficFile.c_str());
        }
      } catch (std::exception e) {
        SYSTEM_LOGGER_ERROR("copy sim file %s error!", strFullName.c_str());
        return HSEC_SimFileCopyError;
      }

      strDestPath = simDestPath.string();
    }
  }

  return HSEC_OK;
}

const wchar_t* CSceneData::GetFileList(std::vector<std::string> relativePathes, std::string strInExt, int offset,
                                       int limit) {
  // boost::filesystem::path p = CEngineConfig::Instance().ImageFileDir();
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  std::vector<std::string>::iterator itr = relativePathes.begin();
  std::vector<boost::filesystem::path> files;

  // 获取文件
  int nOrder = 0;
  int nStart = offset;
  int nEnd = offset + limit;
  for (; itr != relativePathes.end(); ++itr) {
    boost::filesystem::path scenePath = p;
    scenePath.append((*itr));
    // 路径存在
    if (boost::filesystem::exists(scenePath)) {
      boost::filesystem::directory_iterator itr_end;
      boost::filesystem::directory_iterator itr(scenePath);

      for (; itr != itr_end; ++itr) {
        std::string strName = (*itr).path().string();
        std::string strFileName = (*itr).path().stem().string();  // 不含扩展名的全路径
        std::string strExt = (*itr).path().extension().string();  // 返回扩展名
        // if ((!boost::filesystem::is_directory(*itr)) && strExt == strInExt)
        if ((!boost::filesystem::is_directory(*itr)) &&  // 是否是目录
            CSceneUtil::ValidFileFormat(strExt.c_str(), strInExt.c_str())) {
          if (nOrder >= nStart && nOrder < nEnd) {
            files.push_back((*itr).path());
          }
          nOrder++;
        }
      }
    }
  }

  // 排序
  std::sort(files.begin(), files.end(), [](boost::filesystem::path& a, boost::filesystem::path& b) {
    std::string aName = a.stem().string();
    std::string bName = b.stem().string();
    boost::to_lower(aName);
    boost::to_lower(bName);
    if (aName < bName) {
      return true;
    }

    return false;
  });

  Json::Value root;
  if (files.size() > 0) {
    std::vector<boost::filesystem::path>::iterator it = files.begin();

    Json::Value maps;
    root["count"] = Json::Value::UInt64(nOrder);
    int nCount = 0;

    if (files.size() == 0) {
      root["list"].resize(0);

    } else {
      for (; it != files.end(); ++it) {
        Json::Value d;
        // d["id"] = nCount++;
        d["id"] = (*it).stem().string();
        d["from"] = 2;
        d["editable"] = true;
        d["deletable"] = true;
        d["name"] = (*it).stem().string();
        d["filename"] = (*it).filename().string();
        d["path"] = (*it).string();
        maps.append(d);
      }
      root["list"] = maps;
    }
  } else {
    root["list"].resize(0);
    root["count"] = 0;
  }

  std::string strResult = root.toStyledString();

  m_wstrSceneList.clear();
  m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_wstrSceneList.c_str();
}

const wchar_t* CSceneData::GetGISImageList(int offset, int limit) {
  std::vector<std::string> paths;
  paths.push_back("gis_images");
  // paths.push_back("batch_scene");
  return GetFileList(paths, ".jpeg,.png,.jpg,.tiff", offset, limit);
}

const wchar_t* CSceneData::GetGISModelList(int offset, int limit) {
  std::vector<std::string> paths;
  paths.push_back("gis_models");
  // paths.push_back("batch_scene");
  return GetFileList(paths, ".fbx", offset, limit);
}

const wchar_t* CSceneData::SyncScenarioDBFromDisk() {
  CSynchronizer::Instance().SyncDiskToDB2();

  return LR"({"code": 0 })";
}

const wchar_t* CSceneData::GetSceneList(int offset, int limit) {
  std::vector<std::string> paths;
  paths.push_back("scene");
  // paths.push_back("batch_scene");
  return GetFileList(paths, ".sim", offset, limit);
}

const wchar_t* CSceneData::GetSceneListByParam(const wchar_t* wstrParams) {
  if (!wstrParams) {
    return L"{\"err\":-1, \"message\": \"Scene list param is null\"}";
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParams);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("start get scene list %s", strParam.c_str());

  try {
    sTagGetScenarioListParam param;
    int nRet = jParser.ParseScenarioListParams(strParam.c_str(), param);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Scene param parse error!\"}";
    }

    CScenarioEntities ses;

    nRet = CDatabase::Instance().ScenarioList(param, ses);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Get scenario list error!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetSceneListByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Get scenario list error\"}";
}

const wchar_t* CSceneData::GetSceneInfo(const wchar_t* strSceneName) {
  // boost::filesystem::path p = CEngineConfig::Instance().ImageFileDir();
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path scenePath = p;
  scenePath.append("scene");

  std::string strFileName = CEngineConfig::Instance().WStr2MBStr(strSceneName);
  strFileName.append(".sim");
  scenePath.append(strFileName);

  Json::Value root;
  // hadmap 路径存在
  if (boost::filesystem::exists(scenePath)) {
    root["from"] = 2;
    root["editable"] = true;
    root["deletable"] = true;
    root["name"] = scenePath.stem().string();
    root["path"] = scenePath.string();
  } else {
  }

  std::string strResult = root.toStyledString();

  m_wstrSceneInfo.clear();
  m_wstrSceneInfo = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_wstrSceneInfo.c_str();
}

const wchar_t* CSceneData::GetSceneInfoByParam(const wchar_t* wstrID) {
  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrID);

  sTagGetScenarioInfoListParam param;
  param.m_ids.push_back(strParam);
  CScenarioEntities ses;

  try {
    int nRet = CDatabase::Instance().ScenarioInfoList(param, ses);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Scenario info list error!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetScenesInfoByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Get scenario list error\"}";
}

int CSceneData::GetRelatedFiles(const char* strSimFile, CSimulation& sim) { return sim.Parse(strSimFile, false); }

bool CSceneData::Wstr2Json(const wchar_t* wstr, Json::Value& json) {
  // wchar_t * to string
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstr);
  // from string to json
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  int nLen = strlen(strParams.c_str());
  if (!reader->parse(strParams.c_str(), strParams.c_str() + nLen, &json, &errs)) {
    SYSTEM_LOGGER_ERROR("From string to json error, string is : \n%s", strParams.c_str());
    return false;
  }
  return true;
}

std::string CSceneData::GetTrafficFile(const char* strSimFile) {
  CSimulation sim;
  int nRet = sim.Parse(strSimFile, false);
  if (nRet == -1) {
    return "";
  }

  return sim.m_strTraffic;
}

const wchar_t* CSceneData::DeleteScene(const wchar_t* strSceneName) {
  std::string strFileName = CEngineConfig::Instance().WStr2MBStr(strSceneName);
  int nRet = DeleteSingleScene(strFileName.c_str(), "sim");
  if (nRet == 0) {
    return L"true";
  }
  return L"false";
}

const wchar_t* CSceneData::SearchFile(std::string relativePath, std::string strInExt, std::string strSearchName,
                                      int nOffset, int nLimit) {
  // boost::filesystem::path p = CEngineConfig::Instance().ImageFileDir();
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path scenePath = p;
  scenePath.append(relativePath);
  std::vector<boost::filesystem::path> files;

  Json::Value root;
  // 路径存在
  if (boost::filesystem::exists(scenePath)) {
    boost::filesystem::directory_iterator itr_end;
    boost::filesystem::directory_iterator itr(scenePath);

    // std::string strPatten = "[a-zA-Z0-9-_]*?(";
    std::string strPatten = "\\w*?(";
    strPatten.append(strSearchName);
    strPatten.append(")+\\w*?");
    // std::regex rPatten(strPatten);
    boost::regex rPatten(strPatten);

    int nOrder = 0;
    int nStart = nOffset;
    int nEnd = nOffset + nLimit;

    for (; itr != itr_end; ++itr) {
      std::string strName = (*itr).path().string();
      std::string strFileName = (*itr).path().stem().string();
      std::string strExt = (*itr).path().extension().string();
      if ((!boost::filesystem::is_directory(*itr)) && strExt == strInExt &&
          // std::regex_match((*itr).path().stem().string(), rPatten))
          boost::regex_match((*itr).path().stem().string(), rPatten)) {
        // 临时方案，过滤黑名单，后续删掉
        if (boost::iequals(strFileName, "byl0601")) {
          continue;
        }

        if (nOrder >= nStart && nOrder < nEnd) {
          files.push_back((*itr).path());
        }
        nOrder++;
      }
    }

    std::vector<boost::filesystem::path>::iterator it = files.begin();

    Json::Value maps;
    root["count"] = Json::Value::UInt64(nOrder);
    int nCount = 0;

    if (files.size() == 0) {
      root["list"].resize(0);
    } else {
      for (; it != files.end(); ++it) {
        Json::Value d;
        // d["id"] = nCount++;
        d["id"] = (*it).stem().string();
        d["from"] = 2;
        d["editable"] = true;
        d["deletable"] = true;
        d["name"] = (*it).stem().string();
        d["path"] = (*it).string();
        maps.append(d);
      }
      root["list"] = maps;
    }
  } else {
    root["list"].resize(0);
    root["count"] = 0;
  }

  std::string strResult = root.toStyledString();

  m_wstrSceneList.clear();
  m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_wstrSceneList.c_str();
}

const wchar_t* CSceneData::SearchScene(const wchar_t* strSceneName, int nOffset, int nLimit) {
  std::string strSearchName = CEngineConfig::Instance().WStr2MBStr(strSceneName);
  return SearchFile("scene", ".sim", strSearchName, nOffset, nLimit);
}

const wchar_t* CSceneData::SearchSceneByParam(const wchar_t* wstrParam) {
  Json::Value failed;
  failed["list"].resize(0);
  failed["count"] = 0;
  m_wstrSceneList.clear();
  std::string strResult = failed.toStyledString();
  m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  if (!wstrParam) {
    return m_wstrSceneList.c_str();
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  sTagSearchScenarioParam param;
  CParserJson parser;
  try {
    int nRet = parser.ParseScenarioSearchParams(strParam.c_str(), param);
    if (nRet != 0 || param.m_strType.size() < 1) {
      return m_wstrSceneList.c_str();
    }

    CScenarioEntities ses;
    CDatabase::Instance().ScenarioSearch(param, ses);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Scenario search failed!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetScenesInfoByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Get scenario info list error\"}";
}
const wchar_t* CSceneData::GetScenesInfo(const wchar_t* strSceneName) {
  Json::Value failed;
  failed["list"].resize(0);
  failed["count"] = 0;
  m_wstrSceneList.clear();
  std::string strResult = failed.toStyledString();
  m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  if (!strSceneName) {
    return m_wstrSceneList.c_str();
  }

  // boost::filesystem::path p = CEngineConfig::Instance().ImageFileDir();
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path scenePath = p;
  scenePath.append("scene");

  std::string strScenes = CEngineConfig::Instance().WStr2MBStr(strSceneName);

  std::vector<std::string> SplitVec;
  boost::algorithm::split(SplitVec, strScenes, boost::algorithm::is_any_of(","), boost::algorithm::token_compress_on);

  if (SplitVec.size() == 0) {
    return m_wstrSceneList.c_str();
  }

  std::map<std::string, boost::filesystem::path> files;

  // hadmap 路径存在
  if (boost::filesystem::exists(scenePath)) {
    boost::filesystem::directory_iterator itr_end;
    boost::filesystem::directory_iterator itr(scenePath);

    for (; itr != itr_end; ++itr) {
      std::string strName = (*itr).path().string();
      std::string strExt = (*itr).path().extension().string();
      if ((!boost::filesystem::is_directory(*itr)) && strExt == ".sim") {
        std::vector<std::string>::iterator vitr = SplitVec.begin();
        for (; vitr != SplitVec.end(); ++vitr) {
          if (itr->path().stem().string() == (*vitr)) {
            files.insert(std::make_pair((*itr).path().stem().string(), (*itr).path()));
            break;
          }
        }
      }
    }

    Json::Value root;
    Json::Value maps;
    root["count"] = Json::Value::UInt64(SplitVec.size());
    int nCount = 0;
    std::vector<std::string>::iterator vitr = SplitVec.begin();
    for (; vitr != SplitVec.end(); ++vitr) {
      std::map<std::string, boost::filesystem::path>::iterator mitr = files.find((*vitr));

      if (mitr != files.end()) {
        Json::Value d;
        // d["id"] = nCount++;
        d["id"] = (*mitr).second.stem().string();
        d["from"] = 2;
        d["editable"] = true;
        d["deletable"] = true;
        d["name"] = (*mitr).second.stem().string();
        d["path"] = (*mitr).second.string();
        maps.append(d);
      } else {
        Json::Value d;
        maps.append(d);
      }
    }
    root["list"] = maps;
    std::string strResult = root.toStyledString();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  }

  return m_wstrSceneList.c_str();
}

const wchar_t* CSceneData::GetScenesInfoByParam(const wchar_t* wstrParam) {
  Json::Value failed;
  failed["list"].resize(0);
  failed["count"] = 0;
  m_wstrSceneList.clear();
  std::string strResult = failed.toStyledString();
  m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  if (!wstrParam) {
    return m_wstrSceneList.c_str();
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  sTagGetScenarioInfoListParam param;
  CParserJson parser;
  try {
    int nRet = parser.ParseScenarioInfoListParams(strParam.c_str(), param);
    if (nRet != 0 || param.m_ids.size() < 1) {
      return m_wstrSceneList.c_str();
    }

    CScenarioEntities ses;
    nRet = CDatabase::Instance().ScenarioInfoList(param, ses);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"Scenario info list error!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetScenesInfoByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Get scenario info list error\"}";
}

const wchar_t* CSceneData::DeleteScenes(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("delete scenes %s ", strParams.c_str());

  CParserJson jParser;
  sTagScenesDeleteParams param;
  int ret = jParser.ParseScenesDeleteParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1})";
  }

  sTagGetScenarioInfoListParam infoListParam;
  infoListParam.m_ids = param.m_ids;

  CScenarioEntities ses;
  CDatabase::Instance().ScenarioInfoList(infoListParam, ses);

  ScenarioEntities& entities = ses.Entities();
  for (auto entity : entities) {
    int nRet = DeleteSingleScene(entity.m_strName.c_str(), entity.m_strType.c_str());
    SYSTEM_LOGGER_INFO("real delte ret=%d,file=%s", nRet, entity.m_strName.c_str());
    /*
    if (nRet != 0)
    {
            continue;
    }
    */

    // CDatabase::Instance().DeleteScenario(entity.m_nID);
    CDatabase::Instance().DeleteOneScenario(entity.m_nID);
  }
  /*
  std::vector<std::string>::iterator itr = param.m_sceneNames.begin();
  for(;itr != param.m_sceneNames.end(); ++itr){
          DeleteSingleScene(itr->c_str());
  }
  */

  return LR"({"code": 0})";
}

const wchar_t* CSceneData::DownloadScenes(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("download scenes %s ", strParams.c_str());

  CParserJson jParser;
  sTagScenesDownloadParams param;
  int ret = jParser.ParseScenesDownloadParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message": "Parse download param error!" })";
  }

  boost::filesystem::path srcPath = CEngineConfig::Instance().ResourceDir();
  srcPath /= "scene";

  std::vector<std::string>::iterator itr = param.m_sceneNames.begin();
  std::string strDestPath = "";
  for (; itr != param.m_sceneNames.end(); ++itr) {
    CopySingleScene(itr->c_str(), srcPath.string().c_str(), param.m_strDestDir.c_str(), param.m_strExportType,
                    strDestPath);
  }

  return LR"({"code": 0, "message": "ok"})";
}

const wchar_t* CSceneData::DownloadScenesV2(const wchar_t* wstrParams) {
  isStopScenesProgess = false;
  dScenesProgessVal = 0;

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("download scenes %s ", strParams.c_str());

  CParserJson jParser;
  sTagScenesDownloadParams param;
  int ret = jParser.ParseScenesDownloadParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message":"Parse download param error!"})";
  }

  boost::filesystem::path srcPath;  // = CEngineConfig::Instance().ResourceDir();
  // srcPath /= "scene";

  std::vector<std::string>::iterator itr = param.m_sceneNames.begin();
  std::string strDestPath = "";
  std::string strErrorScene = "";

  double nsize = param.m_sceneNames.size();
  double nover = 0;

  std::vector<std::string> vecNameFailed;
  std::vector<std::string> vecFormateFailed;
  std::vector<std::string> vecMapExistFailed;
  std::vector<std::string> vecCopyFailed;
  bool isAllOk = true;

  for (; itr != param.m_sceneNames.end(); ++itr) {
    if (isStopScenesProgess) {
      isStopScenesProgess = false;
      return LR"({"code": -2, "message":"Interrupt"})";
      // m_wstrDownloadSceneResp = LR"({"code": -1, "message":"stop the progress"})";
      // return m_wstrDownloadSceneResp.c_str();
    }

    sTagEntityScenario scenario;
    scenario.m_nID = atoi(itr->c_str());
    CDatabase::Instance().Scenario(scenario);
    if (scenario.m_strName.size() > 0) {
      bool isC0 = IncludeChinese(scenario.m_strName.c_str());
      bool isC1 = IncludeChinese(param.m_strDestDir.c_str());
      if (isC0 || isC1) {
        isAllOk = false;
        vecNameFailed.push_back(scenario.m_strName);
        continue;
      }
      srcPath = scenario.m_strPath;
      srcPath = srcPath.parent_path();

      ret = CopySingleScene(scenario.m_strName.c_str(), srcPath.string().c_str(), param.m_strDestDir.c_str(),
                            strDestPath, param.m_strExportType, scenario.m_strType);
      if (ret != 0) {
        if (ret == HSEC_MapFormatError) {
          // strErrorScene += (scenario.m_strName + ",");
          isAllOk = false;
          vecFormateFailed.push_back(scenario.m_strName);
          continue;
        } else if (ret == HSEC_MapLostError) {
          isAllOk = false;
          vecMapExistFailed.push_back(scenario.m_strName);
          continue;
        } else {
          isAllOk = false;
          vecCopyFailed.push_back(scenario.m_strName);
          continue;
        }
      }
    }
    nover++;
    dScenesProgessVal = nover / nsize;
  }
  dScenesProgessVal = 1;
  std::string strFailedScenes = "";
  if (!isAllOk) {
    std::string strMessage = R"({"code": -1, )";

    if (vecNameFailed.size() > 0) {
      for (auto itr : vecNameFailed) {
        strFailedScenes.append(itr);
        strFailedScenes.append(",");
      }
      strFailedScenes = strFailedScenes.substr(0, strFailedScenes.length() - 1);
      strFailedScenes.append(" Export failed!Chinese character;");
    }

    if (vecFormateFailed.size() > 0) {
      for (auto itr : vecFormateFailed) {
        strFailedScenes.append(itr);
        strFailedScenes.append(",");
      }
      strFailedScenes = strFailedScenes.substr(0, strFailedScenes.length() - 1);
      strFailedScenes.append(" Export failed! Map format does not support;");
    }

    if (vecMapExistFailed.size() > 0) {
      for (auto itr : vecMapExistFailed) {
        strFailedScenes.append(itr);
        strFailedScenes.append(",");
      }
      strFailedScenes = strFailedScenes.substr(0, strFailedScenes.length() - 1);
      strFailedScenes.append(" Export failed! Map does not exist;");
    }

    if (vecCopyFailed.size() > 0) {
      for (auto itr : vecCopyFailed) {
        strFailedScenes.append(itr);
        strFailedScenes.append(",");
      }
      strFailedScenes = strFailedScenes.substr(0, strFailedScenes.length() - 1);
      strFailedScenes.append(" Export failed! copy failed;");
    }

    strMessage.append(R"("message":" )" + strFailedScenes + R"("})");
    m_wstrDownloadSceneResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());
  } else {
    m_wstrDownloadSceneResp = LR"({"code":0, "message": "ok"})";
  }

  return m_wstrDownloadSceneResp.c_str();
}

const wchar_t* CSceneData::UploadScenes(const wchar_t* wstrParams) {
  isStopScenesProgess = false;
  dScenesProgessVal = 0;

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("upload scenes %s ", strParams.c_str());

  CParserJson jParser;
  sTagScenesUploadParams param;
  int ret = jParser.ParseScenesUploadParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message":"Parse upload param error!"})";
  }

  // std::vector<boost::filesystem::path> files;
  std::vector<std::tuple<boost::filesystem::path, std::string>> files;
  boost::filesystem::path srcPath = param.m_strSourceDir;
  srcPath = boost::filesystem::absolute(srcPath, "\\");
  boost::filesystem::path scenePath = srcPath;
  if (boost::iequals(param.m_strType, "dir")) {
    // 路径存在
    if (boost::filesystem::exists(scenePath)) {
      // 获取sim文件
      boost::filesystem::directory_iterator itr_end;
      boost::filesystem::directory_iterator itr(scenePath);

      for (; itr != itr_end; ++itr) {
        std::string strName = (*itr).path().string();
        std::string strExt = (*itr).path().extension().string();
        if ((!boost::filesystem::is_directory(*itr)) && strExt == ".sim") {
          std::tuple<boost::filesystem::path, std::string> temp(itr->path(), "false");
          files.push_back(temp);
        }
      }
    }

  } else if (boost::iequals(param.m_strType, "files")) {
    auto itr = param.m_sceneNames.begin();
    for (; itr != param.m_sceneNames.end(); ++itr) {
      boost::filesystem::path p = srcPath;
      p /= std::get<0>(*itr);
      std::string strReplaceFlag = std::get<1>(*itr);
      std::string strExt = p.extension().string();
      if ((!boost::filesystem::is_directory(p)) &&
          (boost::algorithm::iequals(strExt, ".sim") || boost::algorithm::iequals(strExt, ".xosc"))) {
        std::tuple<boost::filesystem::path, std::string> temp(p, strReplaceFlag);
        files.push_back(temp);
      }
    }

  } else {
    SYSTEM_LOGGER_ERROR("upload scenes param error!");
  }

  int nSetID = 1;
  std::string strSetID = std::to_string(nSetID);
  if (param.m_strScenarioSet.size() > 0) {
    sTagEntityScenarioSet set;
    if (CDatabase::Instance().ScenarioSetExist(param.m_strScenarioSet)) {
      set.m_strName = param.m_strScenarioSet;
      CDatabase::Instance().ScenarioSetByName(set);
    } else {
      set.m_strName = param.m_strScenarioSet;
      set.m_strLabel = param.m_strScenarioSet;
      set.m_strScenes = "";
      CDatabase::Instance().CreateOneScenarioSet(set);
    }
    nSetID = set.m_nID;
    strSetID = std::to_string(nSetID);
  }

  boost::filesystem::path destPath =
      CEngineConfig::Instance().getSceneDataDir();  // CEngineConfig::Instance().ResourceDir();
  // destPath /= "scene";

  std::string strDestPath = "";
  int nRet = 0;

  std::string strFailedScenes = "";
  std::vector<std::string> vecFormateFailed;
  std::vector<std::string> vecNameFailed;
  std::vector<std::string> vecCopyFailed;

  auto itr = files.begin();

  double nsize = files.size();
  double nover = 0;

  bool isAllOk = true;
  for (; itr != files.end(); ++itr) {
    if (isStopScenesProgess) {
      isStopScenesProgess = false;
      return LR"({"code": -2, "message":"Interrupt"})";
      // m_wstrUploadSceneResp = LR"({"code": -1, "message":"stop the progress"})";
      // return m_wstrUploadSceneResp.c_str();
    }
    std::string strName = std::get<0>(*itr).filename().string();
    std::string strStem = std::get<0>(*itr).stem().string();
    std::string strExt = std::get<0>(*itr).extension().string();

    std::string strReplaceFlag = std::get<1>(*itr);

    strExt = strExt.substr(1);

    bool isC0 = IncludeChinese(strName.c_str());
    bool isC1 = IncludeChinese(srcPath.string().c_str());
    if (isC0 || isC1) {
      isAllOk = false;
      vecNameFailed.push_back(strStem);
      continue;
    }

    std::string strNewName;
    // nRet = CopySingleScene(strStem.c_str(), srcPath.string().c_str()
    // , destPath.string().c_str(), strDestPath, "sim", strExt);

    nRet = CopySingleScene(strStem.c_str(), srcPath.string().c_str(), destPath.string().c_str(), strDestPath,
                           strReplaceFlag, strExt, param.m_category, strNewName);

    if (nRet == 0) {
      // if (CDatabase::Instance().ScenarioExist(strStem.c_str()))
      if (CDatabase::Instance().ScenarioExist(strNewName.c_str(), strExt.c_str())) {
        sTagEntityScenario scenario;
        // scenario.m_strName = strStem;
        scenario.m_strName = strNewName;
        nRet = CDatabase::Instance().ScenarioByName(scenario);
        if (scenario.m_strMap != GetMapName()) {
          boost::filesystem::path temppath = GetMapName();
          string str1 = temppath.stem().string();
          string str2 = temppath.extension().string();
          string str = str1 + str2;
          scenario.m_strMap = str;
          CDatabase::Instance().UpdateOneScenario(scenario);
        }

        std::string strSet = scenario.m_strSets;
        boost::algorithm::trim_if(strSet, boost::algorithm::is_any_of(", "));
        int nOldSetID = std::atoi(strSet.c_str());
        if (nOldSetID != nSetID) {
          std::string strSceneID = std::to_string(scenario.m_nID);

          scenario.m_strSets = "," + std::to_string(nSetID) + ",";
          CDatabase::Instance().UpdateOneScenario(scenario);
          sTagEntityScenarioSet oldScenarioSet;
          oldScenarioSet.m_nID = nOldSetID;
          sTagEntityScenarioSet curScenarioSet;
          curScenarioSet.m_nID = nSetID;
          CDatabase::Instance().ScenarioSet(oldScenarioSet);
          CDatabase::Instance().RemoveIDFromIDs(oldScenarioSet.m_strScenes, strSceneID);
          CDatabase::Instance().ScenarioSet(curScenarioSet);
          CDatabase::Instance().AddIDToIDs(strSceneID, oldScenarioSet.m_strScenes);
          CDatabase::Instance().UpdateOneScenarioSet(oldScenarioSet);
          CDatabase::Instance().UpdateOneScenarioSet(curScenarioSet);
        }

      } else {
        CParser parser;
        sTagSimuTraffic sst;
        int nRes = parser.Parse(sst, strDestPath.c_str(), destPath.string().c_str(), false);

        sTagEntityScenario scenario;
        // scenario.m_strName = strStem;
        scenario.m_strName = strNewName;
        scenario.m_strPath = strDestPath;

        boost::filesystem::path mapPath = sst.m_simulation.m_mapFile.m_strMapfile;
        scenario.m_strMap = mapPath.filename().string();
        scenario.m_strInfo = sst.m_simulation.m_strInfo;
        scenario.m_strLabel = "";
        scenario.m_strSets = "," + std::to_string(nSetID) + ",";
        scenario.m_strContent = "0";
        scenario.m_strType = strExt;

        std::string strTrafficExt = "";
        if (!sst.m_simulation.m_strTraffic.empty()) {
          boost::filesystem::path temppath = sst.m_simulation.m_strTraffic;
          std::string strTrafficExt = temppath.extension().string();
          strTrafficExt = strTrafficExt.substr(1);
        }
        scenario.m_strTrafficType = strTrafficExt;
        if (param.m_category == "LOG_SIM") {
          scenario.m_strTrafficType = "simrec";
        }
        SYSTEM_LOGGER_INFO("strTrafficExt=%s", strTrafficExt.c_str());
        CDatabase::Instance().CreateOneScenario(scenario, nSetID);
      }
    } else {
      isAllOk = false;
      vecCopyFailed.push_back(strStem);
    }

    nover++;
    dScenesProgessVal = nover / nsize;
  }
  dScenesProgessVal = 1;
  if (!isAllOk) {
    std::string strMessage = R"({"code": -1, )";

    if (vecNameFailed.size() > 0) {
      for (auto itr : vecNameFailed) {
        strFailedScenes.append(itr);
        strFailedScenes.append(",");
      }
      strFailedScenes = strFailedScenes.substr(0, strFailedScenes.length() - 1);
      strFailedScenes.append(" Upload failed!Chinese character;");
    }

    if (vecCopyFailed.size() > 0) {
      for (auto itr : vecCopyFailed) {
        strFailedScenes.append(itr);
        strFailedScenes.append(",");
      }
      strFailedScenes = strFailedScenes.substr(0, strFailedScenes.length() - 1);
      strFailedScenes.append(" Upload failed!Copy failed");
    }

    strMessage.append(R"("message":" )" + strFailedScenes + R"("})");

    m_wstrUploadSceneResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());

  } else {
    m_wstrUploadSceneResp = LR"({"code": 0, "message":"ok"})";
  }

  return m_wstrUploadSceneResp.c_str();
}

int CSceneData::ExportInOneScene(boost::filesystem::path oneScenePath, std::string strScenarioSetID) {
  boost::filesystem::path destPath = CEngineConfig::Instance().getSceneDataDir();
  // destPath /= "scene";

  std::string strDestPath = "";
  int nRet = 0;

  std::string strFailedScenes = "";

  std::string strName = oneScenePath.filename().string();
  std::string strStem = oneScenePath.stem().string();
  std::string strExt = oneScenePath.extension().string();
  if (strExt == ".xosc") {
    strExt = "xosc";
  } else {
    strExt = "sim";
  }

  // 判断是否同名覆盖
  if (CDatabase::Instance().ScenarioExist(strStem.c_str(), strExt.c_str())) {
    SYSTEM_LOGGER_WARN("exist the same name file %s, not export!", strStem.c_str());
    return HSEC_COPY_SCENE_FILE_EXIST;
  }

  boost::filesystem::path srcPath = oneScenePath.parent_path();
  nRet =
      CopySingleScene(strStem.c_str(), srcPath.string().c_str(), destPath.string().c_str(), strDestPath, "sim", strExt);
  if (nRet == 0) {
    sTagSimuTraffic st;
    CParser parser;
    parser.Parse(st, strDestPath.c_str(), destPath.string().c_str(), false);
    sTagEntityScenario scenario;
    scenario.m_strName = strStem;
    scenario.m_strPath = strDestPath;

    boost::filesystem::path mapPath = st.m_simulation.m_mapFile.m_strMapfile;
    scenario.m_strMap = mapPath.filename().string();
    scenario.m_strInfo = st.m_simulation.m_strInfo;
    scenario.m_strLabel = "";
    scenario.m_strSets = "," + strScenarioSetID + ",";
    scenario.m_strContent = "0";
    scenario.m_strType = strExt;

    int nSetID = atoi(strScenarioSetID.c_str());
    CDatabase::Instance().CreateOneScenario(scenario, nSetID);
  } else {
    SYSTEM_LOGGER_ERROR("copy file %s error!", strStem.c_str());
    return HSEC_COPY_SCENE_FAILED;
  }

  return HSEC_OK;
}

int CSceneData::ExportInBatchGeneratedScenes(const char* strSrcDir, std::string& strExistFiles,
                                             std::string& strFailedFiles) {
  // 获取源目录
  boost::filesystem::path srcPath = CEngineConfig::Instance().getSceneDataDir();
  srcPath /= "batch_scene";
  srcPath /= strSrcDir;
  if (!boost::filesystem::exists(srcPath)) {
    SYSTEM_LOGGER_ERROR("src dir %s not exist", strSrcDir);
    return HSEC_SourceDirNotExist;
  }

  // 获取场景集名称
  std::string strScenarioSetName = srcPath.filename().string();

  // 创建用源目录名称创建场景集
  sTagEntityScenarioSet scenarioSet;
  scenarioSet.m_strName = strScenarioSetName;
  scenarioSet.m_strLabel = strScenarioSetName;
  scenarioSet.m_strScenes = "";
  CDatabase::Instance().CreateOneScenarioSet(scenarioSet);

  std::string strSetID = std::to_string(scenarioSet.m_nID);

  strExistFiles = "";
  strFailedFiles = "";
  std::vector<std::string> ExistedFiles;

  // 导入场景
  boost::filesystem::directory_iterator itr_end;
  boost::filesystem::directory_iterator itr(srcPath);

  for (; itr != itr_end; ++itr) {
    std::string strName = (*itr).path().string();
    std::string strExt = (*itr).path().extension().string();
    if ((!boost::filesystem::is_directory(*itr)) && (strExt == ".sim" || strExt == ".xosc")) {
      int nRet = ExportInOneScene(itr->path(), strSetID);

      if (nRet == HSEC_COPY_SCENE_FILE_EXIST) {
        strExistFiles += strName;
        strExistFiles += " ";
      } else if (nRet == HSEC_COPY_SCENE_FAILED) {
        strFailedFiles += strName;
        strFailedFiles += " ";
      }
    }
  }

  return HSEC_OK;
}

const wchar_t* CSceneData::CopyScenes(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  return LR"("code": 0})";
}

const wchar_t* CSceneData::RenameScenes(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  CParserJson jParser;
  sTagSceneRenameParams param;
  SYSTEM_LOGGER_INFO("start rename scene ");
  int ret = jParser.ParseSceneRenameParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message":"Rename scene err!"})";
  }

  // string 可能要加上有效性判断
  if (param.m_oldName != param.m_newName) {
    // 路径
    boost::filesystem::path scenePath = CEngineConfig::Instance().ResourceDir();
    scenePath.append("scene");
    boost::filesystem::path oldFullNamePath = scenePath / param.m_oldName;
    boost::filesystem::path newFullNamePath = scenePath / param.m_newName;
    std::string strOldStem, strNewStem, strExt;

    if (boost::filesystem::exists(oldFullNamePath) && boost::filesystem::is_regular_file(oldFullNamePath)) {
      strOldStem = oldFullNamePath.stem().string();

      if (strExt == "sim") {
        // 找到traffic 文件
        string stroldTriffic = strOldStem + "_traffic.xml";
        string strnewTriffic = strNewStem + "_traffic.xml";
        boost::filesystem::path FullNamePathTraffic = scenePath / stroldTriffic;
        if (boost::filesystem::exists(FullNamePathTraffic) && boost::filesystem::is_regular_file(FullNamePathTraffic)) {
          SYSTEM_LOGGER_INFO(" rename scene traffic");
          // 重命名traffic
          boost::filesystem::rename(scenePath / stroldTriffic, scenePath / strnewTriffic);

          // 修改sim文件里面的traffic
          SYSTEM_LOGGER_INFO(" rename scene sim_traffic");
          sTagSimuTraffic scene;
          int ret = scene.m_simulation.ChangeTrafficName(oldFullNamePath.string().c_str(), strnewTriffic.c_str());
        } else {
          SYSTEM_LOGGER_INFO(" rename scene Triffic find err ");
          return LR"({"code":-1, "message":"Rename scene err!"})";
        }
      }

      // 重命名
      SYSTEM_LOGGER_INFO(" rename scene sim");
      boost::filesystem::rename(scenePath / param.m_oldName, scenePath / param.m_newName);
    } else {
      return LR"({"code": 0})";
    }

    SYSTEM_LOGGER_INFO(" rename scene data");
    // 数据库
    bool bExist = CDatabase::Instance().ScenarioExist(strOldStem.c_str(), strExt.c_str());
    if (bExist) {
      sTagEntityScenario scenarioInDB;
      scenarioInDB.m_strName = strOldStem;
      CDatabase::Instance().ScenarioByName(scenarioInDB);
      scenarioInDB.m_strName = strNewStem;

      boost::filesystem::path p = scenarioInDB.m_strPath;
      string str1 = p.stem().string();
      string str2 = p.extension().string();
      string str = strNewStem + str2;
      boost::filesystem::path srcPath = CEngineConfig::Instance().ResourceDir();
      srcPath /= "scene";
      srcPath.append(str);
      scenarioInDB.m_strPath = srcPath.string();
      int nRet = CDatabase::Instance().UpdateOneScenario(scenarioInDB);
      if (nRet != 0) {
        SYSTEM_LOGGER_INFO(" rename scene data err");
        return LR"({"code":-1, "message":"Rename scene  data err!"})";
      }
    } else {
      SYSTEM_LOGGER_INFO(" rename scene data err");
      return LR"({"code":-1, "message":"Rename scene err!"})";
    }
  }

  return LR"({"code": 0})";
}

const wchar_t* CSceneData::LoadGlobalSensorConfig() {
  CParserJson jParser;

  SYSTEM_LOGGER_INFO("load global sensor config");

  std::string strJson = jParser.LoadGlobalSensorConfig();

  m_wstrGlobalSensorConfig = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());

  return m_wstrGlobalSensorConfig.c_str();
}

const wchar_t* CSceneData::SaveGlobalSensorConfig(const wchar_t* wstrConfigContent) {
  if (!wstrConfigContent) {
    return L"{\"err\":-1, \"message\": \"Global sensor config content null\"}";
  }

  std::string strGlobalSensorConfigContent = CEngineConfig::Instance().WStr2MBStr(wstrConfigContent);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("start save global sensor config file ");

  try {
    int nRet = jParser.SaveGlobalSensorConfig(strGlobalSensorConfigContent.c_str());

    SYSTEM_LOGGER_INFO("global sensor config file was saved.");

    if (nRet == 0) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveGlobalSensorData exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Save global sensor data error\"}";
}

const wchar_t* CSceneData::LoadGlobalEnvironmentConfig() {
  CParserJson jParser;

  SYSTEM_LOGGER_INFO("load global environment config");

  std::string strJson = jParser.LoadGlobalEnvironmentConfig();

  m_wstrGlobalEnvironmentConfig = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());

  return m_wstrGlobalEnvironmentConfig.c_str();
}

const wchar_t* CSceneData::SaveGlobalEnvironmentConfig(const wchar_t* wstrConfigContent) {
  if (!wstrConfigContent) {
    return L"{\"err\":-1, \"message\": \"Global sensor config content null\"}";
  }

  std::string strGlobalEnvironmentConfigContent = CEngineConfig::Instance().WStr2MBStr(wstrConfigContent);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("start save global environment config file ");

  try {
    int nRet = jParser.SaveGlobalEnvironmentConfig(strGlobalEnvironmentConfigContent.c_str());

    SYSTEM_LOGGER_INFO("global environment config file was saved.");

    if (nRet == 0) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveGlobalEnvironmentData exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Save global environment data error\"}";
}

const wchar_t* CSceneData::GetCatalogList() {
  CCataLog catalog;
  Json::Value root;
  catalog.allModelToJson(CEngineConfig::Instance().getDataCatalogsDir(), root);
  Json::Value catalog_list;
  catalog_list["catalog_list"] = root;
  std::string strResult = catalog_list.toStyledString();
  m_wstrCataloglist.clear();
  m_wstrCataloglist = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  return m_wstrCataloglist.c_str();
}

const wchar_t* CSceneData::AddCatalogList(const wchar_t* wstrInfo) {
  if (!wstrInfo) {
    return L"{\"code\":-1, \"message\": \"AddCatalogList content null\"}";
  }
  Json::Value root;
  if (Wstr2Json(wstrInfo, root)) {
    try {
      int nRet = CCataLog::addEgoModelFromJson(CEngineConfig::Instance().getCataLogDir(), root["data"]);
      if (nRet == 0) {
        return L"{\"err\":0, \"message\": \"ok\"}";
      }
    } catch (std::exception& e) {
      SYSTEM_LOGGER_FATAL("AddCatalogList exception: %s", e.what());
    }
  }
  return L"{\"code\":-1, \"message\": \" Add catalog error\"}";
}

const wchar_t* CSceneData::UpdateCatalog(const wchar_t* wstrInfo) {
  if (!wstrInfo) {
    return L"{\"code\":-1, \"message\": \"Update catalog content null\"}";
  }
  Json::Value root;
  if (Wstr2Json(wstrInfo, root)) {
    try {
      int nRet = CCataLog::addEgoModelFromJson(CEngineConfig::Instance().getCataLogDir(), root["data"]);
      if (nRet == 0) {
        return L"{\"code\":0, \"message\": \"ok\"}";
      }
    } catch (std::exception& e) {
      SYSTEM_LOGGER_FATAL("UpdateCatalog exception: %s", e.what());
    }
  }

  return L"{\"err\":-1, \"message\": \"Update catalog error\"}";
}

const wchar_t* CSceneData::DeleteCatalog(const wchar_t* strVechileCatalog) {
  if (!strVechileCatalog) {
    SYSTEM_LOGGER_DEBUG("DeleteCatalog content null");
    return L"{\"err\":-1, \"message\": \"Delete Catalog content null\"}";
  }
  Json::Value root;
  if (Wstr2Json(strVechileCatalog, root)) {
    try {
      std::string strVariable = root.get("variable", "").asString();
      int nRet = CCataLog::delEgoModelFromJson(CEngineConfig::Instance().getCataLogDir(), strVariable.c_str());
      if (nRet == 0) {
        return L"{\"err\": 0,  \"message\": \"ok\"}";
      } else {
        return L"{\"err\": -1, \"message\": \"Model is preset, not allowed delete !\"}";
      }
    } catch (std::exception& e) {
      SYSTEM_LOGGER_DEBUG("DeleteCatalog exception: %s", e.what());
    }
  }
  return L"{\"err\": -1, \"message\": \"Delete Catalog Error\"}";
}

const wchar_t* CSceneData::SensorLoad() {
  std::string strSensorPath = CEngineConfig::Instance().getSensorPath();
  try {
    SensorV2 sensor;
    sensor.parseXML(strSensorPath.c_str());
    Json::Value root;
    sensor.toJson(root);
    std::string strResult = root.toStyledString();
    m_wstrSensorLoad.clear();
    m_wstrSensorLoad = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
    return m_wstrSensorLoad.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_ERROR("Sensor load exception: %s", e.what());
  }
  return L"{\"err\": -1, \"message\": \"sensor Load error!\"}";
}

const wchar_t* CSceneData::SensorSave(const wchar_t* strSensor) {
  if (!strSensor) {
    return L"{\"err\":-1, \"message\": \"save sensor content null\"}";
  }
  std::string strVechileCatalogInfo = CEngineConfig::Instance().WStr2MBStr(strSensor);
  CParserJson jParser;
  try {
    SensorV2 sensorCatalog;
    int nRet = jParser.ParseSensor(strVechileCatalogInfo.c_str(), sensorCatalog);
    std::string strSensorPath = CEngineConfig::Instance().getSensorPath();
    bool flag = sensorCatalog.saveXmlDefine(strSensorPath.c_str());
    if (nRet == 0 && flag == true) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("save sensor exception: %s", e.what());
  }
  return L"{\"err\":-1, \"message\": \" save sensor error\"}";
}

const wchar_t* CSceneData::SensorAdd(const wchar_t* strSensor) {
  if (!strSensor) {
    return L"{\"err\":-1, \"message\": \"add sensor content null\"}";
  }
  std::string strVechileCatalogInfo = CEngineConfig::Instance().WStr2MBStr(strSensor);
  CParserJson jParser;
  try {
    SensorV2 sensorCatalog;
    SensorBaseV2 base;
    int nRet = jParser.ParseSingleSensor(strVechileCatalogInfo.c_str(), base);
    std::string strSensorPath = CEngineConfig::Instance().getSensorPath();

    bool flag = sensorCatalog.addSensorDefine(strSensorPath.c_str(), base);
    if (nRet == 0 && flag == true) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("Sensor Add exception: %s", e.what());
  }
  return L"{\"err\":-1, \"message\": \"add sensor error\"}";
}

const wchar_t* CSceneData::SensorUpdate(const wchar_t* strSensor) {
  if (!strSensor) {
    return L"{\"err\":-1, \"message\": \"update sensor content null\"}";
  }
  std::string strVechileCatalogInfo = CEngineConfig::Instance().WStr2MBStr(strSensor);
  CParserJson jParser;
  try {
    SensorV2 sensorCatalog;
    SensorBaseV2 base;
    int nRet = jParser.ParseSingleSensor(strVechileCatalogInfo.c_str(), base);
    std::string strSensorPath = CEngineConfig::Instance().getSensorPath();
    bool flag = sensorCatalog.updateSensorDefine(strSensorPath.c_str(), base);
    if (nRet == 0 && flag == true) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("Sensor Update exception: %s", e.what());
  }
  return L"{\"err\":-1, \"message\": \"update sensor error\"}";
}

const wchar_t* CSceneData::SensorDelete(const wchar_t* strSensor) {
  if (!strSensor) {
    return L"{\"err\":-1, \"message\": \"delete sensor content null\"}";
  }
  std::string strVechileCatalogInfo = CEngineConfig::Instance().WStr2MBStr(strSensor);
  CParserJson jParser;
  try {
    SensorV2 sensorCatalog;
    std::string strIdx;
    int nRet = jParser.ParseDeleteInfo(strVechileCatalogInfo.c_str(), strIdx);
    std::string strSensorPath = CEngineConfig::Instance().getSensorPath();
    bool flag = sensorCatalog.deleteSensorDefine(strSensorPath.c_str(), strIdx);
    if (nRet == 0 && flag == true) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("Delete sensor exception: %s", e.what());
  }
  return L"{\"err\":-1, \"message\": \"delete sensor error\"}";
}

const wchar_t* CSceneData::SensorGroupLoad() {
  // 1) read sensor
  SensorV2 sensorCatalog;
  std::string strSensorPath = CEngineConfig::Instance().getSensorPath();
  sensorCatalog.parseXML(strSensorPath.c_str());
  SensorGroupVec sensorGroups = sensorCatalog.getSensorGroup();
  Json::Value root;
  for (auto it : sensorGroups) {
    Json::Value jsonSensorGroup;
    for (auto itsensor : it.m_SensorVec) {
      Json::Value group;
      group["Device"] = itsensor.Device;
      group["ID"] = itsensor.ID;
      group["InstallSlot"] = itsensor.InstallSlot;
      group["LocationX"] = itsensor.LocationX;
      group["LocationY"] = itsensor.LocationY;
      group["LocationZ"] = itsensor.LocationZ;
      group["RotationX"] = itsensor.RotationX;
      group["RotationY"] = itsensor.RotationY;
      group["RotationZ"] = itsensor.RotationZ;
      group["idx"] = std::atoi(itsensor.idx.c_str());
      jsonSensorGroup["group"].append(group);
    }
    jsonSensorGroup["groupName"] = (it.m_name);
    root["sensor_groups"].append(jsonSensorGroup);
  }
  std::string strResult = root.toStyledString();
  m_wstrSensorGroup.clear();
  m_wstrSensorGroup = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  return m_wstrSensorGroup.c_str();
}

const wchar_t* CSceneData::SensorGroupSave(const wchar_t* strSensor) {
  if (!strSensor) {
    return L"{\"err\":-1, \"message\": \"Sensorgroup add content null\"}";
  }
  // parse json
  std::string strDynamicInfo = CEngineConfig::Instance().WStr2MBStr(strSensor);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  Json::Value sensorJson;
  int nLen = strlen(strDynamicInfo.c_str());
  if (!reader->parse(strDynamicInfo.c_str(), strDynamicInfo.c_str() + nLen, &sensorJson, &errs)) {
    return L"{\"code\":-1, \"message\": \"Sensorgroup add failure\"}";
  }
  // add sensorgroup
  try {
    SensorV2 sensorCatalog;
    std::string strSensorPath = CEngineConfig::Instance().getSensorPath();
    SensorGroup group;
    group.parseJson(sensorJson);
    sensorCatalog.addGroupSensor(group, strSensorPath);
    return L"{\"err\":0, \"message\": \"ok\"}";
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("Sensorgroup add exception: %s", e.what());
  }
  return L"{\"err\":-1, \"message\": \"Sensorgroup add error\"}";
}

const wchar_t* CSceneData::SensorGroupDelete(const wchar_t* strSensor) {
  if (!strSensor) {
    return L"{\"err\":-1, \"message\": \"add sensorgroup content null\"}";
  }
  // parse json
  std::string strDynamicInfo = CEngineConfig::Instance().WStr2MBStr(strSensor);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  Json::Value sensorJson;
  int nLen = strlen(strDynamicInfo.c_str());
  if (!reader->parse(strDynamicInfo.c_str(), strDynamicInfo.c_str() + nLen, &sensorJson, &errs)) {
    return L"{\"code\":-1, \"message\": \"delete sensorgroup failure\"}";
  }
  try {
    // delete sensorgroup
    std::string strName = sensorJson.get("groupName", "").asString();
    SensorV2 sensorCatalog;
    std::string strSensorPath = CEngineConfig::Instance().getSensorPath();
    sensorCatalog.deleteGroupSensor(strName, strSensorPath);
    return L"{\"err\":0, \"message\": \"ok\"}";
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("dynamic delete exception: %s", e.what());
    return L"{\"err\":-1, \"message\": \"sensorgroup delete error\"}";
  }
}

const wchar_t* CSceneData::DynamicLoad() {
  CCataLog catalog;
  // 1) read sensor

  catalog.parseDefaultXosc(CEngineConfig::Instance().getDataCatalogsDir(), "");
  std::string strDynamicDir = CEngineConfig::Instance().getDynamicPath();
  std::vector<CDriveCatalog> driveCatalogList = catalog.getDriveCatalogList();
  Json::Value root;
  root["code"] = 0;
  std::set<std::string> _idset;
  for (auto it : driveCatalogList) {
    std::string dynamic_id = it.m_dynamicId;
    if (dynamic_id.empty()) continue;
    if (_idset.find(dynamic_id) != _idset.end()) continue;
    boost::filesystem::path dynamicpath = (strDynamicDir + "dynamic_" + dynamic_id + ".json");
    if (boost::filesystem::exists(dynamicpath)) {
      Json::Value dynamicJson;
      dynamicJson["id"] = dynamic_id;
      std::ifstream infile;
      infile.open(dynamicpath.string().c_str(), std::ios::in);
      if (!infile.is_open()) {
        return L"{\"code\":-1, \"message\": \"DynamicLoad1 failure\"}";
      }
      std::string strDynamicData = "";
      string buf;
      while (getline(infile, buf)) {
        strDynamicData += buf;
      }
      infile.close();
      Json::CharReaderBuilder builder;
      builder["collectComments"] = false;
      JSONCPP_STRING errs;
      Json::CharReader* reader = builder.newCharReader();
      Json::Value dynamicDataJson;
      int nLen = strlen(strDynamicData.c_str());
      if (!reader->parse(strDynamicData.c_str(), strDynamicData.c_str() + nLen, &dynamicDataJson, &errs)) {
        return L"{\"code\":-1, \"message\": \"DynamicLoad2 failure\"}";
      }
      _idset.insert(dynamic_id);
      dynamicJson["dynamicData"] = dynamicDataJson;
      root["dynamics"].append(dynamicJson);
    } else {
      continue;
      // return L"{\"code\":-1, \"message\": \"DynamicLoad3 failure\"}";
    }
  }
  std::string strResult = root.toStyledString();
  m_wstrDynamicLoad.clear();
  m_wstrDynamicLoad = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  return m_wstrDynamicLoad.c_str();
}

const wchar_t* CSceneData::DynamicSave(const wchar_t* strDynamic) {
  if (!strDynamic) {
    return L"{\"err\":-1, \"message\": \"save dynamic content null\"}";
  }
  // pasre json
  std::string strDynamicInfo = CEngineConfig::Instance().WStr2MBStr(strDynamic);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  Json::Value dynamicJson;
  int nLen = strlen(strDynamicInfo.c_str());
  if (!reader->parse(strDynamicInfo.c_str(), strDynamicInfo.c_str() + nLen, &dynamicJson, &errs)) {
    return L"{\"code\":-1, \"message\": \"DynamicLoad failure\"}";
  }
  // deal data
  std::string strId = dynamicJson["id"].asString();
  std::string strDynamicDir = CEngineConfig::Instance().getDynamicPath();
  boost::filesystem::path dynamicpath = (strDynamicDir + "dynamic_" + strId + ".json");
  std::ofstream out(dynamicpath.string().c_str());
  if (out.is_open()) {
    out << dynamicJson["dynamicData"].toStyledString() << std::endl;
    out.close();
    return L"{\"err\":0, \"message\": \"ok\"}";
  } else {
    return L"{\"err\":-1, \"message\": \"dynamic save error\"}";
  }
}

const wchar_t* CSceneData::DynamicDelete(const wchar_t* strDynamic) {
  if (!strDynamic) {
    return L"{\"err\":-1, \"message\": \"delete dynamic error\"}";
  }
  try {
    std::string strDynamicInfo = CEngineConfig::Instance().WStr2MBStr(strDynamic);
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;
    Json::CharReader* reader = builder.newCharReader();
    Json::Value dynamicJson;
    int nLen = strlen(strDynamicInfo.c_str());
    if (!reader->parse(strDynamicInfo.c_str(), strDynamicInfo.c_str() + nLen, &dynamicJson, &errs)) {
      return L"{\"code\":-1, \"message\": \"dynamic delete error\"}";
    }
    std::string strId = dynamicJson["id"].asString();
    std::string strDynamicDir = CEngineConfig::Instance().getDynamicPath();
    boost::filesystem::path dynamicpath = (strDynamicDir + "dynamic_" + strId + ".json");
    if (boost::filesystem::exists(dynamicpath) && boost::filesystem::remove(dynamicpath)) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    } else {
      return L"{\"err\":-1, \"message\": \"dynamic delete error\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("dynamic delete exception: %s", e.what());
    return L"{\"err\":-1, \"message\": \"dynamic delete error\"}";
  }
}

const wchar_t* CSceneData::ModelUpload(const wchar_t* zipPath, const wchar_t* wstrId) {
  if (!zipPath && !wstrId) {
    return L"{\"id\":-1, \"filepath\":[ ]}";
  }
  std::string strZipPath = CEngineConfig::Instance().WStr2MBStr(zipPath);
  std::string strId = CEngineConfig::Instance().WStr2MBStr(wstrId);
  SYSTEM_LOGGER_INFO("ModelUpload  zippath = %s , strId = %s", strZipPath.c_str(), strId.c_str());
  // get cache dir path
  fs::path directory_path = zipPath;
  // vec store all filepath
  std::vector<fs::path> file_paths;
  // get all filepath
  if (fs::exists(directory_path) && fs::is_directory(directory_path)) {
    for (fs::recursive_directory_iterator iter(directory_path), end; iter != end; ++iter) {
      if (fs::is_regular_file(iter->path())) {
        fs::path tmpPath = iter->path();
        file_paths.push_back(boost::filesystem::system_complete(iter->path()));
      }
    }
  }
  // make json
  Json::Value root;
  root["id"] = strId;
  for (const fs::path& file_path : file_paths) {
    root["filepath"].append(file_path.string());
  }
  std::string strResult = root.toStyledString();
  m_wstrUploadModel.clear();
  m_wstrUploadModel = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  return m_wstrUploadModel.c_str();
}

const wchar_t* CSceneData::MapModelSave(const wchar_t* wstrMapModelInfo) {
  SYSTEM_LOGGER_INFO("MapModelSave start");
  if (!wstrMapModelInfo) {
    return L"{\"code\":-1, \"message\":\"MapModelSave param is null\"}";
  }

  // string to json
  Json::Value root;
  if (!Wstr2Json(wstrMapModelInfo, root)) {
    return L"{\"code\":-1, \"message\":\"MapModelSave from string to json is error\"}";
  }

  // get json data
  try {
    std::string strId = root.get("id", "-1").asString();
    // update mapmodel.xml
    int nRet = CMapModel::addModelFromJson(CEngineConfig::Instance().getMapModelPath(), root["data"]);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("MapModelSave error,code =  %d", nRet);
      return L"{\"err\":-1, \"message\": \"MapModelSave error\"}";
    }

    // src dir
    fs::path srcModelDir = CEngineConfig::Instance().getCacheDir();
    srcModelDir /= strId;

    // dst dir
    std::string model3dPath = root["data"].get("model3d", "").asString();
    fs::path dstModel3d = CEngineConfig::Instance().getModelDir();
    dstModel3d.append(model3dPath);
    fs::path dstModelDir = dstModel3d.parent_path();

    // copy file
    if (std::atoi(strId.c_str()) >= 0) {
      if (!fs::exists(dstModelDir)) {
        fs::create_directories(dstModelDir);
        if (fs::exists(srcModelDir) && fs::is_directory(srcModelDir)) {
          for (fs::recursive_directory_iterator iter(srcModelDir), end; iter != end; ++iter) {
            if (fs::is_regular_file(iter->path())) {
              boost::filesystem::path dst_file = dstModelDir / iter->path().filename();
              boost::filesystem::copy_file(iter->path(), dst_file);
            }
          }
        }
      } else {
        if (fs::exists(srcModelDir) && fs::is_directory(srcModelDir)) {
          for (fs::recursive_directory_iterator iter(srcModelDir), end; iter != end; ++iter) {
            if (fs::is_regular_file(iter->path())) {
              boost::filesystem::path dst_file = dstModelDir / iter->path().filename();
              // 检查目标文件是否存在
              if (fs::exists(dst_file)) {
                // 如果目标文件已存在，则删除目标文件
                fs::remove(dst_file);
              }
              boost::filesystem::copy_file(iter->path(), dst_file);
            }
          }
        }
      }
    }

    // return message
    return L"{\"err\":0, \"message\": \"ok\"}";
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("MapModelSave exception: %s", e.what());
    return L"{\"err\":-1, \"message\": \"MapModelSave error\"}";
  }
}

const wchar_t* CSceneData::MapModelDelete(const wchar_t* wstrMapModelInfo) {
  SYSTEM_LOGGER_INFO("MapModelDelete start");
  if (!wstrMapModelInfo) {
    return L"{\"code\":-1, \"message\":\"MapModelSave param is null\"}";
  }

  // string to json
  Json::Value root;
  if (!Wstr2Json(wstrMapModelInfo, root)) {
    return L"{\"code\":-1, \"message\":\"MapModelSave from string to json is error\"}";
  }
  try {
    // update mapmodel.xml
    int nRet = CMapModel::delModelFromName(CEngineConfig::Instance().getMapModelPath(), root["variable"].asString());
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("MapModelDelete error , code =  %d", nRet);
      return L"{\"err\":-1, \"message\": \"MapModelSave error\"}";
    }
    // return ok
    return L"{\"err\":0, \"message\": \"ok\"}";
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("MapModelSave exception: %s", e.what());
    return L"{\"err\":-1, \"message\": \"MapModelDelete error\"}";
  }
}

const wchar_t* CSceneData::GetMapModelList() {
  SYSTEM_LOGGER_INFO("GetMapModelList start");
  Json::Value root;
  int nRet = CMapModel::getModelListJson(CEngineConfig::Instance().getMapModelPath(),
                                         CEngineConfig::Instance().getModelDir(), root);
  if (nRet != 0) {
    SYSTEM_LOGGER_ERROR("MapModelSave error,code =  %d", nRet);
    return L"{\"err\":-1, \"message\": \"GetMapModelList error\"}";
  }
  std::string strResult = root.toStyledString();
  m_wstrMapModelList.clear();
  m_wstrMapModelList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  return m_wstrMapModelList.c_str();
}

const wchar_t* CSceneData::SceneModelSave(const wchar_t* wstrMapModelInfo) {
  SYSTEM_LOGGER_INFO("SceneModelSave start");
  if (!wstrMapModelInfo) {
    return L"{\"code\":-1, \"message\":\"MapModelSave param is null\"}";
  }
  // string to json
  Json::Value root;
  if (!Wstr2Json(wstrMapModelInfo, root)) {
    return L"{\"code\":-1, \"message\":\"SceneModelSave from string to json is error\"}";
  }
  try {
    std::string strId = root.get("id", "-1").asString();
    // deal with catalog files
    std::string strCataLogDir = CEngineConfig::Instance().getDataCatalogsDir();
    int nRet = CCataLog::addModelFromJson(strCataLogDir.c_str(), root["data"]);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("SceneModelSave error,code =  %d", nRet);
      return L"{\"err\":-1, \"message\": \"MapModelSave error\"}";
    }

    //  src dir
    fs::path srcModelDir = CEngineConfig::Instance().getCacheDir();
    srcModelDir /= strId;

    //  dst dir
    std::string model3dPath = root["data"].get("variable", "").asString();
    fs::path dstModel3d = CEngineConfig::Instance().getModelDir();
    dstModel3d /= (model3dPath);
    fs::path dstModelDir = dstModel3d;

    // copy file
    if (std::atoi(strId.c_str()) >= 0) {
      if (!fs::exists(dstModelDir)) {
        fs::create_directories(dstModelDir);
        if (fs::exists(srcModelDir) && fs::is_directory(srcModelDir)) {
          for (fs::recursive_directory_iterator iter(srcModelDir), end; iter != end; ++iter) {
            if (fs::is_regular_file(iter->path())) {
              boost::filesystem::path dst_file = dstModelDir / iter->path().filename();
              boost::filesystem::copy_file(iter->path(), dst_file);
            }
          }
        }
      } else {
        if (fs::exists(srcModelDir) && fs::is_directory(srcModelDir)) {
          for (fs::recursive_directory_iterator iter(srcModelDir), end; iter != end; ++iter) {
            if (fs::is_regular_file(iter->path())) {
              boost::filesystem::path dst_file = dstModelDir / iter->path().filename();
              // 检查目标文件是否存在
              if (fs::exists(dst_file)) {
                // 如果目标文件已存在，则删除目标文件
                fs::remove(dst_file);
              }
              boost::filesystem::copy_file(iter->path(), dst_file);
            }
          }
        }
      }
    }

    // return message
    return L"{\"err\":0, \"message\": \"ok\"}";
  } catch (std::exception& e) {
    SYSTEM_LOGGER_DEBUG("DeleteCatalog exception: %s", e.what());
  }
}

const wchar_t* CSceneData::SceneModelDelete(const wchar_t* wstrMapModelInfo) {
  SYSTEM_LOGGER_INFO("SceneModelDelete start");
  if (!wstrMapModelInfo) {
    return L"{\"code\":-1, \"message\":\"SceneModelDelete param is null\"}";
  }
  // string to json
  Json::Value root;
  if (!Wstr2Json(wstrMapModelInfo, root)) {
    return L"{\"code\":-1, \"message\":\"SceneModelDelete from string to json is error\"}";
  }
  try {
    // deal with catalog files
    std::string strCataLogDir = CEngineConfig::Instance().getDataCatalogsDir();
    SYSTEM_LOGGER_DEBUG("variable = %s", root.get("variable", "").asString().c_str());
    int nRet = CCataLog::delModelFromJson(strCataLogDir.c_str(), root);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("SceneModelDelete error,code =  %d", nRet);
      return L" {\"err\": -1, \"message\": \"Model is preset, not allowed delete !\"}";
    }
    // return message
    return L"{\"err\":0, \"message\": \"ok\"}";
  } catch (std::exception& e) {
    SYSTEM_LOGGER_DEBUG("SceneModelDelete exception: %s", e.what());
  }
}

const wchar_t* CSceneData::GetHadmapSceneList(int offset, int limit) {
  std::vector<std::string> paths;
  paths.push_back("hadmap_scene");
  // return GetFileList(paths, ".xml", offset, limit);
  return GetFileList(paths, ".json", offset, limit);
}

const wchar_t* CSceneData::SearchHadmapScene(const wchar_t* strHadmapSceneName, int nOffset, int nLimit) {
  std::string strSearchName = CEngineConfig::Instance().WStr2MBStr(strHadmapSceneName);
  // return SearchFile("hadmap_scene", ".xml", strSearchName, nOffset, nLimit);
  return SearchFile("hadmap_scene", ".json", strSearchName, nOffset, nLimit);
}

const wchar_t* CSceneData::GetParamSceneCount(uint64_t ullCount, uint64_t ullSize, uint64_t ullTime) {
  Json::Value root;
  root["count"] = Json::UInt64(ullCount);
  root["size"] = Json::UInt64(ullSize);
  root["time"] = Json::UInt64(ullTime);

  std::string strResult = root.toStyledString();

  m_wstrParamSceneCount.clear();
  m_wstrParamSceneCount = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_wstrParamSceneCount.c_str();
}

int CSceneData::ReleaseStr(const wchar_t* strContent) {
  if (!strContent) {
    return -1;
  }

  delete[] strContent;

  return 0;
}

int CSceneData::Convert2OpenScenario(const char* strSrcDir, const char* strDstDir) {
  XOSCConverter conv;
  return conv.ConverterToXOSC(strSrcDir, strDstDir);
}

const wchar_t* CSceneData::StopScenesProgess() {
  isStopScenesProgess = true;

  return L"{\"code\":0, \"message\": \"ok\"}";
}

const wchar_t* CSceneData::GetScenesProgessVal() {
  Json::Value root;
  root["code"] = 0;
  root["message"] = std::to_string(dScenesProgessVal);
  std::string strMessage = root.toStyledString();

  strWMessageProgess = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());
  return strWMessageProgess.c_str();
}

const wchar_t* CSceneData::StopGenScenesProgess() {
  CGeneratorV2::isStopGenScenesProgess = true;

  return L"{\"code\":0, \"message\": \"ok\"}";
}

const wchar_t* CSceneData::GetGenScenesProgessVal() {
  Json::Value root;
  root["code"] = 0;
  root["message"] = std::to_string(CGeneratorV2::dGenScenesProgessVal);
  std::string strMessage = root.toStyledString();

  strGenWMessageProgess = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());
  return strGenWMessageProgess.c_str();
}

bool CSceneData::IncludeChinese(const char* str) {
  char c, c1;
  while (1) {
    c = *str++;
    c1 = *str;
    if (c == 0) break;  // 如果到字符串尾则说明该字符串没有中文字符
    if (c & 0x80)       // 如果字符高位为1且下一字符高位也是1则有中文字符
      if (c1 & 0x80) return true;
  }
  return false;
}

const wchar_t* CSceneData::StartExe(const wchar_t* wstrparam) {
  std::string strJson = CEngineConfig::Instance().WStr2MBStr(wstrparam);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson.c_str());
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    return L"{\"code\":-1, \"message\": \"dynamic delete error\"}";
  }
  // from json to cmd string
  auto make_cmd_string = [](std::string name, std::string param) {
    std::string ret = "--";
    ret += name;
    ret += "=";
    ret += param;
    ret += ",";
    return ret;
  };

  std::string str_cmd = "";
  // --input_mode
  str_cmd += make_cmd_string("input_mode", "excel");
  // --input_data
  std::string strFilePath = root.get("filePath", "").asString();
  str_cmd += make_cmd_string("input_data", strFilePath);
  // --pathdir_output
  boost::filesystem::path output_path = CEngineConfig::Instance().getTadsimDir();
  str_cmd += make_cmd_string("pathdir_output", output_path.string());
  // --pathdir_catalogs
  // boost::filesystem::path catalog_path_data = CEngineConfig::Instance().getCataLogDir();
  // catalog_path_data = catalog_path_data.parent_path().parent_path();
  boost::filesystem::path catalog_path_sys = CEngineConfig::Instance().getSysCatalogsDir();
  str_cmd += make_cmd_string("pathdir_catalogs", catalog_path_sys.string());
  // --pathdir_hadmap
  if (root.get("mapType", 0).asInt()) {
    str_cmd += make_cmd_string("pathdir_hadmap", CEngineConfig::Instance().getHadmapDataDir());
  }
  // --producer_mode
  str_cmd += "--producer_mode=desktop";

  // 保存需要生成场景集名称,用于确定后生成场景集
  m_SemanticSetName = root.get("scenariosSet", "").asString();

  SYSTEM_LOGGER_INFO("cmd is %s", str_cmd.c_str());
  m_wstrStartExe = CEngineConfig::Instance().MBStr2WStr(str_cmd.c_str());
  return m_wstrStartExe.c_str();
}

const wchar_t* CSceneData::CreateCurrentSceneSet() {
  sTagEntityScenarioSet scenarioSet;
  scenarioSet.m_nID = 0;
  scenarioSet.m_strName = m_SemanticSetName;
  int nRet = CDatabase::Instance().CreateOneScenarioSet(scenarioSet);
  if (nRet != 0) {
    return L"{\"code\":-1, \"message\": \"Create One  ScenarioSet error\"}";
  }
  return L"{\"code\":0, \"message\": \"ok\"}";
}

const wchar_t* CSceneData::InsertScenes(const wchar_t* wstrparam) {
  std::string strJson = CEngineConfig::Instance().WStr2MBStr(wstrparam);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson.c_str());
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    return L"{\"code\":-1, \"message\": \"Insert scene error\"}";
  }
  sTagEntityScenarioSet scenarioSet;
  scenarioSet.m_strName = m_SemanticSetName;
  CDatabase::Instance().ScenarioSetByName(scenarioSet);
  sTagSimuTraffic sst;
  sTagEntityScenario scenario;
  scenario.m_strName = root.get("name", "").asString();
  scenario.m_strPath = root.get("path", "").asString();
  scenario.m_strMap = root.get("map", "").asString();
  scenario.m_strInfo = root.get("info", "").asString();
  scenario.m_strSets = "," + std::to_string(scenarioSet.m_nID) + ",";
  scenario.m_strLabel = "";
  scenario.m_strContent = "0";
  scenario.m_strType = root.get("type", "").asString();
  scenario.m_strTrafficType = root.get("traffictype", "").asString();
  // copy scene
  std::string suffix = ".xosc";
  if (suffix.length() < scenario.m_strPath.length() &&
      scenario.m_strPath.substr(scenario.m_strPath.length() - suffix.length()).compare(suffix) != 0) {
    scenario.m_strPath += suffix;
  }
  boost::filesystem::path source_file = scenario.m_strPath;
  boost::filesystem::path target_directory = CEngineConfig::Instance().getSceneDataDir();
  target_directory /= source_file.filename();
  SYSTEM_LOGGER_INFO("source_file: ", scenario.m_strPath.c_str());
  if (boost::filesystem::exists(source_file)) {
    boost::filesystem::copy_file(source_file, target_directory, BOOST_COPY_OPTION);
    scenario.m_strPath = target_directory.string();
    int nRet = CDatabase::Instance().CreateOneScenario(scenario, scenarioSet.m_nID);
    if (nRet != 0) {
      return L"{\"code\":-1, \"message\": \"Insert scene error\"}";
    }
    SYSTEM_LOGGER_INFO("Insert scene success");
    return L"{\"code\":0, \"message\": \"ok\"}";
  }
  return L"{\"code\":-1, \"message\": \"Insert scene error\"}";
}

const wchar_t* CSceneData::InsertOneMap(const wchar_t* strParam) {
  std::string strJson = CEngineConfig::Instance().WStr2MBStr(strParam);
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING errs;
  Json::CharReader* reader = builder.newCharReader();
  Json::Value root;
  int nLen = strlen(strJson.c_str());
  if (!reader->parse(strJson.c_str(), strJson.c_str() + nLen, &root, &errs)) {
    return L"{\"code\":-1, \"message\": \"dynamic param error\"}";
  }
  //
  CMapFile mapfile;
  boost::filesystem::path hadmapDestPath = root.get("map_path", "").asString();
  int nRet = mapfile.ParseMapV2(hadmapDestPath.string().c_str(), false);
  mapfile.Disconect();
  if (nRet != 0) {
    SYSTEM_LOGGER_ERROR("open hadmap %s error", hadmapDestPath.string().c_str());
    return L"{\"code\":-1, \"message\": \"connot to open map\"}";
  }
  int nIndex = 0;
  tagHadmapAttr attr;
  attr.m_name = hadmapDestPath.filename().string();
  attr.m_unRealLevelIndex = nIndex;
  attr.m_strUnrealLevelIndex = std::to_string(nIndex);
  attr.m_dRefLon = mapfile.m_center[0];
  attr.m_dRefLat = mapfile.m_center[1];
  attr.m_dRefAlt = mapfile.m_center[2];
  attr.m_nPreset = 0;
  attr.m_strPreset = "0";
  std::string str;
  std::stringstream ss1, ss2, ss3;
  ss1 << std::fixed << std::setprecision(8) << attr.m_dRefAlt;
  attr.m_strRefAlt = ss1.str();
  attr.m_dRefAlt = stod(attr.m_strRefAlt);

  ss2 << std::fixed << std::setprecision(8) << attr.m_dRefLon;
  attr.m_strRefLon = ss2.str();
  attr.m_dRefLon = stod(attr.m_strRefLon);

  ss3 << std::fixed << std::setprecision(8) << attr.m_dRefLat;
  attr.m_strRefLat = ss3.str();
  attr.m_dRefLat = stod(attr.m_strRefLat);
  SYSTEM_LOGGER_INFO("Insert map name: %s", hadmapDestPath.filename().string().c_str());
  CEngineConfig::Instance().UpdateHadmapAttr(hadmapDestPath.filename().string(), attr);
  std::wstring wstrName = CEngineConfig::Instance().MBStr2WStr(hadmapDestPath.filename().string().c_str());
  CMapDataCache::Instance().RemoveHadmapCacheAll(wstrName.c_str());
  return L"{\"code\":0, \"message\": \"ok\"}";
}

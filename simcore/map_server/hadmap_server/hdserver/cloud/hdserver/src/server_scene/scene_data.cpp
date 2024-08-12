/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scene_data.h"

#include <exception>
#include <sstream>

#include <json/value.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
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
#include "../xml_parser/entity/parser_json.h"
#include "../xml_parser/entity/scenario_set_params.h"
#include "../xml_parser/entity/scene_params.h"
#include "../xml_parser/entity/simulation.h"
#include "common/log/system_logger.h"
#include "common/utils/xsd_validator.h"
#include "common/xml_parser/entity/catalog.h"
#include "common/xml_parser/entity/parser_tools.h"
#include "common/xml_parser/xosc/xosc_converter.h"
#include "common/xml_parser/xosc_replay/xosc_replay_converter.h"
#include "scene_catalog_data.h"
#include "scene_data_proxy.h"

// #include <regex>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/regex.hpp>

CSceneData& CSceneData::Instance() {
  static CSceneData instance;

  return instance;
}

const wchar_t* CSceneData::GetSceneData(const wchar_t* wstrSceneName) {
  if (!wstrSceneName) {
    return L"{\"err\":-1, \"message\": \"scenen name null\"}";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);
  // std::string strImageDir = CEngineConfig::Instance().ImageFileDir();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;

  SYSTEM_LOGGER_INFO("load scene %s ", strSceneName.c_str());

  sTagEntityScenario scenario;
  scenario.m_strName = strSceneName;
  std::string strJson = jParser.XmlToJson(strResourceDir.c_str(), "scene", scenario);

  m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());

  return m_wstrSceneContent.c_str();
}

const wchar_t* CSceneData::GetTemplateData(const wchar_t* wstrTemplateName) {
  if (!wstrTemplateName) {
    return L"{\"err\":-1, \"message\": \"template name null\"}";
  }

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
    return L"{\"err\":-1, \"message\": \"scenen name null\"}";
  }

  if (!wstrSceneContent) {
    return L"{\"err\":-1, \"message\": \"scenen content null\"}";
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
      return L"{\"err\":-1, \"message\": \"parse save scene data error!\"}";
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

  return L"{\"err\":-1, \"message\": \"scene save error\"}";
}

const wchar_t* CSceneData::ParamScene(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  if (!wstrSceneName) {
    return LR"({"err":-1, "message": "scene name null"})";
  }

  if (!wstrParam) {
    return LR"({"err":-1, "message": "scene param null"})";
  }

  std::string strSceneName = CEngineConfig::Instance().WStr2MBStr(wstrSceneName);

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("parse scene param  %s", strParam.c_str());

  try {
    sTagBatchSceneParam param;

    int nRet = jParser.ParseSceneParam(strParam.c_str(), param);

    if (nRet != 0) {
      return LR"({"err":-1, "message": "parse param error"})";
    }

    sTagEntityScenario scenario;
    scenario.m_nID = atoi(strSceneName.c_str());
    nRet = CDatabase::Instance().Scenario(scenario);
    if (nRet != 0) {
      return LR"({"err":-1, "message": "get scene name error"})";
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
      if (lScenarioCount > 1000 || lScenarioSetCount > 1000) {
        return LR"({"err":-1, "message": "scenario count exceed max number!"})";
      }

      std::string strFailedFiles = "";
      std::string strExistFiles = "";
      nRet = ExportInBatchGeneratedScenes(strGeneratedDirName.c_str(), strExistFiles, strFailedFiles);
      if (strExistFiles.size() > 0 || strFailedFiles.size() > 0) {
        std::string strMessage = R"({"code": -1, )";
        strMessage.append(R"("message":"scene: ")" + strFailedFiles + R"(" export failed!, scene: ")" + strExistFiles +
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

  return LR"({"err":-1, "message": "parse param error"})";
}

const wchar_t* CSceneData::ParamSceneCount(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  if (!wstrSceneName) {
    return L"{\"err\":-1, \"message\": \"scene name null\"}";
  }

  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"scene param null\"}";
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
      return L"{\"err\":-1, \"message\": \"parse param error\"}";
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

  return L"{\"err\":-1, \"message\": \"parse param error\"}";
}

const wchar_t* CSceneData::GetSceneDataByParam(const wchar_t* wstrParam) {
  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"scenen param is null\"}";
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;
  sTagEntityScenario scenario;

  try {
    int nRet = jParser.ParseScenarioGetParams(strParam.c_str(), scenario);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"scene param parse error!\"}";
    }

    if (scenario.m_nID < 0) {
      return L"{\"err\":-1, \"message\": \"scenen param is null\"}";
    }

    if (CDatabase::Instance().ScenarioExist(scenario.m_nID)) {
      CDatabase::Instance().Scenario(scenario);

      SYSTEM_LOGGER_INFO("load scene %s ", scenario.m_strName.c_str());

      std::string strJson = jParser.XmlToJson(strResourceDir.c_str(), "scene", scenario);

      m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());

      return m_wstrSceneContent.c_str();
    } else {
      return L"{\"err\":-1, \"message\": \"scene not exist!\"}";
    }
  } catch (std::exception const& e) {
    SYSTEM_LOGGER_FATAL("GetSceneDataByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"get scene by id error!\"}";
}

const wchar_t* CSceneData::SaveSceneDataByParam(const wchar_t* wstrParam) {
  if (!wstrParam) {
    return L"{\"err\":-1, \"message\": \"scene param is null\"}";
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParam);

  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("start save scene file %s", strParam.c_str());

  try {
    sTagEntityScenario scenario;
    int nRet = jParser.ParseScenarioCreateParams(strParam.c_str(), scenario);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"scene param parse error!\"}";
    }

    sTagSimuTraffic scene;
    nRet = jParser.JsonToXml(scenario.m_strContent.c_str(), scene);
    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"parse save scene data error!\"}";
    }

    // SensorV2
    CCataLog catalog;
    catalog.parseDriverXML(CEngineConfig::Instance().getCataLogDir());
    for (auto& dirve : catalog.getDriveCatalogList()) {
      if (dirve.m_strName == scene.m_simulation.m_planner.m_strName) {
        scene.m_simulation.m_planner.m_sensorId = dirve.m_SensorGroup;
      }
    }

    scene.m_simulation.m_strInfo = scenario.m_strInfo;
    nRet = jParser.SaveToFile(strResourceDir.c_str(), scenario.m_strName.c_str(), scene, scenario.m_strPath);

    if (scenario.m_nID < 0) {
      bool bExist = CDatabase::Instance().ScenarioExist(scenario.m_strName.c_str());
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

    SYSTEM_LOGGER_INFO("scene file %s saved.", scenario.m_strName.c_str());

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

  return L"{\"err\":-1, \"message\": \"scene save error\"}";
}

const std::wstring CSceneData::OpenScenarioImpl(const wchar_t* ws_param) {
  if (!ws_param) {
    return MakeRspWithCodeMessageData(-1, "param is null");
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(ws_param);
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;
  sTagEntityScenario scenario;

  try {
    int nRet = jParser.ParseScenarioGetParams(strParam.c_str(), scenario);
    if (nRet != 0) {
      return MakeRspWithCodeMessageData(-1, "scene param parse error");
    }

    if (scenario.m_nID < 0) {
      return MakeRspWithCodeMessageData(-1, "scene param[id] is null");
    }

    std::string hadmap_name = computeMapCompleteName(scenario.m_strMapName, scenario.m_strMapVersion);
    boost::filesystem::path hadmap_path = strResourceDir;
    hadmap_path = (hadmap_path / "hadmap" / hadmap_name);
    if (CSceneDataProxy::Instance().Download(scenario.m_strCosBucket, scenario.m_strMapUrl, hadmap_path.string(),
                                             false)) {
      return MakeRspWithCodeMessageData(-1, "map file not exist");
    }

    if (CSceneDataProxy::Instance().DownloadScenario(scenario) != 0) {
      return MakeRspWithCodeMessageData(-1, "download scenario file error, maybe the key does not exist");
    }
    std::string strJson = jParser.XmlToJson(strResourceDir.c_str(), "scene", scenario);
    Json::Reader reader;
    Json::Value data;
    reader.parse(strJson, data);
    return MakeRspWithCodeMessageData(0, "ok", data);
  } catch (std::exception const& e) {
    SYSTEM_LOGGER_FATAL("OpenScenarioImpl exception: %s", e.what());
  }

  return MakeRspWithCodeMessageData(-1, "open scenario error");
}

const std::wstring CSceneData::SaveScenarioImpl(const wchar_t* ws_param) {
  if (!ws_param) {
    return MakeRspWithCodeMessageData(-1, "param is null");
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(ws_param);
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;
  sTagEntityScenario scenario;

  SYSTEM_LOGGER_INFO("start save scene file %s", strParam.c_str());

  try {
    int nRet = jParser.ParseScenarioCreateParams(strParam.c_str(), scenario);
    if (nRet != 0) {
      return MakeRspWithCodeMessageData(-1, "scene param parse error");
    }

    std::string hadmap_name = computeMapCompleteName(scenario.m_strMapName, scenario.m_strMapVersion);
    boost::filesystem::path hadmap_path = strResourceDir;
    hadmap_path = (hadmap_path / "hadmap" / hadmap_name);
    if (CSceneDataProxy::Instance().Download(scenario.m_strCosBucket, scenario.m_strMapUrl, hadmap_path.string(),
                                             false)) {
      return MakeRspWithCodeMessageData(-1, "map file not exist");
    }

    sTagSimuTraffic scene;
    scene.m_simulation.m_mapFile.m_strMapfile = scenario.m_strMapName;
    scene.m_simulation.m_mapFile.m_strVersion = scenario.m_strMapVersion;
    // JsonToXml may modify map name
    nRet = jParser.JsonToXml(scenario.m_strContent.c_str(), scene);
    if (nRet != 0) {
      return MakeRspWithCodeMessageData(-1, "josn2xml error");
    }

    scene.m_simulation.m_strInfo = scenario.m_strInfo;
    nRet = jParser.SaveToFile(strResourceDir.c_str(), scenario.m_strName.c_str(), scene, scenario.m_strPath);
    if (nRet != 0) {
      return MakeRspWithCodeMessageData(-1, "save to file error");
    }

    if (CSceneDataProxy::Instance().UploadScenario(scenario) != 0) {
      return MakeRspWithCodeMessageData(-1, "upload scenario file error, maybe the connect is invalid");
    }

    if (!scene.m_simulation.m_strNavMeshFileName.empty()) {
      // upload ped tpl map
      std::string map_filename = boost::filesystem::path(scene.m_simulation.m_mapFile.m_strMapfile).filename().string();
      CSceneDataProxy::Instance().Upload(
          scenario.m_strCosBucket,
          (boost::filesystem::path(scenario.m_strMapUrl).parent_path() / map_filename).string(),
          (boost::filesystem::path(strResourceDir) / "hadmap" / map_filename).string());
    }

    Json::Value data;
    data["key"] = scenario.m_strCosKey;
    return MakeRspWithCodeMessageData(0, "ok", data);
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveScenarioImpl exception: %s", e.what());
  }

  return MakeRspWithCodeMessageData(-1, "save scenario error");
}

const std::wstring CSceneData::Xosc2SimImpl(const wchar_t* ws_param) {
  if (!ws_param) {
    return MakeRspWithCodeMessageData(-1, "param is null");
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(ws_param);
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;
  sTagEntityScenario scenario;

  try {
    int nRet = jParser.ParseScenarioGetParams(strParam.c_str(), scenario);
    if (nRet != 0 || scenario.m_nID < 0) {
      return MakeRspWithCodeMessageData(-1, "scene param parse error");
    }

    std::string hadmap_name = computeMapCompleteName(scenario.m_strMapName, scenario.m_strMapVersion);
    boost::filesystem::path hadmap_path = strResourceDir;
    hadmap_path = (hadmap_path / "hadmap" / hadmap_name);

    if (CSceneDataProxy::Instance().Download(scenario.m_strCosBucket, scenario.m_strMapUrl, hadmap_path.string(),
                                             false)) {
      return MakeRspWithCodeMessageData(-1, "map file not exist");
    }

    if (CSceneDataProxy::Instance().DownloadScenario(scenario) != 0) {
      return MakeRspWithCodeMessageData(-1, "download scenario file error, maybe the key does not exist");
    }

    std::string error = jParser.XoscToSim(strResourceDir.c_str(), "scene", scenario);
    if (!error.empty()) {
      return MakeRspWithCodeMessageData(-1, error);
    }

    if (CSceneDataProxy::Instance().UploadScenario(scenario) != 0) {
      return MakeRspWithCodeMessageData(-1, "upload scenario file error, maybe the connect is invalid");
    }

    return MakeRspWithCodeMessageData(0);
  } catch (std::exception const& e) {
    SYSTEM_LOGGER_FATAL("Xosc2SimImpl exception: %s", e.what());
  }

  return MakeRspWithCodeMessageData(-1, "trans scenario error");
}

const std::wstring CSceneData::Xosc2SimrecImpl(const wchar_t* ws_param) {
  if (!ws_param) {
    return MakeRspWithCodeMessageData(-1, "param is null");
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(ws_param);
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  CParserJson jParser;
  sTagEntityScenario scenario;

  try {
    int nRet = jParser.ParseScenarioGetParams(strParam.c_str(), scenario);
    if (nRet != 0 || scenario.m_nID < 0) {
      return MakeRspWithCodeMessageData(-1, "scene param parse error");
    }

    std::string hadmap_name = computeMapCompleteName(scenario.m_strMapName, scenario.m_strMapVersion);
    boost::filesystem::path hadmap_path = strResourceDir;
    hadmap_path = (hadmap_path / "hadmap" / hadmap_name);

    if (CSceneDataProxy::Instance().Download(scenario.m_strCosBucket, scenario.m_strMapUrl, hadmap_path.string(),
                                             false)) {
      return MakeRspWithCodeMessageData(-1, "map file not exist");
    }

    if (CSceneDataProxy::Instance().DownloadScenario(scenario, false) != 0) {
      return MakeRspWithCodeMessageData(-1, "download scenario file error, maybe the key does not exist");
    }
    utils::Status status;
    XOSCReplayConverter::Options opts;
    opts.map_filename = hadmap_name;
    auto frame = boost::filesystem::path(strResourceDir) / "scene" / scenario.m_strName;
    bool is_frame = boost::filesystem::path(frame).extension().string() == ".frames";
    if (is_frame) {
      opts.frame_interval = 0.1;
      status = XOSCReplayConverter{}.ParseFromSimlog(frame.string(), opts);
      if (!status.ok()) {
        return MakeRspWithCodeMessageData(-1, std::string("convert simlog to xosc error: ") + status.ToString());
      }
    }
    auto xosc = frame.parent_path() / (frame.stem().string() + ".xosc");
    status = XOSCReplayConverter{}.SerializeToSimrec(xosc.string(), opts);
    if (!status.ok()) {
      return MakeRspWithCodeMessageData(-1, std::string("convert xosc to sim + simrec error: ") + status.ToString());
    }

    scenario.m_strName = boost::filesystem::path(scenario.m_strName).stem().string() + ".sim";
    if (CSceneDataProxy::Instance().UploadScenario(scenario) != 0) {
      return MakeRspWithCodeMessageData(-1, "upload scenario sim file error, maybe the connect is invalid");
    }
    if (is_frame) {
      scenario.m_strName = boost::filesystem::path(scenario.m_strName).stem().string() + ".xosc";
      if (CSceneDataProxy::Instance().UploadScenario(scenario) != 0) {
        return MakeRspWithCodeMessageData(-1, "upload scenario xosc file error, maybe the connect is invalid");
      }
    }

    return MakeRspWithCodeMessageData(0);
  } catch (const std::exception& e) {
    SYSTEM_LOGGER_FATAL("Xosc2SimImpl exception: %s, %s", e.what(), strParam.c_str());
  } catch (...) {
    SYSTEM_LOGGER_FATAL("Xosc2SimImpl unknown exception");
  }

  return MakeRspWithCodeMessageData(-1, "trans scenario error");
}

const std::wstring CSceneData::GetCatalogImpl(const char* param) {
  Json::Value result(Json::objectValue);
  auto status = SceneCatalogData{}.GetCatalogImpl(param, result);
  return MakeRspWithCodeMessageData(status.ok() ? 0 : -1, status.ToString(), result);
}

const std::wstring CSceneData::SetCatalogImpl(const char* param) {
  // 1. lock
  // 2. download
  // model/Catalogs/Vehicles/default.xosc
  // model/Catalogs/Sensors/SensorCatalog.xosc
  // model/Catalogs/Dynamics/dynamic_*.json
  // 3. rearrange
  // 4. upload
  auto status = SceneCatalogData{}.SetCatalogImpl(param);
  return MakeRspWithCodeMessageData(status.ok() ? 0 : -1, status.ToString());
}

int CSceneData::DeleteSingleScene(const char* strSceneName) {
  boost::filesystem::path scenePath = CEngineConfig::Instance().ResourceDir();
  scenePath.append("scene");

  boost::filesystem::path simPath = CSceneUtil::SceneFullPath(strSceneName);

  bool bRet = false;
  if (boost::filesystem::exists(simPath) && boost::filesystem::is_regular_file(simPath)) {
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

    // delete xosc file
    boost::filesystem::path trafficXOSCPath = scenePath;
    boost::filesystem::path trafficFileName = strTrafficFile;
    std::string strTrafficXOSCFileName = trafficFileName.stem().string() + ".xosc";
    trafficXOSCPath.append(strTrafficXOSCFileName);

    if (strTrafficXOSCFileName.length() > 5 && boost::filesystem::exists(trafficXOSCPath) &&
        boost::filesystem::is_regular_file(trafficXOSCPath)) {
      bRet = boost::filesystem::remove(trafficXOSCPath);
      if (!bRet) {
        SYSTEM_LOGGER_ERROR("remove osc file error!");
      }
    }

    // delete sim file
    bRet = boost::filesystem::remove(simPath);
  }

  if (bRet) {
    return 0;
  }

  return -1;
}

int CSceneData::CopySingleScene(const char* strSceneName, const char* strSrcDir, const char* strDestDir,
                                std::string& strDestPath) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;
  strDestPath.clear();

  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single scene dest dir not exist");
    return HSEC_DestDirNotExist;
  }

  boost::filesystem::path scenePath = srcPath;
  std::string strFullName = CSceneUtil::SceneFullName(strSceneName);
  boost::filesystem::path simPath = CSceneUtil::ScenePath(strSrcDir, strSceneName);

  if (boost::filesystem::exists(simPath) && boost::filesystem::is_regular_file(simPath)) {
    CSimulation sim;
    int nRet = GetRelatedFiles(simPath.string().c_str(), sim);
    if (nRet) {
      SYSTEM_LOGGER_ERROR("get scene traffic file name error!");
      return HSEC_SimFileFormatError;
    }

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
      } catch (std::exception e) {
        SYSTEM_LOGGER_ERROR("copy traffic file %s error!", strTrafficFile.c_str());
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
        std::string strFileName = (*itr).path().stem().string();
        std::string strExt = (*itr).path().extension().string();
        // if ((!boost::filesystem::is_directory(*itr)) && strExt == strInExt)
        if ((!boost::filesystem::is_directory(*itr)) && CSceneUtil::ValidFileFormat(strExt.c_str(), strInExt.c_str())) {
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

const wchar_t* CSceneData::SyncScenarioDBFromDisk() {
  CSynchronizer::Instance().SyncDiskToDB();

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
    return L"{\"err\":-1, \"message\": \"scene list param is null\"}";
  }

  std::string strParam = CEngineConfig::Instance().WStr2MBStr(wstrParams);

  CParserJson jParser;

  SYSTEM_LOGGER_INFO("start get scene list %s", strParam.c_str());

  try {
    sTagGetScenarioListParam param;
    int nRet = jParser.ParseScenarioListParams(strParam.c_str(), param);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"scene param parse error!\"}";
    }

    CScenarioEntities ses;

    nRet = CDatabase::Instance().ScenarioList(param, ses);

    if (nRet != 0) {
      return L"{\"err\":-1, \"message\": \"get scenario list error!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetSceneListByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"get scenario list error\"}";
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
      return L"{\"err\":-1, \"message\": \"scenario info list error!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetScenesInfoByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"get scenario list error\"}";
}

int CSceneData::GetRelatedFiles(const char* strSimFile, CSimulation& sim) { return sim.Parse(strSimFile, false); }

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
  int nRet = DeleteSingleScene(strFileName.c_str());
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
      return L"{\"err\":-1, \"message\": \"scenario search failed!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetScenesInfoByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"get scenario info list error\"}";
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
      return L"{\"err\":-1, \"message\": \"scenario info list error!\"}";
    }

    std::string strResult = ses.ToJson();

    m_wstrSceneList.clear();
    m_wstrSceneList = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrSceneList.c_str();
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("GetScenesInfoByParam exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"get scenario info list error\"}";
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
    int nRet = DeleteSingleScene(entity.m_strName.c_str());
    if (nRet != 0) {
      continue;
    }

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
    return LR"({"code":-1, "message": "parse download param error!" })";
  }

  boost::filesystem::path srcPath = CEngineConfig::Instance().ResourceDir();
  srcPath /= "scene";

  std::vector<std::string>::iterator itr = param.m_sceneNames.begin();
  std::string strDestPath = "";
  for (; itr != param.m_sceneNames.end(); ++itr) {
    CopySingleScene(itr->c_str(), srcPath.string().c_str(), param.m_strDestDir.c_str(), strDestPath);
  }

  return LR"({"code": 0, "message": "ok"})";
}

const wchar_t* CSceneData::DownloadScenesV2(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("download scenes %s ", strParams.c_str());

  CParserJson jParser;
  sTagScenesDownloadParams param;
  int ret = jParser.ParseScenesDownloadParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message":"parse download param error!"})";
  }

  boost::filesystem::path srcPath = CEngineConfig::Instance().ResourceDir();
  srcPath /= "scene";

  std::vector<std::string>::iterator itr = param.m_sceneNames.begin();
  std::string strDestPath = "";
  std::string strErrorScene = "";
  for (; itr != param.m_sceneNames.end(); ++itr) {
    sTagEntityScenario scenario;
    scenario.m_nID = atoi(itr->c_str());
    CDatabase::Instance().Scenario(scenario);
    if (scenario.m_strName.size() > 0) {
      ret = CopySingleScene(scenario.m_strName.c_str(), srcPath.string().c_str(), param.m_strDestDir.c_str(),
                            strDestPath);
      if (ret != 0) {
        strErrorScene += (scenario.m_strName + ",");
      }
    } else {
      strErrorScene += ((*itr) + ",");
    }
  }

  if (strErrorScene.size() > 0) {
    std::string strMessage = R"({"code": -1, )";
    strMessage.append(R"("message": "scene:)" + strErrorScene + R"( download failed!"})");
    m_wstrDownloadSceneResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());
  } else {
    m_wstrDownloadSceneResp = LR"({"code":0, "message": "ok"})";
  }

  return m_wstrDownloadSceneResp.c_str();
}

const wchar_t* CSceneData::UploadScenes(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("upload scenes %s ", strParams.c_str());

  CParserJson jParser;
  sTagScenesUploadParams param;
  int ret = jParser.ParseScenesUploadParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message":"parse upload param error!"})";
  }

  std::vector<boost::filesystem::path> files;
  boost::filesystem::path srcPath = param.m_strSourceDir;

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
          files.push_back((*itr).path());
        }
      }
    }
  } else if (boost::iequals(param.m_strType, "files")) {
    std::vector<std::string>::iterator itr = param.m_sceneNames.begin();
    for (; itr != param.m_sceneNames.end(); ++itr) {
      boost::filesystem::path p = srcPath;
      p /= (*itr);

      std::string strExt = p.extension().string();
      if ((!boost::filesystem::is_directory(p)) && boost::algorithm::iequals(strExt, ".sim")) {
        files.push_back(p);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload scenes param error!");
  }

  boost::filesystem::path destPath = CEngineConfig::Instance().ResourceDir();
  destPath /= "scene";

  std::string strDestPath = "";
  int nRet = 0;

  std::string strFailedScenes = "";
  std::vector<std::string> ExistedFiles;
  std::vector<boost::filesystem::path>::iterator itr = files.begin();
  for (; itr != files.end(); ++itr) {
    std::string strName = itr->filename().string();
    std::string strStem = itr->stem().string();

    // 判断是否同名覆盖
    if (CDatabase::Instance().ScenarioExist(strStem.c_str())) {
      ExistedFiles.push_back(strStem);
      continue;
    }

    nRet = CopySingleScene(strName.c_str(), srcPath.string().c_str(), destPath.string().c_str(), strDestPath);
    if (nRet == 0) {
      CSimulation sim;
      sim.Parse(strDestPath.c_str(), false);
      sTagEntityScenario scenario;
      scenario.m_strName = strStem;
      scenario.m_strPath = strDestPath;

      boost::filesystem::path mapPath = sim.m_mapFile.m_strMapfile;
      scenario.m_strMap = mapPath.filename().string();
      scenario.m_strInfo = sim.m_strInfo;
      scenario.m_strLabel = "";
      scenario.m_strSets = ",1,";
      scenario.m_strContent = "";

      CDatabase::Instance().CreateOneScenario(scenario);
    } else {
      strFailedScenes.append(strName);
      strFailedScenes.append(",");
    }
  }

  if (strFailedScenes.size() > 0) {
    std::string strMessage = R"({"code": -1, )";
    strMessage.append(R"("message":"scene: )" + strFailedScenes + R"( upload failed!"})");
    m_wstrUploadSceneResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());

  } else {
    m_wstrUploadSceneResp = LR"({"code": 0, "message":"ok"})";
  }

  return m_wstrUploadSceneResp.c_str();
}

int CSceneData::ExportInOneScene(boost::filesystem::path oneScenePath, std::string strScenarioSetID) {
  boost::filesystem::path destPath = CEngineConfig::Instance().ResourceDir();
  destPath /= "scene";

  std::string strDestPath = "";
  int nRet = 0;

  std::string strFailedScenes = "";

  std::string strName = oneScenePath.filename().string();
  std::string strStem = oneScenePath.stem().string();

  // 判断是否同名覆盖
  if (CDatabase::Instance().ScenarioExist(strStem.c_str())) {
    SYSTEM_LOGGER_WARN("exist the same name file %s, not export!", strStem.c_str());
    return HSEC_COPY_SCENE_FILE_EXIST;
  }

  boost::filesystem::path srcPath = oneScenePath.parent_path();
  nRet = CopySingleScene(strName.c_str(), srcPath.string().c_str(), destPath.string().c_str(), strDestPath);
  if (nRet == 0) {
    CSimulation sim;
    sim.Parse(strDestPath.c_str(), false);
    sTagEntityScenario scenario;
    scenario.m_strName = strStem;
    scenario.m_strPath = strDestPath;

    boost::filesystem::path mapPath = sim.m_mapFile.m_strMapfile;
    scenario.m_strMap = mapPath.filename().string();
    scenario.m_strInfo = sim.m_strInfo;
    scenario.m_strLabel = "";
    scenario.m_strSets = "," + strScenarioSetID + ",";
    scenario.m_strContent = "";

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
  boost::filesystem::path srcPath = CEngineConfig::Instance().ResourceDir();
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
    if ((!boost::filesystem::is_directory(*itr)) && strExt == ".sim") {
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
    return L"{\"err\":-1, \"message\": \"global sensor config content null\"}";
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

  return L"{\"err\":-1, \"message\": \"save global sensor data error\"}";
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
    return L"{\"err\":-1, \"message\": \"global sensor config content null\"}";
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

  return L"{\"err\":-1, \"message\": \"save global environment data error\"}";
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

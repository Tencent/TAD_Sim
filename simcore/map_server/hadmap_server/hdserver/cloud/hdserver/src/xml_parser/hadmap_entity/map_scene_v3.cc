/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_scene_v3.h"
#include "cross.h"
#include "curve_road.h"
#include "entity_link.h"
#include "group_entity.h"
#include "straight_road.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <sstream>
#include "../../engine/config.h"
#include "common/log/system_logger.h"
/*
#include <tx_od_log.h>
#include <tx_od_op_interface.h>
#include <tx_od_public.h>
#include <OpenDRIVE_1.4H.hxx>
*/

CMapSceneV3::CMapSceneV3() {}

CMapSceneV3::~CMapSceneV3() {}

void CMapSceneV3::SetId(std::string strId) { m_strId = strId; }

void CMapSceneV3::SetDesc(std::string strDesc) { m_strDesc = strDesc; }

void CMapSceneV3::SetName(std::string strName) { m_strName = strName; }

void CMapSceneV3::SetGeometry(std::string strGeometry) { m_strGeometry = strGeometry; }

void CMapSceneV3::SetData(std::string strData) { m_strData = strData; }

int CMapSceneV3::SaveToJson(const char* strSceneName, const char* strData, const char* strGeometry) {
  // LOG(INFO) << "map scene save to xml";
  SYSTEM_LOGGER_INFO("map scene save to json");

  if (!strSceneName || !strData) {
    return -1;
  }

  // 生成文件名，并保存到相应目录
  // hadmap_scene/
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path initPath = strResourceDir;
  boost::filesystem::path hadmapScene = initPath;
  hadmapScene.append("hadmap_scene");
  boost::filesystem::path hadmapGeometry = initPath;
  hadmapGeometry.append("hadmap_geometry");
  boost::filesystem::path pSceneName = strSceneName;

  std::string strExt = pSceneName.extension().string();
  if (strExt.size() == 0 || !boost::algorithm::iequals(strExt, ".json")) {
    std::string strScene = strSceneName;
    strScene.append(".json");
    hadmapScene.append(strScene);
    hadmapGeometry.append(strScene);
  }

  std::ofstream file(hadmapScene.string());
  if (file.is_open()) {
    file << strData;
    file.close();
  }

  std::ofstream geometryFile(hadmapGeometry.string());
  if (geometryFile.is_open()) {
    geometryFile << strGeometry;
    geometryFile.close();
  }

  return 0;
}

std::string CMapSceneV3::ParseFromJson(const char* strSceneName) {
  std::fstream fs(strSceneName);

  std::string strData = "";
  std::string strLine;
  while (fs >> strLine) {
    strData.append(strLine);
  }

  return strData;
}

int CMapSceneV3::SaveOpenDrive(const char* strSceneName, const char* strPreset, const char* strData) {
  SYSTEM_LOGGER_INFO("map scene save to opendrive");

  if (!strSceneName || !strData) {
    return -1;
  }

  // 生成文件名，并保存到相应目录
  // hadmap_editor/
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path initPath = strResourceDir;
  boost::filesystem::path destPath = initPath;
  destPath.append("hadmap");
  boost::filesystem::path pSceneName = strSceneName;

  std::string strExt = pSceneName.extension().string();
  std::string strScene = strSceneName;
  if (strExt.size() == 0 || !boost::algorithm::iequals(strExt, ".xodr")) {
    strScene.append(".xodr");
    // hadmapEditor.append(strScene);
  }
  boost::filesystem::path hadmap = destPath;
  hadmap.append(strScene);
  std::string strName = hadmap.string();

  boost::filesystem::path hadmapDestPath = strName;
  std::string strstem = hadmapDestPath.stem().string();
  std::string strextension = hadmapDestPath.extension().string();
  std::string strFullName = strScene;
  std::string str = strPreset;
  if (boost::iequals(str, "true") && strextension == ".xodr") {
    while (boost::filesystem::exists(hadmapDestPath) && boost::filesystem::is_regular_file(hadmapDestPath)) {
      hadmapDestPath = destPath;
      strstem = strstem + "(1)";
      strFullName = strstem + ".xodr";
      hadmapDestPath /= strFullName;
    }
  }
  int nRet = CMapSceneV3::SaveToOpenDrive(hadmapDestPath.string().c_str(), strData);

  if (nRet != 0) {
    return -1;
  }

  // return CopyHadmap(strName.c_str());
  boost::filesystem::path targetpath = strFullName;
  AddToConfigFile(hadmapDestPath);
  return 0;
}

int CMapSceneV3::SaveToOpenDrive(const char* strSceneName, const char* strData) {
  SYSTEM_LOGGER_INFO("SaveToOpenDrive");

  if (!strSceneName || !strData) {
    return -1;
  }

  std::string strTmp = strData;
  return CEngineConfig::Instance().SaveToXodr(strSceneName, strTmp);
}

std::string CMapSceneV3::ParseFromOpenDrive(const char* strSceneName) {
  std::string strResult = "";
  CEngineConfig::Instance().ParseFromXodr(strSceneName, strResult);
  return strResult;

  /*
  using namespace odr_engine;

  ODR::OdrHdr HODR_open = nullptr;
  std::string strData = "";

  if (ODR::OdrErrCode::succ == ODR::openOdr(strSceneName, &HODR_open))
  {
          if (ODR::OdrErrCode::succ == ODR::checkOdrHander(HODR_open))
          {
                  if (ODR::OdrErrCode::succ == ODR::readRoadNetworkElement(HODR_open, strData))
                  {
                          if (ODR::OdrErrCode::err == ODR::closeOdr(HODR_open))
                          {
                                  SYSTEM_LOGGER_ERROR( "close odr %s failure.", strSceneName);
                                  return "";
                          }
                  }
                  else
                  {
                          SYSTEM_LOGGER_ERROR("write %s xodr data failure.", strSceneName);
                          return "";
                  }
          }
          else
          {
                  SYSTEM_LOGGER_ERROR( "check odr %s failure.", strSceneName);
                  return "";
          }
  }
  else
  {
          SYSTEM_LOGGER_ERROR("open odr %s failure.", strSceneName);
          return "";
  }
  return "";
  */
}
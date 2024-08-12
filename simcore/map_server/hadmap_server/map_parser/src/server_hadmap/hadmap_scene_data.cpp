/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "hadmap_scene_data.h"
#include <json/value.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include "../engine/config.h"
#include "../engine/util/scene_util.h"
#include "../xml_parser/hadmap_entity/image_params.h"
#include "../xml_parser/hadmap_entity/map_scene.h"
#include "../xml_parser/hadmap_entity/map_scene_v2.h"
#include "../xml_parser/hadmap_entity/map_scene_v3.h"
#include "../xml_parser/hadmap_entity/parser_hadmap_json_v2.h"
#include "../xml_parser/hadmap_entity/parser_hadmap_json_v3.h"
#include "common/log/system_logger.h"
// #include <regex>
#include <boost/regex.hpp>
#include <exception>

CHadmapSceneData::CHadmapSceneData() { m_wstrSucceed = L"{\"err\": 0, \"message\": \"success\"}"; }

CHadmapSceneData& CHadmapSceneData::Instance() {
  static CHadmapSceneData instance;

  return instance;
}

const wchar_t* CHadmapSceneData::GetHadmapSceneData(const wchar_t* wstrHadmapSceneName) {
  if (!wstrHadmapSceneName) {
    return L"{\"err\":-1, \"message\": \"hadmap scenen name null\"}";
  }

  std::string strHadmapSceneName = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneName);
  // std::string strImageDir = CEngineConfig::Instance().ImageFileDir();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  boost::filesystem::path p = strResourceDir;
  boost::filesystem::path sceneDirPath = p;
  sceneDirPath.append("hadmap_scene");
  boost::filesystem::path xmlFilePath = sceneDirPath;
  xmlFilePath.append(strHadmapSceneName);
  std::string strExt = xmlFilePath.extension().string();

  std::string strStemName = xmlFilePath.stem().string();
  std::string strSimPath = "";
  if (strExt.size() == 0 || strExt != ".xml") {
    // std::string strName = strStemName + ".xml";
    std::string strName = strHadmapSceneName;
    strName.append(".xml");
    xmlFilePath = sceneDirPath;
    xmlFilePath.append(strName);
  }

  SYSTEM_LOGGER_INFO("load hadmap scene %s ", strHadmapSceneName.c_str());

  if (!boost::filesystem::exists(xmlFilePath)) {
    return L"{\"err\":-1, \"message\": \"hadmap scenen not exist\"}";
  }

  CMapScene* pScene = new CMapScene();
  pScene->ParseFromXml(xmlFilePath.string().c_str());

  CParserHadmapJson jParser;

  std::string strJson = jParser.ToJson(pScene);

  delete pScene;
  pScene = nullptr;

  m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strJson.c_str());

  return m_wstrSceneContent.c_str();
}

const wchar_t* CHadmapSceneData::SaveHadmapSceneData(const wchar_t* wstrHadmapSceneName,
                                                     const wchar_t* wstrHadmapSceneContent) {
  if (!wstrHadmapSceneName) {
    return L"{\"err\":-1, \"message\": \"hadmap scenen name null\"}";
  }

  if (!wstrHadmapSceneContent) {
    return L"{\"err\":-1, \"message\": \"hadmap scenen content null\"}";
  }

  std::string strHadmapSceneName = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneName);

  std::string strHadmapSceneContent = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneContent);

  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  CParserHadmapJson jParser;

  SYSTEM_LOGGER_INFO("start save hadmap scene file %s", strHadmapSceneName.c_str());
  // SYSTEM_LOGGER_DEBUG("save scene content %s", strSceneContent.c_str());

  try {
    CMapScene* pScene = jParser.Parse(strHadmapSceneContent.c_str());
    int nRet = -1;
    if (pScene) {
      nRet = pScene->SaveToXml(strHadmapSceneName.c_str());
      if (nRet == 0) {
        nRet = pScene->SaveToSqlite(strHadmapSceneName.c_str());
      }

    } else {
      SYSTEM_LOGGER_ERROR("scene file %s saved.", strHadmapSceneName.c_str());
    }

    SYSTEM_LOGGER_INFO("scene file %s saved.", strHadmapSceneName.c_str());

    if (nRet == 0) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveHadmapSceneData exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"hadmap scene save error\"}";
}

int CHadmapSceneData::DeleteHadmapSceneData(const wchar_t* wstrHadmapSceneName) {
  if (!wstrHadmapSceneName) {
    return -1;
  }
  std::string strHadmapSceneName = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneName);
  // std::string strImageDir = CEngineConfig::Instance().ImageFileDir();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  boost::filesystem::path p = strResourceDir;
  boost::filesystem::path sceneDirPath_json = p;
  boost::filesystem::path sceneDirPath_xodr = p;
  // sceneDirPath_xodr.append("hadmap_editor");
  boost::filesystem::path xmlFilePath = sceneDirPath_xodr.append("hadmap_editor"); /*sceneDirPath_xodr;*/
  xmlFilePath.append(strHadmapSceneName);                                          // xodr

  // std::string strExt = xmlFilePath.extension().string();
  std::string strStemName = xmlFilePath.stem().string();

  sceneDirPath_json.append("hadmap_scene");
  boost::filesystem::path jsonFilePath = sceneDirPath_json;
  std::string str = strStemName.append(".json");
  jsonFilePath.append(str);
  // delete
  bool bRet = false;
  if (boost::filesystem::exists(jsonFilePath) && boost::filesystem::is_regular_file(jsonFilePath) &&
      boost::filesystem::exists(xmlFilePath) && boost::filesystem::is_regular_file(xmlFilePath)) {
    // delete hadmap file
    if (boost::filesystem::remove(jsonFilePath) && boost::filesystem::remove(xmlFilePath)) {
      return 0;
    }
  }

  return -1;
}

const wchar_t* CHadmapSceneData::GetHadmapSceneDataV3(const wchar_t* wstrHadmapSceneName) {
  if (!wstrHadmapSceneName) {
    return L"{\"err\":-1, \"message\": \"Hadmap scene name null\"}";
  }

  std::string strHadmapSceneName = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneName);
  // std::string strImageDir = CEngineConfig::Instance().ImageFileDir();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  boost::filesystem::path p = strResourceDir;
  boost::filesystem::path sceneDirPath = p;
  // sceneDirPath.append("hadmap_scene");
  sceneDirPath.append("hadmap");
  // sceneDirPath.append("hadmap_editor");
  boost::filesystem::path xmlFilePath = CSceneUtil::HadmapFullPath(strHadmapSceneName.c_str());
  // xmlFilePath.append(strHadmapSceneName);

  // xmlFilePath=CSceneUtil::SceneFullPath(strHadmapSceneName);
  std::string strExt = xmlFilePath.extension().string();
  if (strExt != ".xodr") {
    // 地图编辑器  目前只支持xodr的打开读取可编辑
    return L"{\"err\":-1, \"message\": \"Hadmap formate is non-supported\"}";
  }
  SYSTEM_LOGGER_INFO("1load hadmap scene v3 %s ", xmlFilePath.string().c_str());

  if (!boost::filesystem::exists(xmlFilePath)) {
    return L"{\"err\":-1, \"message\": \"Hadmap scene not exist\"}";
  }

  if (0) {
    std::string strData = CMapSceneV3::ParseFromJson(xmlFilePath.string().c_str());
    CMapSceneV3 pScene;
    pScene.SetName(strHadmapSceneName);
    pScene.SetData(strData);
    CParserHadmapJsonV3 parser;
    std::string strContent = parser.ToJson(&pScene);
    m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strContent.c_str());
  } else {
    std::string strData = CMapSceneV3::ParseFromOpenDrive(xmlFilePath.string().c_str());
    CMapSceneV3 pScene;
    pScene.SetName(strHadmapSceneName);
    pScene.SetData(strData);
    CParserHadmapJsonV3 parser;
    std::string strContent = parser.ToJson(&pScene);
    m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strContent.c_str());
  }

  return m_wstrSceneContent.c_str();
}

const wchar_t* CHadmapSceneData::GetHadmapSceneDataV2(const wchar_t* wstrHadmapSceneName) {
  if (!wstrHadmapSceneName) {
    return L"{\"err\":-1, \"message\": \"hadmap scene name null\"}";
  }

  std::string strHadmapSceneName = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneName);
  // std::string strImageDir = CEngineConfig::Instance().ImageFileDir();
  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  boost::filesystem::path p = strResourceDir;
  boost::filesystem::path sceneDirPath = p;
  sceneDirPath.append("hadmap_scene");
  boost::filesystem::path xmlFilePath = sceneDirPath;
  xmlFilePath.append(strHadmapSceneName);
  std::string strExt = xmlFilePath.extension().string();

  std::string strStemName = xmlFilePath.stem().string();
  std::string strSimPath = "";
  if (strExt.size() == 0 || strExt != ".json") {
    // std::string strName = strStemName + ".xml";
    std::string strName = strHadmapSceneName;
    strName.append(".json");
    xmlFilePath = sceneDirPath;
    xmlFilePath.append(strName);
  }

  SYSTEM_LOGGER_INFO("load hadmap scene %s ", strHadmapSceneName.c_str());

  if (!boost::filesystem::exists(xmlFilePath)) {
    return L"{\"err\":-1, \"message\": \"hadmap scenen not exist\"}";
  }

  std::string strData = CMapSceneV2::ParseFromJson(xmlFilePath.string().c_str());
  CMapSceneV2 pScene;
  pScene.SetName(strHadmapSceneName);
  pScene.SetData(strData);
  CParserHadmapJsonV2 parser;
  std::string strContent = parser.ToJson(&pScene);

  m_wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strContent.c_str());

  return m_wstrSceneContent.c_str();
}

const wchar_t* CHadmapSceneData::SaveHadmapSceneDataV2(const wchar_t* wstrHadmapSceneContent) {
  if (!wstrHadmapSceneContent) {
    return L"{\"err\":-1, \"message\": \"hadmap scenen content null\"}";
  }

  std::string strHadmapSceneContent = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneContent);

  std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  CParserHadmapJsonV2 jParser;

  SYSTEM_LOGGER_INFO("start save hadmap scene file %s", strHadmapSceneContent.c_str());
  // SYSTEM_LOGGER_DEBUG("save scene content %s", strSceneContent.c_str());

  try {
    CMapSceneV2* pScene = jParser.ParseV2(strHadmapSceneContent.c_str());
    int nRet = -1;
    if (pScene) {
      nRet = pScene->SaveToJson(pScene->Name().c_str(), pScene->Data().c_str());
      if (nRet == 0) {
        nRet = pScene->SaveToSqlite(pScene->Name().c_str());
      }
    } else {
      SYSTEM_LOGGER_ERROR("scene file  saved failed.");
    }

    if (nRet == 0) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveHadmapSceneData exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"hadmap scene save error\"}";
}

const wchar_t* CHadmapSceneData::SaveHadmapSceneDataV3(const wchar_t* wstrHadmapSceneContent) {
  if (!wstrHadmapSceneContent) {
    return L"{\"err\":-1, \"message\": \"Hadmap scene v3 content null\"}";
  }

  std::string strHadmapSceneContent = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneContent);

  // std::string strResourceDir = CEngineConfig::Instance().ResourceDir();

  CParserHadmapJsonV3 jParser;

  SYSTEM_LOGGER_INFO("start save hadmap scene file v3 %s", strHadmapSceneContent.c_str());

  try {
    CMapSceneV3* _pScene = jParser.ParseV3(strHadmapSceneContent.c_str());
    std::unique_ptr<CMapSceneV3> pScene(_pScene);
    int nRet = -1;
    if (pScene) {
      // nRet = pScene->SaveToJson(pScene->Name().c_str(), pScene->Data().c_str(), pScene->Geometry().c_str());
      // if (nRet == 0){
      nRet = pScene->SaveOpenDrive(pScene->Name().c_str(), pScene->GetPreset().c_str(), pScene->Geometry().c_str());
      // }
    } else {
      SYSTEM_LOGGER_ERROR("save hadmap scene file v3 failed.");
    }

    SYSTEM_LOGGER_INFO("hadmap scene file %s saved v3.", pScene->Name().c_str());

    if (nRet == 0) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveHadmapSceneDataV3 exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"Hadmap scene v3 save error\"}";
}

int CHadmapSceneData::DeleteSingleGISImage(const char* strFileName) {
  boost::filesystem::path gisImagePath = CSceneUtil::GISImageFullPath(strFileName);

  bool bRet = false;
  if (boost::filesystem::exists(gisImagePath) && boost::filesystem::is_regular_file(gisImagePath)) {
    // delete hadmap file
    bRet = boost::filesystem::remove(gisImagePath);
    if (bRet) {
      return 0;
    }
  }

  return -1;
}

const wchar_t* CHadmapSceneData::deleteGISImage(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("delete gis image %s ", strParams.c_str());

  CParserHadmapJsonV2 jParser;
  sTagGISImageDeleteParams param;
  int ret = jParser.ParseGISImageDeleteParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'err':-1, 'message':'Parse gis delete param error!'}";
  }

  std::vector<std::string>::iterator itr = param.m_imageNames.begin();
  for (; itr != param.m_imageNames.end(); ++itr) {
    DeleteSingleGISImage(itr->c_str());
  }

  return m_wstrSucceed.c_str();
}

int CHadmapSceneData::DeleteSingleGISModel(const char* strFileName) {
  boost::filesystem::path gisImagePath = CSceneUtil::GISModelFullPath(strFileName);

  bool bRet = false;
  if (boost::filesystem::exists(gisImagePath) && boost::filesystem::is_regular_file(gisImagePath)) {
    // delete hadmap file
    bRet = boost::filesystem::remove(gisImagePath);
    if (bRet) {
      return 0;
    }
  }

  return -1;
}

const wchar_t* CHadmapSceneData::deleteGISModel(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("delete gis model %s ", strParams.c_str());

  CParserHadmapJsonV2 jParser;
  sTagGISModelDeleteParams param;
  int ret = jParser.ParseGISModelDeleteParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'err':-1, 'message':'Parse gis model delete param error!'}";
  }

  std::vector<std::string>::iterator itr = param.m_modelNames.begin();
  for (; itr != param.m_modelNames.end(); ++itr) {
    DeleteSingleGISModel(itr->c_str());
  }

  return m_wstrSucceed.c_str();
}

void CHadmapSceneData::FormResponseStr(std::vector<boost::filesystem::path>& files) {
  Json::Value root;
  root["code"] = 0;
  root["count"] = Json::Value::UInt64(files.size());
  int nCount = 0;

  if (files.size() == 0) {
    root["list"].resize(0);
  } else {
    std::vector<boost::filesystem::path>::iterator it = files.begin();
    Json::Value maps;
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

  std::string strResult = root.toStyledString();

  m_wstrSucceedUploadImages.clear();
  m_wstrSucceedUploadImages = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
}

int CHadmapSceneData::CopySingleGISImage(const char* strFileName, const char* strSrcDir, const char* strDestDir) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;

  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single gis image dest dir not exist");
    return -1;
  }

  std::string strFullName = CSceneUtil::GISImageFullName(strFileName);
  boost::filesystem::path gisImageSrcPath = CSceneUtil::GISImagePath(strSrcDir, strFullName.c_str());

  if (boost::filesystem::exists(gisImageSrcPath) && boost::filesystem::is_regular_file(gisImageSrcPath)) {
    // copy gis image file
    boost::filesystem::path gisImageDestPath = destPath;
    gisImageDestPath /= strFullName;

    try {
      boost::filesystem::copy_file(gisImageSrcPath, gisImageDestPath, BOOST_COPY_OPTION);
    } catch (std::exception e) {
      SYSTEM_LOGGER_ERROR("copy gis image file %s error!", strFullName.c_str());
      return -1;
    }
  }

  return 0;
}

const wchar_t* CHadmapSceneData::uploadGISImage(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("upload gis image %s ", strParams.c_str());

  CParserHadmapJsonV2 jParser;
  sTagGISImageUploadParams param;
  int ret = jParser.ParseGISImageUploadParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'err':-1, 'message': 'Parse gis image upload error'}";
  }

  std::vector<boost::filesystem::path> files;
  boost::filesystem::path srcPath = param.m_strSourceDir;
  boost::filesystem::path hadmapSrcPath = srcPath;

  if (boost::iequals(param.m_strType, "dir")) {
    // 路径存在
    if (boost::filesystem::exists(hadmapSrcPath)) {
      // 获取hadmap文件
      boost::filesystem::directory_iterator itr_end;
      boost::filesystem::directory_iterator itr(hadmapSrcPath);

      for (; itr != itr_end; ++itr) {
        std::string strName = (*itr).path().string();
        std::string strExt = (*itr).path().extension().string();
        if ((!boost::filesystem::is_directory(*itr)) && CSceneUtil::ValidGISImageFormat(strExt.c_str())) {
          files.push_back((*itr).path());
        }
      }
    }
  } else if (boost::iequals(param.m_strType, "files")) {
    std::vector<std::string>::iterator itr = param.m_imageNames.begin();
    for (; itr != param.m_imageNames.end(); ++itr) {
      boost::filesystem::path p = srcPath;
      p /= (*itr);

      std::string strExt = p.extension().string();
      if ((!boost::filesystem::is_directory(p)) && CSceneUtil::ValidGISImageFormat(strExt.c_str())) {
        files.push_back(p);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload gis images param error!");
  }

  boost::filesystem::path destPath = CEngineConfig::Instance().GISImageDir();

  // copy gis image file
  std::vector<boost::filesystem::path>::iterator itr = files.begin();
  // succeed files
  std::vector<boost::filesystem::path> succeed_files;

  for (; itr != files.end(); ++itr) {
    // copy file
    std::string strName = itr->filename().string();
    int nRet = CopySingleGISImage(strName.c_str(), hadmapSrcPath.string().c_str(), destPath.string().c_str());
    if (nRet == 0) {
      boost::filesystem::path gisImageDestPath = destPath;
      gisImageDestPath /= strName;
      succeed_files.push_back(gisImageDestPath);
    }
  }

  FormResponseStr(succeed_files);

  return m_wstrSucceedUploadImages.c_str();
}

int CHadmapSceneData::RenameSingleGISImage(const char* strFileName, const char* strNewFileName) {
  if (!strFileName || !strNewFileName || strlen(strFileName) < 1 || strlen(strNewFileName) < 1) {
    return -1;
  }

  boost::filesystem::path gisImagePath = CSceneUtil::GISImageFullPath(strFileName);
  boost::filesystem::path gisNewImagePath = CSceneUtil::GISImageFullPath(strNewFileName);

  bool bRet = false;
  if (boost::filesystem::exists(gisImagePath) && boost::filesystem::is_regular_file(gisImagePath) &&
      !boost::filesystem::exists(gisNewImagePath)) {
    // delete hadmap file
    boost::filesystem::rename(gisImagePath, gisNewImagePath);
    return 0;
  }

  return -1;
}

const wchar_t* CHadmapSceneData::renameGISImage(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("rename gis image %s ", strParams.c_str());

  CParserHadmapJsonV2 jParser;
  sTagGISImageRenameParams param;
  int ret = jParser.ParseGISImageRenameParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'err':-1, 'message':'Parse gis image rename error!'}";
  }

  ret = RenameSingleGISImage(param.m_strOldName.c_str(), param.m_strNewName.c_str());
  if (ret != 0) {
    return L"{'err':-1, 'message':'Rename gis image error!'}";
  }

  return m_wstrSucceed.c_str();
}

int CHadmapSceneData::RenameSingleGISModel(const char* strFileName, const char* strNewFileName) {
  if (!strFileName || !strNewFileName || strlen(strFileName) < 1 || strlen(strNewFileName) < 1) {
    return -1;
  }

  boost::filesystem::path gisModelPath = CSceneUtil::GISModelFullPath(strFileName);
  boost::filesystem::path gisNewModelPath = CSceneUtil::GISModelFullPath(strNewFileName);

  bool bRet = false;
  if (boost::filesystem::exists(gisModelPath) && boost::filesystem::is_regular_file(gisModelPath) &&
      !boost::filesystem::exists(gisNewModelPath)) {
    // delete hadmap file
    boost::filesystem::rename(gisModelPath, gisNewModelPath);
    return 0;
  }

  return -1;
}

const wchar_t* CHadmapSceneData::renameGISModel(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("rename gis model %s ", strParams.c_str());

  CParserHadmapJsonV2 jParser;
  sTagGISModelRenameParams param;
  int ret = jParser.ParseGISModelRenameParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'err':-1, 'message':'Parse gis model rename error!'}";
  }

  ret = RenameSingleGISModel(param.m_strOldName.c_str(), param.m_strNewName.c_str());

  if (ret != 0) {
    return L"{'err':-1, 'message':'Rename gis model error!'}";
  }

  return m_wstrSucceed.c_str();
}

int CHadmapSceneData::CopySingleGISModel(const char* strFileName, const char* strSrcDir, const char* strDestDir) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;

  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single gis model dest dir not exist");
    return -1;
  }

  std::string strFullName = CSceneUtil::GISModelFullName(strFileName);
  boost::filesystem::path gisModelSrcPath = CSceneUtil::GISModelPath(strSrcDir, strFullName.c_str());

  if (boost::filesystem::exists(gisModelSrcPath) && boost::filesystem::is_regular_file(gisModelSrcPath)) {
    // copy gis model file
    boost::filesystem::path gisModelDestPath = destPath;
    gisModelDestPath /= strFullName;

    try {
      boost::filesystem::copy_file(gisModelSrcPath, gisModelDestPath, BOOST_COPY_OPTION);
    } catch (std::exception e) {
      SYSTEM_LOGGER_ERROR("copy gis model file %s error!", strFullName.c_str());
      return -1;
    }
  }

  return 0;
}

const wchar_t* CHadmapSceneData::uploadGISModel(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("upload gis model %s ", strParams.c_str());

  CParserHadmapJsonV2 jParser;
  sTagGISModelUploadParams param;
  int ret = jParser.ParseGISModelUploadParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'err':-1, 'message': 'Parse gis model upload error'}";
  }

  std::vector<boost::filesystem::path> files;
  boost::filesystem::path srcPath = param.m_strSourceDir;
  boost::filesystem::path hadmapSrcPath = srcPath;

  if (boost::iequals(param.m_strType, "dir")) {
    // 路径存在
    if (boost::filesystem::exists(hadmapSrcPath)) {
      // 获取hadmap文件
      boost::filesystem::directory_iterator itr_end;
      boost::filesystem::directory_iterator itr(hadmapSrcPath);

      for (; itr != itr_end; ++itr) {
        std::string strName = (*itr).path().string();
        std::string strExt = (*itr).path().extension().string();
        if ((!boost::filesystem::is_directory(*itr)) && CSceneUtil::ValidGISModelFormat(strExt.c_str())) {
          files.push_back((*itr).path());
        }
      }
    }
  } else if (boost::iequals(param.m_strType, "files")) {
    std::vector<std::string>::iterator itr = param.m_modelNames.begin();
    for (; itr != param.m_modelNames.end(); ++itr) {
      boost::filesystem::path p = srcPath;
      p /= (*itr);

      std::string strExt = p.extension().string();
      if ((!boost::filesystem::is_directory(p)) && CSceneUtil::ValidGISModelFormat(strExt.c_str())) {
        files.push_back(p);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload gis model param error!");
  }

  boost::filesystem::path destPath = CEngineConfig::Instance().GISModelDir();

  // copy gis model file
  std::vector<boost::filesystem::path>::iterator itr = files.begin();
  // succeed files
  std::vector<boost::filesystem::path> succeed_files;

  for (; itr != files.end(); ++itr) {
    // copy file
    std::string strName = itr->filename().string();
    int nRet = CopySingleGISModel(strName.c_str(), hadmapSrcPath.string().c_str(), destPath.string().c_str());
    if (nRet == 0) {
      boost::filesystem::path gisModelDestPath = destPath;
      gisModelDestPath /= strName;
      succeed_files.push_back(gisModelDestPath);
    }
  }

  FormResponseStr(succeed_files);

  return m_wstrSucceedUploadImages.c_str();
}

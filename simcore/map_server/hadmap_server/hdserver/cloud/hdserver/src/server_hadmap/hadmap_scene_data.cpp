/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "hadmap_scene_data.h"
#include "../engine/util/scene_util.h"
#include "../server_map_cache/map_data_proxy.h"
#include "../xml_parser/hadmap_entity/image_params.h"
#include "../xml_parser/hadmap_entity/map_scene.h"
#include "../xml_parser/hadmap_entity/map_scene_v2.h"
#include "../xml_parser/hadmap_entity/map_scene_v3.h"
#include "../xml_parser/hadmap_entity/parser_hadmap_json_v2.h"
#include "../xml_parser/hadmap_entity/parser_hadmap_json_v3.h"
#include "common/utils/oid_generator.h"
#include "common/xml_parser/entity/mapfile.h"
#include "engine/entity/aabb.h"

#include <json/value.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <sstream>
#include "../engine/config.h"
#include "common/log/system_logger.h"
#ifndef UPGRADE_MAP_EDITOR
#  include "tx_od_op_interface.h"
#else
#  include "tx_hadmap_cache.h"
#endif
#include "../server_map_cache/map_data_cache.h"
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

  SYSTEM_LOGGER_INFO("start save hadmap scene file.");
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

// /api/hadmaps/open/v3/map.sqlite?version=1679394774&bucket=hadmap-only-1311246188&mapFileKey=&mapJsonKey=
std::string CHadmapSceneData::GetHadmapSceneDataV3(const wchar_t* wstrHadmapSceneName, const std::string& bucket,
                                                   const std::string& mapKey, const std::string& jsonKey) {
  //
  // 1. get map info from map_service.
  // 2. download json from cos and return;
  //

  if (!wstrHadmapSceneName) {
    return "{\"err\":-1, \"message\": \"hadmap scene name null\"}";
  }

  if (mapKey.empty()) {
    return R"({"err":-1,"message":"invalid paramter: map key must be non-empty"})";
  }

  // download from cos.

  std::string strHadmapSceneName = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneName);

  std::string strContent;
  Json::Value root;
  root["name"] = strHadmapSceneName;
  // for thirdparty opendrive, we don't save it's json file.
  if (jsonKey.size() > 0) {
    SYSTEM_LOGGER_INFO("there is a json file , get and parse it.");
    std::string json_str;
    int res = CMapDataProxy::Instance().getObjectToStr(bucket, jsonKey, json_str);
    if (res == HSEC_OK) {
      Json::Value data;
      misc::ParseJson(json_str.c_str(), &data, "Error parse old map json.");
      root["data"] = data;
    } else {
      SYSTEM_LOGGER_ERROR("Error get json object from cos");
    }
  }
  tad::sim::ObjectIdGenerator og;
  std::string uid = og.Next();
  std::string custom_name = strHadmapSceneName;
  // std::replace( custom_name.begin(), custom_name.end(), '/', '_');
  std::string tmpXODR = "/tmp/" + uid + "_" + custom_name;
  int res = CMapDataProxy::Instance().DownloadMapFile(bucket, mapKey, tmpXODR);

  if (res != HSEC_OK) {
    SYSTEM_LOGGER_ERROR("Error get xodr object from cos: %s to %s", mapKey.c_str(), tmpXODR.c_str());
    return "{\"err\":-1, \"message\": \"fetch data from cos failed(for ).\"}";
  }

  std::string strData;
  res = parseJsonFromOpendrive(tmpXODR.c_str(), strData);
  if (res != HSEC_OK) {
    SYSTEM_LOGGER_ERROR("Error get xodr object from cos");
    return "{\"err\":-1, \"message\": \" parse opendrive file failed(to get json).\"}";
  }

  Json::Value geometry;
  misc::ParseJson(strData.c_str(), &geometry, "Error parse old map json.");
  root["baseVersion"] = mapKey;

#ifdef UPGRADE_MAP_EDITOR
  root["data"] = geometry;
#else
  root["geometry"] = geometry;
#endif
  strContent = Json::FastWriter{}.write(root);
  SYSTEM_LOGGER_INFO("content of get scene data len: %d", strContent.size());
  return strContent;
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

    // SYSTEM_LOGGER_INFO("scene file %s saved.", pScene->Name().c_str());

    if (nRet == 0) {
      return L"{\"err\":0, \"message\": \"ok\"}";
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveHadmapSceneData exception: %s", e.what());
  }

  return L"{\"err\":-1, \"message\": \"hadmap scene save error\"}";
}

int CHadmapSceneData::parseJsonFromOpendrive(const std::string& xodrPath, std::string& jstr) {
  bool nRet;
  SYSTEM_LOGGER_INFO("under get json from opendrive %s.", xodrPath.c_str());
  try {
#ifndef UPGRADE_MAP_EDITOR
    auto xodrPlugin = odr_engine::ODR::explicit_create();

    SYSTEM_LOGGER_INFO("start parser open drive to get json %s!", xodrPath.c_str());
    std::cout << "plugin is: " << xodrPlugin << std::endl;

    nRet = xodrPlugin->parse_opendrive_to_json(xodrPath.c_str(), jstr);
    // std::cout << "json is: " << jstr << std::endl;
#else
    std::stringstream ss;
    nRet = ODR::CHadmap{}.Open(xodrPath.c_str(), true, ss) == 0;
    jstr = ss.str();
#endif
  } catch (boost::system::system_error& err) {
    SYSTEM_LOGGER_ERROR("create plugin and parser opendrive to get json failed (exception thrown). ");
    SYSTEM_LOGGER_ERROR("load plugin error: ", err.what());
    return HSEC_JSON2XODR_ERROR;
  }
  if (!nRet) {
    SYSTEM_LOGGER_ERROR("parse opendrive to get json failed,return not ok.");
    return HSEC_JSON2XODR_ERROR;
  }
  return HSEC_OK;
}

int CHadmapSceneData::saveJsonAsOpendrive(const std::string& fpath, const std::string& jstr) {
  bool nRet;
  SYSTEM_LOGGER_INFO("under save json as opendrive %s.", fpath.c_str());
  try {
#ifndef UPGRADE_MAP_EDITOR
    auto xodrPlugin = odr_engine::ODR::explicit_create();
    // odr_engine::ODR::tx_od_plugin_impl::create();
    SYSTEM_LOGGER_INFO("start save open drive %s!", fpath.c_str());
    nRet = xodrPlugin->save_opendrive_from_json_string(fpath.c_str(), jstr);
#else
    std::stringstream ss;
    ss << jstr;
    nRet = ODR::CHadmap{}.Save(fpath.c_str(), true, ss) == 0;
#endif
  } catch (boost::system::system_error& err) {
    SYSTEM_LOGGER_ERROR("create plugin and save opendrive from json failed (exception thrown). ");
    SYSTEM_LOGGER_ERROR("load plugin error: ", err.what());
    return HSEC_JSON2XODR_ERROR;
  }
  if (!nRet) {
    SYSTEM_LOGGER_ERROR("save opendrive from json failed,return not ok.");
    return HSEC_JSON2XODR_ERROR;
  }
  return HSEC_OK;
}

std::string CHadmapSceneData::SaveHadmapSceneDataV3(const wchar_t* wstrHadmapSceneContent) {
  //
  if (!wstrHadmapSceneContent) {
    return "{\"err\":-1, \"message\": \"Hadmap scene v3 content null\"}";
  }

  std::string strHadmapSceneContent = CEngineConfig::Instance().WStr2MBStr(wstrHadmapSceneContent);

  CParserHadmapJsonV3 jParser;
  SYSTEM_LOGGER_INFO("start save hadmap scene file v3.");

  try {
    CMapSceneV3* _pScene = jParser.ParseV3(strHadmapSceneContent.c_str());
    std::unique_ptr<CMapSceneV3> pScene(_pScene);

#ifndef UPGRADE_MAP_EDITOR
    std::string except_ext = ".xodr";
    if (boost::filesystem::path(pScene->Name()).extension().string() != except_ext) {
      SYSTEM_LOGGER_ERROR("invalid extension: name %s must be %s", pScene->Name().c_str(), except_ext.c_str());
      return "{\"err\":-1, \"message\": \"Hadmap scene v3 save error: invalid extension\"}";
    }
#endif

    sTagServiceHadmapInfo info;
    std::string jsonKey = pScene->GetJsonKey();
    std::string cosKey = jsonKey.substr(0, jsonKey.find_last_of('/')) + "/" + pScene->Name();
    // 1. save opendrive for whatever reason, as we have to parse and get lla from it.
    // save opendrive file and compute new lon lat at here.
    std::string tmpF = "/tmp/" + tad::sim::ObjectIdGenerator{}.Next() + "_" + pScene->Name().c_str();
#ifdef UPGRADE_MAP_EDITOR
    if (!pScene->GetBaseVersion().empty()) {
      SYSTEM_LOGGER_INFO("modify map, copy base version, %s -> %s", pScene->GetBaseVersion().c_str(), tmpF.c_str());
      CMapDataProxy::Instance().DownloadMapFile(pScene->GetBucket(), pScene->GetBaseVersion(), tmpF);
    }
#endif

    SYSTEM_LOGGER_INFO("tmp opendrive f is: %s ", tmpF.c_str());
    int res = CHadmapSceneData::saveJsonAsOpendrive(tmpF, pScene->Geometry());
    // res = CMapSceneV3::SaveToOpenDrive(tmpF.c_str(), pScene->Geometry().c_str());
    if (res != HSEC_OK) {
      SYSTEM_LOGGER_ERROR("Error write tmp opendrive file to local tmp path: %s, ret:%d", tmpF.c_str(), res);
      return "{\"err\":-1, \"message\": \"Hadmap scene v3 save error,map failed to be written to tmp file.\"}";
    }

    SYSTEM_LOGGER_INFO("upload third opendrive to %s", cosKey.c_str());
    res = CMapDataProxy::Instance().uploadFile(pScene->GetBucket(), cosKey, tmpF);
    if (res != HSEC_OK) {
      SYSTEM_LOGGER_ERROR("for thirdparty map . failed to upload saved opendrive to: %s, ret:%d", cosKey.c_str(), res);
      return "{\"err\":-1, \"message\": \"for thirdparty map . failed to upload saved opendrive\"}";
    }

    SYSTEM_LOGGER_INFO("upload json to %s", jsonKey.c_str());
    res = CMapDataProxy::Instance().uploadStr(pScene->GetBucket(), jsonKey, pScene->Data());
    if (res != HSEC_OK) {
      SYSTEM_LOGGER_ERROR("save hadmap scene file v3 failed. failed to upload json to: %s, ret:%d", jsonKey.c_str(),
                          res);
      return "{\"err\":-1, \"message\": \"Hadmap scene v3 save error,json failed to be uploaded\"}";
    }

    // 4.
    // parser file and compute ret
    // get hadmap parameter
    CMapFile mapfile;
    res = mapfile.ParseMapV2(tmpF.c_str(), false);
    if (res != 0) {
      SYSTEM_LOGGER_ERROR("open tmp map file(should be opendrive)  %s error:%d", tmpF.c_str(), res);

      return "{\"err\":-1, \"message\": \"parse tmp map file error.\"}";
    } else {
      Json::Value root;

      Json::Value data;
      root["code"] = 0;
      root["message"] = "ok";

      data["id"] = pScene->GetId();
      data["name"] = pScene->Name();
      data["lon"] = mapfile.m_refPt.X();
      data["lat"] = mapfile.m_refPt.Y();
      data["alt"] = mapfile.m_refPt.Z();
      data["size"] = boost::filesystem::file_size(boost::filesystem::path(tmpF));
      data["mapFileKey"] = cosKey;
      data["mapJsonKey"] = jsonKey;
      data["baseVersion"] = cosKey;

      CVector3 mi(mapfile.m_envelop_min[0], mapfile.m_envelop_min[1], mapfile.m_envelop_min[2]);
      CVector3 ma(mapfile.m_envelop_max[0], mapfile.m_envelop_max[1], mapfile.m_envelop_max[2]);
      CAABB aabbLl(mi, ma);

      auto bld = aabbLl.Min();
      Json::Value bl;
      bl["lon"] = bld.X();
      bl["lat"] = bld.Y();
      bl["alt"] = bld.Z();

      auto trd = aabbLl.Max();
      Json::Value tr;
      tr["lon"] = trd.X();
      tr["lat"] = trd.Y();
      tr["alt"] = trd.Z();

      data["tr"] = tr;
      data["bl"] = bl;

      root["data"] = data;

      std::string strRes = Json::FastWriter{}.write(root);
      SYSTEM_LOGGER_INFO("return save json as string %s", strRes.c_str());
      // std::wstring wstrRes = CEngineConfig::Instance().MBStr2WStr(strRes.c_str());
      return strRes;
    }
  } catch (std::exception& e) {
    SYSTEM_LOGGER_FATAL("SaveHadmapSceneDataV3 exception: %s", e.what());
  }

  return "{\"err\":-1, \"message\": \"Hadmap scene v3 save error\"}";
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
    return L"{'err':-1, 'message':'parse gis delete param error!'}";
  }

  std::vector<std::string>::iterator itr = param.m_imageNames.begin();
  for (; itr != param.m_imageNames.end(); ++itr) {
    DeleteSingleGISImage(itr->c_str());
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
    return L"{'err':-1, 'message': 'parse gis image upload error'}";
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
    return L"{'err':-1, 'message':'parse gis image rename error!'}";
  }

  ret = RenameSingleGISImage(param.m_strOldName.c_str(), param.m_strNewName.c_str());
  if (ret != 0) {
    return L"{'err':-1, 'message':'rename gis image error!'}";
  }

  return m_wstrSucceed.c_str();
}

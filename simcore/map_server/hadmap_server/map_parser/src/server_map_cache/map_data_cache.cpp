/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "map_data_cache.h"
#include <sstream>

#include <json/value.h>
#include <json/writer.h>
#include <mapengine/hadmap_engine.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include "../engine/config.h"
#include "../engine/util/scene_util.h"
#include "../map_data/lane_boundary_data.h"
#include "../map_data/lane_data.h"
#include "../map_data/lane_link.h"
#include "../map_data/road_data.h"
#include "../server_hadmap/hadmap_scene_data.h"
#include "../xml_parser/entity/CMapEdit.h"
#include "../xml_parser/entity/hadmap_params.h"
#include "../xml_parser/entity/parser_json.h"
#include "../xml_parser/entity/query_params.h"
#include "common/coord_trans.h"
#include "common/engine/entity/hadmap.h"
#include "common/log/system_logger.h"
#include "common/map_data/map_data_predef.h"
#include "common/map_data/map_object.h"
#include "common/xml_parser/entity/mapfile.h"
#include "common/xml_parser/entity/signlight.h"
#include "database/entity/scenario.h"
#include "xml_parser/entity/parser.h"
// #include <windows.h>
// #include <Psapi.h>
// #include <tchar.h>

CMapDataCache::CMapDataCache() { m_wstrMapFileFormatError = L"{'err': -2, msg: 'map format error'}"; }

CMapDataCache& CMapDataCache::Instance() {
  static CMapDataCache instance;

  return instance;
}

std::wstring CMapDataCache::ConvertToJson(Json::Value& root) {
  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ss;
  writer->write(root, &ss);
  std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(ss.str().c_str());

  return wstrResult;
}

void CMapDataCache::Init() {
  Json::Value Success;
  Success["data"] = "true";
  Json::Value Failed;
  Failed["data"] = "false";
  Json::Value ConvertToJsonFailed;
  ConvertToJsonFailed["err"] = 1;

  Json::StreamWriterBuilder builder;
  builder.settings_["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::stringstream ssSuccess;
  writer->write(Success, &ssSuccess);
  m_wstrSuccess = CEngineConfig::Instance().MBStr2WStr(ssSuccess.str().c_str());
  std::stringstream ssFailed;
  writer->write(Failed, &ssFailed);
  m_wstrFailed = CEngineConfig::Instance().MBStr2WStr(ssFailed.str().c_str());
  // convert to json failed
  std::stringstream ssConvert2JsonFailed;
  writer->write(ConvertToJsonFailed, &ssConvert2JsonFailed);
  m_wstrConvertToJsonFailed = CEngineConfig::Instance().MBStr2WStr(ssConvert2JsonFailed.str().c_str());

  m_wstrSuccess = L"true";
  m_wstrFailed = L"false";
}

const wchar_t* CMapDataCache::Configuration() { return CEngineConfig::Instance().ConfigWStr(); }

const wchar_t* CMapDataCache::L3StateMachine() { return CEngineConfig::Instance().L3StateMachineStrWStr(); }

const wchar_t* CMapDataCache::LoadHadMap(const wchar_t* wstrHadmapFile) {
  // 文件名为空检查
  if (!wstrHadmapFile) {
    SYSTEM_LOGGER_ERROR("Load hadmap file name is null!");
    return m_wstrFailed.c_str();
  }
  std::string strHadmapFile = CEngineConfig::Instance().WStr2MBStr(wstrHadmapFile);
  CHadmap* pHadmap = HadmapCache(wstrHadmapFile);
  if (pHadmap) {
    SYSTEM_LOGGER_INFO("Hadmap: %s in cache", strHadmapFile.c_str());
    return m_wstrSuccess.c_str();
  }
  // 文件类型检查
  boost::filesystem::path p = strHadmapFile;
  std::string strMapName = p.filename().string();
  std::string strExt = p.extension().string();
  if (!boost::algorithm::iequals(strExt, ".sqlite") && !boost::algorithm::iequals(strExt, ".xodr") &&
      !boost::algorithm::iequals(strExt, ".xml")) {
    SYSTEM_LOGGER_ERROR("Load hadmap file error, ext is %s", strExt.c_str());
    return m_wstrFailed.c_str();
  }

  // boost::filesystem::path initPath = CEngineConfig::Instance().ImageFileDir();
  // boost::filesystem::path initPath = CEngineConfig::Instance().ResourceDir();
  // boost::filesystem::path hadmapPath = initPath;
  // hadmapPath.append("hadmap");

  boost::filesystem::path filePath =
      CSceneUtil::HadmapFullPath(strHadmapFile.c_str());  // hadmapPath.append(strHadmapFile);

  SYSTEM_LOGGER_INFO("hadmap: %s not in cache, now load", strHadmapFile.c_str());

  // cache map data
  {
    // 加锁
    std::unique_lock<std::mutex> ul(m_mutexHadmapMutex);

    CHadmap* pHadmap = HadmapCache(wstrHadmapFile);
    if (pHadmap) {
      SYSTEM_LOGGER_INFO("after blocked hadmap: %s now in cache", strHadmapFile.c_str());
      return m_wstrSuccess.c_str();
    }

    SYSTEM_LOGGER_INFO("hadmap: %s not in cache, now in real load", strHadmapFile.c_str());

    CMapFile mapfile;
    int nRet = mapfile.ParseMapV2(filePath.string().c_str());
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("load hadmap file failed!");
      return m_wstrFailed.c_str();
    }

    SYSTEM_LOGGER_INFO("hadmap: %s load finished. now rearrange data", strHadmapFile.c_str());

    pHadmap = new CHadmap();
    pHadmap->SetHeaderVersion(mapfile.m_version);
    pHadmap->PushRoad(mapfile.m_roads);
    pHadmap->PushLaneLink(mapfile.m_lanelinks);
    pHadmap->PushObject(mapfile.m_mapobjects);
    pHadmap->PushTrafficLights(mapfile.m_lights);
    pHadmap->PushJunctionTraffics(mapfile.m_roadJunctionId);
    pHadmap->RearrangeData();
    // pHadmap->PrepareData();

    // 地图范围信息
    // 中心点
    CVector3 center;
    center.X() = mapfile.m_center[0];
    center.Y() = mapfile.m_center[1];
    center.Z() = mapfile.m_center[2];
    pHadmap->SetCenter(center);

    // 参考点
    CPoint3D refPoint;
    tagHadmapAttr* hadmapAttr = CEngineConfig::Instance().HadmapAttr(strMapName);
    if (!hadmapAttr) {
      SYSTEM_LOGGER_ERROR("hadmap file not exist in config file!");
      // assert(false);
      return m_wstrFailed.c_str();
    }

    refPoint.X() = hadmapAttr->m_dRefLon;
    refPoint.Y() = hadmapAttr->m_dRefLat;
    refPoint.Z() = hadmapAttr->m_dRefAlt;

    pHadmap->SetRefPoint(refPoint);

    CAABB aabbLl;
    aabbLl.Min().X() = mapfile.m_envelop_min[0];
    aabbLl.Min().Y() = mapfile.m_envelop_min[1];
    aabbLl.Min().Z() = mapfile.m_envelop_min[2];
    aabbLl.Max().X() = mapfile.m_envelop_max[0];
    aabbLl.Max().Y() = mapfile.m_envelop_max[1];
    aabbLl.Max().Z() = mapfile.m_envelop_max[2];
    pHadmap->SetAabbLl(aabbLl);

    // 保存地图绝对路径

    pHadmap->SetFullPath(filePath.string().c_str());
    pHadmap->MapQuery().SettxMapHandle(mapfile.pMapHandler);
    pHadmap->MapQuery().SetInitialize(true);
    pHadmap->SetFileName(strMapName);
    pHadmap->ProcessMapData();

    /*
     * 先ComposeCacheData，后PushHadmapCache
     * 这两句顺序不能颠倒，保证cache先于hadmap被设置
     *
     */

    ComposeCacheData(wstrHadmapFile, pHadmap);
    PushHadmapCache(wstrHadmapFile, pHadmap);

    SYSTEM_LOGGER_INFO("hadmap %s rearragne finished.", strHadmapFile.c_str());

    SYSTEM_LOGGER_INFO("hadmap %s loaded.", strHadmapFile.c_str());

    return m_wstrSuccess.c_str();
  }
}

const wchar_t* CMapDataCache::LoadHadMapCloud(const wchar_t* wstrHadmapFile) {
  // 文件名为空检查
  if (!wstrHadmapFile) {
    SYSTEM_LOGGER_ERROR("load hadmap file name is null!");
    return m_wstrFailed.c_str();
  }
  std::string strHadmapFile = CEngineConfig::Instance().WStr2MBStr(wstrHadmapFile);

  SYSTEM_LOGGER_INFO("from net load hadmap file %s", strHadmapFile.c_str());

  // 文件类型检查
  boost::filesystem::path p = strHadmapFile;
  std::string strMapName = p.filename().string();
  std::string strExt = p.extension().string();
  if (!boost::algorithm::iequals(strExt, ".sqlite") && !boost::algorithm::iequals(strExt, ".xodr") &&
      !boost::algorithm::iequals(strExt, ".xml")) {
    SYSTEM_LOGGER_ERROR("load hadmap file ext is %s", strExt.c_str());
    return m_wstrFailed.c_str();
  }
  boost::filesystem::path filePath = strHadmapFile;

  SYSTEM_LOGGER_INFO("hadmap: %s not in cache, now load", strHadmapFile.c_str());

  // cache map data
  {
    // 加锁
    std::unique_lock<std::mutex> ul(m_mutexHadmapMutex);

    CHadmap* pHadmap = HadmapCache(CEngineConfig::Instance().MBStr2WStr(strMapName.c_str()).c_str());
    if (pHadmap) {
      SYSTEM_LOGGER_INFO("after blocked hadmap: %s now in cache", strHadmapFile.c_str());
      return m_wstrSuccess.c_str();
    }

    SYSTEM_LOGGER_INFO("hadmap: %s not in cache, now in real load", strHadmapFile.c_str());

    CMapFile mapfile;
    int nRet = mapfile.ParseMapV2(filePath.string().c_str(), false);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("load hadmap file failed!");
      return m_wstrFailed.c_str();
    }

    SYSTEM_LOGGER_INFO("hadmap: %s load finished. now rearrange data", strHadmapFile.c_str());

    pHadmap = new CHadmap();

    pHadmap->PushRoad(mapfile.m_roads);
    pHadmap->PushLaneLink(mapfile.m_lanelinks);
    pHadmap->PushObject(mapfile.m_mapobjects);
    pHadmap->RearrangeData();
    // pHadmap->PrepareData();

    // 地图范围信息
    // 中心点
    CVector3 center;
    center.X() = mapfile.m_center[0];
    center.Y() = mapfile.m_center[1];
    center.Z() = mapfile.m_center[2];
    pHadmap->SetCenter(center);

    // 参考点
    CPoint3D refPoint;
    refPoint.X() = center.X();
    refPoint.Y() = center.Y();
    refPoint.Z() = center.Z();

    pHadmap->SetRefPoint(refPoint);

    CAABB aabbLl;
    aabbLl.Min().X() = mapfile.m_envelop_min[0];
    aabbLl.Min().Y() = mapfile.m_envelop_min[1];
    aabbLl.Min().Z() = mapfile.m_envelop_min[2];
    aabbLl.Max().X() = mapfile.m_envelop_max[0];
    aabbLl.Max().Y() = mapfile.m_envelop_max[1];
    aabbLl.Max().Z() = mapfile.m_envelop_max[2];
    pHadmap->SetAabbLl(aabbLl);

    // 保存地图绝对路径

    pHadmap->SetFullPath(filePath.string().c_str());
    pHadmap->MapQuery().SettxMapHandle(mapfile.pMapHandler);
    pHadmap->MapQuery().SetInitialize(true);
    pHadmap->SetFileName(strMapName);
    pHadmap->ProcessMapData();

    /*
     * 先ComposeCacheData，后PushHadmapCache
     * 这两句顺序不能颠倒，保证cache先于hadmap被设置
     **/

    ComposeCacheData(CEngineConfig::Instance().MBStr2WStr(strMapName.c_str()).c_str(), pHadmap);
    PushHadmapCache(CEngineConfig::Instance().MBStr2WStr(strMapName.c_str()).c_str(), pHadmap);

    SYSTEM_LOGGER_INFO("hadmap %s rearragne finished.", strHadmapFile.c_str());

    SYSTEM_LOGGER_INFO("hadmap %s loaded.", strHadmapFile.c_str());

    return m_wstrSuccess.c_str();
  }
}

const wchar_t* CMapDataCache::HadmapList() {
  // boost::filesystem::path p = CEngineConfig::Instance().ImageFileDir();
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path hadmapPath = p;
  hadmapPath.append("hadmap");
  boost::filesystem::path scenePath = p;
  scenePath.append("scene");
  std::vector<std::string> files;
  std::vector<std::string> sizes;
  std::vector<std::string> times;
  std::set<std::string> setName;

  Json::Value root;
  // hadmap 路径存在
  if (boost::filesystem::exists(hadmapPath)) {
    boost::filesystem::directory_iterator itr_end;
    boost::filesystem::directory_iterator itr(hadmapPath);

    for (; itr != itr_end; ++itr) {
      std::string strName = (*itr).path().string();
      std::string strExt = (*itr).path().extension().string();
      if ((!boost::filesystem::is_directory(*itr)) &&
          (boost::algorithm::iequals(strExt, ".sqlite") || boost::algorithm::iequals(strExt, ".xodr") ||
           boost::algorithm::iequals(strExt, ".xml"))) {
        files.push_back((*itr).path().filename().string());
        setName.insert((*itr).path().filename().string());

        uintmax_t filesize = boost::filesystem::file_size((*itr).path());
        /*
        float fFileSizeInKB = filesize / 1024;
        boost::format precision2Num("%u");
        precision2Num % fFileSizeInKB;
        std::string strFileSizeInKB = precision2Num.str();
        */
        sizes.push_back(std::to_string(filesize));
        std::time_t lastTm = boost::filesystem::last_write_time((*itr).path());
        boost::posix_time::ptime ptm = boost::posix_time::from_time_t(lastTm);
        times.push_back(boost::posix_time::to_simple_string(ptm));
      }
    }
    boost::filesystem::path pHadmapData = CEngineConfig::Instance().getHadmapDataDir();
    if (boost::filesystem::exists(pHadmapData)) {
      boost::filesystem::directory_iterator itr_end;
      boost::filesystem::directory_iterator itr(pHadmapData);

      for (; itr != itr_end; ++itr) {
        std::string strName = (*itr).path().string();
        std::string strExt = (*itr).path().extension().string();
        if ((!boost::filesystem::is_directory(*itr)) &&
                (boost::algorithm::iequals(strExt, ".sqlite") || boost::algorithm::iequals(strExt, ".xodr")) ||
            boost::algorithm::iequals(strExt, ".xml") &&
                (setName.find((*itr).path().filename().string()) == setName.end())) {
          if (std::find(files.begin(), files.end(), (*itr).path().filename().string()) != files.end()) continue;
          files.push_back((*itr).path().filename().string());

          uintmax_t filesize = boost::filesystem::file_size((*itr).path());
          sizes.push_back(std::to_string(filesize));
          std::time_t lastTm = boost::filesystem::last_write_time((*itr).path());
          boost::posix_time::ptime ptm = boost::posix_time::from_time_t(lastTm);
          times.push_back(boost::posix_time::to_simple_string(ptm));
        }
      }
    }

    std::vector<std::string>::iterator it = files.begin();
    std::vector<std::string>::iterator sit = sizes.begin();
    std::vector<std::string>::iterator tit = times.begin();

    Json::Value maps;
    root["code"] = 0;
    int nCount = 0;

    for (; it != files.end(); ++it, ++sit, ++tit) {
      Json::Value d;
      d["id"] = (Json::Value::Int64)(nCount++);
      d["name"] = (*it);
      d["size"] = (*sit);
      d["createTime"] = (*tit);
      auto ip = CEngineConfig::Instance().HadmapAttrs().find(*it);
      if (ip != CEngineConfig::Instance().HadmapAttrs().end()) {
        d["preset"] = ip->second.m_strPreset;
      } else {
        d["preset"] = "";
      }
      maps.append(d);
    }

    root["data"] = maps;
    root["message"] = "ok";
  } else {
    root["code"] = 0;
    root["message"] = "server error: no hadmap data";
  }

  std::string strResult = root.toStyledString();

  m_mapLists.clear();
  m_mapLists = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_mapLists.c_str();
}

const wchar_t* CMapDataCache::HadmapinfoList() { return CEngineConfig::Instance().HadmapConfigWStr(); }

const wchar_t* CMapDataCache::DeleteHadmaps(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  // SystemLogger::Info("delete hadmaps %s ", strParams.c_str());

  CParserJson jParser;
  sTagHadmapsDeleteParams param;
  int ret = jParser.ParseHadmapsDeleteParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  std::vector<std::string>::iterator itr = param.m_hadmapNames.begin();
  for (; itr != param.m_hadmapNames.end(); ++itr) {
    CEngineConfig::Instance().RemoveHadmapAttr(*itr);
    std::wstring wstrName = CEngineConfig::Instance().MBStr2WStr(itr->c_str());
    RemoveHadmapCacheAll(wstrName.c_str());
    // close map_edit
    CMapEdit::Instance().CloseMap(wstrName.c_str());
    // 再去地图编辑器里面删除可能存在的文件  hadmap_scene   hadmap_editor
    // CHadmapSceneData::Instance().DeleteHadmapSceneData(wstrName.c_str());
    int nRet = DeleteSingleHadmap(itr->c_str());
    if (nRet != 0) {
      return L"{'code':-1}";
    }
    CEngineConfig::Instance().SaveToHadmapConfig();
  }
  CEngineConfig::Instance().ComposeHadmapConfigStr();
  Json::Value root;
  root["code"] = 0;
  root["message"] = "ok";
  std::string strResult = root.toStyledString();
  m_wstrOkJson = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  return m_wstrOkJson.c_str();
}

const wchar_t* CMapDataCache::DownloadHadmaps(const wchar_t* wstrParams) {
  isStopHadmapsProgess = false;
  dHadmapsProgessVal = 0;

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("download hadmaps %s ", strParams.c_str());

  CParserJson jParser;
  sTagHadmapsDownloadParams param;
  int ret = jParser.ParseHadmapsDownloadParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message":"Parse upload param error!"})";
  }

  // boost::filesystem::path srcPath = CEngineConfig::Instance().HadmapDir();

  std::vector<std::string>::iterator itr = param.m_hadmapNames.begin();
  std::string strFailedHadmaps = "";
  std::vector<std::string> vecCopyFailed;
  std::vector<std::string> vecNameFailed;

  double nsize = param.m_hadmapNames.size();
  double nover = 0;
  bool isAllOk = true;
  for (; itr != param.m_hadmapNames.end(); ++itr) {
    if (isStopHadmapsProgess) {
      isStopHadmapsProgess = false;
      return LR"({"code":-2, "message": "Interrupt"})";
      // return LR"({"code": -1, "message":"stop the progress"})";
    }

    bool isC0 = IncludeChinese(param.m_strDestName.c_str());
    bool isC1 = IncludeChinese(itr->c_str());
    if (isC0 || isC1) {
      isAllOk = false;
      vecNameFailed.push_back(itr->c_str());
      continue;
    }

    std::string srcPath = CEngineConfig::Instance().getHadmapDataDir();
    auto mapAttr = CEngineConfig::Instance().HadmapAttrs().find(itr->c_str());
    if (mapAttr != CEngineConfig::Instance().HadmapAttrs().end() && mapAttr->second.m_strPreset == "") {
      srcPath = CEngineConfig::Instance().HadmapDir();
    }

    int ret = CopySingleHadmap(itr->c_str(), srcPath.c_str(), param.m_strDestName.c_str());
    if (ret != 0) {
      vecCopyFailed.push_back(itr->c_str());
    }
    nover++;
    dHadmapsProgessVal = nover / nsize;
  }

  dHadmapsProgessVal = 1;

  if (!isAllOk) {
    std::string strMessage = R"({"code": -1, )";

    if (vecNameFailed.size() > 0) {
      for (auto itr : vecNameFailed) {
        strFailedHadmaps.append(itr);
        strFailedHadmaps.append(",");
      }
      strFailedHadmaps = strFailedHadmaps.substr(0, strFailedHadmaps.length() - 1);
      strFailedHadmaps.append(" Export failed!Chinese character;");
    }

    if (vecCopyFailed.size() > 0) {
      for (auto itr : vecCopyFailed) {
        strFailedHadmaps.append(itr);
        strFailedHadmaps.append(",");
      }
      strFailedHadmaps = strFailedHadmaps.substr(0, strFailedHadmaps.length() - 1);
      strFailedHadmaps.append(" Export failed!Copy failed");
    }

    strMessage.append(R"("message":" )" + strFailedHadmaps + R"("})");
    m_wstrDownloadHadmapResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());
  } else {
    m_wstrDownloadHadmapResp = LR"({"code": 0, "message":"ok"})";
  }
  return m_wstrDownloadHadmapResp.c_str();
}

const wchar_t* CMapDataCache::UploadHadmaps(const wchar_t* wstrParams) {
  isStopHadmapsProgess = false;
  dHadmapsProgessVal = 0;

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("upload hadmaps %s ", strParams.c_str());

  CParserJson jParser;
  sTagHadmapsUploadParams param;
  int ret = jParser.ParseHadmapsUploadParams(strParams.c_str(), param);
  if (ret != 0) {
    return LR"({"code":-1, "message":"Parse upload param error!"})";
  }

  // std::vector<boost::filesystem::path> files;
  std::vector<std::tuple<boost::filesystem::path, std::string>> files;
  boost::filesystem::path srcPath = param.m_strSourceDir;
  srcPath = boost::filesystem::absolute(srcPath, "\\");

  SYSTEM_LOGGER_INFO("absolute path=%s", srcPath.c_str());

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
        if ((!boost::filesystem::is_directory(*itr)) &&
            (strExt == ".sqlite" || strExt == ".xodr" || strExt == ".xml")) {
          std::tuple<boost::filesystem::path, std::string> temp(itr->path(), "false");
          files.push_back(temp);
        }
      }
    }
  } else if (boost::iequals(param.m_strType, "files")) {
    // std::vector<std::string>::iterator itr = param.m_hadmapNames.begin();
    for (auto itr : param.m_hadmapNames) {
      boost::filesystem::path p = srcPath;
      // p /= (itr);
      p /= std::get<0>(itr);

      std::string strReplaceFlag = std::get<1>(itr);
      // p1 /= std::get<1>(itr);

      // std::tuple<boost::filesystem::path, boost::filesystem::path> temp(p, p1);
      std::string strExt = p.extension().string();
      if ((!boost::filesystem::is_directory(p)) &&
          (boost::algorithm::iequals(strExt, ".sqlite") || boost::algorithm::iequals(strExt, ".xodr") ||
           boost::algorithm::iequals(strExt, ".xml"))) {
        std::tuple<boost::filesystem::path, std::string> temp(p, strReplaceFlag);
        // files.push_back(p);
        files.push_back(temp);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload hadmaps param error!");
  }

  boost::filesystem::path destPath = CEngineConfig::Instance().getHadmapDataDir();

  std::vector<std::string> vecNameFailed;
  std::vector<std::string> vecCopyFailed;

  double nsize = files.size();
  double nover = 0;

  bool isAllOk = true;
  for (auto itr : files) {
    if (isStopHadmapsProgess) {
      isStopHadmapsProgess = false;

      return LR"({"code":-2, "message": "Interrupt"})";
      // m_wstrUploadHadmapResp = LR"({"code": -1, "message":"stop the progress"})";
      // return m_wstrUploadHadmapResp.c_str();
    }

    // copy file
    std::string strName = std::get<0>(itr).filename().string();
    std::string strStem = std::get<0>(itr).stem().string();
    std::string strReplaceFlag = std::get<1>(itr);

    bool isC0 = IncludeChinese(hadmapSrcPath.string().c_str());
    bool isC1 = IncludeChinese(strName.c_str());
    if (isC0 || isC1) {
      isAllOk = false;
      vecNameFailed.push_back(strStem);
      continue;
    }

    // ********************temp caode*******************//
    std::string strNewName;
    // 存在不拷贝（理论上替换掉的话是要把数据也换掉的）
    int ret = CopySingleHadmap(strName.c_str(), hadmapSrcPath.string().c_str(), destPath.string().c_str(),
                               strReplaceFlag.c_str(), strNewName);
    SYSTEM_LOGGER_INFO("CopySingleHadmap name=%s,src=%s,dest=%s,flag=%s,new=%s,ret=%d ", strName.c_str(),
                       hadmapSrcPath.string().c_str(), destPath.string().c_str(), strReplaceFlag.c_str(),
                       strNewName.c_str(), ret);
    if (ret != 0) {
      isAllOk = false;
      vecCopyFailed.push_back(strStem);
      continue;
    }
    // 存在不覆盖 （理论上替换掉的话是要把数据也换掉的）
    tagHadmapAttr* pHA = CEngineConfig::Instance().HadmapAttr(strNewName);
    if (!pHA) {
      // get hadmap parameter
      CMapFile mapfile;
      boost::filesystem::path hadmapDestPath = destPath;
      std::string strDestFullName = CSceneUtil::HadmapFullName(strNewName.c_str());
      hadmapDestPath /= strDestFullName;
      // std::string temp = std::get<1>(itr).string();
      int nRet = mapfile.ParseMapV2(hadmapDestPath.string().c_str(), false);
      mapfile.Disconect();
      if (nRet != 0) {
        SYSTEM_LOGGER_ERROR("open hadmap %s error", hadmapDestPath.string().c_str());
        // return m_wstrMapFileFormatError.c_str();
        isAllOk = false;
        vecCopyFailed.push_back(strStem);
        continue;
      } else {
        int nIndex = CEngineConfig::Instance().HadmapIndex(strNewName.c_str());
        // chanage attr 1
        tagHadmapAttr attr;
        attr.m_name = strNewName;
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

        CEngineConfig::Instance().UpdateHadmapAttr(strNewName, attr);
        std::wstring wstrName = CEngineConfig::Instance().MBStr2WStr(strNewName.c_str());
        RemoveHadmapCacheAll(wstrName.c_str());
      }
    }
    // ********************temp caode*******************//

    nover++;
    dHadmapsProgessVal = nover / nsize;
  }
  dHadmapsProgessVal = 1;

  std::string strFailedHadmaps = "";
  if (!isAllOk) {
    std::string strMessage = R"({"code": -1, )";

    if (vecNameFailed.size() > 0) {
      for (auto itr : vecNameFailed) {
        strFailedHadmaps.append(itr);
        strFailedHadmaps.append(",");
      }
      strFailedHadmaps = strFailedHadmaps.substr(0, strFailedHadmaps.length() - 1);
      strFailedHadmaps.append(" Upload failed!Chinese character;");
    }
    if (vecCopyFailed.size() > 0) {
      for (auto itr : vecCopyFailed) {
        strFailedHadmaps.append(itr);
        strFailedHadmaps.append(",");
      }
      strFailedHadmaps = strFailedHadmaps.substr(0, strFailedHadmaps.length() - 1);
      strFailedHadmaps.append(" Upload failed!Copy failed");
    }

    strMessage.append(R"("message":" )" + strFailedHadmaps + R"("})");

    m_wstrUploadHadmapResp = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());

  } else {
    m_wstrUploadHadmapResp = LR"({"code": 0, "message":"ok"})";
  }

  return m_wstrUploadHadmapResp.c_str();
}

const wchar_t* CMapDataCache::QueryNearbyInfo(const wchar_t* wstrQueryString) {
  std::unique_lock<std::mutex> ul(m_mutexQueryNearByInfo);

  // 查询字符串为空检查
  if (!wstrQueryString) {
    SYSTEM_LOGGER_ERROR("QueryNearbyInfo query string is null!");
    return m_wstrConvertToJsonFailed.c_str();
  }

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrQueryString);
  CParserJson jParser;
  sTagQueryNearbyInfoParams param;
  int ret = jParser.ParseQueryNearbyInfoParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(param.m_strHadmapName.c_str());

  try {
    CHadmap* pHadmap = HadmapCache(wstrMapName.c_str());
    if (!pHadmap) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    SYSTEM_LOGGER_INFO("query near by info. Lon: %d, Lat: %d start", param.m_dLon, param.m_dLat);

    sTagQueryNearbyInfoResult result;
    int nRet = pHadmap->QueryNearbyInfo(param.m_dLon, param.m_dLat, result);

    SYSTEM_LOGGER_INFO("query near by lane. Lon: %d, Lat: %d finished. convert to wstr", param.m_dLon, param.m_dLat);

    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    Json::Value root;
    root["err"] = 0;
    root["type"] = result.m_elemType;
    root["rid"] = Json::Value::UInt64(result.m_uRoadID);
    root["sid"] = Json::Value::UInt64(result.m_uSecID);
    root["lid"] = Json::Value::Int64(result.m_laneID);
    root["llid"] = Json::Value::UInt64(result.m_lanelinkID);

    // m_wstrConvertToJsonSucceed = ConvertToJson(root);
    std::wstring wstrConverted = ConvertToJson(root);

    SYSTEM_LOGGER_INFO("query near by lane finished. result : %s", wstrConverted.c_str());

    if (wstrConverted.length() > 0) {
      wchar_t* pContent = new wchar_t[wstrConverted.length() + 1];
      wcscpy(pContent, wstrConverted.c_str());

      return pContent;
    }

    return m_wstrConvertToJsonFailed.c_str();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("query near by lane error! %s", e->what());
  }

  return m_wstrConvertToJsonFailed.c_str();
}

const wchar_t* CMapDataCache::QueryNearbyLane(const wchar_t* wstrMapName, const wchar_t* wstrLon,
                                              const wchar_t* wstrLat) {
  std::unique_lock<std::mutex> ul(m_mutexQueryNearBy);

  // 文件名为空检查
  if (!wstrMapName || !wstrLon || !wstrLat) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return m_wstrConvertToJsonFailed.c_str();
  }

  try {
    CHadmap* pHadmap = HadmapCache(wstrMapName);
    if (!pHadmap) {
      return m_wstrConvertToJsonFailed.c_str();
    }
    std::string strLon = CEngineConfig::Instance().WStr2MBStr(wstrLon);
    std::string strLat = CEngineConfig::Instance().WStr2MBStr(wstrLat);

    SYSTEM_LOGGER_INFO("query near by lane. Lon: %s, Lat: %s start", strLon.c_str(), strLat.c_str());

    double dLon = std::atof(strLon.c_str());
    double dLat = std::atof(strLat.c_str());
    roadpkid roadID = 0;
    sectionpkid secID = 0;
    lanepkid laneID = 0;

    int nRet = pHadmap->QueryNearbyLane(dLon, dLat, roadID, secID, laneID);

    SYSTEM_LOGGER_INFO("query near by lane. Lon: %s, Lat: %s finished. convert to wstr", strLon.c_str(),
                       strLat.c_str());
    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    Json::Value root;
    root["err"] = 0;
    root["rid"] = Json::Value::UInt64(roadID);
    root["sid"] = Json::Value::UInt64(secID);
    root["lid"] = Json::Value::Int64(laneID);

    // m_wstrConvertToJsonSucceed = ConvertToJson(root);
    std::wstring wstrConverted = ConvertToJson(root);

    SYSTEM_LOGGER_INFO("query near by lane finished. result : %s", wstrConverted.c_str());

    if (wstrConverted.length() > 0) {
      wchar_t* pContent = new wchar_t[wstrConverted.length() + 1];
      wcscpy(pContent, wstrConverted.c_str());

      return pContent;
    }

    return m_wstrConvertToJsonFailed.c_str();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("query near by lane error! %s", e->what());
  }

  return m_wstrConvertToJsonFailed.c_str();
}

const wchar_t* CMapDataCache::QueryInfoByPt(const wchar_t* wstrMapName, const wchar_t* wstrLon, const wchar_t* wstrLat,
                                            const wchar_t* wstrSearchDistance) {
  std::unique_lock<std::mutex> ul(m_mutexQueryNearbyPt);

  // 文件名为空检查
  if (!wstrMapName || !wstrLon || !wstrLat) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return m_wstrConvertToJsonFailed.c_str();
  }
  try {
    CHadmap* pHadmap = HadmapCache(wstrMapName);
    if (!pHadmap) {
      return m_wstrConvertToJsonFailed.c_str();
    }
    std::string strLon = CEngineConfig::Instance().WStr2MBStr(wstrLon);
    std::string strLat = CEngineConfig::Instance().WStr2MBStr(wstrLat);
    std::string strSearchDistance = CEngineConfig::Instance().WStr2MBStr(wstrSearchDistance);
    SystemLogger::Info("query info by pt. Lon: %s, Lat: %s ,SearchDistance: %s start", strLon.c_str(), strLat.c_str(),
                       strSearchDistance.c_str());

    std::string strType = "";
    double dLon = std::atof(strLon.c_str());
    double dLat = std::atof(strLat.c_str());
    double dDistance = std::atof(strSearchDistance.c_str());

    lanelinkpkid lanelinkID = 0;
    roadpkid roadID = 0;
    sectionpkid secID = 0;
    lanepkid laneID = 0;
    double dLaneLon = -999;
    double dLaneLat = -999;
    double dLaneAlt = -999;
    double dLaneDist = 0;
    double dLaneWidth = 0;
    double dLaneOffset = 0;
    double dLanelinkLon = -999;
    double dLanelinkLat = -999;
    double dLanelinkAlt = -999;
    double dLanelinkDist = 0;
    double dLanelinkOffset = 0;
    double dYaw = 0;

    int nRetLane = pHadmap->GetLaneByPointLL(dLon, dLat, roadID, secID, laneID, dLaneLon, dLaneLat, dLaneAlt, dLaneDist,
                                             dLaneWidth, dLaneOffset, dYaw, dDistance);

    int nRetLanelink = pHadmap->GetLanelinkByPointLL(dLon, dLat, lanelinkID, dLanelinkLon, dLanelinkLat, dLanelinkAlt,
                                                     dLanelinkDist, dLanelinkOffset, dYaw, dDistance);

    // SYSTEM_LOGGER_INFO("QueryInfoByPt query finished! input(%s, %s)", strLon.c_str(), strLat.c_str());
    if (nRetLane != 0) {
      SYSTEM_LOGGER_INFO("result: no return lane.");
    } else {
      SYSTEM_LOGGER_INFO("result: lane id(%u,%u,%d), paddle (%.12f, %.12f, %f), dist: %f, width: %f, offset: %f",
                         roadID, secID, laneID, dLaneLon, dLaneLat, dLaneAlt, dLaneDist, dLaneWidth, dLaneOffset);
    }
    if (nRetLanelink != 0) {
      SYSTEM_LOGGER_INFO("result: no return lanelink.");
    } else {
      SYSTEM_LOGGER_INFO("result: lanelink(%u), paddle (%.12f, %.12f, %f), dist: %f, offset: %f finished.", lanelinkID,
                         dLanelinkLon, dLanelinkLat, dLanelinkAlt, dLanelinkDist, dLanelinkOffset);
    }

    Json::Value root;

    if (nRetLane != 0 && nRetLanelink != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    } else if (nRetLane != 0 && nRetLanelink == 0) {
      root["err"] = 0;
      root["type"] = "lanelink";
      root["llid"] = Json::Value::UInt64(lanelinkID);
      root["lon"] = dLanelinkLon;
      root["lat"] = dLanelinkLat;
      root["alt"] = dLanelinkAlt;
      root["dist"] = dLanelinkDist;
      root["offset"] = dLanelinkOffset;
      root["yaw"] = transform_angle(dYaw, true);

    } else if (nRetLane == 0 && nRetLanelink != 0) {
      root["err"] = 0;
      root["type"] = "lane";
      root["rid"] = Json::Value::UInt64(roadID);
      root["sid"] = Json::Value::UInt64(secID);
      root["lid"] = Json::Value::Int64(laneID);
      root["lon"] = dLaneLon;
      root["lat"] = dLaneLat;
      root["alt"] = dLaneAlt;
      root["dist"] = dLaneDist;
      root["offset"] = dLaneOffset;
      root["width"] = dLaneWidth;
      root["yaw"] = transform_angle(dYaw, true);

    } else if (nRetLane == 0 && nRetLanelink == 0) {
      double dAbsLanelinkOffset = dLanelinkOffset > 0 ? dLanelinkOffset : -1 * dLanelinkOffset;
      double dAbsLaneOffset = dLaneOffset > 0 ? dLaneOffset : -1 * dLaneOffset;
      /*
      if (dAbsLanelinkOffset < dAbsLaneOffset && dLaneDist > 0.1)
      {
              root["err"] = 0;
              root["type"] = "lanelink";
              root["llid"] = lanelinkID;
              root["lon"] = dLanelinkLon;
              root["lat"] = dLanelinkLat;
              root["alt"] = dLanelinkAlt;
              root["dist"] = dLanelinkDist;
              root["offset"] = dLanelinkOffset;

      } else {

              root["err"] = 0;
              root["type"] = "lane";
              root["rid"] = roadID;
              root["sid"] = secID;
              root["lid"] = laneID;
              root["lon"] = dLaneLon;
              root["lat"] = dLaneLat;
              root["alt"] = dLaneAlt;
              root["dist"] = dLaneDist;
              root["offset"] = dLaneOffset;
              root["width"] = dLaneWidth;
      }
      */
      if (dAbsLaneOffset <= dAbsLanelinkOffset || dLaneDist < 0.01) {
        root["err"] = 0;
        root["type"] = "lane";
        root["rid"] = Json::Value::UInt64(roadID);
        root["sid"] = Json::Value::UInt64(secID);
        root["lid"] = Json::Value::Int64(laneID);
        root["lon"] = dLaneLon;
        root["lat"] = dLaneLat;
        root["alt"] = dLaneAlt;
        root["dist"] = dLaneDist;
        root["offset"] = dLaneOffset;
        root["width"] = dLaneWidth;
        root["yaw"] = transform_angle(dYaw, true);
      } else {
        root["err"] = 0;
        root["type"] = "lanelink";
        root["llid"] = Json::Value::UInt64(lanelinkID);
        root["lon"] = dLanelinkLon;
        root["lat"] = dLanelinkLat;
        root["alt"] = dLanelinkAlt;
        root["dist"] = dLanelinkDist;
        root["offset"] = dLanelinkOffset;
        root["yaw"] = transform_angle(dYaw, true);
      }
    }

    // m_wstrConvertToJsonSucceed = ConvertToJson(root);
    std::wstring wstrConverted = ConvertToJson(root);

    SYSTEM_LOGGER_INFO("query info by pt finished. result : %s", wstrConverted.c_str());

    if (wstrConverted.length() > 0) {
      wchar_t* pContent = new wchar_t[wstrConverted.length() + 1];
      wcscpy(pContent, wstrConverted.c_str());

      return pContent;
    }

    return m_wstrConvertToJsonFailed.c_str();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("query info by pt error! %s", e->what());
  }

  return m_wstrConvertToJsonFailed.c_str();
}

const wchar_t* CMapDataCache::QueryInfoByLocalPt(const wchar_t* wstrMapName, const wchar_t* wstrX, const wchar_t* wstrY,
                                                 const wchar_t* wstrZ) {
  std::unique_lock<std::mutex> ul(m_mutexQueryNearbyPt);

  // 文件名为空检查
  if (!wstrMapName || !wstrX || !wstrY || !wstrZ) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return m_wstrConvertToJsonFailed.c_str();
  }

  try {
    CHadmap* pHadmap = HadmapCache(wstrMapName);
    if (!pHadmap) {
      return m_wstrConvertToJsonFailed.c_str();
    }
    std::string strX = CEngineConfig::Instance().WStr2MBStr(wstrX);
    std::string strY = CEngineConfig::Instance().WStr2MBStr(wstrY);
    std::string strZ = CEngineConfig::Instance().WStr2MBStr(wstrZ);

    SYSTEM_LOGGER_INFO("query info by local pt. X: %s, Y: %s, Z: %s start", strX.c_str(), strY.c_str(), strZ.c_str());

    double dLon = std::atof(strX.c_str());
    double dLat = std::atof(strY.c_str());
    double dAlt = std::atof(strZ.c_str());
    int nRet = pHadmap->LocalToLonlat(dLon, dLat, dAlt);
    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    roadpkid roadID = 0;
    sectionpkid secID = 0;
    lanepkid laneID = 0;
    double dLaneLon = -999;
    double dLaneLat = -999;
    double dLaneAlt = -999;
    double dDist = 0;
    double dWidth = 0;
    double dOffset = 0;
    double dYaw = 0;
    nRet = pHadmap->GetLaneByPointLL(dLon, dLat, roadID, secID, laneID, dLaneLon, dLaneLat, dLaneAlt, dDist, dWidth,
                                     dOffset, dYaw);

    SYSTEM_LOGGER_INFO("query info by pt. X: %s, Y: %s , Z: %s finished. convert to wstr", strX.c_str(), strY.c_str(),
                       strZ.c_str());
    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    /*double dTmpLon = 0;
    double dTmpLat = 0;
    double dTmpWidth = 0;
    pHadmap->QueryLonLat(roadID, secID, laneID, dDist, dOffset, dTmpLon, dTmpLat,dTmpWidth);
    double dTmpLon1 = dTmpLon;
    double dTmpLat1 = dTmpLat;
    double dTmpAlt1 = 0;
    pHadmap->LonlatToLocal(dTmpLon1, dTmpLat1, dTmpAlt1);*/

    Json::Value root;
    root["err"] = 0;
    root["rid"] = Json::Value::UInt64(roadID);
    root["sid"] = Json::Value::UInt64(secID);
    root["lid"] = Json::Value::Int64(laneID);
    root["lon"] = dLaneLon;
    root["lat"] = dLaneLat;
    root["alt"] = dLaneAlt;
    root["dist"] = dDist;
    root["width"] = dWidth;
    root["offset"] = dOffset;

    // m_wstrConvertToJsonSucceed = ConvertToJson(root);
    std::wstring wstrConverted = ConvertToJson(root);

    SYSTEM_LOGGER_INFO("query info by pt finished. result : %s", wstrConverted.c_str());

    if (wstrConverted.length() > 0) {
      wchar_t* pContent = new wchar_t[wstrConverted.length() + 1];
      wcscpy(pContent, wstrConverted.c_str());

      return pContent;
    }

    return m_wstrConvertToJsonFailed.c_str();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("query info by pt error! %s", e->what());
  }

  return m_wstrConvertToJsonFailed.c_str();
}

const wchar_t* CMapDataCache::QueryNextLane(const wchar_t* wstrMapName, const wchar_t* wstrRoadID,
                                            const wchar_t* wstrSecID, const wchar_t* wstrLaneID) {
  std::unique_lock<std::mutex> ul(m_mutexQueryNextLane);

  // 文件名为空检查
  if (!wstrMapName || !wstrRoadID || !wstrSecID || !wstrLaneID) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return m_wstrConvertToJsonFailed.c_str();
  }

  try {
    CHadmap* pHadmap = HadmapCache(wstrMapName);
    if (!pHadmap) {
      return m_wstrConvertToJsonFailed.c_str();
    }
    std::string strRoadID = CEngineConfig::Instance().WStr2MBStr(wstrRoadID);
    std::string strSecID = CEngineConfig::Instance().WStr2MBStr(wstrSecID);
    std::string strLaneID = CEngineConfig::Instance().WStr2MBStr(wstrLaneID);

    SYSTEM_LOGGER_INFO("query next lane (%s, %s, %s)", strRoadID.c_str(), strSecID.c_str(), strLaneID.c_str());

    roadpkid roadID = atoll(strRoadID.c_str());
    sectionpkid secID = atol(strSecID.c_str());
    lanepkid laneID = atol(strLaneID.c_str());
    roadpkid nextRoadID = 0;
    sectionpkid nextSecID = 0;
    lanepkid nextLaneID = 0;

    int nRet = pHadmap->QueryNextLane(roadID, secID, laneID, nextRoadID, nextSecID, nextLaneID);

    SYSTEM_LOGGER_INFO("query next lane (%s, %s, %s) finished. convert to wstr", strRoadID.c_str(), strSecID.c_str(),
                       strLaneID.c_str());
    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    Json::Value root;
    root["err"] = 0;
    root["rid"] = Json::Value::UInt64(nextRoadID);
    root["sid"] = Json::Value::UInt64(nextSecID);
    root["lid"] = Json::Value::Int64(nextLaneID);

    // m_wstrConvertToJsonSucceed = ConvertToJson(root);
    std::wstring wstrConverted = ConvertToJson(root);

    SYSTEM_LOGGER_INFO("query next lane finished. result : %s", wstrConverted.c_str());

    if (wstrConverted.length() > 0) {
      wchar_t* pContent = new wchar_t[wstrConverted.length() + 1];
      wcscpy(pContent, wstrConverted.c_str());

      return pContent;
    }

    return m_wstrConvertToJsonFailed.c_str();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("query next lane error! %s", e->what());
  }

  return m_wstrConvertToJsonFailed.c_str();
}

const wchar_t* CMapDataCache::QueryLonLat(const wchar_t* wstrMapName, const wchar_t* wstrRoadID,
                                          const wchar_t* wstrSecID, const wchar_t* wstrLaneID, const wchar_t* wstrShift,
                                          const wchar_t* wstrOffset) {
  std::unique_lock<std::mutex> ul(m_mutexQueryLonLat);

  // 文件名为空检查
  if (!wstrMapName || !wstrRoadID || !wstrSecID || !wstrLaneID || !wstrShift || !wstrOffset) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return m_wstrConvertToJsonFailed.c_str();
  }

  try {
    CHadmap* pHadmap = HadmapCache(wstrMapName);
    if (!pHadmap) {
      return m_wstrConvertToJsonFailed.c_str();
    }
    std::string strRoadID = CEngineConfig::Instance().WStr2MBStr(wstrRoadID);
    std::string strSecID = CEngineConfig::Instance().WStr2MBStr(wstrSecID);
    std::string strLaneID = CEngineConfig::Instance().WStr2MBStr(wstrLaneID);
    std::string strShift = CEngineConfig::Instance().WStr2MBStr(wstrShift);
    std::string strOffset = CEngineConfig::Instance().WStr2MBStr(wstrOffset);

    SYSTEM_LOGGER_INFO("query lon lat input: lane(%s, %s, %s) st (%s, %s)", strRoadID.c_str(), strSecID.c_str(),
                       strLaneID.c_str(), strShift.c_str(), strOffset.c_str());

    roadpkid roadID = atoll(strRoadID.c_str());
    sectionpkid secID = atol(strSecID.c_str());
    lanepkid laneID = atol(strLaneID.c_str());
    double dShift = std::atof(strShift.c_str());
    double dOffset = std::atof(strOffset.c_str());
    double dLon = -999;
    double dLat = -999;
    double dAlt = -999;
    double dWidth = -999;
    int nRet = pHadmap->QueryLonLat(roadID, secID, laneID, dShift, dOffset, dLon, dLat, dAlt, dWidth);

    SYSTEM_LOGGER_INFO("query lon lat input: lane(%s, %s, %s) st (%s, %s) result:(%f, %f, %f). convert to wstr",
                       strRoadID.c_str(), strSecID.c_str(), strLaneID.c_str(), strShift.c_str(), strOffset.c_str(),
                       dLon, dLat, dAlt);

    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    Json::Value root;
    root["err"] = 0;
    root["lon"] = dLon;
    root["lat"] = dLat;
    root["alt"] = dAlt;
    root["width"] = dWidth;

    // m_wstrConvertToJsonSucceed = ConvertToJson(root);
    std::wstring wstrConverted = ConvertToJson(root);

    SYSTEM_LOGGER_INFO("query lonlat finished. result : %s", wstrConverted.c_str());

    if (wstrConverted.length() > 0) {
      wchar_t* pContent = new wchar_t[wstrConverted.length() + 1];
      wcscpy(pContent, wstrConverted.c_str());

      return pContent;
    }

    return m_wstrConvertToJsonFailed.c_str();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("query next lane error! %s", e->what());
  }

  return m_wstrConvertToJsonFailed.c_str();
}

const wchar_t* CMapDataCache::QueryLonLatByPoint(const wchar_t* wstrMapName, const wchar_t* wstrLon,
                                                 const wchar_t* wstrLat, const wchar_t* wstrElemType,
                                                 const wchar_t* wstrElemID, const wchar_t* wstrShift,
                                                 const wchar_t* wstrOffset, bool isStart) {
  std::unique_lock<std::mutex> ul(m_mutexQueryLonLat);

  // 文件名为空检查
  if (!wstrMapName || !wstrLon || !wstrLat || !wstrElemType || !wstrElemID || !wstrShift || !wstrOffset) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return m_wstrConvertToJsonFailed.c_str();
  }

  try {
    CHadmap* pHadmap = HadmapCache(wstrMapName);
    if (!pHadmap) {
      return m_wstrConvertToJsonFailed.c_str();
    }
    std::string strLon = CEngineConfig::Instance().WStr2MBStr(wstrLon);
    std::string strLat = CEngineConfig::Instance().WStr2MBStr(wstrLat);
    std::string strElemType = CEngineConfig::Instance().WStr2MBStr(wstrElemType);
    std::string strElemID = CEngineConfig::Instance().WStr2MBStr(wstrElemID);
    std::string strShift = CEngineConfig::Instance().WStr2MBStr(wstrShift);
    std::string strOffset = CEngineConfig::Instance().WStr2MBStr(wstrOffset);
    double dLon = std::atof(strLon.c_str());
    double dLat = std::atof(strLat.c_str());
    double dShift = std::atof(strShift.c_str());
    double dOffset = std::atof(strOffset.c_str());
    double dFinalLon = -999;
    double dFinalLat = -999;
    double dFinalAlt = -999;
    double dWidth = -999;
    double dFinalShift = -999;

    int nRet = -1;
    if (boost::iequals(strElemType, "lane")) {
      lanepkid laneID = atol(strElemID.c_str());
      nRet = pHadmap->QueryLonLatByPoint(dLon, dLat, laneID, dShift, dOffset, dFinalLon, dFinalLat, dFinalAlt, dWidth,
                                         dFinalShift, isStart);

    } else if (boost::iequals(strElemType, "lanelink")) {
      dWidth = 0;
      lanelinkpkid lanelinkID = atol(strElemID.c_str());
      nRet = pHadmap->QueryLonLatByPointOnLanelink(dLon, dLat, lanelinkID, dShift, dOffset, dFinalLon, dFinalLat,
                                                   dFinalAlt, dFinalShift);
    } else {
      SYSTEM_LOGGER_ERROR("queryLonLatByPoint elemtype error!");
    }
    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    Json::Value root;
    root["err"] = 0;
    root["lon"] = dFinalLon;
    root["lat"] = dFinalLat;
    root["alt"] = dFinalAlt;
    root["width"] = dWidth;
    root["finalshift"] = dFinalShift;
    // m_wstrConvertToJsonSucceed = ConvertToJson(root);
    std::wstring wstrConverted = ConvertToJson(root);

    SYSTEM_LOGGER_INFO("query position finished. result : %s", wstrConverted.c_str());

    if (wstrConverted.length() > 0) {
      wchar_t* pContent = new wchar_t[wstrConverted.length() + 1];
      wcscpy(pContent, wstrConverted.c_str());

      return pContent;
    }

    return m_wstrConvertToJsonFailed.c_str();
  } catch (std::exception* e) {
    SYSTEM_LOGGER_ERROR("query next lane error! %s", e->what());
  }

  return m_wstrConvertToJsonFailed.c_str();
}

int CMapDataCache::DeleteSingleHadmap(const char* strFileName) {
  boost::filesystem::path hadmapPath = CSceneUtil::HadmapFullPath(strFileName);
  SYSTEM_LOGGER_INFO("delete hadmap file=%s", hadmapPath.string().c_str());
  bool bRet = false;
  if (boost::filesystem::exists(hadmapPath) && boost::filesystem::is_regular_file(hadmapPath)) {
    // delete hadmap file
    try {
      bRet = boost::filesystem::remove(hadmapPath);
    } catch (const std::exception& e) {
      std::string str = e.what();

      SYSTEM_LOGGER_INFO("remove file error=%s", str.c_str());
    }

    if (bRet) {
      return 0;
    }
  }

  return -1;
}

int CMapDataCache::CopySingleHadmap(const char* strFileName, const char* strSrcDir, const char* strDestDir) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;
  SYSTEM_LOGGER_INFO("CopySingleHadmap strFileName=%s,srcDir=%s,destDir=%s", strFileName, strSrcDir, strDestDir);
  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single hadmap dest dir not exist");
    return -1;
  }
  std::string strFullName = CSceneUtil::HadmapFullName(strFileName);
  // std::string strDestFullName = CSceneUtil::HadmapFullName(strDestName);
  boost::filesystem::path hadmapSrcPath = CSceneUtil::HadmapPath(strSrcDir, strFullName.c_str());

  if (boost::filesystem::exists(hadmapSrcPath) && boost::filesystem::is_regular_file(hadmapSrcPath)) {
    // copy hadmap file
    boost::filesystem::path hadmapDestPath = destPath;
    hadmapDestPath /= strFullName;
    try {
      boost::filesystem::copy_file(hadmapSrcPath, hadmapDestPath, BOOST_COPY_OPTION);
    } catch (std::exception e) {
      SYSTEM_LOGGER_ERROR("copy hadmap file %s error!", strFullName.c_str());
      return -1;
    }
  }

  return 0;
}
int CMapDataCache::CopySingleHadmap(const char* strFileName, const char* strSrcDir, const char* strDestDir,
                                    const char* strExistType, std::string& newname) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;

  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single hadmap dest dir not exist");
    return -1;
  }

  std::string strFullName = CSceneUtil::HadmapFullName(strFileName);
  // std::string strDestFullName = CSceneUtil::HadmapFullName(strDestName);
  boost::filesystem::path hadmapSrcPath = CSceneUtil::HadmapPath(strSrcDir, strFullName.c_str());
  std::string strextension = hadmapSrcPath.extension().string();
  newname = strFullName;
  if (boost::filesystem::exists(hadmapSrcPath) && boost::filesystem::is_regular_file(hadmapSrcPath)) {
    // copy hadmap file
    boost::filesystem::path hadmapDestPath = destPath;
    hadmapDestPath /= strFullName;

    if (boost::filesystem::exists(hadmapDestPath)) {
      std::string str = strExistType;
      if (str == "false") {
        std::string strstemsrc = hadmapDestPath.stem().string();
        hadmapDestPath = destPath;
        std::string strstem;

        int n = 1;
        std::string strindex = "(" + std::to_string(n) + ")";
        strstem = strstemsrc + strindex;
        strFullName = strstem + strextension;
        hadmapDestPath /= strFullName;

        while (boost::filesystem::exists(hadmapDestPath) && boost::filesystem::is_regular_file(hadmapDestPath)) {
          n++;
          strindex = "(" + std::to_string(n) + ")";
          hadmapDestPath = destPath;
          strstem = strstemsrc + strindex;
          strFullName = strstem + strextension;
          hadmapDestPath /= strFullName;
        }
        newname = strFullName;
      }
    }

    try {
      boost::filesystem::copy_file(hadmapSrcPath, hadmapDestPath, BOOST_COPY_OPTION);
    } catch (std::exception e) {
      SYSTEM_LOGGER_ERROR("copy hadmap file %s error!", strFullName.c_str());
      return -1;
    }
  }

  return 0;
}
void CMapDataCache::ComposeCacheData(const wchar_t* wstrHadmapFile, CHadmap* pHadmap) {
  // 文件名为空检查
  if (!pHadmap || !wstrHadmapFile) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return;
  }
  SYSTEM_LOGGER_INFO("compose lanelinks data begin");
  boost::format LocalCoordiation("%.3f");

// #define COMPOSE_DOUBLE(X) ((double)((std::int64_t)((X)*1000.0))*0.001)
#define COMPOSE_DOUBLE(X) (static_cast<double>((std::int64_t)((X) * 100000000.0)) * 0.00000001)
  // lanlinks
  {
    LaneLinkVec& lanelinks = pHadmap->LaneLinks();
    Json::Value root;

    root["count"] = Json::Value::UInt64(lanelinks.size());
    LaneLinkVec::iterator llItr = lanelinks.begin();
    for (; llItr != lanelinks.end(); ++llItr) {
      PointsVec& datas = (*llItr)->Data();

      Json::Value ll;
      ll["count"] = Json::Value::UInt64(datas.size());
      ll["id"] = (Json::Value::Int64)((*llItr)->Id());
      ll["frid"] = Json::Value::UInt64((*llItr)->FromId().m_roadID);
      ll["fsid"] = Json::Value::UInt64((*llItr)->FromId().m_sectionID);
      ll["fid"] = Json::Value::Int64((*llItr)->FromId().m_laneID);
      ll["trid"] = Json::Value::UInt64((*llItr)->ToId().m_roadID);
      ll["tsid"] = Json::Value::UInt64((*llItr)->ToId().m_sectionID);
      ll["tid"] = Json::Value::Int64((*llItr)->ToId().m_laneID);
      ll["len"] = Json::Value((*llItr)->GetLength());
      ll["junctionid"] = Json::Value((*llItr)->JunctionId());
      PointsVec::iterator pitr = datas.begin();
      for (; pitr != datas.end(); ++pitr) {
        Json::Value point;
        double x = COMPOSE_DOUBLE(pitr->X());
        double y = COMPOSE_DOUBLE(pitr->Y());
        double z = COMPOSE_DOUBLE(pitr->Z());
        point["x"] = Json::Value(x);
        point["y"] = Json::Value(y);
        point["z"] = Json::Value(z);

        ll["array"].append(point);
      }

      root["array"].append(ll);
    }

    std::string strResult = CParserJson::ToJsonString(root);
    PushLaneLinkCache(wstrHadmapFile, strResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose lanelinks data end");

  SYSTEM_LOGGER_INFO("compose map object data begin");
  // map object
  {
    MapObjectVec& mapObjests = pHadmap->Objects();
    Json::Value root;

    root["count"] = Json::Value::UInt64(mapObjests.size());
    root["version"] = pHadmap->Version();
    MapObjectVec::iterator moItr = mapObjests.begin();
    for (; moItr != mapObjests.end(); ++moItr) {
      Json::Value ll;

      PointsVec& datas = (*moItr)->Data();
      ll["count"] = Json::Value::UInt64(datas.size());

      bool isUserData = false;
      for (const auto& user : (*moItr)->UserData()) {
        Json::Value juser;
        juser[user.first] = Json::Value(user.second);
        ll["userData"].append(juser);
        ll["count"] = Json::Value::UInt64(0);
        isUserData = true;
      }
      ll["id"] = Json::Value::Int64((*moItr)->Id());
      ll["type"] = Json::Value((*moItr)->Type());
      ll["strType"] = Json::Value((*moItr)->StrType());
      ll["name"] = Json::Value((*moItr)->Name());
      ll["strSubType"] = Json::Value((*moItr)->StrSubType());
      ll["length"] = Json::Value((*moItr)->Length());
      ll["height"] = Json::Value((*moItr)->Height());
      ll["width"] = Json::Value((*moItr)->Width());
      ll["roll"] = Json::Value((*moItr)->Roll());
      ll["pitch"] = Json::Value((*moItr)->Pitch());
      ll["yaw"] = Json::Value((*moItr)->Yaw());
      ll["groundHeight"] = Json::Value((*moItr)->GroundHeight());
      // PointsVec::iterator pitr = datas.begin();
      ll["x"] = Json::Value((*moItr)->Position().X());
      ll["y"] = Json::Value((*moItr)->Position().Y());
      ll["z"] = Json::Value((*moItr)->Position().Z());
      // 将物理灯
      std::vector<LaneID> temp;
      temp = (*moItr)->ReliedLaneIds();
      for (const auto& itr : temp) {
        // 通过m_roadID  m_sectionID  m_laneID  算个lane   再求终止点  求个方向  和 偏向角度
        hadmap::txLanePtr lanePtr;
        hadmap::txLaneId laneId;
        laneId.roadId = itr.m_roadID;
        laneId.sectionId = itr.m_sectionID;
        laneId.laneId = itr.m_laneID;
        hadmap::txMapHandle* pMapHandle = pHadmap->MapQuery().GettxMapHandle();
        int nRet = hadmap::getLane(pMapHandle, laneId, lanePtr);
        if (lanePtr == nullptr) {
          SYSTEM_LOGGER_INFO("get lane error nullptr!");
          continue;
        }
        const hadmap::txCurve* pCurve = lanePtr->getGeometry();
        const hadmap::txLineCurve* pLineCurve = dynamic_cast<const hadmap::txLineCurve*>(pCurve);
        hadmap::txPoint endpoint = pLineCurve->getEnd();  // 经纬度
        // 转成enu
        double refLon, refLat, refAlt;
        pHadmap->MapQuery().RefPoint(refLon, refLat, refAlt);
        coord_trans_api::lonlat2enu(endpoint.x, endpoint.y, endpoint.z, refLon, refLat, refAlt);

        HADMAP_LANE_ARROW arrow = static_cast<HADMAP_LANE_ARROW>(lanePtr->getLaneArrow());

        double dyaw;
        dyaw = lanePtr->getYaw(lanePtr->getLength());

        Json::Value rslID;
        rslID["x"] = Json::Value(endpoint.x);
        rslID["y"] = Json::Value(endpoint.y);
        rslID["z"] = Json::Value(endpoint.z);
        rslID["arrow"] = Json::Value(arrow);
        rslID["yaw"] = Json::Value(dyaw);

        ll["reliedInfo"].append(rslID);
      }

      if ((*moItr)->Type() == HADMAP_OBJECT_TYPE_ParkingSpace) {
        ll["strType"] = Json::Value("parkingSpace");
        for (auto it : (*moItr)->Outlines()) {
          Json::Value outline;
          Json::Value Point;
          int i = 0;
          for (auto itPt : it.second) {
            Point["x"] = itPt.X();
            Point["y"] = itPt.Y();
            Point["z"] = itPt.Z();
            Point["pointid"] = i;
            i++;
            outline["outline"].append(Point);
          }
          outline["id"] = it.first;
          ll["outlines"].append(outline);
        }
        Json::Value marking;
        marking["markwidth"] = std::atof((*moItr)->MarkWidth().c_str());
        marking["markcolor"] = (*moItr)->MarkColor();
        ll["marking"] = marking;

        // if((*moItr)->Outlines)
      }

      if (!isUserData && (*moItr)->Type() == HADMAP_OBJECT_TYPE_ParkingSpace) {
        PointsVec::iterator pitr = datas.begin();
        for (; pitr != datas.end(); ++pitr) {
          Json::Value point;
          //*
          double x = COMPOSE_DOUBLE(pitr->X());
          double y = COMPOSE_DOUBLE(pitr->Y());
          double z = COMPOSE_DOUBLE(pitr->Z());
          point["x"] = Json::Value(x);
          point["y"] = Json::Value(y);
          point["z"] = Json::Value(z);

          ll["array"].append(point);
        }
      }

      for (const auto& rep : (*moItr)->Repeat()) {
        Json::Value point;
        double x = COMPOSE_DOUBLE(rep.X());
        double y = COMPOSE_DOUBLE(rep.Y());
        double z = COMPOSE_DOUBLE(rep.Z());

        point["x"] = Json::Value(x);
        point["y"] = Json::Value(y);
        point["z"] = Json::Value(z);
        point["w"] = Json::Value(COMPOSE_DOUBLE(rep.w));
        point["h"] = Json::Value(COMPOSE_DOUBLE(rep.h));
        ll["repeat"].append(point);
      }

      if ((*moItr)->hasParkingSpace()) {
        auto parking = (*moItr)->ParkingSpace();
        Json::Value jparking;
        jparking["access"] = Json::Value(parking.access);
        jparking["restrictions"] = Json::Value(parking.restrictions);
        for (const auto& mk : parking.marks) {
          Json::Value jmark;
          jmark["side"] = Json::Value(mk.side);
          jmark["type"] = Json::Value(mk.type);
          jmark["width"] = Json::Value(mk.width);
          jmark["color"] = Json::Value(std::string(HadMapObjectColor((HADMAP_OBJECT_COLOR)mk.color)));
          jparking["marks"].append(jmark);
        }
        // auto str = jparking.toStyledString();
        ll["parking"] = jparking;
        // str = ll.toStyledString();
      }
      if ((*moItr)->getGeomData().sPoints.size() > 0) {
        Json::Value geomData;
        geomData["geomType"] = (*moItr)->getGeomData().sType;
        for (auto itpts : (*moItr)->getGeomData().sPoints) {
          Json::Value points;
          for (auto itp : itpts) {
            Json::Value point;
            point["x"] = itp.x;
            point["y"] = itp.y;
            point["z"] = itp.z;
            points["point"].append(point);
          }
          geomData["points"].append(points);
        }
        ll["geomdata"] = geomData;
      }
      root["array"].append(ll);
    }

    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root);

    // std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushObjectCache(wstrHadmapFile, strResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose map object data end");

  SYSTEM_LOGGER_INFO("compose road data begin");
  // road data
  {
    RoadVec& roads = pHadmap->Roads();
    Json::Value root;

    root["count"] = Json::Value::UInt64(roads.size());
    RoadVec::iterator rItr = roads.begin();
    for (; rItr != roads.end(); ++rItr) {
      PointsVec& datas = (*rItr)->Data();

      Json::Value ll;
      ll["count"] = Json::Value::UInt64(datas.size());
      ll["roadid"] = Json::Value::UInt64((*rItr)->Id());
      ll["type"] = Json::Value((*rItr)->Type());
      ll["len"] = Json::Value((*rItr)->Length());
      ll["dir"] = Json::Value((*rItr)->Direction());
      ll["mat"] = Json::Value((*rItr)->Material());
      PointsVec::iterator pitr = datas.begin();
      for (; pitr != datas.end(); ++pitr) {
        Json::Value point;
        //*
        double x = COMPOSE_DOUBLE(pitr->X());
        double y = COMPOSE_DOUBLE(pitr->Y());
        double z = COMPOSE_DOUBLE(pitr->Z());
        point["x"] = Json::Value(x);
        point["y"] = Json::Value(y);
        point["z"] = Json::Value(z);
        //*/
        /*LocalCoordiation.clear();
        LocalCoordiation % pitr->X();
        point["x"] = LocalCoordiation.str();
        LocalCoordiation.clear();
        LocalCoordiation % pitr->Y();
        point["y"] = LocalCoordiation.str();
        LocalCoordiation.clear();
        LocalCoordiation % pitr->Z();
        point["z"] = LocalCoordiation.str();*/

        ll["array"].append(point);
      }

      root["array"].append(ll);
    }

    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root);

    // std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushRoadCache(wstrHadmapFile, strResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose road data end");

  SYSTEM_LOGGER_INFO("compose lane data begin");
  // lane data
  {
    LaneVec& lanes = pHadmap->Lanes();
    Json::Value root;

    root["count"] = Json::Value::UInt64(lanes.size());
    LaneVec::iterator lItr = lanes.begin();
    for (; lItr != lanes.end(); ++lItr) {
      PointsVec& datas = (*lItr)->Data();

      Json::Value ll;
      ll["count"] = Json::Value::Int64(datas.size());
      ll["rid"] = Json::Value::UInt64((*lItr)->RoadId());
      ll["sid"] = Json::Value::UInt64((*lItr)->SectionId());
      ll["id"] = Json::Value::Int64((*lItr)->LaneId());
      ll["lbid"] = Json::Value::UInt64((*lItr)->LeftBoundaryId());
      ll["rbid"] = Json::Value::UInt64((*lItr)->RightBoundaryId());
      ll["sl"] = Json::Value((*lItr)->SpeedLimit());
      if ((*lItr)->LaneType() == 31) {
        ll["type"] = Json::Value(HADMAP_LANE_TYPE_Driving);
      } else if ((*lItr)->LaneType() == 32) {
        ll["type"] = Json::Value(HADMAP_LANE_TYPE_Driving);
      } else {
        ll["type"] = Json::Value((*lItr)->LaneType());
      }

      ll["arr"] = Json::Value((*lItr)->LaneArrow());
      ll["len"] = Json::Value((*lItr)->Length());
      ll["wid"] = Json::Value((*lItr)->Width());
      PointsVec::iterator pitr = datas.begin();
      for (; pitr != datas.end(); ++pitr) {
        Json::Value point;
        //*
        double x = COMPOSE_DOUBLE(pitr->X());
        double y = COMPOSE_DOUBLE(pitr->Y());
        double z = COMPOSE_DOUBLE(pitr->Z());
        point["x"] = Json::Value(x);
        point["y"] = Json::Value(y);
        point["z"] = Json::Value(z);
        ll["array"].append(point);
      }

      root["array"].append(ll);
    }

    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root);

    // std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushLaneCache(wstrHadmapFile, strResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose lane data end");

  SYSTEM_LOGGER_INFO("compose lane boundary data begin");
  // laneboudary data
  {
    LaneBoundaryVec& laneboundaries = pHadmap->LaneBoundaries();
    Json::Value root;

    root["count"] = Json::Value::UInt64(laneboundaries.size());
    LaneBoundaryVec::iterator lbItr = laneboundaries.begin();
    for (; lbItr != laneboundaries.end(); ++lbItr) {
      PointsVec& datas = (*lbItr)->Data();

      Json::Value ll;
      ll["count"] = Json::Value::UInt64(datas.size());
      ll["id"] = Json::Value::Int64((*lbItr)->Id());
      ll["mark"] = Json::Value(((*lbItr)->LaneMark() & 0xFFFFFF));  // delete mark color
      // ll["color"] = Json::Value((*lbItr)->Color());

      // ll["type"] = Json::Value((*lbItr)->Type());
      // ll["width"] = Json::Value((*lbItr)->LaneWidth());
      // ll["inverval"] = Json::Value((*lbItr)->DubleInterval());
      // ll["length"] = Json::Value((*lbItr)->DotLength());
      // ll["space"] = Json::Value((*lbItr)->DotSpace());
      // ll["offset"] = Json::Value((*lbItr)->DotOffset());

      PointsVec::iterator pitr = datas.begin();
      for (; pitr != datas.end(); ++pitr) {
        Json::Value point;
        double x = COMPOSE_DOUBLE(pitr->X());
        double y = COMPOSE_DOUBLE(pitr->Y());
        double z = COMPOSE_DOUBLE(pitr->Z());
        point["x"] = Json::Value(x);
        point["y"] = Json::Value(y);
        point["z"] = Json::Value(z);
        ll["array"].append(point);
      }

      root["array"].append(ll);
    }

    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root);

    // std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushLaneBoundaryCache(wstrHadmapFile, strResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose lane boundary data end");
}

void CMapDataCache::RemoveHadmapCacheAll(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }
  RemoveHadmapCache(wstrMapName);
  RemoveRoadCache(wstrMapName);
  RemoveLaneCache(wstrMapName);
  RemoveLaneBoundaryCache(wstrMapName);
  RemoveLaneLinkCache(wstrMapName);
  RemoveObjectCache(wstrMapName);
}

void CMapDataCache::RemoveHadmapCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }

  // may be dead lock
  std::unique_lock<std::mutex> ulHadmap(m_mutexHadmapMutex);
  std::unique_lock<std::mutex> ulQueryNearBy(m_mutexQueryNearBy);
  std::unique_lock<std::mutex> ulQueryLonLat(m_mutexQueryLonLat);
  std::unique_lock<std::mutex> ulQueryNearbyPt(m_mutexQueryNearbyPt);
  std::unique_lock<std::mutex> ulQueryNextLane(m_mutexQueryNextLane);
  std::unique_lock<std::mutex> ulQueryNearByInfo(m_mutexQueryNearByInfo);
  HadmapCaches::iterator itr = m_hadmapCaches.find(wstrMapName);
  if (itr != m_hadmapCaches.end()) {
    delete itr->second;
    m_hadmapCaches.erase(itr);
  }
}

CHadmap* CMapDataCache::HadmapCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }

  HadmapCaches::iterator itr = m_hadmapCaches.find(wstrMapName);
  if (itr != m_hadmapCaches.end()) {
    return itr->second;
  }

  return nullptr;
}

void CMapDataCache::PushHadmapCache(const wchar_t* wstrMapName, CHadmap* pHadmap) {
  if (!wstrMapName || !pHadmap) {
    return;
  }

  HadmapCaches::iterator itr = m_hadmapCaches.find(wstrMapName);
  if (itr != m_hadmapCaches.end()) {
    return;
  }

  m_hadmapCaches.insert(std::make_pair(wstrMapName, pHadmap));
}

void CMapDataCache::RemoveRoadCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }

  MapDataCaches::iterator itr = m_mapRoadCaches.find(wstrMapName);
  if (itr != m_mapRoadCaches.end()) {
    m_mapRoadCaches.erase(itr);
  }
}

const char* CMapDataCache::RoadDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }

  MapDataCaches::iterator itr = m_mapRoadCaches.find(wstrMapName);
  if (itr != m_mapRoadCaches.end()) {
    return itr->second.c_str();
  }

  return nullptr;
}

void CMapDataCache::PushRoadCache(const wchar_t* wstrMapName, const char* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  MapDataCaches::iterator itr = m_mapRoadCaches.find(wstrMapName);
  if (itr != m_mapRoadCaches.end()) {
    return;
  }

  m_mapRoadCaches.insert(std::make_pair(wstrMapName, wstrContent));
}

void CMapDataCache::RemoveLaneCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }

  MapDataCaches::iterator itr = m_mapLaneCaches.find(wstrMapName);
  if (itr != m_mapLaneCaches.end()) {
    m_mapLaneCaches.erase(itr);
  }
}

const char* CMapDataCache::LaneDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }

  MapDataCaches::iterator itr = m_mapLaneCaches.find(wstrMapName);
  if (itr != m_mapLaneCaches.end()) {
    return itr->second.c_str();
  }

  return nullptr;
}

void CMapDataCache::PushLaneCache(const wchar_t* wstrMapName, const char* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  MapDataCaches::iterator itr = m_mapLaneCaches.find(wstrMapName);
  if (itr != m_mapLaneCaches.end()) {
    return;
  }

  m_mapLaneCaches.insert(std::make_pair(wstrMapName, wstrContent));
}

void CMapDataCache::RemoveLaneBoundaryCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }

  MapDataCaches::iterator itr = m_mapLaneBoundaryCaches.find(wstrMapName);
  if (itr != m_mapLaneBoundaryCaches.end()) {
    m_mapLaneBoundaryCaches.erase(itr);
  }
}

const char* CMapDataCache::LaneBoundaryDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }

  MapDataCaches::iterator itr = m_mapLaneBoundaryCaches.find(wstrMapName);
  if (itr != m_mapLaneBoundaryCaches.end()) {
    return itr->second.c_str();
  }

  return nullptr;
}

void CMapDataCache::PushLaneBoundaryCache(const wchar_t* wstrMapName, const char* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  MapDataCaches::iterator itr = m_mapLaneBoundaryCaches.find(wstrMapName);
  if (itr != m_mapLaneBoundaryCaches.end()) {
    return;
  }

  m_mapLaneBoundaryCaches.insert(std::make_pair(wstrMapName, wstrContent));
}

void CMapDataCache::RemoveLaneLinkCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }

  MapDataCaches::iterator itr = m_mapLanelinkCaches.find(wstrMapName);
  if (itr != m_mapLanelinkCaches.end()) {
    m_mapLanelinkCaches.erase(itr);
  }
}
const char* CMapDataCache::LaneLinkDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }

  MapDataCaches::iterator itr = m_mapLanelinkCaches.find(wstrMapName);
  if (itr != m_mapLanelinkCaches.end()) {
    return itr->second.c_str();
  }

  return nullptr;
}

void CMapDataCache::PushLaneLinkCache(const wchar_t* wstrMapName, const char* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  MapDataCaches::iterator itr = m_mapLanelinkCaches.find(wstrMapName);
  if (itr != m_mapLanelinkCaches.end()) {
    return;
  }

  m_mapLanelinkCaches.insert(std::make_pair(wstrMapName, wstrContent));
}

void CMapDataCache::RemoveObjectCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }

  MapDataCaches::iterator itr = m_mapObjCaches.find(wstrMapName);
  if (itr != m_mapObjCaches.end()) {
    m_mapObjCaches.erase(itr);
  }
}
const char* CMapDataCache::ObjectDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }

  MapDataCaches::iterator itr = m_mapObjCaches.find(wstrMapName);
  if (itr != m_mapObjCaches.end()) {
    return itr->second.c_str();
  }

  return nullptr;
}

void CMapDataCache::PushObjectCache(const wchar_t* wstrMapName, const char* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  MapDataCaches::iterator itr = m_mapObjCaches.find(wstrMapName);
  if (itr != m_mapObjCaches.end()) {
    return;
  }

  m_mapObjCaches.insert(std::make_pair(wstrMapName, wstrContent));
}

void CMapDataCache::PushWDataCache(const wchar_t* wstrMapName, const wchar_t* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  std::map<std::wstring, std::wstring>::iterator itr = m_mapWDataCache.find(wstrMapName);
  if (itr != m_mapWDataCache.end()) {
    return;
  }

  m_mapWDataCache.insert(std::make_pair(wstrMapName, wstrContent));
}

const wchar_t* CMapDataCache::WMapDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }

  std::map<std::wstring, std::wstring>::iterator itr = m_mapWDataCache.find(wstrMapName);
  if (itr != m_mapWDataCache.end()) {
    return itr->second.c_str();
  }

  return nullptr;
}

std::string CMapDataCache::GetTrafficLights(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return nullptr;
  }
  if (CEngineConfig::Instance().findTpl(CEngineConfig::Instance().WStr2MBStr(wstrMapName)) != nullptr) {
    //
    std::string strTemplateName = CEngineConfig::Instance().findTpl(CEngineConfig::Instance().WStr2MBStr(wstrMapName));
    std::string strResourceDir = CEngineConfig::Instance().ResourceDir();
    CParserJson jParser;
    sTagEntityScenario scenario;
    scenario.m_strName = strTemplateName;
    sTagSimuTraffic sim;
    SYSTEM_LOGGER_INFO("load template %s ", strTemplateName.c_str());
    std::string ret = jParser.XmlToTraffic(strResourceDir.c_str(), "template", scenario, sim);
    // traffic lights to json
    Json::Value root;
    root["activePlan"] = "0";
    std::map<int, std::map<int, int>> junction2TrafficRoadMap;
    for (auto it : sim.m_traffic.m_mapSignlights) {
      Json::Value signlight;
      signlight["id"] = it.second.m_strID.c_str();
      signlight["routeID"] = it.second.m_strRouteID.c_str();
      signlight["start_s"] = it.second.m_strStartShift;
      signlight["start_t"] = it.second.m_strStartTime;
      signlight["l_offset"] = it.second.m_strOffset.c_str();
      signlight["direction"] = it.second.m_strDirection;
      signlight["time_green"] = it.second.m_strTimeGreen;
      signlight["time_yellow"] = it.second.m_strTimeYellow;
      signlight["time_red"] = it.second.m_strTimeRed;
      signlight["phase"] = it.second.m_strPhase;
      signlight["lane"] = it.second.m_strLane;
      signlight["enabled"] = ((it.second.m_strStatus == "Activated") ? true : false);
      signlight["plan"] = it.second.m_strPlan;
      signlight["junction"] = it.second.m_strJunction;
      signlight["phaseNumber"] = it.second.m_strPhasenumber;
      std::string strSignal = "";
      for (int i = 0; i < it.second.m_strSignalheadVec.size(); ++i) {
        strSignal += it.second.m_strSignalheadVec.at(i);
        if (i != it.second.m_strSignalheadVec.size() - 1) {
          strSignal += ",";
        }
      }
      signlight["signalHead"] = strSignal;
      // getlanelink
      CHadmap* pHadmap = HadmapCache(wstrMapName);
      if (!pHadmap) {
        SYSTEM_LOGGER_INFO("load map error");
        return "";
      }
      // route
      Json::Value route;
      CRoute croute = sim.m_traffic.m_mapRoutes.find(it.second.m_strID)->second;
      croute.ConvertToValue();
      // roadid
      int roadid;
      std::vector<linkData> linkdatas;
      pHadmap->getLaneLink(croute.m_dStartLon, croute.m_dStartLat, roadid, linkdatas);
      for (auto itLink : linkdatas) {
        Json::Value relative;
        relative["phase"] = itLink.phase;
        relative["lane"] = itLink.lane;
        relative["tolanelinkids"] = itLink.tolanelinkids;
        signlight["relative_lane"].append(relative);
      }
      //
      if (junction2TrafficRoadMap.find(std::atoi(it.second.m_strJunction.c_str())) != junction2TrafficRoadMap.end()) {
        auto& tmp = junction2TrafficRoadMap.find(std::atoi(it.second.m_strJunction.c_str()))->second;
        for (auto lightId : it.second.m_strSignalheadVec) {
          tmp[std::atoi(lightId.c_str())] = roadid;
        }
      } else {
        std::map<int, int> tmp;
        for (auto lightId : it.second.m_strSignalheadVec) {
          tmp[std::atoi(lightId.c_str())] = roadid;
        }
        junction2TrafficRoadMap[std::atoi(it.second.m_strJunction.c_str())] = tmp;
      }
      root["signlights"].append(signlight);
      route["start"] = std::to_string(croute.m_dStartLon) + "," + std::to_string(croute.m_dStartLat);
      route["end"] = "";
      route["id"] = it.second.m_strID;
      route["type"] = "start_end";
      root["routes"].append(route);
    }

    for (auto it : junction2TrafficRoadMap) {
      Json::Value junction;
      junction["id"] = std::to_string(it.first);
      for (auto itLight : it.second) {
        Json::Value light;
        light["id"] = std::to_string(itLight.first);
        light["roadid"] = std::to_string(itLight.second);
        junction["tafficlights"].append(light);
      }
      root["junctions"].append(junction);
    }
    std::string jsonStr = CParserJson::ToJsonString(root);
    return jsonStr;
  }
  std::map<std::wstring, CHadmap*>::iterator itr = m_hadmapCaches.find(wstrMapName);
  if (itr != m_hadmapCaches.end()) {
    LogicLightVec vec = itr->second->GetTrafficLights();
    Json::Value root;
    root["activePlan"] = "0";
    for (auto it : vec) {
      for (int n = 0; n < 1; n++) {
        Json::Value signlight;
        signlight["id"] = std::to_string(atoi(it->m_strID.c_str()) + n);
        signlight["routeID"] = std::to_string(atoi(it->m_strRouteID.c_str()) + n);
        signlight["start_s"] = it->m_strStartShift;
        signlight["start_t"] = it->m_strStartTime;
        signlight["l_offset"] = std::to_string(std::atof(it->m_strOffset.c_str()));
        signlight["direction"] = it->m_strDirection;
        signlight["time_green"] = it->m_strTimeGreen;
        signlight["time_yellow"] = it->m_strTimeYellow;
        signlight["time_red"] = it->m_strTimeRed;
        signlight["phase"] = "A";
        signlight["lane"] = it->m_strLane;
        signlight["enabled"] = ((it->m_strStatus == "Activated") ? true : false);
        signlight["plan"] = it->m_strPlan;
        signlight["junction"] = it->m_strJunction;
        signlight["phaseNumber"] = it->m_strPhasenumber;
        std::string strSignal = "";
        for (int i = 0; i < it->m_strSignalheadVec.size(); ++i) {
          strSignal += it->m_strSignalheadVec.at(i);
          if (i != it->m_strSignalheadVec.size() - 1) {
            strSignal += ",";
          }
        }
        signlight["signalHead"] = strSignal;
        for (auto& itPharse : it->m_parsetolanes) {
          Json::Value relative;
          relative["phase"] = itPharse.first;
          std::string _lanes;
          std::string _lanelinks;
          int index = 0;
          sort(itPharse.second.begin(), itPharse.second.end());
          itPharse.second.erase(unique(itPharse.second.begin(), itPharse.second.end()), itPharse.second.end());
          std::sort(itPharse.second.rbegin(), itPharse.second.rend());
          for (auto itP = itPharse.second.begin(); itP != itPharse.second.end(); itP++) {
            _lanes += std::to_string((*itP));
            if (index != itPharse.second.size() - 1) {
              _lanes += ",";
            }
            index++;
          }
          relative["lane"] = _lanes;
          if (it->m_parsetolanelinks.find(itPharse.first) != it->m_parsetolanelinks.end()) {
            int nIndex = 0;
            for (auto ait : it->m_parsetolanelinks[itPharse.first]) {
              _lanelinks += std::to_string(ait);
              if (nIndex != it->m_parsetolanelinks[itPharse.first].size() - 1) {
                _lanelinks += ",";
              }
              nIndex++;
            }
          }
          relative["tolanelinkids"] = _lanelinks;
          signlight["relative_lane"].append(relative);
        }
        root["signlights"].append(signlight);
        // route
        Json::Value route;
        route["start"] = std::to_string(it->m_frouteLon) + "," + std::to_string(it->m_frouteLat);
        route["end"] = "";
        route["id"] = std::to_string(atoi(it->m_strRouteID.c_str()) + n);
        route["type"] = "start_end";
        root["routes"].append(route);
        //
      }
    }
    std::map<int, std::map<int, int>> junction2TrafficRoadMap = itr->second->GetJunction2TrafficRoadMap();
    for (auto it : junction2TrafficRoadMap) {
      Json::Value junction;
      junction["id"] = std::to_string(it.first);
      for (auto itLight : it.second) {
        Json::Value light;
        light["id"] = std::to_string(itLight.first);
        light["roadid"] = std::to_string(itLight.second);
        junction["tafficlights"].append(light);
      }
      root["junctions"].append(junction);
    }
    std::string jsonStr = CParserJson::ToJsonString(root);
    return jsonStr;
  }
  return "";
}

const wchar_t* CMapDataCache::StopHadmapsProgess() {
  isStopHadmapsProgess = true;

  return L"{\"code\":0, \"message\": \"ok\"}";
}

const wchar_t* CMapDataCache::GetHadmapsProgessVal() {
  Json::Value root;
  root["code"] = 0;
  root["message"] = std::to_string(dHadmapsProgessVal);
  std::string strMessage = root.toStyledString();

  strWMessageProgess = CEngineConfig::Instance().MBStr2WStr(strMessage.c_str());
  return strWMessageProgess.c_str();
}

bool CMapDataCache::IncludeChinese(const char* str) {
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

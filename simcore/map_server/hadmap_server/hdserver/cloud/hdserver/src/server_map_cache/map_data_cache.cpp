/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "map_data_cache.h"

#include <json/value.h>
#include <json/writer.h>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <thread>

#include "../engine/config.h"
#include "../engine/error_code.h"
#include "../engine/util/scene_util.h"
#include "../map_data/lane_boundary_data.h"
#include "../map_data/lane_data.h"
#include "../map_data/lane_link.h"
#include "../map_data/road_data.h"
#include "../xml_parser/entity/hadmap_params.h"
#include "../xml_parser/entity/parser_json.h"
#include "../xml_parser/entity/query_params.h"
#include "./map_data_proxy.h"
#include "MD5.h"
#include "common/engine/entity/hadmap.h"
#include "common/log/system_logger.h"
#include "common/map_data/map_object.h"
#include "common/third/gutil/integral_types.h"
#include "common/utils/stringhelper.h"
#include "common/xml_parser/entity/mapfile.h"
#include "engine/error_code.h"
#include "scene_wrapper_linux/param_adapter.h"
#include "server_hadmap/hadmap_scene_data.h"
#include "structs/map_structs.h"

// #include <windows.h>
// #include <Psapi.h>
// #include <tchar.h>

const std::wstring CMapDataCache::m_wstrSuccess = L"true";
const std::wstring CMapDataCache::m_wstrFailed = L"false";

CMapDataCache::CMapDataCache() { m_wstrMapFileFormatError = L"{'err': -2, msg: 'map format error'}"; }

CMapDataCache& CMapDataCache::Instance() {
  static CMapDataCache instance;

  return instance;
}
Json::Value CMapDataCache::buildHdserverDefaultRetStr() {
  Json::Value root;
  root["code"] = 0;
  root["message"] = "ok";
  root["data"] = Json::Value();

  return root;
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
  std::stringstream ssFailed;
  writer->write(Failed, &ssFailed);
  std::stringstream ssConvert2JsonFailed;
  writer->write(ConvertToJsonFailed, &ssConvert2JsonFailed);
  m_wstrConvertToJsonFailed = CEngineConfig::Instance().MBStr2WStr(ssConvert2JsonFailed.str().c_str());
}

const wchar_t* CMapDataCache::Configuration() { return CEngineConfig::Instance().ConfigWStr(); }

const wchar_t* CMapDataCache::L3StateMachine() { return CEngineConfig::Instance().L3StateMachineStrWStr(); }

const wchar_t* CMapDataCache::GetRoadRel(const wchar_t* wstrParams, std::wstring& res) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("sTagGetRoadRelParam %s ", strParams.c_str());

  CParserJson jParser;
  sTagGetRoadRelParam param;
  int ret = jParser.ParseGetRoadRelParam(strParams.c_str(), param);
  if (ret != 0) {
    res = L"{'code':-1}";
    return m_wstrFailed.c_str();
  }

  Json::Value root;
  root["code"] = 0;
  root["message"] = "ok";

  Json::Value maps;

  Json::Value empty_v;

  for (auto hadmapName : param.m_hadmapNames) {
    std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(hadmapName.c_str());

    auto retJsonV = RoadRelDataCache(wstrMapName.c_str());
    auto retPrevJsonV = RoadPrevRelDataCache(wstrMapName.c_str());

    if (retJsonV == empty_v && retPrevJsonV == empty_v) {
      SYSTEM_LOGGER_ERROR("hadmap %s 's road rel has not been computed yet.", hadmapName.c_str());
    }
    Json::Value d;
    d["name"] = hadmapName;
    d["rel_map"] = retJsonV;
    d["prev_map"] = retPrevJsonV;
    // d["msg"] = "no lane rel yet.";
    maps.append(d);
  }

  root["data"] = maps;

  std::string strResult = root.toStyledString();

  res = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_wstrSuccess.c_str();
}

const wchar_t* CMapDataCache::GetLaneRel(const wchar_t* wstrParams, std::wstring& res) {
  // parse the req: hadmap key(cos key) array
  // for get lane rel from cache, return empty if it has not been computed
  // yet.

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("sTagGetLaneRelParam %s ", strParams.c_str());

  CParserJson jParser;
  sTagGetLaneRelParam param;
  int ret = jParser.ParseGetLaneRelParam(strParams.c_str(), param);
  if (ret != 0) {
    res = L"{'code':-1}";
    return m_wstrFailed.c_str();
  }

  Json::Value root;
  root["code"] = 0;
  root["message"] = "ok";

  Json::Value maps;

  Json::Value empty_v;

  for (auto hadmapName : param.m_hadmapNames) {
    std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(hadmapName.c_str());

    auto retJsonV = LaneRelDataCache(wstrMapName.c_str());
    auto retPrevJsonV = LanePrevRelDataCache(wstrMapName.c_str());

    if (retJsonV == empty_v && retPrevJsonV == empty_v) {
      SYSTEM_LOGGER_ERROR("hadmap %s 's lane rel has not been computed yet.", hadmapName.c_str());
    }
    Json::Value d;
    d["name"] = hadmapName;
    d["rel_map"] = retJsonV;
    d["prev_map"] = retPrevJsonV;
    // d["msg"] = "no lane rel yet.";
    maps.append(d);
  }

  root["data"] = maps;

  std::string strResult = root.toStyledString();

  res = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

  return m_wstrSuccess.c_str();
}

const wchar_t* CMapDataCache::GetForwardPoints(const wchar_t* wstrParams, std::wstring& res) {
  // parse the req: hadmap key(cos key) array
  // for each hadmap do load it.
  // get res bbox and center info , fold them as array, return it.

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("sTagGetForwardPointsParam %s ", strParams.c_str());

  CParserJson jParser;
  sTagGetForwardPointsParam param;
  int ret = jParser.ParseGetForwardPointsParam(strParams.c_str(), param);
  if (ret != 0) {
    res = L"{'code':-1}";
    return m_wstrFailed.c_str();
  }

  auto root = buildHdserverDefaultRetStr();

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(param.hadmapName.c_str());

  auto retStr = LoadHadMap(wstrMapName.c_str());

  SYSTEM_LOGGER_INFO("Ready to get forward points.");

  if (wcscmp(retStr, m_wstrSuccess.c_str()) == 0) {
    CHadmap* pHadmap = HadmapCache(wstrMapName.c_str());
    if (pHadmap) {
      std::vector<std::vector<hadmap::txLaneId>> laneIds;
      std::vector<std::pair<double, hadmap::PointVec>> points;
      int ret = pHadmap->QueryForwardPoints(param, &laneIds, &points);
      if (ret != 0) {
        res = L"{'code':-1}";
        return m_wstrFailed.c_str();
      } else {
        Json::Value data;
        Json::Value ids;
        Json::Value pts;

        Json::Value pts2d;

        for (auto& pr : points) {
          Json::Value tmp;
          Json::Value ptTmp;
          tmp["curve_length"] = pr.first;
          for (auto& pt : pr.second) {
            Json::Value ptV;
            ptV["x"] = pt.x;
            ptV["y"] = pt.y;
            ptV["z"] = pt.z;

            ptTmp.append(ptV);
          }
          tmp["samples"] = ptTmp;
          pts2d.append(tmp);
        }

        Json::Value ids2d;
        for (auto& idArr : laneIds) {
          Json::Value tmp;
          for (auto& id : idArr) {
            Json::Value idTmp;
            idTmp["rid"] = id.roadId;
            idTmp["sid"] = id.sectionId;
            idTmp["lid"] = id.laneId;

            tmp.append(idTmp);
          }
          ids2d.append(tmp);
        }
        data["laneids2d"] = ids2d;
        data["pt_samples"] = pts2d;

        root["data"] = data;
      }

    } else {
      root["code"] = -1;
      root["message"] = "hadmap not cached(it should not happen). ";
    }

    std::string strResult = root.toStyledString();

    res = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
    // res = wStr;
    // std::wcout << "wStr is: " << wStr << "#" << std::endl;
    return m_wstrSuccess.c_str();
  } else {
    root["code"] = -1;
    root["message"] = "hadmap load failed(it should not happen). ";

    std::string strResult = root.toStyledString();

    res = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    return m_wstrFailed.c_str();
  }
}

const wchar_t* CMapDataCache::CloudHadmapUploadCallback(const wchar_t* wstrParams, std::wstring& res) {
  // parse the req: hadmap key(cos key) array
  // for each hadmap do load it.
  // get res bbox and center info , fold them as array, return it.

  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("cloud hadmap upload callback. sTagCloudHadmapUploadCallbackParam %s ", strParams.c_str());

  CParserJson jParser;
  sTagCloudHadmapUploadCallbackParam param;
  int ret = jParser.ParseCloudHadmapUploadCallbackParam(strParams.c_str(), param);
  if (ret != 0) {
    res = L"{'code':-1}";
    return m_wstrFailed.c_str();
  }

  Json::Value root;
  root["code"] = 0;
  root["message"] = "ok";

  Json::Value maps;

  std::cout << "no of hadmaps: " << param.m_hadmapNames.size() << std::endl;

  for (auto hadmapName : param.m_hadmapNames) {
    std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(hadmapName.c_str());
    auto cosInfo = param.m_hadmapCosInfos[hadmapName];
    auto retStr = LoadHadMap(wstrMapName.c_str(), cosInfo.first, cosInfo.second);
    std::cout << "ret val of loadhadmap: " << retStr << " for hadmap: " << hadmapName
              << "succ check: " << wcscmp(retStr, m_wstrSuccess.c_str())

              << std::endl;
    if (wcscmp(retStr, m_wstrSuccess.c_str()) == 0) {
      Json::Value d;
      d["name"] = hadmapName;

      // tagHadmapAttr *pHA =
      // CEngineConfig::Instance().HadmapAttr(hadmapName);

      CHadmap* pHadmap = HadmapCache(wstrMapName.c_str());
      if (pHadmap) {
        d["status"] = "succ";
        auto abbl = pHadmap->AabbLl();
        auto min = abbl.Min();
        auto max = abbl.Max();

        // CAABB
        // auto center = pHadmap->RefPoint();

        d["center"] = pHadmap->RefPoint().ToString();
        // CVector3
        d["min"] = min.ToString();
        d["max"] = max.ToString();

      } else {
        SYSTEM_LOGGER_ERROR("load hadmap file succeed, but it is not cached. %s", hadmapName.c_str());
        d["status"] = "failed";
        d["msg"] = "load hadmap succeed but not cached.";
      }

      maps.append(d);

    } else {
      SYSTEM_LOGGER_ERROR("load hadmap(hadmap upload callback) failed for %s", hadmapName.c_str());
      Json::Value d;
      d["name"] = hadmapName;
      d["status"] = "failed";
      d["msg"] = "load hadmap failed.";
      maps.append(d);
    }
  }

  if (param.m_hadmapNames.size() == 0) {
    SYSTEM_LOGGER_ERROR(
        "There is no hadmap names provided. for this "
        "upload hadmap callback");
  }

  root["data"] = maps;

  std::string strResult = root.toStyledString();

  res = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());
  // res = wStr;
  // std::wcout << "wStr is: " << wStr << "#" << std::endl;
  return m_wstrSuccess.c_str();
}

const wchar_t* CMapDataCache::LoadHadMap(const wchar_t* wstrHadmapFile, const std::string& cosBucket,
                                         const std::string& cosKey) {
  SYSTEM_LOGGER_INFO("LoadHadMap entering");
  // 文件名为空检查
  if (!wstrHadmapFile) {
    SYSTEM_LOGGER_ERROR("load hadmap file name is null!");
    return m_wstrFailed.c_str();
  }
  std::string strHadmapFile = CEngineConfig::Instance().WStr2MBStr(wstrHadmapFile);

  // 文件类型检查
  boost::filesystem::path p = strHadmapFile;
  std::string strMapName = p.filename().string();
  std::string strExt = p.extension().string();
  if (kHadmapTypeDict.count(strExt) <= 0) {
    SYSTEM_LOGGER_ERROR("load hadmap failed, invalid ext: %s", strExt.c_str());
    return m_wstrFailed.c_str();
  }

  SYSTEM_LOGGER_INFO("start to load hadmap file %s", strHadmapFile.c_str());

  sTagServiceHadmapInfo info;
  info.m_strCosBucket = cosBucket;
  info.m_strCosKey = cosKey;

  // cache check
  CHadmap* pHadmap = HadmapCache(wstrHadmapFile);
  if (pHadmap) {
    int nRet = CMapDataProxy::Instance().GetHadmapInfoFromService(strHadmapFile.c_str(), info);
    if (nRet != HSEC_OK) {
      SYSTEM_LOGGER_ERROR("get hadmap: %s info from service error, use cache data", strHadmapFile.c_str());
      return m_wstrSuccess.c_str();
    }
    nRet = CMapDataProxy::Instance().GetHadmapInfoFromCos(strHadmapFile.c_str(), info);
    if (nRet != HSEC_OK) {
      SYSTEM_LOGGER_ERROR("get hadmap: %s info from cos error", strHadmapFile.c_str());
      return m_wstrSuccess.c_str();
    }

    info.ToTimeStamp();
    if (info.m_llTimestamp <= pHadmap->LastModifiedTime()) {
      SYSTEM_LOGGER_INFO("hadmap: %s in cache and is the latest, now use it", strHadmapFile.c_str());
      return m_wstrSuccess.c_str();
    } else {
      SYSTEM_LOGGER_INFO(
          "hadmap: %s in cache and is not the latest, now "
          "reload from cos",
          strHadmapFile.c_str());
      SYSTEM_LOGGER_INFO("remove all hadmap: %s cache data", strHadmapFile.c_str());
      RemoveHadmapCacheAll(wstrHadmapFile, true);
      DeleteSingleHadmap(strHadmapFile.c_str());
    }
  }

  // load from disk
  boost::filesystem::path initPath = CEngineConfig::Instance().ResourceDir();
  boost::filesystem::path hadmapPath = initPath;
  hadmapPath.append("hadmap");

  boost::filesystem::path filePath = hadmapPath.append(strHadmapFile);

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

    SYSTEM_LOGGER_INFO("hadmap: %s not in cache, now start to load from local", strHadmapFile.c_str());

    if (!boost::filesystem::exists(filePath)) {
      SYSTEM_LOGGER_INFO("hadmap: %s not in local, now download from s3", strHadmapFile.c_str());

      int res = CMapDataProxy::Instance().FetchHadmap(filePath.filename().string().c_str(), info);
      if (res != HSEC_OK) {
        SYSTEM_LOGGER_ERROR("Fetch hamp failed  (in load hadmap)");
        return m_wstrFailed.c_str();
      }
    }

    SYSTEM_LOGGER_INFO("hadmap: %s in local, start to load from local", strHadmapFile.c_str());

    CMapFile mapfile;
    int nRet = mapfile.ParseMapV2(filePath.string().c_str());
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("load hadmap file failed!");
      return m_wstrFailed.c_str();
    }

    SYSTEM_LOGGER_INFO("hadmap: %s load finished. now rearrange data", strHadmapFile.c_str());

    pHadmap = new CHadmap();
    pHadmap->MapQuery().SettxMapHandle(mapfile.pMapHandler);
    pHadmap->MapQuery().SetInitialize(true);

    pHadmap->SetEtag(info.m_strEtag);
    pHadmap->SetFileSize(boost::filesystem::file_size(filePath));
    // set last modified time
    std::time_t fileTime = boost::filesystem::last_write_time(filePath);
    int64_t llLastedModifiedTime = fileTime;
    pHadmap->SetLastModifiedTime(llLastedModifiedTime);
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
    // it seems this center is used nowhere. Comment it now.
    // CVector3 center;
    // center.X() = mapfile.m_center[0];
    // center.Y() = mapfile.m_center[1];
    // center.Z() = mapfile.m_center[2];
    // pHadmap->SetCenter(center);

    // 参考点
    /*CPoint3D refPoint;
    tagHadmapAttr *hadmapAttr =
        CEngineConfig::Instance().HadmapAttr(strMapName);
    if (!hadmapAttr) {
        SYSTEM_LOGGER_ERROR("hadmap file not exist in config file!");
        assert(false);
        return m_wstrFailed.c_str();
    }*/
    pHadmap->SetRefPoint(mapfile.m_refPt);

    CVector3 mi(mapfile.m_envelop_min[0], mapfile.m_envelop_min[1], mapfile.m_envelop_min[2]);
    CVector3 ma(mapfile.m_envelop_max[0], mapfile.m_envelop_max[1], mapfile.m_envelop_max[2]);
    CAABB aabbLl(mi, ma);

    pHadmap->SetAabbLl(aabbLl);

    // 保存地图绝对路径
    pHadmap->SetFullPath(filePath.string().c_str());
    pHadmap->SetFileName(strMapName);
    pHadmap->ProcessMapData();
    /*
     * 先ComposeCacheData，后PushHadmapCache
     * 这两句顺序不能颠倒，保证cache先于hadmap被设置
     **/

    ComposeCacheData(wstrHadmapFile, pHadmap);
    PushHadmapCache(wstrHadmapFile, pHadmap);

    SYSTEM_LOGGER_INFO("hadmap %s rearragne finished.", strHadmapFile.c_str());

    SYSTEM_LOGGER_INFO("hadmap %s loaded.", strHadmapFile.c_str());

    return m_wstrSuccess.c_str();
  }
}

const std::wstring CMapDataCache::LoadHadMap(const wchar_t* wstrHadmapFile, bool fineRsp, const std::string& cosBucket,
                                             const std::string& cosKey) {
  auto res = LoadHadMap(wstrHadmapFile, cosBucket, cosKey);
  if (!fineRsp) {
    return res;
  }

  if (wcscmp(res, m_wstrSuccess.c_str()) != 0) {
    return MakeRspWithCodeMessageData(-1, "laodhadmap failed.");
  }

  CHadmap* hadmap = HadmapCache(wstrHadmapFile);
  if (!hadmap) {
    return MakeRspWithCodeMessageData(-2, "not found in cache");
  }

  // get unrealLevelIndex from hadmap_config.xml
  std::string map_name = hadmap->Name();
  auto pos = map_name.find_first_of(VERSION_NAME_DELIM);
  if (pos != std::string::npos) {
    map_name = map_name.substr(pos + 1);
  }
  int unrealLevelIndex = 0;
  tagHadmapAttr* attr = CEngineConfig::Instance().HadmapAttr(map_name);
  if (attr) {
    unrealLevelIndex = attr->m_unRealLevelIndex;
  }

  Json::Value data;
  data["unrealLevelIndex"] = unrealLevelIndex;
  data["min"] = hadmap->AabbLl().Min().ToString();
  data["max"] = hadmap->AabbLl().Max().ToString();
  data["center"] = hadmap->RefPoint().ToString();
  data["etag"] = hadmap->Etag();
  data["fileSize"] = hadmap->FileSize();
  return MakeRspWithCodeMessageData(0, "ok", data);
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

  Json::Value root;
  // hadmap 路径存在
  if (boost::filesystem::exists(hadmapPath)) {
    boost::filesystem::directory_iterator itr_end;
    boost::filesystem::directory_iterator itr(hadmapPath);

    for (; itr != itr_end; ++itr) {
      std::string strName = (*itr).path().string();
      std::string strExt = (*itr).path().extension().string();
      if ((!boost::filesystem::is_directory(*itr)) && kHadmapTypeDict.count(strExt) > 0) {
        files.push_back((*itr).path().filename().string());

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
  SYSTEM_LOGGER_INFO("delete hadmaps %s ", strParams.c_str());

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
    CHadmapSceneData::Instance().DeleteHadmapSceneData(wstrName.c_str());

    DeleteSingleHadmap(itr->c_str());
    CEngineConfig::Instance().SaveToHadmapConfig();
  }

  return m_wstrConvertToJsonSucceed.c_str();
}

const wchar_t* CMapDataCache::DownloadHadmaps(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("download hadmaps %s ", strParams.c_str());

  CParserJson jParser;
  sTagHadmapsDownloadParams param;
  int ret = jParser.ParseHadmapsDownloadParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  boost::filesystem::path srcPath = CEngineConfig::Instance().HadmapDir();

  std::vector<std::string>::iterator itr = param.m_hadmapNames.begin();
  for (; itr != param.m_hadmapNames.end(); ++itr) {
    CopySingleHadmap(itr->c_str(), srcPath.string().c_str(), param.m_strDestName.c_str());
  }
  return m_wstrConvertToJsonSucceed.c_str();
}

const wchar_t* CMapDataCache::UploadHadmaps(const wchar_t* wstrParams) {
  std::string strParams = CEngineConfig::Instance().WStr2MBStr(wstrParams);
  SYSTEM_LOGGER_INFO("upload hadmaps %s ", strParams.c_str());

  CParserJson jParser;
  sTagHadmapsUploadParams param;
  int ret = jParser.ParseHadmapsUploadParams(strParams.c_str(), param);
  if (ret != 0) {
    return L"{'code':-1}";
  }

  std::vector<boost::filesystem::path> files;
  boost::filesystem::path srcPath = param.m_strSourceDir;
  srcPath = boost::filesystem::absolute(srcPath, "\\");
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
        if ((!boost::filesystem::is_directory(*itr)) && kHadmapTypeDict.count(strExt) > 0) {
          files.push_back(itr->path());
        }
      }
    }
  } else if (boost::iequals(param.m_strType, "files")) {
    std::vector<std::string>::iterator itr = param.m_hadmapNames.begin();
    for (; itr != param.m_hadmapNames.end(); ++itr) {
      boost::filesystem::path p = srcPath;
      p /= (*itr);

      std::string strExt = p.extension().string();
      if ((!boost::filesystem::is_directory(p)) && kHadmapTypeDict.count(strExt) > 0) {
        files.push_back(p);
      }
    }
  } else {
    SYSTEM_LOGGER_ERROR("upload hadmaps param error!");
  }

  boost::filesystem::path destPath = CEngineConfig::Instance().HadmapDir();

  // copy hadmap file
  std::vector<boost::filesystem::path>::iterator itr = files.begin();
  for (; itr != files.end(); ++itr) {
    // copy file
    std::string strName = itr->filename().string();
    // 存在不拷贝
    CopySingleHadmap(strName.c_str(), hadmapSrcPath.string().c_str(), destPath.string().c_str());

    // 存在不覆盖
    UpdateHadmapConfig(itr->string().c_str(), strName.c_str());
  }

  return m_wstrConvertToJsonSucceed.c_str();
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

    SYSTEM_LOGGER_INFO(
        "query near by lane. Lon: %d, Lat: %d finished. convert to "
        "wstr",
        param.m_dLon, param.m_dLat);

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

    double dLon = atof(strLon.c_str());
    double dLat = atof(strLat.c_str());
    roadpkid roadID = 0;
    sectionpkid secID = 0;
    lanepkid laneID = 0;

    int nRet = pHadmap->QueryNearbyLane(dLon, dLat, roadID, secID, laneID);

    SYSTEM_LOGGER_INFO(
        "query near by lane. Lon: %s, Lat: %s finished. convert to "
        "wstr",
        strLon.c_str(), strLat.c_str());
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

const wchar_t* CMapDataCache::QueryInfoByPt(const wchar_t* wstrMapName, const wchar_t* wstrLon,
                                            const wchar_t* wstrLat) {
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

    SYSTEM_LOGGER_INFO("query info by pt. Lon: %s, Lat: %s start", strLon.c_str(), strLat.c_str());

    std::string strType = "";
    double dLon = atof(strLon.c_str());
    double dLat = atof(strLat.c_str());
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
    std::string rname;

    int nRetLane = pHadmap->GetLaneByPointLL(dLon, dLat, roadID, secID, laneID, dLaneLon, dLaneLat, dLaneAlt, dLaneDist,
                                             dLaneWidth, dLaneOffset, rname);

    int nRetLanelink = pHadmap->GetLanelinkByPointLL(dLon, dLat, lanelinkID, dLanelinkLon, dLanelinkLat, dLanelinkAlt,
                                                     dLanelinkDist, dLanelinkOffset);

    SYSTEM_LOGGER_INFO("QueryInfoByPt query finished! input(%s, %s)", strLon.c_str(), strLat.c_str());
    if (nRetLane != 0) {
      SYSTEM_LOGGER_INFO("result: no return lane.");
    } else {
      SYSTEM_LOGGER_INFO(
          "result: lane id(%u,%u,%d), paddle (%f, %f, "
          "%f), dist: %f, width: %f, offset: %f",
          roadID, secID, laneID, dLaneLon, dLaneLat, dLaneAlt, dLaneDist, dLaneWidth, dLaneOffset);
    }
    if (nRetLanelink != 0) {
      SYSTEM_LOGGER_INFO("result: no return lanelink.");
    } else {
      SYSTEM_LOGGER_INFO(
          "result: lanelink(%u), paddle (%f, %f, %f), "
          "dist: %f, offset: %f finished.",
          lanelinkID, dLanelinkLon, dLanelinkLat, dLanelinkAlt, dLanelinkDist, dLanelinkOffset);
    }

    Json::Value root;

    root["ranme"] = rname;

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
      } else {
        root["err"] = 0;
        root["type"] = "lanelink";
        root["llid"] = Json::Value::UInt64(lanelinkID);
        root["lon"] = dLanelinkLon;
        root["lat"] = dLanelinkLat;
        root["alt"] = dLanelinkAlt;
        root["dist"] = dLanelinkDist;
        root["offset"] = dLanelinkOffset;
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

    double dLon = atof(strX.c_str());
    double dLat = atof(strY.c_str());
    double dAlt = atof(strZ.c_str());
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

    nRet = pHadmap->GetLaneByPointLL(dLon, dLat, roadID, secID, laneID, dLaneLon, dLaneLat, dLaneAlt, dDist, dWidth,
                                     dOffset);

    SYSTEM_LOGGER_INFO(
        "query info by pt. X: %s, Y: %s , Z: %s finished. convert to "
        "wstr",
        strX.c_str(), strY.c_str(), strZ.c_str());
    if (nRet != 0) {
      return m_wstrConvertToJsonFailed.c_str();
    }

    /*double dTmpLon = 0;
    double dTmpLat = 0;
    double dTmpWidth = 0;
    pHadmap->QueryLonLat(roadID, secID, laneID, dDist, dOffset, dTmpLon,
    dTmpLat,dTmpWidth); double dTmpLon1 = dTmpLon; double dTmpLat1 =
    dTmpLat; double dTmpAlt1 = 0;
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

    roadpkid roadID = atol(strRoadID.c_str());
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

    roadpkid roadID = atol(strRoadID.c_str());
    sectionpkid secID = atol(strSecID.c_str());
    lanepkid laneID = atol(strLaneID.c_str());
    double dShift = atof(strShift.c_str());
    double dOffset = atof(strOffset.c_str());
    double dLon = -999;
    double dLat = -999;
    double dAlt = -999;
    double dWidth = -999;
    int nRet = pHadmap->QueryLonLat(roadID, secID, laneID, dShift, dOffset, dLon, dLat, dAlt, dWidth);

    SYSTEM_LOGGER_INFO(
        "query lon lat input: lane(%s, %s, %s) st (%s, %s) "
        "result:(%f, %f, %f). convert to wstr",
        strRoadID.c_str(), strSecID.c_str(), strLaneID.c_str(), strShift.c_str(), strOffset.c_str(), dLon, dLat, dAlt);

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
                                                 const wchar_t* wstrOffset, bool start) {
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

    SYSTEM_LOGGER_INFO("query position (%s, %s)-(%s: %s)-(%s, %s)", strLon.c_str(), strLat.c_str(), strElemType.c_str(),
                       strElemID.c_str(), strShift.c_str(), strOffset.c_str());

    double dLon = atof(strLon.c_str());
    double dLat = atof(strLat.c_str());
    double dShift = atof(strShift.c_str());
    double dOffset = atof(strOffset.c_str());
    double dFinalLon = -999;
    double dFinalLat = -999;
    double dFinalAlt = -999;
    double dWidth = -999;
    double dFinalShift = -999;

    int nRet = -1;
    if (boost::iequals(strElemType, "lane")) {
      lanepkid laneID = atol(strElemID.c_str());
      nRet = pHadmap->QueryLonLatByPoint(dLon, dLat, laneID, dShift, dOffset, dFinalLon, dFinalLat, dFinalAlt, dWidth,
                                         dFinalShift, start);

    } else if (boost::iequals(strElemType, "lanelink")) {
      dWidth = 0;
      lanelinkpkid lanelinkID = atol(strElemID.c_str());
      nRet = pHadmap->QueryLonLatByPointOnLanelink(dLon, dLat, lanelinkID, dShift, dOffset, dFinalLon, dFinalLat,
                                                   dFinalAlt, dFinalShift);
    } else {
      SYSTEM_LOGGER_ERROR("queryLonLatByPoint elemtype error!");
    }

    SYSTEM_LOGGER_INFO("query position(%s, %s)-(%s, %s) finished. convert to wstr", strLon.c_str(), strLat.c_str(),
                       strShift.c_str(), strOffset.c_str());
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

const std::wstring CMapDataCache::SearchPathImpl(const std::string& param) {
  Json::Value root;
  std::string error_msg;
  if (misc::ParseJson(param.c_str(), &root, error_msg) != HSEC_OK) {
    return MakeRspWithCodeMessageData(-1, error_msg);
  }

  if (!root.isObject() || !root["name"].isString() || !root["version"].isString() || !root["via_points"].isArray()) {
    return MakeRspWithCodeMessageData(-1, std::string("Param is invalid: ") + param);
  }

  std::string hadmap_name = computeMapCompleteName(root["name"].asString(), root["version"].asString());
  auto w_hadmap_name = CEngineConfig::Instance().MBStr2WStr(hadmap_name.c_str());
  CHadmap* hadmap = HadmapCache(w_hadmap_name.c_str());
  if (!hadmap) {
    return MakeRspWithCodeMessageData(-2, "Map not found in cache, need to call loadhadmap first");
  }

  hadmap::PointVec from_to;
  for (auto& point : root.get("via_points", Json::Value(Json::arrayValue))) {
    try {
      from_to.emplace_back(hadmap::txPoint(point["x"].asDouble(), point["y"].asDouble(), point["z"].asDouble()));
    } catch (const std::exception& ex) {
      return MakeRspWithCodeMessageData(-1, std::string("Param is invalid: ") + Json::FastWriter{}.write(point));
    }
  }
  if (from_to.empty()) {
    return MakeRspWithCodeMessageData(-1, std::string("Param is invalid: ") + param);
  }

  hadmap::PointVec geoms;
  try {
    hadmap->MapQuery().SearchPath(from_to, geoms);
  } catch (const std::exception& ex) {
    SYSTEM_LOGGER_ERROR("search path ex: %s", ex.what());
  }
  Json::Value data(Json::arrayValue);
  for (auto&& geom : geoms) {
    Json::Value one(Json::objectValue);
    one["x"] = geom.x;
    one["y"] = geom.y;
    one["z"] = geom.z;
    data.append(one);
  }
  return MakeRspWithCodeMessageData(0, data.empty() ? "Not found" : "Found", data);
}

int CMapDataCache::DeleteSingleHadmap(const char* strFileName) {
  boost::filesystem::path hadmapPath = CSceneUtil::HadmapFullPath(strFileName);

  bool bRet = false;
  if (boost::filesystem::exists(hadmapPath) && boost::filesystem::is_regular_file(hadmapPath)) {
    // delete hadmap file
    bRet = boost::filesystem::remove(hadmapPath);
    if (bRet) {
      return 0;
    }
  }

  return -1;
}

int CMapDataCache::CopySingleHadmap(const char* strFileName, const char* strSrcDir, const char* strDestDir) {
  boost::filesystem::path srcPath = strSrcDir;
  boost::filesystem::path destPath = strDestDir;

  if (!boost::filesystem::exists(destPath) || boost::filesystem::is_regular_file(destPath) ||
      !boost::filesystem::exists(srcPath) || boost::filesystem::is_regular_file(srcPath)) {
    SYSTEM_LOGGER_ERROR("copy single hadmap dest dir not exist");
    return HSEC_DIRECTORY_NOT_EXIST;
  }

  std::string strFullName = CSceneUtil::HadmapFullName(strFileName);
  boost::filesystem::path hadmapSrcPath = CSceneUtil::HadmapPath(strSrcDir, strFullName.c_str());

  if (boost::filesystem::exists(hadmapSrcPath) && boost::filesystem::is_regular_file(hadmapSrcPath)) {
    // copy hadmap file
    boost::filesystem::path hadmapDestPath = destPath;
    hadmapDestPath /= strFullName;

    if (boost::filesystem::exists(hadmapDestPath)) {
      SYSTEM_LOGGER_ERROR("copy hadmap file ignore : %s already exist!", strFullName.c_str());
      return HSEC_OK;
    }

    try {
      boost::filesystem::copy_file(hadmapSrcPath, hadmapDestPath, BOOST_COPY_OPTION);
    } catch (std::exception e) {
      SYSTEM_LOGGER_ERROR("copy hadmap file %s error!", strFullName.c_str());
      return HSEC_COPY_HADMAP_FAILED;
    }
  }

  return HSEC_OK;
}

int CMapDataCache::UpdateHadmapConfig(const char* strFullName, const char* strName) {
  // 存在不覆盖
  tagHadmapAttr* pHA = CEngineConfig::Instance().HadmapAttr(strName);
  if (!pHA) {
    // get hadmap parameter
    CMapFile mapfile;
    int nRet = mapfile.ParseMapV2(strFullName, false);
    if (nRet != 0) {
      SYSTEM_LOGGER_ERROR("open hadmap %s error", strFullName);
      return nRet;
      // return m_wstrMapFileFormatError.c_str();
    } else {
      int nIndex = CEngineConfig::Instance().HadmapIndex(strName);
      // chanage attr 1
      tagHadmapAttr attr;
      attr.m_name = strName;
      attr.m_unRealLevelIndex = nIndex;
      attr.m_strUnrealLevelIndex = std::to_string(nIndex);
      attr.m_dRefLon = mapfile.m_center[0];
      attr.m_dRefLat = mapfile.m_center[1];
      attr.m_dRefAlt = mapfile.m_center[2];
      attr.m_strRefAlt = std::to_string(attr.m_dRefAlt);
      attr.m_strRefLon = std::to_string(attr.m_dRefLon);
      attr.m_strRefLat = std::to_string(attr.m_dRefLat);

      CEngineConfig::Instance().UpdateHadmapAttr(strName, attr);
      std::wstring wstrName = CEngineConfig::Instance().MBStr2WStr(strName);
      RemoveHadmapCacheAll(wstrName.c_str(), false);
    }
  }

  return HSEC_OK;
}

void CMapDataCache::ComposeCacheData(const wchar_t* wstrHadmapFile, CHadmap* pHadmap) {
  // 文件名为空检查
  if (!pHadmap || !wstrHadmapFile) {
    SYSTEM_LOGGER_ERROR("hadmap is null!");
    return;
  }

  std::string strHadmapFile = CEngineConfig::Instance().WStr2MBStr(wstrHadmapFile);

  boost::format LocalCoordiation("%.3f");
  SYSTEM_LOGGER_INFO("compose lanelinks data begin");

  auto abbl = pHadmap->AabbLl();
  auto min = abbl.Min();
  auto max = abbl.Max();

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
        /*
        point["x"] = Json::Value(pitr->X());
        point["y"] = Json::Value(pitr->Y());
        point["z"] = Json::Value(pitr->Z());
        */

        double x = COMPOSE_DOUBLE(pitr->X());
        double y = COMPOSE_DOUBLE(pitr->Y());
        double z = COMPOSE_DOUBLE(pitr->Z());
        point["x"] = Json::Value(x);
        point["y"] = Json::Value(y);
        point["z"] = Json::Value(z);

        /*
        std::string s = point.toStyledString();
        s = ll.toStyledString();
        */
        ll["array"].append(point);
      }

      root["array"].append(ll);
    }
    root["center"] = pHadmap->RefPoint().ToString();
    root["min"] = min.ToString();
    root["max"] = max.ToString();
    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root, true);

    std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushLaneLinkCache(wstrHadmapFile, wstrResult.c_str());
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

      root["array"].append(ll);
    }
    root["center"] = pHadmap->RefPoint().ToString();
    root["min"] = min.ToString();
    root["max"] = max.ToString();
    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root, true);

    std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushObjectCache(wstrHadmapFile, wstrResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose map object data end");

  SYSTEM_LOGGER_INFO("compose traffic lights data begin");
  // traffic lights
  {
    LogicLightVec& logiclights = pHadmap->GetTrafficLights();

    Json::Value root;
    root["activePlan"] = "0";

    for (auto it : logiclights) {
      for (int n = 0; n < 1; n++) {
        Json::Value signlight;
        signlight["id"] = std::to_string(atoi(it->m_strID.c_str()) + n);
        signlight["routeID"] = std::to_string(atoi(it->m_strRouteID.c_str()) + n);
        signlight["start_s"] = it->m_strStartShift;
        signlight["start_t"] = it->m_strStartTime;
        signlight["l_offset"] = std::to_string(atof(it->m_strOffset.c_str()));
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
        signlight["signalHead"] = utils::Join(it->m_strSignalheadVec, ",");
        for (auto& itPharse : it->m_parsetolanes) {
          Json::Value relative;
          relative["phase"] = itPharse.first;
          std::sort(itPharse.second.begin(), itPharse.second.end());
          itPharse.second.erase(std::unique(itPharse.second.begin(), itPharse.second.end()), itPharse.second.end());
          std::sort(itPharse.second.rbegin(), itPharse.second.rend());

          relative["lane"] = utils::Join(itPharse.second, ",");
          relative["tolanelinkids"] = it->m_parsetolanelinks.count(itPharse.first) <= 0
                                          ? ""
                                          : utils::Join(it->m_parsetolanelinks.at(itPharse.first), ",");
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
      }
    }
    auto& junction2TrafficRoadMap = pHadmap->GetJunction2TrafficRoadMap();
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
    std::string strResult = CParserJson::ToJsonString(root, true);

    std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushTrafficLightCache(wstrHadmapFile, wstrResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose traffic lights data end");

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
        /*
        point["x"] = Json::Value(pitr->X());
        point["y"] = Json::Value(pitr->Y());
        point["z"] = Json::Value(pitr->Z());
        */
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
    root["center"] = pHadmap->RefPoint().ToString();
    root["min"] = min.ToString();
    root["max"] = max.ToString();
    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root, true);

    std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushRoadCache(wstrHadmapFile, wstrResult.c_str());
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
        /*
        point["x"] = Json::Value(pitr->X());
        point["y"] = Json::Value(pitr->Y());
        point["z"] = Json::Value(pitr->Z());
        */
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
    root["center"] = pHadmap->RefPoint().ToString();
    root["min"] = min.ToString();
    root["max"] = max.ToString();
    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root, true);

    std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushLaneCache(wstrHadmapFile, wstrResult.c_str());
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
      ll["mark"] = Json::Value((*lbItr)->LaneMark());
      PointsVec::iterator pitr = datas.begin();
      for (; pitr != datas.end(); ++pitr) {
        Json::Value point;
        /*
        point["x"] = Json::Value(pitr->X());
        point["y"] = Json::Value(pitr->Y());
        point["z"] = Json::Value(pitr->Z());
        */

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
    root["center"] = pHadmap->RefPoint().ToString();
    root["min"] = min.ToString();
    root["max"] = max.ToString();
    // std::string strResult = root.toStyledString();
    std::string strResult = CParserJson::ToJsonString(root, true);

    std::wstring wstrResult = CEngineConfig::Instance().MBStr2WStr(strResult.c_str());

    PushLaneBoundaryCache(wstrHadmapFile, wstrResult.c_str());
  }
  SYSTEM_LOGGER_INFO("compose lane boundary data end");

  SYSTEM_LOGGER_INFO("start compute lane rel (bg)");

  auto tmpLanes = pHadmap->Lanes();

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  std::map<uint64, std::set<uint64>> roadRelsM;
  std::thread bg_lane_rel([&]() {
    Json::Value mapLaneRelM;
    Json::Value mapLaneRRelM;  // 反向关系
    std::cout << "total lanes: " << tmpLanes.size() << std::endl;

    for (auto l : tmpLanes) {
      auto roadID = l->RoadId();
      auto secID = l->SectionId();
      auto laneID = l->LaneId();

      // Json::Value relM;
      ::hadmap::tx_lane_id_t t(roadID, secID, laneID);

      LaneIDVec LaneIds;
      int nRet = pHadmap->QueryNextLanes(roadID, secID, laneID, LaneIds);
      if (nRet != 0) {
        SYSTEM_LOGGER_ERROR("start compute lane rel failed for: %d, %d, %d", roadID, secID, laneID);
      } else {
        for (size_t i = 0, isize = LaneIds.size(); i < isize; i++) {
          roadpkid nextRoadID = LaneIds[i].m_roadID;
          sectionpkid nextSecID = LaneIds[i].m_sectionID;
          lanepkid nextLaneID = LaneIds[i].m_laneID;
          // 正向关系，next
          Json::Value root;
          root["rid"] = Json::Value::UInt64(nextRoadID);
          root["sid"] = Json::Value::UInt64(nextSecID);
          root["lid"] = Json::Value::Int64(nextLaneID);
          mapLaneRelM[t.ToString()].append(root);

          // 反向关系，prev
          Json::Value rroot;
          rroot["rid"] = Json::Value::UInt64(roadID);
          rroot["sid"] = Json::Value::UInt64(secID);
          rroot["lid"] = Json::Value::Int64(laneID);
          ::hadmap::tx_lane_id_t rt(nextRoadID, nextSecID, nextLaneID);
          mapLaneRRelM[rt.ToString()].append(rroot);

          // update RoadRelMap
          if (nextRoadID != roadID) {
            auto rIter = roadRelsM.find(roadID);
            if (rIter != roadRelsM.end()) {
              std::set<uint64>& toRoadS = rIter->second;
              toRoadS.insert(nextRoadID);
            } else {
              std::set<uint64> tmp;
              tmp.insert(nextRoadID);
              roadRelsM.insert(std::make_pair(roadID, tmp));
            }
          }
        }
      }
    }

    Json::Value finalRoadRelM;   // next
    Json::Value finalRoadRRelM;  // prev
    for (auto p : roadRelsM) {
      auto currId = p.first;
      auto sets = p.second;

      Json::Value currRoad;
      currRoad["rid"] = std::to_string(currId);

      Json::Value toRoads;
      for (auto toId : sets) {
        Json::Value root;
        root["rid"] = std::to_string(toId);
        toRoads.append(root);

        finalRoadRRelM[std::to_string(toId)].append(currRoad);
      }
      finalRoadRelM[std::to_string(currId)] = toRoads;
    }

    this->PushRoadRelDataCache(wstrHadmapFile, finalRoadRelM, finalRoadRRelM);
    this->PushLaneRelDataCache(wstrHadmapFile, mapLaneRelM, mapLaneRRelM);
    // this->m_laneRelM.insert(std::make_pair());

    std::cout << " bg thread done." << std::endl;
  });
  bg_lane_rel.join();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  // bg_lane_rel.detach();

  // bg_threads[strHadmapFile] = std::move(bg_lane_rel);

  SYSTEM_LOGGER_INFO("start compute lane rel (bg) done. cost %d", (end - begin));

  /*Json::FastWriter writer;
  std::string strWrite = writer.write(root);
  nLen = strWrite.size();
  wchar_t* pWstrWrite = new wchar_t[nLen];
  memset(pWstrWrite, 0, nLen * sizeof(wchar_t));
  mbstowcs(pWstrWrite, strWrite.c_str(), strWrite.size());*/
}

void CMapDataCache::RemoveHadmapCacheAll(const wchar_t* wstrMapName, bool lock /*=true*/) {
  if (!wstrMapName) {
    return;
  }
  RemoveHadmapCache(wstrMapName, lock);
  RemoveRoadCache(wstrMapName);
  RemoveLaneCache(wstrMapName);
  RemoveLaneBoundaryCache(wstrMapName);
  RemoveLaneLinkCache(wstrMapName);
  RemoveObjectCache(wstrMapName);
  RemoveTrafficLightCache(wstrMapName);
  RemoveLaneRelDataCache(wstrMapName);
  RemoveRoadRelDataCache(wstrMapName);
}

void CMapDataCache::RemoveHadmapCache(const wchar_t* wstrMapName, bool lock /*= true*/) {
  if (!wstrMapName) {
    return;
  }
  // todo: may be dead lock
  if (lock) {
    m_mutexHadmapMutex.lock();
  }
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

  if (lock) {
    m_mutexHadmapMutex.unlock();
  }
}

CHadmap* CMapDataCache::HadmapCache(const wchar_t* wstrMapName) {
  std::string name = CEngineConfig::Instance().WStr2MBStr(wstrMapName);
  std::string mapName = adapter::HadmapOptionsAdapter(name);
  if (!wstrMapName) {
    return nullptr;
  }

  HadmapCaches::iterator itr = m_hadmapCaches.find(CEngineConfig::Instance().MBStr2WStr(mapName.c_str()));
  if (itr != m_hadmapCaches.end()) {
    return itr->second;
  }

  return nullptr;
}

std::string CMapDataCache::HadmapEtagCache(const wchar_t* wstrMapName) {
  std::unique_lock<std::mutex> _(m_mutexHadmapMutex);

  auto hadmap = HadmapCache(wstrMapName);
  if (!hadmap || !(hadmap->Etag().empty())) {
    return !hadmap ? "" : hadmap->Etag();
  }

  boost::filesystem::path file = CEngineConfig::Instance().ResourceDir();
  file /= "hadmap";
  file /= CEngineConfig::Instance().WStr2MBStr(wstrMapName);
  hadmap->SetEtag(boost::filesystem::exists(file) ? GenMD5Sum(file.string()) : "");
  return hadmap->Etag();
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

const wchar_t* CMapDataCache::RoadDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return L"";
  }

  MapDataCaches::iterator itr = m_mapRoadCaches.find(wstrMapName);
  if (itr != m_mapRoadCaches.end()) {
    return itr->second.c_str();
  }

  return L"";
}

void CMapDataCache::PushRoadCache(const wchar_t* wstrMapName, const wchar_t* wstrContent) {
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

const wchar_t* CMapDataCache::LaneDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return L"";
  }

  MapDataCaches::iterator itr = m_mapLaneCaches.find(wstrMapName);
  if (itr != m_mapLaneCaches.end()) {
    return itr->second.c_str();
  }

  return L"";
}

void CMapDataCache::PushLaneCache(const wchar_t* wstrMapName, const wchar_t* wstrContent) {
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

const wchar_t* CMapDataCache::LaneBoundaryDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return L"";
  }

  MapDataCaches::iterator itr = m_mapLaneBoundaryCaches.find(wstrMapName);
  if (itr != m_mapLaneBoundaryCaches.end()) {
    return itr->second.c_str();
  }

  return L"";
}

void CMapDataCache::PushLaneBoundaryCache(const wchar_t* wstrMapName, const wchar_t* wstrContent) {
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
const wchar_t* CMapDataCache::LaneLinkDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return L"";
  }

  MapDataCaches::iterator itr = m_mapLanelinkCaches.find(wstrMapName);
  if (itr != m_mapLanelinkCaches.end()) {
    return itr->second.c_str();
  }

  return L"";
}

void CMapDataCache::PushLaneLinkCache(const wchar_t* wstrMapName, const wchar_t* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  MapDataCaches::iterator itr = m_mapLanelinkCaches.find(wstrMapName);
  if (itr != m_mapLanelinkCaches.end()) {
    return;
  }

  m_mapLanelinkCaches.insert(std::make_pair(wstrMapName, wstrContent));
}

void CMapDataCache::RemoveRoadRelDataCache(const wchar_t* strMapName) {
  if (!strMapName) {
    return;
  }

  auto itr = mRoadRelM.find(strMapName);
  if (itr != mRoadRelM.end()) {
    mRoadRelM.erase(itr);
  }

  auto itrPrev = mRoadPrevRelM.find(strMapName);
  if (itrPrev != mRoadPrevRelM.end()) {
    mRoadPrevRelM.erase(itrPrev);
  }
}

Json::Value CMapDataCache::RoadRelDataCache(const wchar_t* strMapName) {
  if (!strMapName) {
    return "";
  }
  auto itr = mRoadRelM.find(strMapName);
  if (itr != mRoadRelM.end()) {
    return itr->second;
  }

  return Json::Value();
}

Json::Value CMapDataCache::RoadPrevRelDataCache(const wchar_t* strMapName) {
  if (!strMapName) {
    return "";
  }
  auto itr = mRoadPrevRelM.find(strMapName);
  if (itr != mRoadPrevRelM.end()) {
    return itr->second;
  }

  return Json::Value();
}

void CMapDataCache::PushRoadRelDataCache(const wchar_t* strMapName, Json::Value v, Json::Value PrevV) {
  if (!strMapName) {
    return;
  }

  if (v != "") {
    auto itr = mRoadRelM.find(strMapName);
    if (itr != mRoadRelM.end()) {
      SYSTEM_LOGGER_INFO(
          "Found the lane rel data cache already exist for: %s overwrite "
          "it "
          "!!!",
          strMapName);
    }
    mRoadRelM.insert(std::make_pair(strMapName, v));
  }

  if (PrevV != "") {
    auto itr = mRoadPrevRelM.find(strMapName);
    if (itr != mRoadPrevRelM.end()) {
      SYSTEM_LOGGER_INFO(
          "Found the lane prev rel data cache already exist for: %s "
          "overwrite it "
          "!!!",
          strMapName);
    }
    mRoadPrevRelM.insert(std::make_pair(strMapName, PrevV));
  }
}

void CMapDataCache::RemoveLaneRelDataCache(const wchar_t* strMapName) {
  if (!strMapName) {
    return;
  }

  auto itr = m_laneRelM.find(strMapName);
  if (itr != m_laneRelM.end()) {
    m_laneRelM.erase(itr);
  }

  auto itrPrev = m_lanePrevRelM.find(strMapName);
  if (itrPrev != m_lanePrevRelM.end()) {
    m_lanePrevRelM.erase(itrPrev);
  }
}

Json::Value CMapDataCache::LaneRelDataCache(const wchar_t* strMapName) {
  if (!strMapName) {
    return "";
  }

  auto itr = m_laneRelM.find(strMapName);
  if (itr != m_laneRelM.end()) {
    return itr->second;
  }

  return Json::Value();
}

Json::Value CMapDataCache::LanePrevRelDataCache(const wchar_t* strMapName) {
  if (!strMapName) {
    return "";
  }

  auto itr = m_lanePrevRelM.find(strMapName);
  if (itr != m_lanePrevRelM.end()) {
    return itr->second;
  }

  return Json::Value();
}

void CMapDataCache::PushLaneRelDataCache(const wchar_t* strMapName, Json::Value v, Json::Value PrevV) {
  if (!strMapName) {
    return;
  }

  if (v != "") {
    auto itr = m_laneRelM.find(strMapName);
    if (itr != m_laneRelM.end()) {
      SYSTEM_LOGGER_INFO(
          "Found the lane rel data cache already exist for: %s overwrite "
          "it "
          "!!!",
          strMapName);
    }
    m_laneRelM.insert(std::make_pair(strMapName, v));
  }

  if (PrevV != "") {
    auto itr = m_lanePrevRelM.find(strMapName);
    if (itr != m_lanePrevRelM.end()) {
      SYSTEM_LOGGER_INFO(
          "Found the lane prev rel data cache already exist for: %s "
          "overwrite it "
          "!!!",
          strMapName);
    }
    m_lanePrevRelM.insert(std::make_pair(strMapName, PrevV));
  }
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

const wchar_t* CMapDataCache::ObjectDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return L"";
  }

  MapDataCaches::iterator itr = m_mapObjCaches.find(wstrMapName);
  if (itr != m_mapObjCaches.end()) {
    return itr->second.c_str();
  }

  return L"";
}

void CMapDataCache::PushObjectCache(const wchar_t* wstrMapName, const wchar_t* wstrContent) {
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
    return L"";
  }

  std::map<std::wstring, std::wstring>::iterator itr = m_mapWDataCache.find(wstrMapName);
  if (itr != m_mapWDataCache.end()) {
    return itr->second.c_str();
  }

  return L"";
}

void CMapDataCache::RemoveTrafficLightCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return;
  }

  MapDataCaches::iterator itr = m_mapTrafficLightCaches.find(wstrMapName);
  if (itr != m_mapTrafficLightCaches.end()) {
    m_mapTrafficLightCaches.erase(itr);
  }
}

const wchar_t* CMapDataCache::TrafficLightDataCache(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    return L"";
  }

  MapDataCaches::iterator itr = m_mapTrafficLightCaches.find(wstrMapName);
  if (itr != m_mapTrafficLightCaches.end()) {
    return itr->second.c_str();
  }

  return L"";
}

void CMapDataCache::PushTrafficLightCache(const wchar_t* wstrMapName, const wchar_t* wstrContent) {
  if (!wstrMapName || !wstrContent) {
    return;
  }

  MapDataCaches::iterator itr = m_mapTrafficLightCaches.find(wstrMapName);
  if (itr != m_mapTrafficLightCaches.end()) {
    return;
  }

  m_mapTrafficLightCaches.insert(std::make_pair(wstrMapName, wstrContent));
}

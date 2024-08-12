// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <json/value.h>
#include <json/writer.h>
#include <iostream>
#include "engine/config.h"

#include <chrono>
#include <thread>
#include "server_map_cache/map_data_cache.h"
using namespace std;

const char* target_map = "NASTway.xodr";

std::string createUploadCloudMapCallReqNormal() {
  Json::Value root;
  Json::Value maps;

  Json::Value d1;
  d1["name"] = "sihuan.sqlite";
  d1["version"] = "test_version";
  maps.append(d1);

  root["hadmaps"] = maps;

  return root.toStyledString();
}

std::string createGetLaneRelReq() {
  Json::Value root;
  Json::Value maps;

  Json::Value d1;
  d1["name"] = target_map;
  maps.append(d1);

  root["hadmaps"] = maps;

  return root.toStyledString();
}

void writeObjFile() {
  /*
  hadmap::MAP_DATA_TYPE mdp = hadmap::OPENDRIVE;
  hadmap::txMapHandle* pHandle = nullptr;
  hadmap::hadmapConnect(fname.c_str(), mdp, &pHandle) == TX_HADMAP_HANDLE_OK;
  double ref_x = 114.40561802372;
  double ref_y = 22.7467593653359;
  std::vector<std::vector<hadmap::txLaneId>> laneids;
  auto paths = hadmap::getForwardPoints(
      pHandle, hadmap::txPoint(114.4050249, 22.7406655, -99999999), -200, 1,
      &laneids);

  for (size_t i = 0; i < paths.size(); i++) {
      std::cout << i << ": " << paths[i].first << std::endl;
      std::vector<hadmap::PointVec> lines;
      lines.push_back(paths[i].second);
      for (auto& p : lines.back()) {
          coord_trans_api::lonlat2local(p.x, p.y, p.z, ref_x, ref_y, 0);
      }
      WtireObj(std::vector<hadmap::PointVec>(), lines,
               std::string("f:/") + std::to_string(i) + ".obj");
  }
  */
}

std::string createForwardPointReq() {
  Json::Value ret;

  // ret["x"] = 114.40561802372;
  // ret["y"] = 22.7467593653359;

  ret["x"] = 116.2971753;
  ret["y"] = 39.9837811;

  ret["z"] = -99999999;
  ret["length"] = -100.0;
  ret["interval"] = 1.0;

  ret["hadmapName"] = "sihuan.sqlite";

  return ret.toStyledString();
}

TEST(testMapCacheCase, testForwardPoints) {
  auto req = createForwardPointReq();
  cout << "get forward points req is: " << req << endl;

  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(req.c_str());

  std::wstring wstrRes;

  std::wstring status = CMapDataCache::Instance().GetForwardPoints(wstrParams.c_str(), wstrRes);
  EXPECT_EQ(status, CMapDataCache::m_wstrSuccess);

  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  std::cout << "forward points res: " << strRes << std::endl;
}

TEST(testMapCacheCase, testNormal) {
  std::string target = "";  // 测试数据

  // target.erase(std::remove(target.begin(), target.end(), '\t'),
  // target.end());
  target.erase(std::remove(target.begin(), target.end(), ' '), target.end());
  std::cerr << "target after trim: #" << target << "#" << std::endl;

  auto req = createUploadCloudMapCallReqNormal();
  cout << "cloud hadmap upload callback req is: " << req << endl;

  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(req.c_str());

  std::wstring wname = CEngineConfig::Instance().MBStr2WStr("sihuan.sqlite");

  std::wstring wstrRes;

  std::wstring status = CMapDataCache::Instance().CloudHadmapUploadCallback(wstrParams.c_str(), wstrRes);
  EXPECT_EQ(status, CMapDataCache::m_wstrSuccess);

  cout << "cloud hadmap upload callback res: " << wstrRes.c_str() << std::endl;
  // check here.
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  std::cout << strRes << std::endl;
  strRes.erase(std::remove(strRes.begin(), strRes.end(), '\t'), strRes.end());
  strRes.erase(std::remove(strRes.begin(), strRes.end(), '\n'), strRes.end());
  strRes.erase(std::remove(strRes.begin(), strRes.end(), ' '), strRes.end());

  cerr << "return res after trim:" << strRes << endl;
  // target.erase(std::remove(target.begin(), target.end(), ' '),
  // target.end());
  cout << "cloud hadmap upload callback res: #" << strRes.c_str() << "#" << std::endl;
  EXPECT_EQ(target, strRes);
  cout << "test Normal done and sleep here" << std::endl;

  const wchar_t* da_cache = CMapDataCache::Instance().RoadDataCache(wname.c_str());

  std::string roaddata_str = CEngineConfig::Instance().WStr2MBStr(da_cache);
  cout << roaddata_str << endl;

  std::this_thread::sleep_for(std::chrono::seconds(10));
  // EXPECT_EQ(add(2, 3), 5);
}

TEST(testMapCacheCase, testWaitSyncer) {
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    cout << " sleep 3 done and loop." << endl;
  }
}

TEST(testMapCacheCase, testGetLaneRel) {
  auto req = createGetLaneRelReq();
  cout << "get lane rel is: " << req << endl;

  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(req.c_str());

  std::wstring wstrRes;

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "ready for get lane rel " << std::endl;

  std::wstring status = CMapDataCache::Instance().CloudHadmapUploadCallback(wstrParams.c_str(), wstrRes);
  EXPECT_EQ(status, CMapDataCache::m_wstrSuccess);

  std::wstring wtargetMapName = CEngineConfig::Instance().MBStr2WStr(target_map);

  Json::Value jval = CMapDataCache::Instance().LaneRelDataCache(wtargetMapName.c_str());
  std::cout << "lane rel: \n" << jval.toStyledString() << std::endl;
  // road 25, sec 0, road -1.
  Json::Value res = jval["25,0,-1"];

  EXPECT_EQ(res["rid"].asInt(), 25);
  EXPECT_EQ(res["sid"].asInt(), 1);
  EXPECT_EQ(res["lid"].asInt(), -1);
}

std::string createGetRoadRelReq() {
  // As req is same for roadRel query and laneRel query.
  // call createGetLaneRelReq here.
  return createGetLaneRelReq();
}

TEST(testMapCacheCase, testGetRoadRel) {
  auto req = createGetRoadRelReq();
  cout << "get road rel is: " << req << endl;

  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(req.c_str());

  std::wstring wstrRes;

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "ready for get road rel " << std::endl;

  std::wstring status = CMapDataCache::Instance().CloudHadmapUploadCallback(wstrParams.c_str(), wstrRes);
  EXPECT_EQ(status, CMapDataCache::m_wstrSuccess);

  std::wstring wtargetMapName = CEngineConfig::Instance().MBStr2WStr(target_map);

  Json::Value jval = CMapDataCache::Instance().RoadRelDataCache(wtargetMapName.c_str());
  // std::cout << "lane rel: \n" << jval.toStyledString() << std::endl;
  // road 25, sec 0, road -1.

  auto targetSrcId = 999005;
  auto targetDstId = 999039;

  auto toIds = jval[std::to_string(targetSrcId)];
  int count = 0;
  for (auto& idVal : toIds) {
    // std::cout << idVal << std::endl;
    auto xx = idVal["rid"].asString();
    // std::cout << "xx: " << xx << std::endl;

    // count++;
    // if (count > 3) break;
    EXPECT_EQ(std::to_string(targetDstId), xx);
  }

  auto ret = CMapDataCache::Instance().GetRoadRel(wstrParams.c_str(), wstrRes);

  EXPECT_EQ(status, CMapDataCache::m_wstrSuccess);

  std::string final = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  // std::cout << "outer functiont est" << final << std::endl;
}

TEST(testMapData, checkMap) {
  std::wstring wtargetMapName = CEngineConfig::Instance().MBStr2WStr("sihuan.sqlite");
  std::this_thread::sleep_for(std::chrono::seconds(20));
  Json::Value jval = CMapDataCache::Instance().LaneRelDataCache(wtargetMapName.c_str());
  std::cout << "lane rel: \n" << jval.toStyledString() << std::endl;
}

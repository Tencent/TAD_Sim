// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "engine/error_code.h"
#include "server_map_cache/map_data_proxy.h"
#include "server_map_cache/map_syncer.h"

#include <chrono>
#include "engine/config.h"
#include "log/log.h"

using namespace std;

using namespace std::chrono;
TEST(testMapDataProxy, testNotExists) {
  cout << "test map data proxy not exists" << endl;

  EXPECT_EQ(CEngineConfig::Instance().IsInitialized(), true);

  auto& proxy = CMapDataProxy::Instance();
  auto ret = proxy.GetCosEnvironments();
  EXPECT_EQ(ret, HSEC_OK);

  milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

  std::string strHadmapFile = "test_not_exists.sqlite";
  sTagServiceHadmapInfo info;
  int nRet = CMapDataProxy::Instance().GetHadmapInfoFromService(strHadmapFile.c_str(), info);
  EXPECT_EQ(nRet, HSEC_OK);
  // hadmapname should be numpty string.
  EXPECT_EQ(info.m_strHadmapName, std::string());
  std::cout << "map name:#" << info.ToString() << "#" << std::endl;

  /*if (nRet != HSEC_OK)
  {
      SystemLogger::Error("get hadmap: %s info from service error, use cache data"
          , strHadmapFile.c_str());
      return m_wstrSuccess.c_str();
  }*/
}

TEST(testMapDataProxy, testGetAll) {
  cout << "test map data proxy not exists" << endl;

  EXPECT_EQ(CEngineConfig::Instance().IsInitialized(), true);

  auto& proxy = CMapDataProxy::Instance();
  auto ret = proxy.GetCosEnvironments();
  EXPECT_EQ(ret, HSEC_OK);

  milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

  std::vector<sTagServiceHadmapInfo> info;
  int nRet = CMapDataProxy::Instance().GetAllHadmapInfoFromService(info);
  EXPECT_EQ(nRet, HSEC_OK);
  // hadmapname should be numpty string.
  // EXPECT_EQ(info.m_strHadmapName, std::string());
  // std::cout << "map name:#" <<  info.ToString() << "#" << std::endl;
}

TEST(testMapDataProxy, testExists) {
  cout << "test map data proxy exists" << endl;

  EXPECT_EQ(CEngineConfig::Instance().IsInitialized(), true);
  CMapDataProxy& proxy = CMapDataProxy::Instance();
  // assume sihuan.sqlite is already there.
  std::string strHadmapFile = "sihuan.sqlite";
  sTagServiceHadmapInfo info;
  int nRet = CMapDataProxy::Instance().GetHadmapInfoFromService(strHadmapFile.c_str(), info);
  EXPECT_EQ(nRet, HSEC_OK);
  // hadmapname should be numpty string.
  EXPECT_EQ(info.m_strHadmapName, "sihuan.sqlite");
  std::cout << "info: #" << info.ToString() << "#" << std::endl;
}

TEST(testMapDataProxy, testAllMap) {
  cout << "test get all hadmap info " << endl;

  EXPECT_EQ(CEngineConfig::Instance().IsInitialized(), true);
  CMapDataProxy& proxy = CMapDataProxy::Instance();
  // assume sihuan.sqlite is already there.
  // std::string strHadmapFile = "sihuan.sqlite";
  std::vector<sTagServiceHadmapInfo> info;
  int nRet = CMapDataProxy::Instance().GetAllHadmapInfoFromService(info);
  EXPECT_EQ(nRet, HSEC_OK);
  // hadmapname should be numpty string.
  // EXPECT_EQ(info.m_strHadmapName, "sihuan.sqlite");
  std::cout << "size of hadmap info: " << info.size() << std::endl;
  // std::cout << "info: #" <<  info.ToString() << "#" << std::endl;

  std::cout << "xxxxxxxxxxxxx" << std::endl;
}

TEST(testMapDataProxy, testMapSyncer) {
  cout << "test data proxy map syncer." << std::endl;
  auto ms_ptr = MapSyncer::Create();
  EXPECT_EQ(ms_ptr->IsRunning(), false);

  ms_ptr->Start();
  EXPECT_EQ(ms_ptr->IsRunning(), true);

  ms_ptr->Stop();
  EXPECT_EQ(ms_ptr->IsRunning(), false);

  cout << "test data proxy map syncer done." << std::endl;
}

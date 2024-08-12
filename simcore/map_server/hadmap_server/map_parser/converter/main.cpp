/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include <glog/logging.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
// #include <gflags/gflags.h>

#include "../src/engine/config.h"
#include "../src/server_map_cache/map_data_cache.h"
#include "../src/server_scene/scene_data.h"
#include "../src/xml_parser/entity/parser.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/xosc/xosc_converter.h"

int main(int argc, char** argv) {
  std::string strHome = "./glogs/";
  google::InitGoogleLogging(argv[0]);
  std::string strInfoLog = strHome + "infos";
  // google::SetLogDestination(google::INFO, strInfoLog.c_str());

  LOG(INFO) << "start to convert openscenario !";
  /*
  if(argc < 3){
          std::cout << "please input converter src dst" << std::endl;
          return 0;
  }
  */
  std::string strResourcePath = "./";
  std::string strAppInstallDir = "./";
  CLog::Initialize(strResourcePath.c_str());
  CEngineConfig::Instance().Init(strResourcePath.c_str(), strAppInstallDir.c_str());
  const char* ptest = CSceneData::Instance().SceneData_xosc2sim(
      strResourcePath.c_str(), strAppInstallDir.c_str(), "123.xosc", "d2d_20190726.xodr", "12.1", "34.1", "2.1", "2");
  std::cout << "return=" << ptest;
  /*
  CMapDataCache::Instance().Init();


  XOSCReader_1_0_v3 reader;
  sTagSimuTraffic st;
  std::string strFile = "C:\\Workspace\\bin\\dst\\NAST_ICV_001.xosc";
  reader.ParseXOSC(strFile.c_str(), st);

  return 0;


  std::cout << "start to convert" << std::endl;

  std::string strSrc = argv[1];
  std::string strDst = argv[2];

  boost::filesystem::path pSrc = strSrc;
  boost::filesystem::path pDst = strDst;

  if (!boost::filesystem::exists(pSrc) || !boost::filesystem::is_directory(pSrc)) {
    std::cout << "src dir: " << strSrc << " is not exist" << std::endl;
    return -1;
  }

  if (!boost::filesystem::exists(pDst) || !boost::filesystem::is_directory(pDst)) {
    std::cout << "dst dir: " << strDst << " is not exist" << std::endl;
    return -1;
  }

  XOSCConverter converter;
  converter.ConverterToXOSC(strSrc.c_str(), strDst.c_str());

  std::cout << "convert finished" << std::endl;
  */
  return 0;
}

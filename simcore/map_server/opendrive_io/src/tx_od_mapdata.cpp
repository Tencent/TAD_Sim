// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_od_mapdata.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include "tx_od_log.h"

txMapDataCache& txMapDataCache::Instance() {
  static txMapDataCache instance;
  return instance;
}

const char* txMapDataCache::LoadHadMap(const char* hadmapPath) {
  // Assign the hadmapPath value to m_currentFilePath
  m_currentFilePath = hadmapPath;

  // Check if hadmapPath is null, and log a warning if it is
  if (!hadmapPath) {
    LogWarn << ("load hadmap file name is null!");
    return m_strFailed.c_str();
  }

  // Check if the HAD map is in the cache, and log an info message if it is
  ODR::CHadmap* pHadmap = HadmapCache(hadmapPath);
  if (pHadmap) {
    LogInfo << "hadmap:in cache";
    return m_strSuccess.c_str();
  } else {
    LogInfo << "hadmap not in cache";
  }

  // Create a boost::filesystem::path object p and assign the hadmapPath value to it
  boost::filesystem::path p = hadmapPath;

  // Get the filename and extension of the HAD map file
  std::string strMapName = p.filename().string();
  std::string strExt = p.extension().string();

  // Check if the file extension is valid, and log a warning if it is not
  if (!boost::algorithm::iequals(strExt, TX_XODR_TYPE) && !boost::algorithm::iequals(strExt, TX_SQLITE_TYPE)) {
    LogWarn << ("load hadmap file ext is %s", strExt.c_str());
    return m_strFailed.c_str();
  }

  // Create a new CHadmap object and parse the HAD map file
  pHadmap = new ODR::CHadmap;
  int nRet = pHadmap->ParseMap(hadmapPath);

  // Check if the parsing was successful, and log a warning if it failed
  if (nRet != 0) {
    LogWarn << ("load hadmap file failed!");
    return m_strFailed.c_str();
  }

  // Add the parsed HAD map to the cache
  m_hadmapCaches[hadmapPath] = pHadmap;
  return m_strSuccess.c_str();
}

const char* txMapDataCache::CreateHadMap(const char* hadmapPath, const Base::txStringStream& cmd_json) {
  // Assign the hadmapPath value to m_currentFilePath
  m_currentFilePath = hadmapPath;

  // Check if hadmapPath is null, and log a warning if it is
  if (!hadmapPath) {
    LogWarn << ("load hadmap file name is null!");
    return m_strFailed.c_str();
  }

  // Check if the HAD map is in the cache, and log an info message if it is
  ODR::CHadmap* pHadmap = HadmapCache(hadmapPath);
  if (pHadmap) {
    LogInfo << "hadmap is in cache";
    return m_strSuccess.c_str();
  }

  // Get the filename and extension of the HAD map file
  boost::filesystem::path p = hadmapPath;
  std::string strMapName = p.filename().string();
  std::string strExt = p.extension().string();

  // Check if the file extension is valid, and log a warning if it is not
  if (!boost::algorithm::iequals(strExt, TX_SQLITE_TYPE) && (!boost::algorithm::iequals(strExt, TX_XODR_TYPE))) {
    LogWarn << ("load hadmap file ext is %s", strExt.c_str());
    return m_strFailed.c_str();
  }

  // Create a new CHadmap object and create the HAD map using the provided command JSON
  pHadmap = new ODR::CHadmap;
  int nRet = pHadmap->CreateMap(hadmapPath, cmd_json);

  // Check if the creation was successful, and log a warning if it failed
  if (nRet != 0) {
    LogWarn << ("load hadmap file failed!");
    return m_strFailed.c_str();
  }

  // Add the created HAD map to the cache
  m_hadmapCaches[hadmapPath] = pHadmap;
  return m_strSuccess.c_str();
}

const char* txMapDataCache::SaveHadMap(const char* hadmapPath, const char* outhadmap) {
  // Check if the input file path is null
  if (!hadmapPath) {
    LogWarn << ("save hadmap file name is null!");
    return m_strFailed.c_str();
  }

  // Get the HAD map from the cache
  ODR::CHadmap* pHadmap = HadmapCache(hadmapPath);

  // Check if the HAD map is null
  if (!pHadmap) {
    return m_strFailed.c_str();
  }

  // Log the input and output file paths
  LogInfo << "srchadmap" << hadmapPath;
  LogInfo << "outhadmap" << outhadmap;

  // Check if the input and output file paths are the same
  if (std::string(hadmapPath) == std::string(outhadmap)) {
    return m_strSuccess.c_str();
  }

  // Copy the file
  try {
#ifdef BOOST_NEW_API
    boost::filesystem::copy_file(hadmapPath, outhadmap, boost::filesystem::copy_options::overwrite_existing);
#else
    boost::filesystem::copy_file(hadmapPath, outhadmap, boost::filesystem::copy_option::overwrite_if_exists);
#endif
  } catch (std::exception& e) {
    LogInfo << "copy file error : " << e.what();
    return m_strFailed.c_str();
  }

  // Load the copied HAD map
  return LoadHadMap(outhadmap);

  // Save the HAD map
  int nRet = pHadmap->SaveMap(outhadmap);

  // Check if the save was successful
  if (nRet != 0) {
    LogWarn << ("save hadmap file failed!");
    return m_strFailed.c_str();
  }
}

const char* txMapDataCache::ModifyHadMap(const char* hadmapPath, const Base::txStringStream& cmd_json) {
  LogInfo << "start modify hadmap";

  // Check if the input file path is null
  if (!hadmapPath) {
    LogWarn << ("load hadmap file name is null!");
    return m_strFailed.c_str();
  }

  // Get the HAD map from the cache
  ODR::CHadmap* pHadmap = HadmapCache(hadmapPath);

  // Check if the HAD map is null
  if (!pHadmap) {
    LogWarn << "hadmap not in cache";
    return m_strFailed.c_str();
  }

  // Modify the HAD map
  int nRet = pHadmap->ModifyMap(cmd_json);

  // Check if the modification was successful
  if (nRet != 0) {
    LogWarn << ("modify hadmap file failed!");
    return m_strFailed.c_str();
  }

  LogInfo << "modify hadmap end!";
  return m_strSuccess.c_str();
}

const char* txMapDataCache::CloseHadMap(const char* hadmapPath) {
  // Get the HAD map from the cache
  ODR::CHadmap* pHadmap = HadmapCache(hadmapPath);

  // Check if the HAD map is null
  if (!pHadmap) {
    LogWarn << "Map is not in cache";
    return m_strSuccess.c_str();
  }

  // Remove the HAD map from the cache
  m_hadmapCaches.erase(hadmapPath);

  // Close the HAD map
  pHadmap->CloseMap();

  return m_strSuccess.c_str();
}

const char* txMapDataCache::ToJson(Base::txStringStream& cmd_json) {
  // Get the HAD map from the cache
  ODR::CHadmap* pHadmap = HadmapCache(m_currentFilePath.c_str());

  // Check if the HAD map is null
  if (Null_Pointer(pHadmap)) {
    LogWarn << "hadmap not in cache!";
    return "Data is error";
  }

  // Convert the HAD map data to JSON
  int nRet = pHadmap->ParseDataToJson(cmd_json);

  // Check if the conversion was successful
  if (nRet != 0) {
    LogWarn << ("load hadmap file failed!");
    return "Data is error";
  }

  return m_strSuccess.c_str();
}

ODR::CHadmap* txMapDataCache::HadmapCache(const char* hadmapFile) {
  LogInfo << "hadmapFile:" << hadmapFile;
  if (!hadmapFile) {
    return nullptr;
  }
  HadmapCaches::iterator itr = m_hadmapCaches.find(std::string(hadmapFile));
  if (itr != m_hadmapCaches.end()) {
    return itr->second;
  }
  return nullptr;
}

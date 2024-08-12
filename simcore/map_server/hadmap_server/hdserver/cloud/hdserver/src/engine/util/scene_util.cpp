/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scene_util.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "../config.h"
#include "common/xml_parser/entity/parser_tools.h"

uint64_t CSceneUtil::FileSize(const char* strFullPath) {
  boost::filesystem::path p = strFullPath;
  if (boost::filesystem::exists(p)) {
    return boost::filesystem::file_size(p);
  }

  return 0;
}

std::string CSceneUtil::SceneFullPath(const char* strName) {
  if (!strName) {
    return "";
  }

  boost::filesystem::path sceneDirPath = CEngineConfig::Instance().SceneDir();
  boost::filesystem::path simFilePath = sceneDirPath;
  simFilePath.append(strName);
  std::string strExt = simFilePath.extension().string();

  std::string strStemName = simFilePath.stem().string();
  std::string strSimPath = "";
  if (strExt.size() == 0 || strExt != ".sim") {
    std::string strFileName = strName;
    strFileName.append(".sim");
    simFilePath = sceneDirPath;
    simFilePath.append(strFileName);
  }

  return simFilePath.string();
}

std::string CSceneUtil::SceneFullName(const char* strName) {
  std::string strFileName = strName;
  boost::filesystem::path p = strName;
  std::string strExt = p.extension().string();
  std::string strStemName = p.stem().string();
  if (strExt.size() == 0 || strExt != ".sim") {
    strFileName = strName;
    strFileName.append(".sim");
  }

  return strFileName;
}

std::string CSceneUtil::ScenePath(const char* strDir, const char* strName) {
  boost::filesystem::path p = strDir;
  std::string strFullName = SceneFullName(strName);
  p /= strFullName;
  return p.string();
}

std::string CSceneUtil::HadmapFullPath(const char* strName) {
  if (!strName) {
    return nullptr;
  }

  boost::filesystem::path hadmapDirPath = CEngineConfig::Instance().HadmapDir();
  boost::filesystem::path hadmapFilePath = hadmapDirPath;
  hadmapFilePath.append(strName);
  std::string strExt = hadmapFilePath.extension().string();

  std::string strStemName = hadmapFilePath.stem().string();
  std::string strHadmapPath = "";
  if (strExt.size() == 0 || (strExt != ".sqlite" && strExt != ".xodr")) {
    std::string strFileName = strName;
    strFileName.append(".sqlite");
    hadmapFilePath = hadmapDirPath;
    hadmapFilePath.append(strFileName);
  }

  return hadmapFilePath.string();
}

std::string CSceneUtil::HadmapFullName(const char* strName) {
  std::string strFileName = strName;
  boost::filesystem::path p = strName;
  std::string strExt = p.extension().string();
  std::string strStemName = p.stem().string();
  if (strExt.size() == 0 || (strExt != ".sqlite" && strExt != ".xodr")) {
    strFileName = strName;
    strFileName.append(".sqlite");
  }

  return strFileName;
}

std::string CSceneUtil::HadmapPath(const char* strDir, const char* strName) {
  boost::filesystem::path p = strDir;
  std::string strFullName = HadmapFullName(strName);
  p /= strFullName;
  return p.string();
}

std::string CSceneUtil::GISImageFullPath(const char* strName) {
  if (!strName) {
    return nullptr;
  }

  boost::filesystem::path gisImageDirPath = CEngineConfig::Instance().GISImageDir();
  boost::filesystem::path gisImageFilePath = gisImageDirPath;
  gisImageFilePath.append(strName);
  std::string strExt = gisImageFilePath.extension().string();

  std::string strStemName = gisImageFilePath.stem().string();
  std::string strGISImagePath = "";

  if (strExt.size() == 0 || !CSceneUtil::ValidGISImageFormat(strExt.c_str())) {
    std::string strFileName = strName;
    strFileName.append(".jpeg");
    gisImageFilePath = gisImageDirPath;
    gisImageFilePath.append(strFileName);
  }

  return gisImageFilePath.string();
}

std::string CSceneUtil::GISImageFullName(const char* strName) {
  std::string strFileName = strName;
  boost::filesystem::path p = strName;
  std::string strExt = p.extension().string();
  std::string strStemName = p.stem().string();

  if (strExt.size() == 0 || !CSceneUtil::ValidGISImageFormat(strExt.c_str())) {
    strFileName = strName;
    strFileName.append(".jpeg");
  }

  return strFileName;
}

std::string CSceneUtil::GISImagePath(const char* strDir, const char* strName) {
  boost::filesystem::path p = strDir;
  std::string strFullName = GISImageFullName(strName);
  p /= strFullName;
  return p.string();
}

bool CSceneUtil::ValidGISImageFormat(const char* cstrExt) {
  std::vector<std::string> validExts{".jpeg", ".png", ".jpg", ".tiff"};
  std::string strExt = cstrExt;
  size_t validExt = std::count_if(validExts.begin(), validExts.end(), [strExt](std::string ext) {
    if (boost::algorithm::iequals(strExt, ext)) {
      return true;
    }
    return false;
  });

  return validExt > 0 ? true : false;
}

bool CSceneUtil::ValidFileFormat(const char* cstrExt, const char* cstrExts) {
  std::vector<std::string> validExts;
  CParseTools::SplitString(cstrExts, validExts, ",");
  std::string strExt = cstrExt;
  size_t validExt = std::count_if(validExts.begin(), validExts.end(), [strExt](std::string ext) {
    if (boost::algorithm::iequals(strExt, ext)) {
      return true;
    }
    return false;
  });

  return validExt > 0 ? true : false;
}

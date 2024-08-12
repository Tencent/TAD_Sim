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
#include "scene_validator.h"

uint64_t CSceneUtil::FileSize(const char* strFullPath) {
  boost::filesystem::path p = strFullPath;
  if (boost::filesystem::exists(p)) {
    return boost::filesystem::file_size(p);
  }

  return 0;
}
//
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
  if (boost::iequals(strExt, ".xosc")) {
  } else if (strExt.size() == 0 || strExt != ".sim") {
    std::string strFileName = strName;
    strFileName.append(".sim");
    simFilePath = sceneDirPath;
    simFilePath.append(strFileName);
  }
  return simFilePath.string();
}

std::string CSceneUtil::DataSceneFullPath(const char* strName) {
  if (!strName) {
    return "";
  }
  boost::filesystem::path sceneDirPath = CEngineConfig::Instance().getSceneDataDir();
  boost::filesystem::path simFilePath = sceneDirPath;
  simFilePath.append(strName);
  std::string strExt = simFilePath.extension().string();
  std::string strStemName = simFilePath.stem().string();
  std::string strSimPath = "";
  if (boost::iequals(strExt, ".xosc")) {
  } else if (strExt.size() == 0 || strExt != ".sim") {
    std::string strFileName = strName;
    strFileName.append(".sim");
    simFilePath = sceneDirPath;
    simFilePath.append(strFileName);
  }
  return simFilePath.string();
}

std::string CSceneUtil::GetSceneFullPath(const char* strName, SCENE_PATH_TYPE mode) {
  switch (mode) {
    case CSceneUtil::SYS_SCENE:
      return SceneFullPath(strName);
      break;
    case CSceneUtil::DATA_SCENE:
      return DataSceneFullPath(strName);
      break;
    case CSceneUtil::ALL_PATH: {
      std::string filePath = SceneFullPath(strName);
      if (!CSceneValidator::SceneExist(filePath.c_str())) return DataSceneFullPath(strName);
    }
      return SceneFullPath(strName);
      break;
    default:
      break;
  }
  return "";
}

std::string CSceneUtil::SceneFullName(const char* strName) {
  std::string strFileName = strName;
  boost::filesystem::path p = strName;
  std::string strExt = p.extension().string();
  std::string strStemName = p.stem().string();
  if (boost::iequals(strExt, ".xosc")) {
    return strFileName;
  } else {
    strFileName = strStemName;
    strFileName.append(".sim");
  }

  return strFileName;
}

std::string CSceneUtil::SceneFullName(const char* strName, const char* strInExt, bool bIsAllName) {
  std::string strFileName = strName;
  boost::filesystem::path p = strName;
  std::string strExt;
  if (bIsAllName) {
    strExt = p.extension().string();
  } else {
    strExt = "";
  }

  // std::string strStemName = p.stem().string();

  if (strExt.size() == 0) {
    if ((!strInExt || strlen(strInExt) == 0)) {
      if (strExt.size() == 0) {
        strFileName = strName;
        strFileName.append(".sim");
      }

    } else {
      strFileName = strName;
      strFileName.append(".");
      strFileName.append(strInExt);
    }
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

  std::string srcPath = CEngineConfig::Instance().getHadmapDataDir();
  auto mapAttr = CEngineConfig::Instance().HadmapAttrs().find(strName);
  if (mapAttr != CEngineConfig::Instance().HadmapAttrs().end() && mapAttr->second.m_strPreset == "") {
    srcPath = CEngineConfig::Instance().HadmapDir();
  }

  boost::filesystem::path hadmapDirPath = srcPath;
  boost::filesystem::path hadmapFilePath = hadmapDirPath;
  hadmapFilePath.append(strName);
  std::string strExt = hadmapFilePath.extension().string();

  std::string strStemName = hadmapFilePath.stem().string();

  if (strExt.size() == 0 || (strExt != ".sqlite" && strExt != ".xodr" && strExt != ".xml")) {
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
  if (strExt.size() == 0 || (strExt != ".sqlite" && strExt != ".xodr" && strExt != ".xml")) {
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

bool CSceneUtil::ValidGISModelFormat(const char* cstrExt) {
  std::vector<std::string> validExts{".fbx"};
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

std::string CSceneUtil::GISModelFullPath(const char* strName) {
  if (!strName) {
    return nullptr;
  }

  boost::filesystem::path gisModelDirPath = CEngineConfig::Instance().GISModelDir();
  boost::filesystem::path gisModelFilePath = gisModelDirPath;
  gisModelFilePath.append(strName);
  std::string strExt = gisModelFilePath.extension().string();

  std::string strStemName = gisModelFilePath.stem().string();
  std::string strGISModelPath = "";

  if (strExt.size() == 0 || !CSceneUtil::ValidGISModelFormat(strExt.c_str())) {
    std::string strFileName = strName;
    strFileName.append(".fbx");
    gisModelFilePath = gisModelDirPath;
    gisModelFilePath.append(strFileName);
  }

  return gisModelFilePath.string();
}

std::string CSceneUtil::GISModelFullName(const char* strName) {
  std::string strFileName = strName;
  boost::filesystem::path p = strName;
  std::string strExt = p.extension().string();
  std::string strStemName = p.stem().string();

  if (strExt.size() == 0 || !CSceneUtil::ValidGISModelFormat(strExt.c_str())) {
    strFileName = strName;
    strFileName.append(".fbx");
  }

  return strFileName;
}

std::string CSceneUtil::GISModelPath(const char* strDir, const char* strName) {
  boost::filesystem::path p = strDir;
  std::string strFullName = GISModelFullName(strName);
  p /= strFullName;
  return p.string();
}

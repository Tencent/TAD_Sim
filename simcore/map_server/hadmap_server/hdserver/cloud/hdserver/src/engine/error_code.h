/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/
#pragma once

#include <string>

enum HadmapServerErrorCode {
  HSEC_JSON2XODR_ERROR = -1000,
  // cos error
  HSEC_DOWNLOAD_HADMAP_FAILED = -399,
  HSEC_PARSE_COS_ADDR_ERROR,
  HSEC_HTTP_GET_ERROR,
  HSEC_COS_BUCKET_OR_KEY_STRING_NULL_ERROR,
  HSEC_COS_OP_ERROR,
  HSEC_COS_NOT_FOUND,
  HSEC_MAP_FILE_FORMAT_ERROR,
  // copy scene file error
  HSEC_COPY_SCENE_FAILED = -201,
  HSEC_COPY_SCENE_FILE_EXIST = -200,
  HSEC_COPY_HADMAP_FAILED = -199,
  // scene file error
  HSEC_SimFileFormatError = -111,
  HSEC_SimFileCopyError = -110,
  HSEC_TrafficFileNotExist = -109,
  HSEC_TrafficFileCopyError = -108,
  HSEC_EnvironmentFileNotExist = -107,
  HSEC_EnvironmentFileCopyError = -106,
  HSEC_GradingFileNotExist = -105,
  HSEC_GradingFileCopyError = -104,
  HSEC_SensorFileNotExist = -103,
  HSEC_SensorFileCopyError = -102,
  HSEC_SourceDirNotExist = -101,
  HSEC_DestDirNotExist = -100,
  // common error
  HSEC_DIRECTORY_NOT_EXIST = -2,
  // error
  HSEC_Error = -1,
  // success
  HSEC_OK = 0,
};

std::string HadmapServerErrorMessage(int nErrorCode);

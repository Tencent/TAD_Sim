/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

enum HadmapServerErrorCode {
  // copy scene file error
  HSEC_COPY_SCENE_FAILED = -201,
  HSEC_COPY_SCENE_FILE_EXIST = -200,
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
  HSEC_XOSCFileCopyError = -109,
  HSEC_MapLostError = -112,
  HSEC_MapFormatError = -113,
  // common error
  HSEC_DIRECTORY_NOT_EXIST = -2,
  // error
  HSEC_Error = -1,
  // success
  HSEC_OK = 0,
};

std::string HadmapServerErrorMessage(int nErrorCode);

/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "grading.h"
#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "../../../engine/config.h"
#include "common/log/system_logger.h"
#include "common/log/xml_logger.h"

int CGrading::Parse(const char* strGrading) {
  if (!strGrading) return -1;

  return 0;
}

int CGrading::Save(const char* strGrading) {
  if (!strGrading) return -1;

  boost::filesystem::path p = CEngineConfig::Instance().GradingTemplateFile();
  if (boost::filesystem::exists(p)) {
    try {
      boost::filesystem::copy_file(p, strGrading, BOOST_COPY_OPTION);
    } catch (std::exception* e) {
      SYSTEM_LOGGER_ERROR("save grading file failed!");
      return -1;
    }
  } else {
    SYSTEM_LOGGER_ERROR("grading template file not exist!");
    return -1;
  }

  return 0;
}

int CGrading::Delete(const char* strGrading) {
  if (!strGrading) return -1;

  return 0;
}

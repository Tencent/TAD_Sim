/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc/xosc_converter.h"
#include <boost/filesystem.hpp>
#include "common/log/system_logger.h"
#include "common/xml_parser/xosc/xosc_writer_1_0_v4.h"
#include "xml_parser/entity/parser.h"
int XOSCConverter::ConverterToXOSC(const char* strSrcDir, const char* strDstDir) {
  if (!strSrcDir || !strDstDir) {
    assert(false);
    SYSTEM_LOGGER_ERROR("source directory or destination directory is null");
    return -1;
  }

  boost::filesystem::path curP = boost::filesystem::current_path();
  boost::filesystem::path pathSrc = curP;
  pathSrc.append(strSrcDir);
  boost::filesystem::path pathDst = curP;
  pathDst.append(strDstDir);

  if (!boost::filesystem::exists(pathDst)) {
    assert(false);
    SYSTEM_LOGGER_ERROR("destination directory is null!");
    return -1;
  }

  if (!boost::filesystem::exists(pathSrc)) {
    assert(false);
    SYSTEM_LOGGER_ERROR("source directory is null!");
    return -1;
  }

  boost::filesystem::directory_iterator itr_end;
  boost::filesystem::directory_iterator itr(pathSrc);
  std::vector<boost::filesystem::path> files;
  std::string strInExt = ".sim";

  for (; itr != itr_end; ++itr) {
    std::string strName = (*itr).path().string();
    std::string strExt = (*itr).path().extension().string();
    if ((!boost::filesystem::is_directory(*itr)) && strExt == strInExt) {
      files.push_back((*itr).path());
    }
  }

  std::vector<boost::filesystem::path>::iterator fItr = files.begin();
  for (; fItr != files.end(); ++fItr) {
    CParser parser;
    sTagSimuTraffic sst;
    int nRes = parser.Parse(sst, (*fItr).string().c_str(), pathSrc.string().c_str(), false);

    CSimulation& sim = sst.m_simulation;
    CTraffic& traffic = sst.m_traffic;
    CEnvironment& environment = sst.m_environment;

    std::string strFileName = fItr->stem().string();
    strFileName += ".xosc";
    boost::filesystem::path p = pathDst;
    p /= strFileName;
    std::string strPath = p.lexically_normal().make_preferred().string();
    XOSCWriter_1_0_v4 writer;
    writer.Save2XOSC(strPath.c_str(), sst);
  }

  return 0;
}

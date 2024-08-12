/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "hadmap_scene_data.h"

#include <fstream>
#include <iostream>
#include <memory>

#include "gflags/gflags.h"
#include "gtest/gtest.h"
#include "xml_parser/hadmap_entity/map_scene_v3.h"
#include "xml_parser/hadmap_entity/parser_hadmap_json_v3.h"

#include <xsd/cxx/parser/elements.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/error-handler.hxx>
#include <xsd/cxx/xml/dom/auto-ptr.hxx>
#include <xsd/cxx/xml/dom/parsing-source.hxx>
#include <xsd/cxx/xml/string.hxx>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLUni.hpp>      // xercesc::fg*
#include <xercesc/util/XMLUniDefs.hpp>  // chLatin_L, etc

DEFINE_string(hadmap_scene_data_test_xodr_path, "/data/hdserver/cloud/hdserver/test/data/哈哈哈.xodr", "");
DEFINE_string(hadmap_scene_data_test_json_path, "./hdserver/cloud/hdserver/test/data/savexodr.json", "");

class HadmapSceneDataTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    xercesc::XMLPlatformUtils::Initialize();
    EXPECT_TRUE(FLAGS_hadmap_scene_data_test_xodr_path.empty() ||
                FLAGS_hadmap_scene_data_test_xodr_path.find(".xodr") != std::string::npos);
  }
};

TEST_F(HadmapSceneDataTest, parseJsonFromOpendrive) {
  std::string path = FLAGS_hadmap_scene_data_test_xodr_path;
  xsd::cxx::xml::string xmlstr = xsd::cxx::xml::string(path);
  ASSERT_EQ(xsd::cxx::xml::transcode<char>(xmlstr.c_str()), path);

  xercesc::MemoryManager* mm = xercesc::XMLPlatformUtils::fgMemoryManager;
  xercesc::InputSource* src =
      new (mm) xercesc::LocalFileInputSource(xercesc::XMLString::transcode(path.c_str(), mm), mm);
  ASSERT_EQ(xercesc::XMLString::equals(xercesc::XMLString::transcode(path.c_str(), mm), src->getSystemId()), true);
  ASSERT_EQ(std::string(xercesc::XMLString::transcode(src->getSystemId(), mm)), path);
  ASSERT_NE(xercesc::XMLPlatformUtils::openFile(src->getSystemId(), mm), nullptr);
  xercesc::BinFileInputStream* retStrm = new (mm) xercesc::BinFileInputStream(src->getSystemId(), mm);
  ASSERT_TRUE(retStrm->getIsOpen());
  ASSERT_TRUE(src->makeStream() != nullptr);
  {
    xsd::cxx::tree::error_handler<char> h;
    xsd::cxx::xml::auto_initializer i(true, true);
    xsd::cxx::xml::dom::parse<char>(*src, h, xsd::cxx::tree::properties<char>(), 1024);
    ASSERT_NO_THROW(h.throw_if_failed<xsd::cxx::tree::parsing<char> >());
  }

  std::string data;
  CHadmapSceneData::Instance().parseJsonFromOpendrive(FLAGS_hadmap_scene_data_test_xodr_path, data);
  EXPECT_TRUE(!data.empty());
}

TEST_F(HadmapSceneDataTest, saveJsonAsOpendrive) {
  std::ifstream ifs(FLAGS_hadmap_scene_data_test_json_path);
  std::ostringstream oss;
  oss << ifs.rdbuf();

  CMapSceneV3* scene = CParserHadmapJsonV3{}.ParseV3(oss.str().c_str());
  ASSERT_TRUE(scene != nullptr);
  CHadmapSceneData::Instance().saveJsonAsOpendrive("./hdserver/cloud/hdserver/test/data/xodr_save_as.sqlite",
                                                   scene->Geometry());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  google::ParseCommandLineFlags(&argc, &argv, false);
  return RUN_ALL_TESTS();
}
/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc_replay/type_adapter.h"
#include "common/xml_parser/xosc_replay/xosc_replay_converter.h"

#include <chrono>
#include <thread>

#include "asam.h"
#include "engine/config.h"
#include "xml_parser/entity/parser.h"

#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace internal {
constexpr char kStringLOC[] = "loc";
constexpr char kStringTFC[] = "tfc";
}  // namespace internal

class XOSCReplayConverterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    xercesc::XMLPlatformUtils::Initialize();
    CEngineConfig::Instance().Init("/home/hdserver", "/home/hdserver");
  }
};

TEST_F(XOSCReplayConverterTest, GenSim) {
  // EXPECT_EQ(converter_->GenSim("./data/TAS_01.xosc"), "./data/TAS_01.sim");
}

TEST_F(XOSCReplayConverterTest, SerializeToSimrecSingle) {
  utils::Status status;
  XOSCReplayConverter::Options opts;
  opts.write_debug_json = true;
  status = XOSCReplayConverter{}.SerializeToSimrec("./hdserver/cloud/hdserver/test/data/1604480161.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();

  status = XOSCReplayConverter{}.SerializeToSimrec("./hdserver/cloud/hdserver/test/data/1500521764_copy.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();
}

TEST_F(XOSCReplayConverterTest, SerializeToSimrec) {
  try {
    OpenSCENARIO("./data/TAS_01_ver1.0.xosc", xml_schema::flags::dont_validate);
  } catch (const std::exception& ex) {
    ASSERT_TRUE(false) << ex.what();
  }

  utils::Status status;
  XOSCReplayConverter::Options opts;
  opts.write_debug_json = true;
  status = XOSCReplayConverter{}.SerializeToSimrec("./data/TAS_01_ver1.0.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();

  status = XOSCReplayConverter{}.SerializeToSimrec("./data/TAS_02_ver1.0.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();

  status = XOSCReplayConverter{}.SerializeToSimrec("./data/TAS_12_ver1.0.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();

  status = XOSCReplayConverter{}.SerializeToSimrec("./data/TAS_36_ver1.0.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();

  status = XOSCReplayConverter{}.SerializeToSimrec("./data/TAS_38_ver1.0.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();

  status = XOSCReplayConverter{}.SerializeToSimrec("./data/TAS_40_ver1.0.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();

  status = XOSCReplayConverter{}.SerializeToSimrec("./data/TAS_46_ver1.0.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();
}

TEST_F(XOSCReplayConverterTest, ParseFromSimrec) {
  const std::string frame_path = "./hdserver/cloud/hdserver/test/data/SimCity_01.frames";
  ASSERT_TRUE(XOSCReplayConverter{}.ParseFromSimlog(frame_path, "re_project_mannul_0414.xodr").ok());
  try {
    OpenSCENARIO("./hdserver/cloud/hdserver/test/data/SimCity_01.xosc", xml_schema::flags::dont_validate);
  } catch (const std::exception& ex) {
    ASSERT_TRUE(false) << ex.what();
  }
  XOSCReplayConverter::Options opts;
  auto status = XOSCReplayConverter{}.SerializeToSimrec("./hdserver/cloud/hdserver/test/data/SimCity_01.xosc", opts);
  EXPECT_TRUE(status.ok()) << status.ToString();
}

TEST_F(XOSCReplayConverterTest, ParsePerf) {
  OpenSCENARIO("./hdserver/cloud/hdserver/test/data/SimCity_01.xosc", xml_schema::flags::dont_validate);
}
TEST_F(XOSCReplayConverterTest, ParsePerf1) { SIM::OSC::Load("./hdserver/cloud/hdserver/test/data/SimCity_01.xosc"); }

TEST_F(XOSCReplayConverterTest, PB) {
  // {
  //   std::ifstream ifs("./data/LSVXE80T3K2055068_2.simrec");
  //   sim_msg::TrafficRecords4Logsim simrec;
  //   simrec.ParseFromIstream(&ifs);

  //   std::string s;
  //   google::protobuf::util::MessageToJsonString(simrec, &s, {});
  //   std::ofstream ofs("./data/LSVXE80T3K2055068_2.simrec.json");
  //   ofs << s;
  // }
}

TEST_F(XOSCReplayConverterTest, stod) {
  try {
    std::stod("");
  } catch (const std::exception& ex) {
    EXPECT_EQ(ex.what(), std::string("stod"));
  }

  try {
    std::stod("-1.1228941048103e-05");
    EXPECT_TRUE(true);
  } catch (const std::exception& ex) {
    EXPECT_EQ(ex.what(), std::string("stod"));
  }
}

TEST_F(XOSCReplayConverterTest, ptree) {
  std::string xodr = R"(
    <?xml version="1.0" standalone="yes"?>
    <OpenDRIVE>
      <header revMajor="1" revMinor="4" name="none" version="1.0" date="2019/ 7/26 13:17" 
      north="0.1000000000000000e+000" south="1.0000000000000000e+000" east="0.0000000000000000e+000" 
      west="0.0000000000000000e+000" vendor="KBD">
        <geoReference><![CDATA[+proj=utm +zone=51 +ellps=WGS84 +datum=WGS84 +units=m +no_defs]]></geoReference>
      </header>
    </OpenDRIVE>
  )";

  try {
    std::istringstream iss(xodr);
    boost::property_tree::ptree tree;
    boost::property_tree::read_xml(iss, tree);
    // const auto& header = tree.get_child("OpenDRIVE.header").get_child("<xmlattr>");
    // BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, header){
    //   std::cout << "First: " << v.first.data() << " Second: " << v.second.data() << std::endl;
    // }
    EXPECT_EQ(tree.get<int>("OpenDRIVE.header.<xmlattr>.revMinor"), 4);
    auto geo_ref = tree.get_child("OpenDRIVE.header.geoReference").data();
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  try {
    std::istringstream iss(xodr);
    SIM::ODR::OpenDrivePtr odr = SIM::ODR::Load(iss);
    EXPECT_EQ(odr->header.geoReference, "+proj=utm +zone=51 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
    EXPECT_GE(odr->header.south, 1);
  } catch (const std::exception& e) {
  } catch (...) {
    EXPECT_TRUE(false);
  }
}



TEST_F(XOSCReplayConverterTest, Pre) {
  {
    utils::Status status;
    XOSCReplayConverter::Options opts;
    opts.frame_interval = 0.1;
    opts.write_debug_json = true;
    status = XOSCReplayConverter{}.SerializeToSimrec("./hdserver/cloud/hdserver/test/data/simcity.xosc", opts);
    EXPECT_TRUE(status.ok()) << status.ToString();
  }

  // {
  //   XOSCReplayConverter::Options opts;
  //   opts.write_debug_json = true;
  //   XOSCReplayConverter{}.SerializeToSimrec("./hdserver/cloud/hdserver/test/data/TAS_11_ver1.0.xosc", opts);
  // }
}

TEST_F(XOSCReplayConverterTest, Adapter) {
  EXPECT_EQ(123, SIM::OSC::Name2Id(0, "123"));
  EXPECT_EQ(123, SIM::OSC::Name2Id(0, "car_123"));
  EXPECT_EQ(123, SIM::OSC::Name2Id(0, "xxxx123"));
  EXPECT_EQ(123, SIM::OSC::Name2Id(0, "xxxx123xxx"));
  EXPECT_EQ(123, SIM::OSC::Name2Id(0, "xxxx123xxxxxx3456"));
  EXPECT_EQ(0, SIM::OSC::Name2Id(0, "xxx"));
  EXPECT_EQ(0, SIM::OSC::Name2Id(0, "car_"));
  EXPECT_EQ(0, SIM::OSC::Name2Id(0, ""));
}

TEST_F(XOSCReplayConverterTest, DisplaySimrec) {
  std::ifstream ifs("./hdserver/cloud/hdserver/test/data/1500130585_logsim.simrec");
  std::stringstream ss;
  ss << ifs.rdbuf();
  ifs.close();
  sim_msg::TrafficRecords4Logsim simrec;
  simrec.ParseFromString(ss.str());

  std::ofstream ofs("./hdserver/cloud/hdserver/test/data/1500130585_logsim.json");
  std::string s;
  google::protobuf::util::JsonPrintOptions opts;
  // 是否把枚举值当作字符整形数,缺省是字符串
  opts.always_print_enums_as_ints = true;
  // 是否把下划线字段更改为驼峰格式,缺省时更改
  opts.preserve_proto_field_names = true;
  // 是否输出仅有默认值的原始字段,缺省时忽略
  opts.always_print_primitive_fields = true;
  google::protobuf::util::Status status = google::protobuf::util::MessageToJsonString(simrec, &s, opts);
  ofs << s;
  ofs.close();
}

TEST_F(XOSCReplayConverterTest, DisplayFrames) {
  std::ifstream ifs("./hdserver/cloud/hdserver/test/data/SimCity_01.frames");
  // std::ofstream ofs("./")
  for (std::string line; std::getline(ifs, line);) {
    if (line.empty()) {
      continue;
    }
    Json::Value one;
    if (!Json::Reader{}.parse(line, one)) {
      continue;
    }

    Json::Value root(Json::arrayValue);
    one.isObject() ? root.append(one) : (root = one);
    for (auto&& record : root) {
      if (!record.isObject() || !record.isMember(internal::kStringLOC) || !record[internal::kStringLOC].isString() ||
          !record.isMember(internal::kStringTFC) || !record[internal::kStringTFC].isString()) {
        continue;
      }
      {
        sim_msg::Location msg;
        std::string s = record[internal::kStringLOC].asString();
        msg.ParseFromString(utils::Base64Decode(std::begin(s), std::end(s)));
        std::cout << msg.ShortDebugString() << std::endl;
      }
      {
        sim_msg::Traffic msg;
        std::string s = record[internal::kStringTFC].asString();
        msg.ParseFromString(utils::Base64Decode(std::begin(s), std::end(s)));
        std::cout << msg.ShortDebugString() << std::endl;
      }
    }
  }
}

TEST_F(XOSCReplayConverterTest, LoadODR) {
  std::vector<std::thread> t;
  for (int i = 0; i < 100; i++) {
    t.push_back(std::thread([&]() {
      auto ptr = SIM::ODR::Load("./hdserver/cloud/hdserver/test/data/1.0#apollo_vectormap_G3_12141742_sp_re.xml");
      std::cout << *ptr << std::endl;
    }));
  }

  for (auto& _ : t) {
    _.join();
  }
}

TEST_F(XOSCReplayConverterTest, LoadOSC) {
  std::vector<SIM::OSC::OpenScenarioPtr> ptrs;
  for (int i = 0; i < 100; i++) {
    auto ptr = SIM::OSC::Load("./hdserver/cloud/hdserver/test/data/1500327035.xosc");
    ptrs.push_back(ptr);
  }
  std::this_thread::sleep_for(std::chrono::seconds(30));
}

TEST_F(XOSCReplayConverterTest, SerializeToPoints) {
  auto status = XOSCReplayConverter{}.SerializeToPoints("./hdserver/cloud/hdserver/test/data/1503555178.xosc");
  EXPECT_TRUE(status.ok()) << status.ToString();
}

TEST_F(XOSCReplayConverterTest, ContractCatalogReference) {
  std::string xosc = R"(

  )";
  XOSCReplayConverter converter;
  // converter.osc_ = SIM::OSC::Load();
}
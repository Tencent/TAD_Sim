// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HybridParserExt.h"
#include "inc/car_common.h"
#include "inc/car_log.h"
#include "inc/proto_helper.h"
#include "car.pb.h"

namespace tx_car {
namespace hybrid_parser {
tx_car::CarType cartype;
tx_car::Pow_ECU ecuWalue;
tx_car::Propulsion g_propulsion;

HybridParserExt::HybridParserExt(/* args */) {}

HybridParserExt::~HybridParserExt() {}

void HybridParserExt::initialize() {
  initMDL();  // 初始化模型 导入json文件 解析为 pb pb传参给模型
  HybridParser::initialize();
}

void HybridParserExt::step() { HybridParser::step(); }

void HybridParserExt::terminate() { HybridParser::terminate(); }

bool HybridParserExt::parsingParameterFromJson(char *errorLog) {
  LOG_2 << "init cartype parameter.\n";
  if (cartype.ByteSizeLong()) {
    if (tx_car::CarType_Propulsion_Type_ICE_Engine == cartype.propulsion_type()) {
      LOG_ERROR << "cartype is ICE, hybrid expected.\n";
      throw std::runtime_error("cartype is ICE, hybrid expected.");
    } else if (tx_car::CarType_Propulsion_Type_Electric_Drive == cartype.propulsion_type()) {
      LOG_ERROR << "cartype is EV, hybrid expected.\n";
      throw std::runtime_error("cartype is EV, hybrid expected.");
    } else if (tx_car::CarType_Propulsion_Type_Hybrid_Gen_P2 == cartype.propulsion_type()) {
      HybridParser_P.EnableP2 = 1;
      HybridParser_P.EnableP3 = 0;
      HybridParser_P.EnableP4 = 0;
    } else if (tx_car::CarType_Propulsion_Type_Hybrid_Gen_P3 == cartype.propulsion_type()) {
      HybridParser_P.EnableP2 = 0;
      HybridParser_P.EnableP3 = 1;
      HybridParser_P.EnableP4 = 0;
    } else if (tx_car::CarType_Propulsion_Type_Hybrid_Gen_P2P4 == cartype.propulsion_type()) {
      HybridParser_P.EnableP2 = 1;
      HybridParser_P.EnableP3 = 0;
      HybridParser_P.EnableP4 = 1;
    } else if (tx_car::CarType_Propulsion_Type_Hybrid_Gen_P3P4 == cartype.propulsion_type()) {
      HybridParser_P.EnableP2 = 0;
      HybridParser_P.EnableP3 = 1;
      HybridParser_P.EnableP4 = 1;
    } else {
      LOG_ERROR << "cartype is unknown with " << static_cast<uint32_t>(cartype.propulsion_type())
                << ", hybrid expected.\n";
      throw std::runtime_error("cartype is unknown, hybrid expected.");
    }

    HybridParser_P.Engine_Clutch_TimeConst = std::max(g_propulsion.hybrid_parameter().t_clutch().val(), 0.1);
    HybridParser_P.ReGen_Motor_GearRatio = std::max(g_propulsion.hybrid_parameter().generator_gear_ratio().val(), 0.5);

    return true;
  } else {
    if (errorLog) {
      char failed[] = "parsing json to pb file for cartype parameters failed!\n";
      memcpy(errorLog, failed, strlen(failed));
      errorLog[strlen(failed)] = '\0';
    }
    return false;
  }
  LOG_0 << "parsing cartype parameter from json file sucessfully\n";
  return true;
}

bool HybridParserExt::loadParam(const std::string &par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      if (tx_car::jsonToProto(car_json, car_param)) {
        cartype.CopyFrom(car_param.car_type());
        ecuWalue.CopyFrom(car_param.ecu());
        g_propulsion.CopyFrom(car_param.propulsion());
        return true;
      }
    }
  }
  return false;
}

/* init model by loaded json file */
bool HybridParserExt::initParam() {
  char log[1024];
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    // LOG_0<<"parsing power ecu parameters failed:\n "<<log<<"\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool HybridParserExt::initMDL() {
#ifdef LOCAL_TEST_MODE
  const std::string par_path = "E:\\work\\VehicleDynamics\\param\\car_chassis.json";
#else
  const std::string par_path = tx_car::car_config::getParPath();
#endif  // LOCAL_TEST_MODE

  if (loadParam(par_path)) {
    return initParam();
  }
  return false;
}
}  // namespace hybrid_parser
}  // namespace tx_car

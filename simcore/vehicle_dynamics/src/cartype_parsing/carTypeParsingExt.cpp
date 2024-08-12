// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "carTypeParsingExt.h"
#include "../../inc/car_common.h"
#include "../../inc/car_log.h"
#include "../../inc/proto_helper.h"
#include "car.pb.h"

namespace tx_car {
namespace cartype {
tx_car::CarType cartype;
tx_car::Pow_ECU ecuWalue;

CarTypeParsingExt::CarTypeParsingExt(/* args */) {}

CarTypeParsingExt::~CarTypeParsingExt() {}

void CarTypeParsingExt::initialize() {
  initMDL();  // 初始化模型 导入json文件 解析为 pb pb传参给模型
  carTypeParsing::initialize();
}

void CarTypeParsingExt::step() { carTypeParsing::step(); }

void CarTypeParsingExt::terminate() { carTypeParsing::terminate(); }

bool CarTypeParsingExt::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init cartype parameter.\n";
  if (cartype.ByteSizeLong()) {
    auto proptype = cartype.propulsion_type();
    switch (proptype) {
      case tx_car::CarType_Propulsion_Type_ICE_Engine:
        carTypeParsing_P.proplutiontype = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
      case tx_car::CarType_Propulsion_Type_Electric_Drive:
        carTypeParsing_P.proplutiontype = 2;  // DriveLinePowType 1-ICE 2-MOT
        break;
      default:
        carTypeParsing_P.proplutiontype = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
    }
    auto dirveType = cartype.drivetrain_type();
    switch (dirveType) {
      case tx_car::CarType_DriveTrain_Type_FrontWheel_Drive:
        carTypeParsing_P.drivetype = 1;  // DriveType 1-FD 2-RD 3-4WD
        break;
      case tx_car::CarType_DriveTrain_Type_RearWheel_Drive:
        carTypeParsing_P.drivetype = 2;  // DriveType 1-FD 2-RD 3-4WD
        break;
      case tx_car::CarType_DriveTrain_Type_FourWheel_Drive:
        carTypeParsing_P.drivetype = 3;  // DriveType 1-FD 2-RD 3-4WD
        break;
      default:
        carTypeParsing_P.drivetype = 1;  // DriveType 1-FD 2-RD 3-4WD
        break;
    }

    double ratio = ecuWalue.soft_vcu().fwdmotortorquesplitratio().val();  // 电动四驱----前电机扭矩/前后总扭矩
    ratio = std::max(std::min(ratio, 1.0), 0.0);
    carTypeParsing_P.parsingbck_01_Value = ratio;
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

bool CarTypeParsingExt::loadParam(const std::string& par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // LOG_0 << "json string:\n" << car_json << "\n";
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        cartype.CopyFrom(car_param.car_type());
        ecuWalue.CopyFrom(car_param.ecu());
        return true;
      }
    }
  }
  return false;
}

/* init model by loaded json file */
bool CarTypeParsingExt::initParam() {
  char log[1024];
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    // LOG_0<<"parsing power ecu parameters failed:\n "<<log<<"\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool CarTypeParsingExt::initMDL() {
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
}  // namespace cartype
}  // namespace tx_car

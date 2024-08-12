// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "BatteryExt.h"

#include "../../inc/car_log.h"
#include "car.pb.h"
#include "inc/proto_helper.h"

namespace tx_car {
namespace power {
tx_car::Battery battery;

BatteryExt::BatteryExt(/* args */) {}

BatteryExt::~BatteryExt() {}

void BatteryExt::initialize() {
  initMDL();
  Battery::initialize();
}

/* model step function */

void BatteryExt::step() { Battery::step(); }

/* model terminate function */
void BatteryExt::terminate() { Battery::terminate(); }

bool BatteryExt::loadParam(const std::string& par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // LOG_0 << "json string:\n" << car_json << "\n";
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        battery.CopyFrom(car_param.propulsion().batt_paramter());
        return true;
      }
    }
  }
  return false;
}
/* init model by loaded json file */
bool BatteryExt::initParam() {
  char log[1024];
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing battery parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool BatteryExt::initMDL() {
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

bool BatteryExt::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init battey parameter.\n";

  if (battery.ByteSizeLong()) {
    Battery_P.BattInitSoc = battery.battinitsoc().val();
    Battery_P.BattChargeMax = battery.battchargemax().val();
    Battery_P.Np = battery.np().val();
    Battery_P.Ns = battery.ns().val();

    int ocvSocNum = battery.batt_ocv_soc_table().u0_axis().data_size();
    int ocvNum = battery.batt_ocv_soc_table().y0_axis().data_size();
    LOG_2 << "ocvSocNum:" << ocvSocNum << ", ocvNum:" << ocvNum << "\n";
    if (ocvNum != ocvSocNum) {
      LOG_ERROR << "battery paramter ocv table soc points number is not equal ocv points number";
      return false;
    }

    // open circuit voltage vs soc
    tx_car::initAxis(Battery_P.CapLUTBp, battery.batt_ocv_soc_table().u0_axis());
    tx_car::initAxis(Battery_P.Em, battery.batt_ocv_soc_table().y0_axis());
    setBattOcvSocNum(ocvSocNum);

    // batter internal resistance vs temperature and soc
    int resisTableTempNum = battery.batt_resis_temp_soc().u0_axis().data_size();
    int resisTableSocNum = battery.batt_resis_temp_soc().u1_axis().data_size();
    int resisTableResisNum = battery.batt_resis_temp_soc().y0_axis().data_size();
    LOG_2 << "resisTableTempNum:" << resisTableTempNum << ", resisTableSocNum:" << resisTableSocNum
          << ", resisTableResisNum:" << resisTableResisNum << "\n";
    if (resisTableResisNum != resisTableSocNum * resisTableTempNum) {
      LOG_ERROR << "battery paramter resistance table resistance points number is not equal tempreture points number "
                   "multilpy soc point number";
      return false;
    }

    tx_car::initAxis(Battery_P.BattTempBp, battery.batt_resis_temp_soc().u0_axis());
    tx_car::initAxis(Battery_P.CapSOCBp, battery.batt_resis_temp_soc().u1_axis());
    tx_car::initAxis(Battery_P.RInt, battery.batt_resis_temp_soc().y0_axis());

    setBattRisisTableTempNum(resisTableTempNum);
    Battery_P.R_maxIndex[0] = resisTableTempNum - 1;
    Battery_P.R_maxIndex[1] = resisTableSocNum - 1;

    return true;
  } else {
    LOG_ERROR << "battery parameter is empty.\n";
  }

  LOG_2 << "end of init battey parameter.\n";

  return false;
}

void BatteryExt::setBattOcvSocNum(int num) {  // set the battery ocv table SOC point number
  m_batt_ocv_map_num = num;
}

void BatteryExt::setBattRisisTableTempNum(int num) {  // set the battery resis table temp point number
  m_resis_map_temp_num = num;
}
}  // namespace power
}  // namespace tx_car

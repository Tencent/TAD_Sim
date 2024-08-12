// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "EngineExt.h"
#include "../../inc/car_log.h"
#include "car.pb.h"
#include "inc/proto_helper.h"

#include <algorithm>

namespace tx_car {
namespace power {
tx_car::Propulsion propulsion;
tx_car::Pow_ECU power_ecu;

EngineExt::EngineExt(/* args */) {}

EngineExt::~EngineExt() {}

void EngineExt::initialize() {
  initMDL();
  Engine::initialize();
}

/* model step function */

void EngineExt::step() { Engine::step(); }

/* model terminate function */
void EngineExt::terminate() { Engine::terminate(); }

bool EngineExt::loadParam(const std::string& par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // LOG_0 << "json string:\n" << car_json << "\n";
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        propulsion.CopyFrom(car_param.propulsion());
        power_ecu.CopyFrom(car_param.ecu());
        return true;
      }
    }
  }
  return false;
}
/* init model by loaded json file */
bool EngineExt::initParam() {
  char log[1024];
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing engine parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool EngineExt::initMDL() {
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

bool EngineExt::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init engine parameter.\n";

  if (propulsion.engine_parameter().ByteSizeLong()) {
    // setEngineMaxSpd(propulsion.engine_parameter().maxspd().val());
    // Engine_P.EngMaxSpd = propulsion.engine_parameter().maxspd().val();
    Engine_P.fuelDensity = propulsion.engine_parameter().fueldensity().val();
    // Engine_P.EngMaxTrq = power_ecu.soft_ems().engmaxtrq().val();
    Engine_P.ConstTimeEng = propulsion.engine_parameter().tmcst().val();
    Engine_P.ConstTimeEng = std::max(Engine_P.ConstTimeEng, 0.05);

    auto engineMap = propulsion.engine_parameter().engine_outtrq_map();
    int throttleNum = engineMap.u0_axis().data_size();  //
    int spdNum = engineMap.u1_axis().data_size();
    int outTrqNum = engineMap.y0_axis().data_size();  //

    auto engineFuelMap = propulsion.engine_parameter().engine_fuelrate_map();
    int fuelMapTrqNum = engineFuelMap.u0_axis().data_size();
    int fuelMapSpdNum = engineFuelMap.u1_axis().data_size();
    int fuelRateNum = engineFuelMap.y0_axis().data_size();
    if (outTrqNum != throttleNum * spdNum || throttleNum == 0 || spdNum == 0) {
      LOG_ERROR << "engine spd or  trq num is 0; or engine max out trq num is not equal spd num * trq num! "
                << "\n";
      throw("engine spd or  trq num is 0; or engine max out trq num is not equal spd num * trq num! ");
      return false;
    }
    if (throttleNum != fuelMapTrqNum || spdNum != fuelMapSpdNum) {
      LOG_ERROR << "Engine torque map u0 or u1 number is not the same as engine fuel rate map u0 or u1 number!"
                << "\n";
      throw("Engine torque map u0 or u1 number is not the same as engine fuel rate map u0 or u1 number!");
      return false;
    }
    if (outTrqNum != fuelRateNum) {
      LOG_ERROR << "shape of engine torque map and engine fuel map should be exactly the same";
      throw("shape of engine torque map and engine fuel map should be exactly the same");
      return false;
    }
    for (size_t i = 0; i < throttleNum; i++) {
      Engine_P.f_tbrake_t_bpt[i] = engineMap.u0_axis().data(i);
    }
    for (size_t i = 0; i < spdNum; i++) {
      Engine_P.f_tbrake_n_bpt[i] = engineMap.u1_axis().data(i);
    }
    for (size_t i = 0; i < outTrqNum; i++) {
      Engine_P.f_tbrake[i] = engineMap.y0_axis().data(i);
    }
    for (size_t i = 0; i < fuelRateNum; i++) {
      Engine_P.f_fuel[i] = engineFuelMap.y0_axis().data(i);
    }
    LOG_2 << "trqNum:" << throttleNum << ", spdNum:" << spdNum << ".\n";
    Engine_P.uDLookupTable_maxIndex[0] = throttleNum - 1;
    Engine_P.uDLookupTable_maxIndex[1] = spdNum - 1;

    Engine_P.FuelFlwTable_maxIndex[0] = fuelMapTrqNum - 1;
    Engine_P.FuelFlwTable_maxIndex[1] = fuelMapSpdNum - 1;

    m_eng_map_spd_num = spdNum;
    m_eng_trq_req_num = throttleNum;
    m_throttle_num = throttleNum;

    LOG_2 << "end of init engine parameter.\n";

    return true;
  } else {
    if (errorLog) {
      char failed[] = "parsing json to pb file failed!";
      memcpy(errorLog, failed, strlen(failed));
      errorLog[strlen(failed)] = '\0';
    }
    return false;
  }
  return false;
}
}  // namespace power
}  // namespace tx_car

// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/proto_helper.h"
#include "inc/writer_body.h"
#include "inc/writer_cartype.h"
#include "inc/writer_driveline.h"
#include "inc/writer_dynamicsteer.h"
#include "inc/writer_ecu.h"
#include "inc/writer_power.h"
#include "inc/writer_steermapped.h"
#include "inc/writer_susp.h"
#include "inc/writer_wheel.h"

void dumpJson(const std::string &json_path, const tx_car::car &m_car) {
  std::string content;
  tx_car::protoToJson<tx_car::car>(m_car, content);
  tx_car::car_config::dumpToFile(content, json_path);
}

int main() {
  const std::string m_json_path = "./param/txcar_template_hybrid.json";

  tx_car::setLogFlag(2);

  tx_car::car car;

  // 1. wheel tire
  {
    WheelTireParamWriter wheeltire_writer;

    wheeltire_writer.fillBrake(car);
    wheeltire_writer.fillWheelTire(car);
  }

  // 2. body
  {
    VehBodyWriter body_writer;

    body_writer.fillSprungMass(car);
    body_writer.fillVehbody(car);
  }

  // 3. suspension
  {
    SuspWriter susp_writer;
    susp_writer.fillSusp(car);
  }

  // 4. driveline
  {
    DrivelineParamWriter driveline_writer;
    driveline_writer.fillDriveline(car);
  }

  // 5. steer
  {
    SteerMappedWriter steer_mapped_writer;
    steer_mapped_writer.fillMappedSteer(car);

    DynamicSteerWriter dynamicSteerWriter;
    dynamicSteerWriter.fillDynamicSteer(car);
  }

  // 6. power
  {
    PowerParamWriter power_para_writer;
    power_para_writer.fillEngine(car);
    power_para_writer.fillBattery(car);
    power_para_writer.fillFrontMotor(car);
    power_para_writer.fillRearMotor(car);
    power_para_writer.fillHybrid(car);
  }

  // 7. ECU
  {
    EcuParaWriter ecu_para_writer;
    ecu_para_writer.fillPowEcu(car);
    ecu_para_writer.fillHCU(car);
  }

  {
    CarTypeWriter carTypeWriter;
    carTypeWriter.fillCarType(car);
  }

  dumpJson(m_json_path, car);

  return 0;
}

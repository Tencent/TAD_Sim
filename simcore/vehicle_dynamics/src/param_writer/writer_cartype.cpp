// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/writer_cartype.h"
#include "inc/proto_helper.h"

void CarTypeWriter::fillCarType(tx_car::car &m_car) {
  auto carType = m_car.mutable_car_type();

  // carType->set_suspension_type(tx_car::CarType_Suspension_Type_I_I);
  carType->set_drivetrain_type(tx_car::CarType_DriveTrain_Type_FourWheel_Drive);
  carType->set_propulsion_type(tx_car::CarType_Propulsion_Type_Electric_Drive);
  carType->set_steer_type(tx_car::CarType_Steer_Type_Mapped_Steer);
  // carType->set_steer_type(tx_car::CarType_Steer_Type_Dynamic_Steer);
}

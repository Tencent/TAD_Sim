// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

// ////////******hooks of vehicle model******///////////

// declare hook funtion type
typedef void *(*GetVehicleModelFunc)();

// hook function name
#define Hook_Get_VehicleModel "getVehicleModel"

// hook function decleration
extern "C" {
#ifdef _WIN32
__declspec(dllexport) void *getVehicleModel();
#else
void *getVehicleModel();
#endif
}

// define implemention of hook
#define Declare_Vehicle_Model(VehicleModelClass)              \
  extern "C" {                                                \
  void *getVehicleModel() { return new VehicleModelClass(); } \
  }

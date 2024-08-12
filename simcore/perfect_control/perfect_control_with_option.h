// Copyright (c) 2019 Tencent. All rights reserved.
// Written by ChitianSun
#pragma once
#include <math.h>
#include <Eigen/Eigen>
#include <iostream>
#include "common/coord_trans.h"
#include "hadmap.h"
#include "txsim_messenger.h"
#include "txsim_module.h"

// define location message's option type
enum Option {
  Global,  // for L4, using lon lat
  Local,   // for L3, using local frame
  Count    // option count
};

// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/TNOTireParser.h"
#include "inc/car_log.h"

namespace tx_car {
const char *TNODirect(const char *ptr) { return ptr; }
TNOTireParser::TNOTireParser() { m_tir_path[0] = '\0'; }
TNOTireParser::~TNOTireParser() {
  m_tir_path[0] = '\0';
  m_TNODb.clear();
}

template <class Ty, class ConvFunc>
bool TNOParser(const std::string &key, Ty &val, const std::map<std::string, std::string> &db, ConvFunc &func) {
  auto iter = db.find(key);
  if (iter != db.end()) {
    try {
      val = func(iter->second.c_str());
      LOG_3 << "tir | " << key << ":" << val << "\n";
      return true;
    } catch (const char *msg) {
      LOG_1 << "exception while get value of " << key << ", " << msg << "\n";
      return false;
    }
  } else {
    LOG_1 << "fail to find " << key << "\n";
    return false;
  }
  return false;
}

bool TNOTire_Model::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(FITTYP, db, TNODirect);
      TNOParserMarco(TYRESIDE, db, TNODirect);
      TNOParserMarco(LONGVL, db, std::atof);
      TNOParserMarco(VXLOW, db, std::atof);
      TNOParserMarco(ROAD_INCREMENT, db, std::atof);
      TNOParserMarco(ROAD_DIRECTION, db, std::atoi);
      for (const auto &supportVersion : m_TNO_Version_List) {
        if (FITTYP.second == supportVersion) {
          LOG_1 << "tir FITTYP is " << supportVersion << "\n";
          return true;
        }
      }
      return false;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_Dimenson::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(UNLOADED_RADIUS, db, std::atof);
      TNOParserMarco(WIDTH, db, std::atof);
      TNOParserMarco(RIM_RADIUS, db, std::atof);
      TNOParserMarco(RIM_WIDTH, db, std::atof);
      TNOParserMarco(ASPECT_RATIO, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_OperatingCondition::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(INFLPRES, db, std::atof);
      TNOParserMarco(NOMPRES, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_Inertia::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(MASS, db, std::atof);
      TNOParserMarco(IXX, db, std::atof);
      TNOParserMarco(IYY, db, std::atof);
      TNOParserMarco(BELT_MASS, db, std::atof);
      TNOParserMarco(BELT_IXX, db, std::atof);
      TNOParserMarco(BELT_IYY, db, std::atof);
      TNOParserMarco(GRAVITY, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_Vertical::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(FNOMIN, db, std::atof);
      TNOParserMarco(VERTICAL_STIFFNESS, db, std::atof);
      TNOParserMarco(VERTICAL_DAMPING, db, std::atof);
      TNOParserMarco(MC_CONTOUR_A, db, std::atof);
      TNOParserMarco(MC_CONTOUR_B, db, std::atof);
      TNOParserMarco(BREFF, db, std::atof);
      TNOParserMarco(DREFF, db, std::atof);
      TNOParserMarco(FREFF, db, std::atof);
      TNOParserMarco(Q_RE0, db, std::atof);
      TNOParserMarco(Q_V1, db, std::atof);
      TNOParserMarco(Q_V2, db, std::atof);
      TNOParserMarco(Q_FZ2, db, std::atof);
      TNOParserMarco(Q_FCX, db, std::atof);
      TNOParserMarco(Q_FCY, db, std::atof);
      TNOParserMarco(Q_CAM, db, std::atof);
      TNOParserMarco(PFZ1, db, std::atof);
      TNOParserMarco(Q_FCY2, db, std::atof);
      TNOParserMarco(Q_CAM1, db, std::atof);
      TNOParserMarco(Q_CAM2, db, std::atof);
      TNOParserMarco(Q_CAM3, db, std::atof);
      TNOParserMarco(Q_FYS1, db, std::atof);
      TNOParserMarco(Q_FYS2, db, std::atof);
      TNOParserMarco(Q_FYS3, db, std::atof);
      TNOParserMarco(BOTTOM_OFFST, db, std::atof);
      TNOParserMarco(BOTTOM_STIFF, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_Structural::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(LONGITUDINAL_STIFFNESS, db, std::atof);
      TNOParserMarco(LATERAL_STIFFNESS, db, std::atof);
      TNOParserMarco(YAW_STIFFNESS, db, std::atof);
      TNOParserMarco(FREQ_LONG, db, std::atof);
      TNOParserMarco(FREQ_LAT, db, std::atof);
      TNOParserMarco(FREQ_YAW, db, std::atof);
      TNOParserMarco(FREQ_WINDUP, db, std::atof);
      TNOParserMarco(DAMP_LONG, db, std::atof);
      TNOParserMarco(DAMP_LAT, db, std::atof);
      TNOParserMarco(DAMP_YAW, db, std::atof);
      TNOParserMarco(DAMP_WINDUP, db, std::atof);
      TNOParserMarco(DAMP_RESIDUAL, db, std::atof);
      TNOParserMarco(DAMP_VLOW, db, std::atof);
      TNOParserMarco(Q_BVX, db, std::atof);
      TNOParserMarco(Q_BVT, db, std::atof);
      TNOParserMarco(PCFX1, db, std::atof);
      TNOParserMarco(PCFX2, db, std::atof);
      TNOParserMarco(PCFX3, db, std::atof);
      TNOParserMarco(PCFY1, db, std::atof);
      TNOParserMarco(PCFY2, db, std::atof);
      TNOParserMarco(PCFY3, db, std::atof);
      TNOParserMarco(PCMZ1, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_ContactPatch::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(Q_RA1, db, std::atof);
      TNOParserMarco(Q_RA2, db, std::atof);
      TNOParserMarco(Q_RB1, db, std::atof);
      TNOParserMarco(Q_RB2, db, std::atof);
      TNOParserMarco(ELLIPS_SHIFT, db, std::atof);
      TNOParserMarco(ELLIPS_LENGTH, db, std::atof);
      TNOParserMarco(ELLIPS_HEIGHT, db, std::atof);
      TNOParserMarco(ELLIPS_ORDER, db, std::atof);
      TNOParserMarco(ELLIPS_MAX_STEP, db, std::atof);
      TNOParserMarco(ELLIPS_NWIDTH, db, std::atof);
      TNOParserMarco(ELLIPS_NLENGTH, db, std::atof);
      TNOParserMarco(ENV_C1, db, std::atof);
      TNOParserMarco(ENV_C2, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_InflationPressureRng::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(PRESMIN, db, std::atof);
      TNOParserMarco(PRESMAX, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_VerticalForceRng::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(FZMIN, db, std::atof);
      TNOParserMarco(FZMAX, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_LongSlipRng::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(KPUMIN, db, std::atof);
      TNOParserMarco(KPUMAX, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_SlipAngRng::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(ALPMIN, db, std::atof);
      TNOParserMarco(ALPMAX, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_InclinationAngRng::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(CAMMIN, db, std::atof);
      TNOParserMarco(CAMMAX, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_ScalingCoef::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(LFZO, db, std::atof);
      TNOParserMarco(LCX, db, std::atof);
      TNOParserMarco(LMUX, db, std::atof);
      TNOParserMarco(LEX, db, std::atof);
      TNOParserMarco(LKX, db, std::atof);
      TNOParserMarco(LHX, db, std::atof);
      TNOParserMarco(LVX, db, std::atof);
      TNOParserMarco(LCY, db, std::atof);
      TNOParserMarco(LMUY, db, std::atof);
      TNOParserMarco(LEY, db, std::atof);
      TNOParserMarco(LKY, db, std::atof);
      TNOParserMarco(LKYC, db, std::atof);
      TNOParserMarco(LKZC, db, std::atof);
      TNOParserMarco(LHY, db, std::atof);
      TNOParserMarco(LVY, db, std::atof);
      TNOParserMarco(LTR, db, std::atof);
      TNOParserMarco(LRES, db, std::atof);
      TNOParserMarco(LXAL, db, std::atof);
      TNOParserMarco(LYKA, db, std::atof);
      TNOParserMarco(LVYKA, db, std::atof);
      TNOParserMarco(LS, db, std::atof);
      TNOParserMarco(LMX, db, std::atof);
      TNOParserMarco(LVMX, db, std::atof);
      TNOParserMarco(LMY, db, std::atof);
      TNOParserMarco(LMP, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_LongCoef::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(PCX1, db, std::atof);
      TNOParserMarco(PDX1, db, std::atof);
      TNOParserMarco(PDX2, db, std::atof);
      TNOParserMarco(PDX3, db, std::atof);
      TNOParserMarco(PEX1, db, std::atof);
      TNOParserMarco(PEX2, db, std::atof);
      TNOParserMarco(PEX3, db, std::atof);
      TNOParserMarco(PEX4, db, std::atof);
      TNOParserMarco(PKX1, db, std::atof);
      TNOParserMarco(PKX2, db, std::atof);
      TNOParserMarco(PKX3, db, std::atof);
      TNOParserMarco(PHX1, db, std::atof);
      TNOParserMarco(PHX2, db, std::atof);
      TNOParserMarco(PVX1, db, std::atof);
      TNOParserMarco(PVX2, db, std::atof);
      TNOParserMarco(PPX3, db, std::atof);
      TNOParserMarco(PPX2, db, std::atof);
      TNOParserMarco(PPX4, db, std::atof);
      TNOParserMarco(RBX1, db, std::atof);
      TNOParserMarco(RBX2, db, std::atof);
      TNOParserMarco(RBX3, db, std::atof);
      TNOParserMarco(RCX1, db, std::atof);
      TNOParserMarco(REX1, db, std::atof);
      TNOParserMarco(REX2, db, std::atof);
      TNOParserMarco(RHX1, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_OverTurnCoef::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(QSX1, db, std::atof);
      TNOParserMarco(QSX2, db, std::atof);
      TNOParserMarco(QSX3, db, std::atof);
      TNOParserMarco(QSX4, db, std::atof);
      TNOParserMarco(QSX5, db, std::atof);
      TNOParserMarco(QSX6, db, std::atof);
      TNOParserMarco(QSX7, db, std::atof);
      TNOParserMarco(QSX8, db, std::atof);
      TNOParserMarco(QSX9, db, std::atof);
      TNOParserMarco(QSX10, db, std::atof);
      TNOParserMarco(QSX11, db, std::atof);
      TNOParserMarco(QSX12, db, std::atof);
      TNOParserMarco(QSX13, db, std::atof);
      TNOParserMarco(QSX14, db, std::atof);
      TNOParserMarco(PPMX1, db, std::atof);
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_LatCoef::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(PCY1, db, std::atof);  // Shape factor Cfy for lateral forces
      TNOParserMarco(PDY1, db, std::atof);  // Lateral friction Muy
      TNOParserMarco(PDY2, db, std::atof);  // Variation of friction Muy with load
      TNOParserMarco(PDY3, db, std::atof);  // Variation of friction Muy with squared camber
      TNOParserMarco(PEY1, db, std::atof);  // Lateral curvature Efy at Fznom
      TNOParserMarco(PEY2, db, std::atof);  // Variation of curvature Efy with load
      TNOParserMarco(PEY3, db, std::atof);  // Zero order camber dependency of curvature Efy
      TNOParserMarco(PEY4, db, std::atof);  // Variation of curvature Efy with camber
      TNOParserMarco(PEY5, db, std::atof);  // Variation of curvature Efy with camber squared
      TNOParserMarco(PKY1, db, std::atof);  // Maximum value of stiffness Kfy / Fznom
      TNOParserMarco(PKY2, db, std::atof);  // Load at which Kfy reaches maximum value
      TNOParserMarco(PKY3, db, std::atof);  // Variation of Kfy / Fznom with camber
      TNOParserMarco(PKY4, db, std::atof);  // Curvature of stiffness Kfy
      TNOParserMarco(PKY5, db, std::atof);  // Peak stiffness variation with camber squared
      TNOParserMarco(PKY6, db, std::atof);  // Fy camber stiffness factor
      TNOParserMarco(PKY7, db, std::atof);  // Vertical load dependency of camber stiffness
      TNOParserMarco(PHY1, db, std::atof);  // Horizontal shift Shy at Fznom
      TNOParserMarco(PHY2, db, std::atof);  // Variation of shift Shy with load
      TNOParserMarco(PVY1, db, std::atof);  // Vertical shift in Svy / Fz at Fznom
      TNOParserMarco(PVY2, db, std::atof);  // Variation of shift Svy / Fz with load
      TNOParserMarco(PVY3, db, std::atof);  // Variation of shift Svy / Fz with camber
      TNOParserMarco(PVY4, db, std::atof);  // Variation of shift Svy / Fz with camber and load
      TNOParserMarco(PPY1, db, std::atof);  // influence of inflation pressure on cornering stiffness
      TNOParserMarco(
          PPY2, db,
          std::atof);  // influence of inflation pressure on dependency of nominal tyre load on cornering stiffness
      TNOParserMarco(PPY3, db, std::atof);  // linear influence of inflation pressure on lateral peak friction
      TNOParserMarco(PPY4, db, std::atof);  // quadratic influence of inflation pressure on lateral peak friction
      TNOParserMarco(PPY5, db, std::atof);  // Influence of inflation pressure on camber stiffness
      TNOParserMarco(RBY1, db, std::atof);  // Slope factor for combined Fy reduction
      TNOParserMarco(RBY2, db, std::atof);  // Variation of slope Fy reduction with alpha
      TNOParserMarco(RBY3, db, std::atof);  // Shift term for alpha in slope Fy reduction
      TNOParserMarco(RBY4, db, std::atof);  // Influence of camber on stiffness of Fy combined
      TNOParserMarco(RCY1, db, std::atof);  // Shape factor for combined Fy reduction
      TNOParserMarco(REY1, db, std::atof);  // Curvature factor of combined Fy
      TNOParserMarco(REY2, db, std::atof);  // Curvature factor of combined Fy with load
      TNOParserMarco(RHY1, db, std::atof);  // Shift factor for combined Fy reduction
      TNOParserMarco(RHY2, db, std::atof);  // Shift factor for combined Fy reduction with load
      TNOParserMarco(RVY1, db, std::atof);  // Kappa induced side force Svyk / Muy * Fz at Fznom
      TNOParserMarco(RVY2, db, std::atof);  // Variation of Svyk / Muy * Fz with load
      TNOParserMarco(RVY3, db, std::atof);  // Variation of Svyk / Muy * Fz with camber
      TNOParserMarco(RVY4, db, std::atof);  // Variation of Svyk / Muy * Fz with alpha
      TNOParserMarco(RVY5, db, std::atof);  // Variation of Svyk / Muy * Fz with kappa
      TNOParserMarco(RVY6, db, std::atof);  // Variation of Svyk / Muy * Fz with atan(kappa)
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_RollingCoef::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(QSY1, db, std::atof);  // Rolling resistance torque coefficient
      TNOParserMarco(QSY2, db, std::atof);  // Rolling resistance torque depending on Fx
      TNOParserMarco(QSY3, db, std::atof);  // Rolling resistance torque depending on speed
      TNOParserMarco(QSY4, db, std::atof);  // Rolling resistance torque depending on speed ^ 4
      TNOParserMarco(QSY5, db, std::atof);  // Rolling resistance torque depending on camber squared
      TNOParserMarco(QSY6, db, std::atof);  // Rolling resistance torque depending on load and camber squared
      TNOParserMarco(QSY7, db, std::atof);  // Rolling resistance torque coefficient load dependency
      TNOParserMarco(QSY8, db, std::atof);  // Rolling resistance torque coefficient pressure dependency
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_AlignCoef::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(QBZ1, db, std::atof);   // Trail slope factor for trail Bpt at Fznom
      TNOParserMarco(QBZ2, db, std::atof);   // Variation of slope Bpt with load
      TNOParserMarco(QBZ3, db, std::atof);   // Variation of slope Bpt with load squared
      TNOParserMarco(QBZ4, db, std::atof);   // Variation of slope Bpt with camber
      TNOParserMarco(QBZ5, db, std::atof);   // Variation of slope Bpt with absolute camber
      TNOParserMarco(QBZ9, db, std::atof);   // Factor for scaling factors of slope factor Br of Mzr
      TNOParserMarco(QBZ10, db, std::atof);  // Factor for dimensionless cornering stiffness of Br of Mzr
      TNOParserMarco(QCZ1, db, std::atof);   // Shape factor Cpt for pneumatic trail
      TNOParserMarco(QDZ1, db, std::atof);   // Peak trail Dpt, Dpt * (Fz / Fznom * R0)
      TNOParserMarco(QDZ2, db, std::atof);   // Variation of peak Dpt with load
      TNOParserMarco(QDZ3, db, std::atof);   // Variation of peak Dpt with camber
      TNOParserMarco(QDZ4, db, std::atof);   // Variation of peak Dpt with camber squared
      TNOParserMarco(QDZ6, db, std::atof);   // Peak residual torque Dmr, Dmr / (Fz*R0)
      TNOParserMarco(QDZ7, db, std::atof);   // Variation of peak factor Dmr with load
      TNOParserMarco(QDZ8, db, std::atof);   // Variation of peak factor Dmr with camber
      TNOParserMarco(QDZ9, db, std::atof);   // Variation of peak factor Dmr with camber and load
      TNOParserMarco(QDZ10, db, std::atof);  // Variation of peak factor Dmr with camber squared
      TNOParserMarco(QDZ11, db, std::atof);  // Variation of Dmr with camber squared and load
      TNOParserMarco(QEZ1, db, std::atof);   // Trail curvature Ept at Fznom
      TNOParserMarco(QEZ2, db, std::atof);   // Variation of curvature Ept with load
      TNOParserMarco(QEZ3, db, std::atof);   // Variation of curvature Ept with load squared
      TNOParserMarco(QEZ4, db, std::atof);   // Variation of curvature Ept with sign of Alpha - t
      TNOParserMarco(QEZ5, db, std::atof);   // Variation of Ept with camber and sign Alpha - t
      TNOParserMarco(QHZ1, db, std::atof);   // Trail horizontal shift Sht at Fznom
      TNOParserMarco(QHZ2, db, std::atof);   // Variation of shift Sht with load
      TNOParserMarco(QHZ3, db, std::atof);   // Variation of shift Sht with camber
      TNOParserMarco(QHZ4, db, std::atof);   // Variation of shift Sht with camber and load
      TNOParserMarco(PPZ1, db, std::atof);   // effect of inflation pressure on length of pneumatic trail
      TNOParserMarco(PPZ2, db, std::atof);   // Influence of inflation pressure on residual aligning torque
      TNOParserMarco(SSZ1, db, std::atof);   // Nominal value of s / R0: effect of Fx on Mz
      TNOParserMarco(SSZ2, db, std::atof);   // Variation of distance s / R0 with Fy / Fznom
      TNOParserMarco(SSZ3, db, std::atof);   // Variation of distance s / R0 with camber
      TNOParserMarco(SSZ4, db, std::atof);   // Variation of distance s / R0 with load and camber
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

bool TNOTire_TurnSlipCoef::parseFrom(const TNODb &models) {
  auto iter = models.find(id);

  if (iter != models.end()) {
    const TNOPairs &db = iter->second;
    if (db.size() > 0) {
      TNOParserMarco(PDXP1, db, std::atof);  // Peak Fx reduction due to spin parameter
      TNOParserMarco(PDXP2, db, std::atof);  // Peak Fx reduction due to spin with varying load parameter
      TNOParserMarco(PDXP3, db, std::atof);  // Peak Fx reduction due to spin with kappa parameter
      TNOParserMarco(PKYP1, db, std::atof);  // Cornering stiffness reduction due to spin
      TNOParserMarco(PDYP1, db, std::atof);  // Peak Fy reduction due to spin parameter
      TNOParserMarco(PDYP2, db, std::atof);  // Peak Fy reduction due to spin with varying load parameter
      TNOParserMarco(PDYP3, db, std::atof);  // Peak Fy reduction due to spin with alpha parameter
      TNOParserMarco(PDYP4, db, std::atof);  // Peak Fy reduction due to square root of spin parameter
      TNOParserMarco(PHYP1, db, std::atof);  // Fy - alpha curve lateral shift limitation
      TNOParserMarco(PHYP2, db, std::atof);  // Fy - alpha curve maximum lateral shift parameter
      TNOParserMarco(PHYP3, db, std::atof);  // Fy - alpha curve maximum lateral shift varying with load parameter
      TNOParserMarco(PHYP4, db, std::atof);  // Fy - alpha curve maximum lateral shift parameter
      TNOParserMarco(PECP1, db, std::atof);  // Camber w.r.t.spin reduction factor parameter in camber stiffness
      TNOParserMarco(PECP2, db,
                     std::atof);  // Camber w.r.t.spin reduction factor varying with load parameter in camber stiffness
      TNOParserMarco(QDTP1, db, std::atof);  // Pneumatic trail reduction factor due to turn slip parameter
      TNOParserMarco(QCRP1, db, std::atof);  // Turning moment at constant turning and zero forward speed parameter
      TNOParserMarco(QCRP2, db, std::atof);  // Turn slip moment(at alpha, 90deg) parameter for increase with spin
      TNOParserMarco(QBRP1, db, std::atof);  // Residual(spin) torque reduction factor parameter due to side slip
      TNOParserMarco(QDRP1, db, std::atof);  // Turn slip moment peak magnitude parameter
      return true;
    } else {
      LOG_1 << ".tir does not contain any data of " << id << ".\n";
    }
  } else {
    LOG_1 << "can not find " << id << " from .tir file.\n";
  }

  return false;
}

/* set TNO tir path */
void TNOTireParser::setTirPath(const std::string &tir_path) {
  if (tx_car::isFileExist(tir_path)) m_tir_path = tir_path;
}

/* get TNO tir path */
const std::string &TNOTireParser::getTirPath() { return m_tir_path; }

/* parse TNO tir */
bool TNOTireParser::parseTNOTir() {
  if (tx_car::isFileExist(m_tir_path)) {
    // content of .tir file
    std::list<std::string> content;

    // load all data from .tir
    if (tx_car::car_config::loadFromFile(content, m_tir_path)) {
      std::string model_key = "";

      // load .tir from file
      for (auto iter = content.begin(); iter != content.end(); ++iter) {
        // pre-process of one line data
        std::string &line = *iter;
        preProcessLine(line);

        // if line not empty after processed
        if (line.size() > 0) {
          if (tx_car::beginWithChar(line, model_character)) {
            model_key = line;
            m_TNODb[model_key] = TNOPairs();
          } else {
            // one key-value pair in TNO .tir file
            std::vector<std::string> splitted;
            tx_car::splitString(line, splitted);
            if (splitted.size() == 2) {
              m_TNODb[model_key][splitted.at(0)] = splitted.at(1);
              // LOG(INFO) << splitted.at(0) << ":" << splitted.at(1) << "\n";
            }
          }
        }
      }

      // parse .tir
      {
        bool parse_state = true;
        parse_state = m_tir.MODEL.parseFrom(m_TNODb);
        if (parse_state) {
          m_tir.DIMENSION.parseFrom(m_TNODb);
          m_tir.OPERATING_CONDITIONS.parseFrom(m_TNODb);
          m_tir.INERTIA.parseFrom(m_TNODb);
          m_tir.VERTICAL.parseFrom(m_TNODb);
          m_tir.STRUCTURAL.parseFrom(m_TNODb);
          m_tir.CONTACT_PATCH.parseFrom(m_TNODb);
          m_tir.INFLATION_PRESSURE_RANGE.parseFrom(m_TNODb);
          m_tir.VERTICAL_FORCE_RANGE.parseFrom(m_TNODb);
          m_tir.LONG_SLIP_RANGE.parseFrom(m_TNODb);
          m_tir.SLIP_ANGLE_RANGE.parseFrom(m_TNODb);
          m_tir.INCLINATION_ANGLE_RANGE.parseFrom(m_TNODb);
          m_tir.SCALING_COEFFICIENTS.parseFrom(m_TNODb);
          m_tir.LONGITUDINAL_COEFFICIENTS.parseFrom(m_TNODb);
          m_tir.OVERTURNING_COEFFICIENTS.parseFrom(m_TNODb);
          m_tir.LATERAL_COEFFICIENTS.parseFrom(m_TNODb);
          m_tir.ROLLING_COEFFICIENTS.parseFrom(m_TNODb);
          m_tir.ALIGNING_COEFFICIENTS.parseFrom(m_TNODb);
          m_tir.TURNSLIP_COEFFICIENTS.parseFrom(m_TNODb);
          LOG_2 << "TNO parsing done with file " << m_tir_path << "\n";
        } else {
          LOG_ERROR << "support TNO .tir version 62. check " << m_tir_path;
        }

        return parse_state;
      }
      return true;
    } else {
      LOG_ERROR << "fail to load TNO .tir. " << m_tir_path << ".\n";
    }
  } else {
    LOG_ERROR << "TNO .tir file does not exist. " << m_tir_path << ".\n";
  }

  return false;
}

/* pre-process of one line */
void TNOTireParser::preProcessLine(std::string &line) {
  for (auto i = 0ul; i < comment_charcter.size(); ++i) {
    if (tx_car::beginWithChar(line, comment_charcter.at(i))) {
      line = "";
      return;
    }
  }

  tx_car::trim(line, '\n');
  tx_car::trim(line, '\r');

  for (auto i = 0ul; i < comment_charcter.size(); ++i) {
    tx_car::trim(line, comment_charcter.at(i));
  }
  for (auto i = 0ul; i < m_stop_characters.size(); ++i) {
    tx_car::rmCharFromString(line, m_stop_characters.at(i));
  }
}

/* get TNO tire parameter */
const TNOTire &TNOTireParser::getTNOTir() { return m_tir; }
}  // namespace tx_car

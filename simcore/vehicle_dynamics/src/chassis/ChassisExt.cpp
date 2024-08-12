// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "ChassisExt.h"
#include <cmath>
#include "car.pb.h"
#include "inc/TNOTireParser.h"
#include "inc/car_log.h"
#include "inc/proto_helper.h"

namespace tx_car {
namespace chassis {
tx_car::SprungMassDD g_sprung;
tx_car::SuspensionDD g_susp;
tx_car::WheelTireDD g_wheel;
tx_car::BrakeDD g_brake;
tx_car::VehBodyDD g_body;
tx_car::TNOTireParser g_TNO_parser;

const int g_flip = -1;         // use carsim compliance data
const int g_susp_fliper = -1;  // suspension left and right fliper

ChassisExt::ChassisExt() {
  g_sprung.Clear();
  g_susp.Clear();
  g_wheel.Clear();
  g_brake.Clear();
  g_body.Clear();
}

ChassisExt::~ChassisExt() {}

/* Initial conditions function */
void ChassisExt::initialize() {
  initMDL();
  Chassis::initialize();
}

/* model step function */
void ChassisExt::step() { Chassis::step(); }

/* model terminate function */
void ChassisExt::terminate() { Chassis::terminate(); }

bool ChassisExt::loadParam(const std::string& par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // LOG_DEBUG << "json string:\n" << car_json << "\n";
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        g_sprung.CopyFrom(car_param.sprung_mass());
        g_susp.CopyFrom(car_param.susp());
        g_wheel.CopyFrom(car_param.wheel_tire());
        g_brake.CopyFrom(g_wheel.brake());
        g_body.CopyFrom(car_param.body());
        return true;
      }
    }
  }

  return false;
}

/* init model by loaded json file */
bool ChassisExt::initParam() {
  LOG_2 << "init chassis parameter.\n";

  // init sprung mass
  { tx_car::initVehParam(ChassisExt::Chassis_P.VEH, g_sprung); }

  // init chassis
  {
    initParamBody();
    initParamSusp();
    initParamWheelTire();
  }

  LOG_2 << "end of init chassis parameter.\n";

  return true;
}

bool ChassisExt::initParamBody() {
  LOG_2 << "init body parameter.\n";

  {
    ChassisExt::Chassis_P.UnsprungMasses_Gain =
        g_sprung.unsprungmassfrontaxle().val() + g_sprung.unsprungmassrearaxle().val();
  }

  // init chassis
  {
    ChassisExt::Chassis_P.VehicleBody6DOF_d = 0;  // g_body.chassis().vehiclebody6dof_d().val();

    ChassisExt::Chassis_P.VehicleBody6DOF_Xe_o[0] = ChassisExt::Chassis_P.VEH.InitialLongPosition;
    ChassisExt::Chassis_P.VehicleBody6DOF_Xe_o[1] = ChassisExt::Chassis_P.VEH.InitialLatPosition;
    ChassisExt::Chassis_P.VehicleBody6DOF_Xe_o[2] = ChassisExt::Chassis_P.VEH.InitialVertPosition;

    ChassisExt::Chassis_P.VehicleBody6DOF_xbdot_o[0] =
        ChassisExt::Chassis_P.VEH.InitialLongVel;          // g_sprung.initiallongvel().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_xbdot_o[1] = 0;  // g_sprung.initiallatvel().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_xbdot_o[2] = 0;  // g_sprung.initialvertvel().val();

    ChassisExt::Chassis_P.VehicleBody6DOF_eul_o[0] = 0.0;  // g_sprung.initialrollangle().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_eul_o[1] =
        ChassisExt::Chassis_P.VEH.InitialPitchAngle;  // g_sprung.initialpitchangle().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_eul_o[2] =
        ChassisExt::Chassis_P.VEH.InitialYawAngle;  // g_sprung.initialyawangle().val();

    ChassisExt::Chassis_P.VehicleBody6DOF_p_o[0] = 0.0;  // g_sprung.initialrollrate().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_p_o[1] = 0.0;  // g_sprung.initialpitchrate().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_p_o[2] = 0.0;  // g_sprung.initialyawrate().val();

    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[0] =
        ChassisExt::Chassis_P.VEH.RollMomentInertia;  // g_sprung.rollmomentinertia().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[1] = 0.0;
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[2] = 0.0;
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[3] = 0.0;
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[4] =
        ChassisExt::Chassis_P.VEH.PitchMomentInertia;  // g_sprung.pitchmomentinertia().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[5] = 0.0;
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[6] = 0.0;
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[7] = 0.0;
    ChassisExt::Chassis_P.VehicleBody6DOF_Iveh[8] =
        ChassisExt::Chassis_P.VEH.YawMomentInertia;  // g_sprung.yawmomentinertia().val();
  }

  // init aerodynamics
  {
    ChassisExt::Chassis_P.VehicleBody6DOF_Cd = g_body.aerodynamics().vehiclebody6dof_cd().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_Cl = g_body.aerodynamics().vehiclebody6dof_cl().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_Cpm = g_body.aerodynamics().vehiclebody6dof_cpm().val();

    // 1-d map data format checke
    if (!tx_car::map1DFormatChecker(g_body.aerodynamics().beta_w_cs_1dmap())) {
      std::abort();
    }

    if (!tx_car::map1DFormatChecker(g_body.aerodynamics().beta_w_cym_1dmap())) {
      std::abort();
    }

    if (g_body.aerodynamics().beta_w_cs_1dmap().u0_axis().data_size() !=
        g_body.aerodynamics().beta_w_cym_1dmap().u0_axis().data_size()) {
      LOG_ERROR << "[Body]:" << g_body.aerodynamics().beta_w_cs_1dmap().disp_name() << " vs "
                << g_body.aerodynamics().beta_w_cym_1dmap().disp_name() << ", axis u0 data size mismatch.\n";
      std::abort();
    }

    tx_car::initAxis(ChassisExt::Chassis_P.VehicleBody6DOF_beta_w, g_body.aerodynamics().beta_w_cs_1dmap().u0_axis());
    tx_car::initAxis(ChassisExt::Chassis_P.VehicleBody6DOF_Cs, g_body.aerodynamics().beta_w_cs_1dmap().y0_axis());
    tx_car::initAxis(ChassisExt::Chassis_P.VehicleBody6DOF_Cym, g_body.aerodynamics().beta_w_cym_1dmap().y0_axis());

    m_beta_w_cs_maxIndex = g_body.aerodynamics().beta_w_cs_1dmap().y0_axis().data_size() - 1;
    m_beta_w_cym_maxIndex = g_body.aerodynamics().beta_w_cym_1dmap().y0_axis().data_size() - 1;

    ChassisExt::Chassis_P.VehicleBody6DOF_Pabs = g_body.aerodynamics().vehiclebody6dof_pabs().val();
    ChassisExt::Chassis_P.VehicleBody6DOF_Tair = g_body.aerodynamics().vehiclebody6dof_tair().val();
  }

  // simulation
  {
    /*
        ChassisExt::Chassis_P.VehicleBody6DOF_xdot_tol = g_body.simulation().vehiclebody6dof_xdot_tol().val();
        ChassisExt::Chassis_P.VehicleBody6DOF_longOff = g_body.simulation().vehiclebody6dof_longoff().val();
        ChassisExt::Chassis_P.VehicleBody6DOF_latOff = g_body.simulation().vehiclebody6dof_latoff().val();
        ChassisExt::Chassis_P.VehicleBody6DOF_vertOff = g_body.simulation().vehiclebody6dof_vertoff().val();
    */
    ChassisExt::Chassis_P.VehicleBody6DOF_xdot_tol = 0.01;
    ChassisExt::Chassis_P.VehicleBody6DOF_longOff = 0;
    ChassisExt::Chassis_P.VehicleBody6DOF_latOff = 0;
    ChassisExt::Chassis_P.VehicleBody6DOF_vertOff = 0.00112;
  }

  LOG_2 << "end of init body parameter.\n";

  return true;
}

bool ChassisExt::initParamWheelTire() {
  LOG_2 << "init wheel tire parameter.\n";

  // set brake info
  ChassisExt::Chassis_P.CombinedSlipWheel2DOF_mu_static = g_brake.mu_static().val();
  ChassisExt::Chassis_P.CombinedSlipWheel2DOF_mu_kinetic = g_brake.mu_kinetic().val();

  {
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_mu_static = g_brake.mu_static().val();
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_mu_kinetic = g_brake.mu_kinetic().val();

    const auto& u0Axis = g_brake.brakemap().u0_axis();
    const auto& u1Axis = g_brake.brakemap().u1_axis();
    const auto& y0Axis = g_brake.brakemap().y0_axis();

    if (u0Axis.data_size() * u1Axis.data_size() == y0Axis.data_size() && y0Axis.data_size() > 0) {
      m_mapped_brake_pressure_maxIndex = u0Axis.data_size() - 1;
      for (auto i = 0ul; i <= m_mapped_brake_pressure_maxIndex; ++i) {
        ChassisExt::Chassis_P.CombinedSlipWheel2DOF_brake_p_bpt[i] = u0Axis.data().at(i);
      }

      m_mapped_brake_wheelspeed_maxIndex = u1Axis.data_size() - 1;
      for (auto i = 0ul; i <= m_mapped_brake_wheelspeed_maxIndex; ++i) {
        ChassisExt::Chassis_P.CombinedSlipWheel2DOF_brake_n_bpt[i] = u1Axis.data().at(i);
      }

      m_mapped_brake_torque_maxIndex = y0Axis.data_size() - 1;
      for (auto i = 0ul; i <= m_mapped_brake_torque_maxIndex; ++i) {
        ChassisExt::Chassis_P.CombinedSlipWheel2DOF_f_brake_t[i] = y0Axis.data().at(i);
      }
      ChassisExt::Chassis_P.uDLookupTable_maxIndex[0] = m_mapped_brake_pressure_maxIndex;
      ChassisExt::Chassis_P.uDLookupTable_maxIndex[1] = m_mapped_brake_wheelspeed_maxIndex;
    } else {
      LOG_2 << "brake map data size error. use default brake map.\n";
    }
  }

  // set default wheel radius
  g_wheel.mutable_tire()->mutable_wheel_radius()->set_val(0.3411);
  ChassisExt::Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS = 0.3411;

  // set tire pressure
  ChassisExt::Chassis_P.Pressure_const = 230000;
  ChassisExt::Chassis_P.Saturation_LowerSat_f = 200 * M_PI;
  Chassis_P.MagicTireConstInput_vdynMF[3] = 1;

  // set wheel init height
  /*
  {
      ChassisExt::Chassis_P.InitTireDisplacementZ[0] = g_wheel.fl_init_height().val();
      ChassisExt::Chassis_P.InitTireDisplacementZ[1] = g_wheel.fr_init_height().val();
      ChassisExt::Chassis_P.InitTireDisplacementZ[2] = g_wheel.rl_init_height().val();
      ChassisExt::Chassis_P.InitTireDisplacementZ[3] = g_wheel.rr_init_height().val();
  }
  */

  // parse TNO tir
  LOG_0 << ".tir path is " << g_wheel.tire().tno_tir_path().str_var() << "\n";
  g_TNO_parser.setTirPath(g_wheel.tire().tno_tir_path().str_var());

  if (g_TNO_parser.parseTNOTir()) {
    const TNOTire& tno_tire = g_TNO_parser.getTNOTir();
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_LONGVL = tno_tire.MODEL.LONGVL.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_VXLOW = tno_tire.MODEL.VXLOW.second;
    LOG_3 << "TNO .tir [MODEL]:ROAD_INCREMENT not used.\n";

    LOG_2 << "origin width of tir is " << ChassisExt::Chassis_P.CombinedSlipWheel2DOF_WIDTH << "\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS = tno_tire.DIMENSION.UNLOADED_RADIUS.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_WIDTH = tno_tire.DIMENSION.WIDTH.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RIM_RADIUS = tno_tire.DIMENSION.RIM_RADIUS.second;
    LOG_3 << "TNO .tir [DIMENSION]:RIM_WIDTH not used.\n";
    LOG_3 << "TNO .tir [DIMENSION]:ASPECT_RATIO not used.\n";

    g_wheel.mutable_tire()->mutable_wheel_radius()->set_val(
        ChassisExt::Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS);

    LOG_3 << "current width of tir is " << ChassisExt::Chassis_P.CombinedSlipWheel2DOF_WIDTH << "\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_NOMPRES = tno_tire.OPERATING_CONDITIONS.NOMPRES.second;
    ChassisExt::Chassis_P.Pressure_const = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_NOMPRES;
    LOG_3 << "TNO .tir [OPERATING_CONDITIONS]:INFLPRES not used.\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_MASS = tno_tire.INERTIA.MASS.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_IYY = tno_tire.INERTIA.IYY.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_GRAVITY = tno_tire.INERTIA.GRAVITY.second;
    LOG_3 << "TNO .tir [INERTIA]:IXX not used.\n";
    LOG_3 << "TNO .tir [INERTIA]:BELT_MASS not used.\n";
    LOG_3 << "TNO .tir [INERTIA]:BELT_IXX not used.\n";
    LOG_3 << "TNO .tir [INERTIA]:BELT_IYY not used.\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_FNOMIN = tno_tire.VERTICAL.FNOMIN.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS = tno_tire.VERTICAL.VERTICAL_STIFFNESS.second;
    Chassis_P.MagicTireConstInput_vdynMF[38] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_VERTICAL_STIFFNESS;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_VERTICAL_DAMPING = tno_tire.VERTICAL.VERTICAL_DAMPING.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_BREFF = tno_tire.VERTICAL.BREFF.second;
    Chassis_P.MagicTireConstInput_vdynMF[42] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_BREFF;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_DREFF = tno_tire.VERTICAL.DREFF.second;
    Chassis_P.MagicTireConstInput_vdynMF[43] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_DREFF;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_FREFF = tno_tire.VERTICAL.FREFF.second;
    Chassis_P.MagicTireConstInput_vdynMF[44] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_FREFF;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RE0 = tno_tire.VERTICAL.Q_RE0.second;
    Chassis_P.MagicTireConstInput_vdynMF[45] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RE0;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_V1 = tno_tire.VERTICAL.Q_V1.second;
    Chassis_P.MagicTireConstInput_vdynMF[46] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_V1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_V2 = tno_tire.VERTICAL.Q_V2.second;
    Chassis_P.MagicTireConstInput_vdynMF[47] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_V2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FZ2 = tno_tire.VERTICAL.Q_FZ2.second;
    Chassis_P.MagicTireConstInput_vdynMF[49] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FZ2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FCX = tno_tire.VERTICAL.Q_FCX.second;
    Chassis_P.MagicTireConstInput_vdynMF[51] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FCX;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FCY = tno_tire.VERTICAL.Q_FCY.second;
    Chassis_P.MagicTireConstInput_vdynMF[52] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FCY;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PFZ1 = tno_tire.VERTICAL.PFZ1.second;
    Chassis_P.MagicTireConstInput_vdynMF[54] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PFZ1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FCY2 = tno_tire.VERTICAL.Q_FCY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[55] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_FCY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_BOTTOM_OFFST = tno_tire.VERTICAL.BOTTOM_OFFST.second;
    Chassis_P.MagicTireConstInput_vdynMF[62] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_BOTTOM_OFFST;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_BOTTOM_STIFF = tno_tire.VERTICAL.BOTTOM_STIFF.second;
    Chassis_P.MagicTireConstInput_vdynMF[63] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_BOTTOM_STIFF;
    LOG_3 << "TNO .tir [VERTICAL]:MC_CONTOUR_A not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:MC_CONTOUR_B not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:Q_CAM not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:Q_CAM1 not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:Q_CAM2 not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:Q_CAM3 not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:Q_FYS1 not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:Q_FYS2 not used.\n";
    LOG_3 << "TNO .tir [VERTICAL]:Q_FYS3 not used.\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS =
        tno_tire.STRUCTURAL.LONGITUDINAL_STIFFNESS.second;
    Chassis_P.MagicTireConstInput_vdynMF[64] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_LONGITUDINAL_STIFFNESS;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_LATERAL_STIFFNESS = tno_tire.STRUCTURAL.LATERAL_STIFFNESS.second;
    Chassis_P.MagicTireConstInput_vdynMF[65] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_LATERAL_STIFFNESS;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFX1 = tno_tire.STRUCTURAL.PCFX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[79] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFX2 = tno_tire.STRUCTURAL.PCFX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[80] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFX3 = tno_tire.STRUCTURAL.PCFX3.second;
    Chassis_P.MagicTireConstInput_vdynMF[81] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFX3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFY1 = tno_tire.STRUCTURAL.PCFY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[82] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFY2 = tno_tire.STRUCTURAL.PCFY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[83] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFY3 = tno_tire.STRUCTURAL.PCFY3.second;
    Chassis_P.MagicTireConstInput_vdynMF[84] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCFY3;
    LOG_3 << "TNO .tir [STRUCTURAL]:YAW_STIFFNESS not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:FREQ_LONG not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:FREQ_LAT not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:FREQ_YAW not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:FREQ_WINDUP not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:DAMP_LONG not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:DAMP_LAT not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:DAMP_YAW not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:DAMP_WINDUP not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:DAMP_RESIDUAL not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:DAMP_VLOW not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:Q_BVX not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:Q_BVT not used.\n";
    LOG_3 << "TNO .tir [STRUCTURAL]:PCMZ1 not used.\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RA1 = tno_tire.CONTACT_PATCH.Q_RA1.second;
    Chassis_P.MagicTireConstInput_vdynMF[86] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RA1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RA2 = tno_tire.CONTACT_PATCH.Q_RA2.second;
    Chassis_P.MagicTireConstInput_vdynMF[87] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RA2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RB1 = tno_tire.CONTACT_PATCH.Q_RB1.second;
    Chassis_P.MagicTireConstInput_vdynMF[88] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RB1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RB2 = tno_tire.CONTACT_PATCH.Q_RB2.second;
    Chassis_P.MagicTireConstInput_vdynMF[89] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_Q_RB2;
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ELLIPS_SHIFT not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ELLIPS_LENGTH not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ELLIPS_HEIGHT not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ELLIPS_ORDER not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ELLIPS_MAX_STEP not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ELLIPS_NWIDTH not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ELLIPS_NLENGTH not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ENV_C1 not used.\n";
    LOG_3 << "TNO .tir [CONTACT_PATCH]:ENV_C2 not used.\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PRESMAX = tno_tire.INFLATION_PRESSURE_RANGE.PRESMAX.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PRESMIN = tno_tire.INFLATION_PRESSURE_RANGE.PRESMIN.second;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_FZMAX = tno_tire.VERTICAL_FORCE_RANGE.FZMAX.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_FZMIN = tno_tire.VERTICAL_FORCE_RANGE.FZMIN.second;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_KPUMAX = tno_tire.LONG_SLIP_RANGE.KPUMAX.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_KPUMIN = tno_tire.LONG_SLIP_RANGE.KPUMIN.second;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_ALPMAX = tno_tire.SLIP_ANGLE_RANGE.ALPMAX.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_ALPMIN = tno_tire.SLIP_ANGLE_RANGE.ALPMIN.second;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_CAMMAX = tno_tire.INCLINATION_ANGLE_RANGE.CAMMAX.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_CAMMIN = tno_tire.INCLINATION_ANGLE_RANGE.CAMMIN.second;

    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LFZO not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LCX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LMUX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LEX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LKX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LHX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LVX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LCY not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LMUY not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LEY not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LKY not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LKYC not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LKZC not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LHY not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LVY not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LTR not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LRES not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LXAL not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LYKA not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LVYKA not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LS not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LMX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LVMX not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LMY not used.\n";
    LOG_3 << "TNO .tir [SCALING_COEFFICIENTS]:LMP not used.\n";

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.PCX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[134] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.PDX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[135] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.PDX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[136] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDX3 = tno_tire.LONGITUDINAL_COEFFICIENTS.PDX3.second;
    Chassis_P.MagicTireConstInput_vdynMF[137] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDX3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.PEX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[138] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.PEX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[139] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX3 = tno_tire.LONGITUDINAL_COEFFICIENTS.PEX3.second;
    Chassis_P.MagicTireConstInput_vdynMF[140] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX4 = tno_tire.LONGITUDINAL_COEFFICIENTS.PEX4.second;
    Chassis_P.MagicTireConstInput_vdynMF[141] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEX4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.PKX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[142] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.PKX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[143] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKX3 = tno_tire.LONGITUDINAL_COEFFICIENTS.PKX3.second;
    Chassis_P.MagicTireConstInput_vdynMF[144] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKX3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.PHX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[145] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.PHX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[146] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.PVX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[147] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.PVX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[148] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.PPX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[149] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.PPX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[150] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX3 = tno_tire.LONGITUDINAL_COEFFICIENTS.PPX3.second;
    Chassis_P.MagicTireConstInput_vdynMF[151] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX4 = tno_tire.LONGITUDINAL_COEFFICIENTS.PPX4.second;
    Chassis_P.MagicTireConstInput_vdynMF[152] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPX4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.RBX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[153] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.RBX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[154] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBX3 = tno_tire.LONGITUDINAL_COEFFICIENTS.RBX3.second;
    Chassis_P.MagicTireConstInput_vdynMF[155] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBX3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RCX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.RCX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[156] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RCX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.REX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[157] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REX1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REX2 = tno_tire.LONGITUDINAL_COEFFICIENTS.REX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[158] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RHX1 = tno_tire.LONGITUDINAL_COEFFICIENTS.RHX1.second;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCY1 = tno_tire.LATERAL_COEFFICIENTS.PCY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[175] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PCY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDY1 = tno_tire.LATERAL_COEFFICIENTS.PDY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[176] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDY2 = tno_tire.LATERAL_COEFFICIENTS.PDY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[177] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDY3 = tno_tire.LATERAL_COEFFICIENTS.PDY3.second;
    Chassis_P.MagicTireConstInput_vdynMF[178] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDY3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY1 = tno_tire.LATERAL_COEFFICIENTS.PEY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[179] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY2 = tno_tire.LATERAL_COEFFICIENTS.PEY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[180] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY3 = tno_tire.LATERAL_COEFFICIENTS.PEY3.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY4 = tno_tire.LATERAL_COEFFICIENTS.PEY4.second;
    Chassis_P.MagicTireConstInput_vdynMF[182] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY5 = tno_tire.LATERAL_COEFFICIENTS.PEY5.second;
    Chassis_P.MagicTireConstInput_vdynMF[183] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PEY5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY1 = tno_tire.LATERAL_COEFFICIENTS.PKY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[184] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY2 = tno_tire.LATERAL_COEFFICIENTS.PKY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[185] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY3 = tno_tire.LATERAL_COEFFICIENTS.PKY3.second;
    Chassis_P.MagicTireConstInput_vdynMF[186] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY4 = tno_tire.LATERAL_COEFFICIENTS.PKY4.second;
    Chassis_P.MagicTireConstInput_vdynMF[187] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY5 = tno_tire.LATERAL_COEFFICIENTS.PKY5.second;
    Chassis_P.MagicTireConstInput_vdynMF[188] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY6 = tno_tire.LATERAL_COEFFICIENTS.PKY6.second;
    Chassis_P.MagicTireConstInput_vdynMF[189] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY6;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY7 = tno_tire.LATERAL_COEFFICIENTS.PKY7.second;
    Chassis_P.MagicTireConstInput_vdynMF[190] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKY7;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHY1 = tno_tire.LATERAL_COEFFICIENTS.PHY1.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHY2 = tno_tire.LATERAL_COEFFICIENTS.PHY2.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVY1 = tno_tire.LATERAL_COEFFICIENTS.PVY1.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVY2 = tno_tire.LATERAL_COEFFICIENTS.PVY2.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVY3 = tno_tire.LATERAL_COEFFICIENTS.PVY3.second;
    Chassis_P.MagicTireConstInput_vdynMF[195] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVY3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVY4 = tno_tire.LATERAL_COEFFICIENTS.PVY4.second;
    Chassis_P.MagicTireConstInput_vdynMF[196] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PVY4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY1 = tno_tire.LATERAL_COEFFICIENTS.PPY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[197] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY2 = tno_tire.LATERAL_COEFFICIENTS.PPY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[198] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY3 = tno_tire.LATERAL_COEFFICIENTS.PPY3.second;
    Chassis_P.MagicTireConstInput_vdynMF[199] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY4 = tno_tire.LATERAL_COEFFICIENTS.PPY4.second;
    Chassis_P.MagicTireConstInput_vdynMF[200] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY5 = tno_tire.LATERAL_COEFFICIENTS.PPY5.second;
    Chassis_P.MagicTireConstInput_vdynMF[201] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPY5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBY1 = tno_tire.LATERAL_COEFFICIENTS.RBY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[202] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBY2 = tno_tire.LATERAL_COEFFICIENTS.RBY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[203] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBY3 = tno_tire.LATERAL_COEFFICIENTS.RBY3.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBY4 = tno_tire.LATERAL_COEFFICIENTS.RBY4.second;
    Chassis_P.MagicTireConstInput_vdynMF[205] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RBY4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RCY1 = tno_tire.LATERAL_COEFFICIENTS.RCY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[206] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RCY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REY1 = tno_tire.LATERAL_COEFFICIENTS.REY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[207] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REY2 = tno_tire.LATERAL_COEFFICIENTS.REY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[208] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_REY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RHY1 = tno_tire.LATERAL_COEFFICIENTS.RHY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[209] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RHY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RHY2 = tno_tire.LATERAL_COEFFICIENTS.RHY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[210] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RHY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY1 = tno_tire.LATERAL_COEFFICIENTS.RVY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[211] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY2 = tno_tire.LATERAL_COEFFICIENTS.RVY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[212] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY3 = tno_tire.LATERAL_COEFFICIENTS.RVY3.second;
    Chassis_P.MagicTireConstInput_vdynMF[213] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY4 = tno_tire.LATERAL_COEFFICIENTS.RVY4.second;
    Chassis_P.MagicTireConstInput_vdynMF[214] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY5 = tno_tire.LATERAL_COEFFICIENTS.RVY5.second;
    Chassis_P.MagicTireConstInput_vdynMF[215] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY6 = tno_tire.LATERAL_COEFFICIENTS.RVY6.second;
    Chassis_P.MagicTireConstInput_vdynMF[216] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_RVY6;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX1 = tno_tire.OVERTURNING_COEFFICIENTS.QSX1.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX2 = tno_tire.OVERTURNING_COEFFICIENTS.QSX2.second;
    Chassis_P.MagicTireConstInput_vdynMF[161] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX3 = tno_tire.OVERTURNING_COEFFICIENTS.QSX3.second;
    Chassis_P.MagicTireConstInput_vdynMF[162] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX4 = tno_tire.OVERTURNING_COEFFICIENTS.QSX4.second;
    Chassis_P.MagicTireConstInput_vdynMF[163] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX5 = tno_tire.OVERTURNING_COEFFICIENTS.QSX5.second;
    Chassis_P.MagicTireConstInput_vdynMF[164] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX6 = tno_tire.OVERTURNING_COEFFICIENTS.QSX6.second;
    Chassis_P.MagicTireConstInput_vdynMF[165] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX6;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX7 = tno_tire.OVERTURNING_COEFFICIENTS.QSX7.second;
    Chassis_P.MagicTireConstInput_vdynMF[166] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX7;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX8 = tno_tire.OVERTURNING_COEFFICIENTS.QSX8.second;
    Chassis_P.MagicTireConstInput_vdynMF[167] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX8;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX9 = tno_tire.OVERTURNING_COEFFICIENTS.QSX9.second;
    Chassis_P.MagicTireConstInput_vdynMF[168] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX9;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX10 = tno_tire.OVERTURNING_COEFFICIENTS.QSX10.second;
    Chassis_P.MagicTireConstInput_vdynMF[169] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX10;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX11 = tno_tire.OVERTURNING_COEFFICIENTS.QSX11.second;
    Chassis_P.MagicTireConstInput_vdynMF[170] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX11;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX12 = tno_tire.OVERTURNING_COEFFICIENTS.QSX12.second;
    Chassis_P.MagicTireConstInput_vdynMF[171] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX12;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX13 = tno_tire.OVERTURNING_COEFFICIENTS.QSX13.second;
    Chassis_P.MagicTireConstInput_vdynMF[172] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX13;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX14 = tno_tire.OVERTURNING_COEFFICIENTS.QSX14.second;
    Chassis_P.MagicTireConstInput_vdynMF[173] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSX14;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPMX1 = tno_tire.OVERTURNING_COEFFICIENTS.PPMX1.second;
    Chassis_P.MagicTireConstInput_vdynMF[174] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPMX1;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY1 = tno_tire.ROLLING_COEFFICIENTS.QSY1.second;
    Chassis_P.MagicTireConstInput_vdynMF[217] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY2 = tno_tire.ROLLING_COEFFICIENTS.QSY2.second;
    Chassis_P.MagicTireConstInput_vdynMF[218] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY3 = tno_tire.ROLLING_COEFFICIENTS.QSY3.second;
    Chassis_P.MagicTireConstInput_vdynMF[219] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY4 = tno_tire.ROLLING_COEFFICIENTS.QSY4.second;
    Chassis_P.MagicTireConstInput_vdynMF[220] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY5 = tno_tire.ROLLING_COEFFICIENTS.QSY5.second;
    Chassis_P.MagicTireConstInput_vdynMF[221] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY6 = tno_tire.ROLLING_COEFFICIENTS.QSY6.second;
    Chassis_P.MagicTireConstInput_vdynMF[222] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY6;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY7 = tno_tire.ROLLING_COEFFICIENTS.QSY7.second;
    Chassis_P.MagicTireConstInput_vdynMF[223] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY7;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY8 = tno_tire.ROLLING_COEFFICIENTS.QSY8.second;
    Chassis_P.MagicTireConstInput_vdynMF[224] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QSY8;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ1 = tno_tire.ALIGNING_COEFFICIENTS.QBZ1.second;
    Chassis_P.MagicTireConstInput_vdynMF[225] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ2 = tno_tire.ALIGNING_COEFFICIENTS.QBZ2.second;
    Chassis_P.MagicTireConstInput_vdynMF[226] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ3 = tno_tire.ALIGNING_COEFFICIENTS.QBZ3.second;
    Chassis_P.MagicTireConstInput_vdynMF[227] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ4 = tno_tire.ALIGNING_COEFFICIENTS.QBZ4.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ5 = tno_tire.ALIGNING_COEFFICIENTS.QBZ5.second;
    Chassis_P.MagicTireConstInput_vdynMF[229] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ9 = tno_tire.ALIGNING_COEFFICIENTS.QBZ9.second;
    Chassis_P.MagicTireConstInput_vdynMF[230] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ9;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ10 = tno_tire.ALIGNING_COEFFICIENTS.QBZ10.second;
    Chassis_P.MagicTireConstInput_vdynMF[231] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBZ10;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QCZ1 = tno_tire.ALIGNING_COEFFICIENTS.QCZ1.second;
    Chassis_P.MagicTireConstInput_vdynMF[232] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QCZ1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ1 = tno_tire.ALIGNING_COEFFICIENTS.QDZ1.second;
    Chassis_P.MagicTireConstInput_vdynMF[233] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ2 = tno_tire.ALIGNING_COEFFICIENTS.QDZ2.second;
    Chassis_P.MagicTireConstInput_vdynMF[234] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ3 = tno_tire.ALIGNING_COEFFICIENTS.QDZ3.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ4 = tno_tire.ALIGNING_COEFFICIENTS.QDZ4.second;
    Chassis_P.MagicTireConstInput_vdynMF[237] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ6 = tno_tire.ALIGNING_COEFFICIENTS.QDZ6.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ7 = tno_tire.ALIGNING_COEFFICIENTS.QDZ7.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ8 = tno_tire.ALIGNING_COEFFICIENTS.QDZ8.second;
    Chassis_P.MagicTireConstInput_vdynMF[240] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ8;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ9 = tno_tire.ALIGNING_COEFFICIENTS.QDZ9.second;
    Chassis_P.MagicTireConstInput_vdynMF[241] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ9;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ10 = tno_tire.ALIGNING_COEFFICIENTS.QDZ10.second;
    Chassis_P.MagicTireConstInput_vdynMF[242] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ10;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ11 = tno_tire.ALIGNING_COEFFICIENTS.QDZ11.second;
    Chassis_P.MagicTireConstInput_vdynMF[243] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDZ11;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ1 = tno_tire.ALIGNING_COEFFICIENTS.QEZ1.second;
    Chassis_P.MagicTireConstInput_vdynMF[244] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ2 = tno_tire.ALIGNING_COEFFICIENTS.QEZ2.second;
    Chassis_P.MagicTireConstInput_vdynMF[245] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ3 = tno_tire.ALIGNING_COEFFICIENTS.QEZ3.second;
    Chassis_P.MagicTireConstInput_vdynMF[246] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ4 = tno_tire.ALIGNING_COEFFICIENTS.QEZ4.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ5 = tno_tire.ALIGNING_COEFFICIENTS.QEZ5.second;
    Chassis_P.MagicTireConstInput_vdynMF[248] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QEZ5;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QHZ1 = tno_tire.ALIGNING_COEFFICIENTS.QHZ1.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QHZ2 = tno_tire.ALIGNING_COEFFICIENTS.QHZ2.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QHZ3 = tno_tire.ALIGNING_COEFFICIENTS.QHZ3.second;
    Chassis_P.MagicTireConstInput_vdynMF[251] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QHZ3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QHZ4 = tno_tire.ALIGNING_COEFFICIENTS.QHZ4.second;
    Chassis_P.MagicTireConstInput_vdynMF[252] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QHZ4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPZ1 = tno_tire.ALIGNING_COEFFICIENTS.PPZ1.second;
    Chassis_P.MagicTireConstInput_vdynMF[253] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPZ1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPZ2 = tno_tire.ALIGNING_COEFFICIENTS.PPZ2.second;
    Chassis_P.MagicTireConstInput_vdynMF[254] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PPZ2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_SSZ1 = tno_tire.ALIGNING_COEFFICIENTS.SSZ1.second;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_SSZ2 = tno_tire.ALIGNING_COEFFICIENTS.SSZ2.second;
    Chassis_P.MagicTireConstInput_vdynMF[256] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_SSZ2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_SSZ3 = tno_tire.ALIGNING_COEFFICIENTS.SSZ3.second;
    Chassis_P.MagicTireConstInput_vdynMF[257] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_SSZ3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_SSZ4 = tno_tire.ALIGNING_COEFFICIENTS.SSZ4.second;
    Chassis_P.MagicTireConstInput_vdynMF[258] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_SSZ4;

    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDXP1 = tno_tire.TURNSLIP_COEFFICIENTS.PDXP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[259] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDXP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDXP2 = tno_tire.TURNSLIP_COEFFICIENTS.PDXP2.second;
    Chassis_P.MagicTireConstInput_vdynMF[260] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDXP2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDXP3 = tno_tire.TURNSLIP_COEFFICIENTS.PDXP3.second;
    Chassis_P.MagicTireConstInput_vdynMF[261] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDXP3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKYP1 = tno_tire.TURNSLIP_COEFFICIENTS.PKYP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[262] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PKYP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP1 = tno_tire.TURNSLIP_COEFFICIENTS.PDYP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[263] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP2 = tno_tire.TURNSLIP_COEFFICIENTS.PDYP2.second;
    Chassis_P.MagicTireConstInput_vdynMF[264] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP3 = tno_tire.TURNSLIP_COEFFICIENTS.PDYP3.second;
    Chassis_P.MagicTireConstInput_vdynMF[265] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP4 = tno_tire.TURNSLIP_COEFFICIENTS.PDYP4.second;
    Chassis_P.MagicTireConstInput_vdynMF[266] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PDYP4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP1 = tno_tire.TURNSLIP_COEFFICIENTS.PHYP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[267] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP2 = tno_tire.TURNSLIP_COEFFICIENTS.PHYP2.second;
    Chassis_P.MagicTireConstInput_vdynMF[268] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP3 = tno_tire.TURNSLIP_COEFFICIENTS.PHYP3.second;
    Chassis_P.MagicTireConstInput_vdynMF[269] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP3;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP4 = tno_tire.TURNSLIP_COEFFICIENTS.PHYP4.second;
    Chassis_P.MagicTireConstInput_vdynMF[270] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PHYP4;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PECP1 = tno_tire.TURNSLIP_COEFFICIENTS.PECP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[271] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PECP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PECP2 = tno_tire.TURNSLIP_COEFFICIENTS.PECP2.second;
    Chassis_P.MagicTireConstInput_vdynMF[272] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_PECP2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDTP1 = tno_tire.TURNSLIP_COEFFICIENTS.QDTP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[273] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDTP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QCRP1 = tno_tire.TURNSLIP_COEFFICIENTS.QCRP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[274] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QCRP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QCRP2 = tno_tire.TURNSLIP_COEFFICIENTS.QCRP2.second;
    Chassis_P.MagicTireConstInput_vdynMF[275] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QCRP2;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBRP1 = tno_tire.TURNSLIP_COEFFICIENTS.QBRP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[276] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QBRP1;
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDRP1 = tno_tire.TURNSLIP_COEFFICIENTS.QDRP1.second;
    Chassis_P.MagicTireConstInput_vdynMF[277] = ChassisExt::Chassis_P.CombinedSlipWheel2DOF_QDRP1;
  } else {
    LOG_ERROR << "fail  to parse .tir file, " << g_wheel.tire().tno_tir_path().str_var() << "\n";
  }

  // set init data of tire
  {
    // ChassisExt::Chassis_P.CombinedSlipWheel2DOF_omegao = ChassisExt::Chassis_P.VEH.InitialLongVel /
    // ChassisExt::Chassis_P.CombinedSlipWheel2DOF_UNLOADED_RADIUS / 0.95; // init rotation velocity
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_br = 0.001;   // rotation dampings
    ChassisExt::Chassis_P.CombinedSlipWheel2DOF_zdoto = 0.0;  // init vertical velocity
  }

  LOG_2 << "end of init wheel tire parameter.\n";

  return true;
}

bool ChassisExt::initParamSusp() {
  LOG_2 << "init susp parameter.\n";

  // lambda
  auto wheel2Map1dFiller = [](const tx_car::Real1DMap& map_axle, real_T l_bp[], real_T l_table[], real_T r_bp[],
                              real_T r_table[], uint32_t& max_index) {
    // map checker
    if (!tx_car::map1DFormatChecker(map_axle)) {
      std::abort();
    }

    max_index = map_axle.y0_axis().data_size() - 1;

    // fl
    tx_car::initAxis(l_bp, map_axle.u0_axis(), map_axle.u0_axis().data_size());
    tx_car::initAxis(l_table, map_axle.y0_axis(), map_axle.y0_axis().data_size());
    // fr
    tx_car::initAxis(r_bp, map_axle.u0_axis(), map_axle.u0_axis().data_size());
    tx_car::initAxis(r_table, map_axle.y0_axis(), map_axle.y0_axis().data_size());
  };

  auto wheel2SymmetricalFiller = [](const tx_car::Real1DMap& map_axle, real_T l_bp[], real_T l_table[], real_T r_bp[],
                                    real_T r_table[], uint32_t& max_index) {
    // map checker
    if (!tx_car::map1DFormatChecker(map_axle)) {
      std::abort();
    }

    max_index = map_axle.y0_axis().data_size() - 1;

    // l
    tx_car::initAxis(l_bp, map_axle.u0_axis(), map_axle.u0_axis().data_size());
    tx_car::initAxis(l_table, map_axle.y0_axis(), map_axle.y0_axis().data_size());
    // r
    tx_car::initAxis(r_bp, map_axle.u0_axis(), map_axle.u0_axis().data_size());
    tx_car::initAxis(r_table, map_axle.y0_axis(), map_axle.y0_axis().data_size());

    // flip y axis of right table
    for (auto i = 0u; i <= max_index; ++i) {
      r_table[i] = r_table[i] * g_susp_fliper;
    }
  };

  auto bounceRatioFiller = [](const real_T jounce_ratio, real_T bp_x[], real_T bp_y[], uint32_t size = 3) {
    bp_x[0] = -1.0;
    bp_x[1] = 0.0;
    bp_x[2] = 1.0;
    bp_y[0] = jounce_ratio;
    bp_y[1] = jounce_ratio;
    bp_y[2] = jounce_ratio;
  };

  // vehicle adapter
  {
    ChassisExt::Chassis_P.InertialFrameCGtoAxleOffset_Value[2] =
        ChassisExt::Chassis_P.VEH.HeightCG;  // g_sprung.heightcg().val();
    ChassisExt::Chassis_P.InertialFrameCGtoAxleOffset_Value[5] =
        ChassisExt::Chassis_P.VEH.HeightCG;  // g_sprung.heightcg().val();
    ChassisExt::Chassis_P.InertialFrameCGtoAxleOffset_Value[8] =
        ChassisExt::Chassis_P.VEH.HeightCG;  // g_sprung.heightcg().val();
    ChassisExt::Chassis_P.InertialFrameCGtoAxleOffset_Value[11] =
        ChassisExt::Chassis_P.VEH.HeightCG;  // g_sprung.heightcg().val();
  }

  // dynamic track witdth calculate
  {
    ChassisExt::Chassis_P.FrontTrack_Value = ChassisExt::Chassis_P.VEH.TrackWidth;  // g_sprung.trackwidth().val();
    ChassisExt::Chassis_P.RearTrack_Value = ChassisExt::Chassis_P.VEH.TrackWidth;   // g_sprung.trackwidth().val();
  }

  // shock
  {
    // flip and exchange
    flipAndExchangeAxisData(*g_susp.mutable_shock()->mutable_shockdampingforce_front()->mutable_y0_axis());
    flipAndExchangeAxisData(*g_susp.mutable_shock()->mutable_shockdampingforce_rear()->mutable_y0_axis());

    // front damping
    wheel2Map1dFiller(g_susp.shock().shockdampingforce_front(), ChassisExt::Chassis_P.CoreSubsys.XYFL_bp01Data_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_bp01Data_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_p, m_shckdamping_vs_comprate_maxIndex);

    // rear damping
    wheel2Map1dFiller(g_susp.shock().shockdampingforce_rear(), ChassisExt::Chassis_P.CoreSubsys.XYRL_bp01Data_p1,
                      ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_bp01Data_p1,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_p1, m_shckdamping_vs_comprate_maxIndex_rear);

    bounceRatioFiller(g_susp.shock().jounceratio_front().val(), ChassisExt::Chassis_P.CoreSubsys.XYFL1_bp01Data_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYFL1_tableData_p, 3);
    bounceRatioFiller(g_susp.shock().jounceratio_front().val(), ChassisExt::Chassis_P.CoreSubsys.XYFR1_bp01Data_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR1_tableData_p, 3);
    bounceRatioFiller(g_susp.shock().jounceratio_rear().val(), ChassisExt::Chassis_P.CoreSubsys.XYRL1_bp01Data_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYRL1_tableData_p, 3);
    bounceRatioFiller(g_susp.shock().jounceratio_rear().val(), ChassisExt::Chassis_P.CoreSubsys.XYRR1_bp01Data_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR1_tableData_p, 3);
  }

  // bounce
  {
    // flip and exchange
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_bumptoe_front()->mutable_y0_axis());
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_bumptoe_rear()->mutable_y0_axis());

    // toe vs compression
    wheel2SymmetricalFiller(g_susp.bounce().bumptoe_front(), ChassisExt::Chassis_P.CoreSubsys.XYFL_bp01Data_n,
                            ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_n,
                            ChassisExt::Chassis_P.CoreSubsys.XYFR_bp01Data_e,
                            ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_h, m_bumptoe_vs_compress_maxIndex);
    wheel2SymmetricalFiller(g_susp.bounce().bumptoe_rear(), ChassisExt::Chassis_P.CoreSubsys.XYRL_bp01Data_j,
                            ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_k,
                            ChassisExt::Chassis_P.CoreSubsys.XYRR_bp01Data_p,
                            ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_p, m_bumptoe_vs_compress_maxIndex_rear);

    // flip and exchange
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_bumpcamber_front()->mutable_y0_axis());
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_bumpcamber_rear()->mutable_y0_axis());

    // camber vs compression
    wheel2Map1dFiller(g_susp.bounce().bumpcamber_front(), ChassisExt::Chassis_P.CoreSubsys.XYFL_bp01Data_g,
                      ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_o,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_bp01Data_g,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_j, m_bumpcamber_vs_compress_maxIndex);
    wheel2Map1dFiller(g_susp.bounce().bumpcamber_rear(), ChassisExt::Chassis_P.CoreSubsys.XYRL_bp01Data_p,
                      ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_a,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_bp01Data_i,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_l, m_bumpcamber_vs_compress_maxIndex_rear);

    // flip and exchange
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_bumpcaster()->mutable_y0_axis());
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_bumpcaster_rear()->mutable_y0_axis());

    // caster vs compression
    wheel2Map1dFiller(g_susp.bounce().bumpcaster(), ChassisExt::Chassis_P.CoreSubsys.XYFL_bp01Data_l,
                      ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_n0,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_bp01Data_b,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_n, m_bumpcaster_vs_compress_maxIndex);
    wheel2Map1dFiller(g_susp.bounce().bumpcaster_rear(), ChassisExt::Chassis_P.CoreSubsys.XYRL_bp01Data_d,
                      ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_as,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_bp01Data_d,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_pz, m_bumpcaster_vs_compress_maxIndex_rear);
    // flip as carsim use "Dive"
    for (auto i = 0ul; i <= m_bumpcaster_vs_compress_maxIndex; ++i) {
      ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_n0[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_n0[i] * g_susp_fliper;
      ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_n[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_n[i] * g_susp_fliper;
    }
    for (auto i = 0ul; i <= m_bumpcaster_vs_compress_maxIndex_rear; ++i) {
      ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_as[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_as[i] * g_susp_fliper;
      ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_pz[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_pz[i] * g_susp_fliper;
    }

    // flip and exchange
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_latwhlctrdisplacement_front()->mutable_y0_axis());
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_latwhlctrdisplacement_rear()->mutable_y0_axis());

    // lateral displacement vs compression
    wheel2SymmetricalFiller(
        g_susp.bounce().latwhlctrdisplacement_front(), ChassisExt::Chassis_P.CoreSubsys.XYFL_bp01Data_g3,
        ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_g, ChassisExt::Chassis_P.CoreSubsys.XYFR_bp01Data_g3,
        ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_g, m_latdisp_vs_compress_maxIndex);
    wheel2SymmetricalFiller(
        g_susp.bounce().latwhlctrdisplacement_rear(), ChassisExt::Chassis_P.CoreSubsys.XYRL_bp01Data_g,
        ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_g, ChassisExt::Chassis_P.CoreSubsys.XYRR_bp01Data_g,
        ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_g, m_latdisp_vs_compress_maxIndex_rear);
    for (auto i = 0ul; i <= m_latdisp_vs_compress_maxIndex; ++i) {
      ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_g[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_g[i] * g_susp_fliper;
      ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_g[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_g[i] * g_susp_fliper;
    }
    for (auto i = 0ul; i <= m_latdisp_vs_compress_maxIndex_rear; ++i) {
      ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_g[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_g[i] * g_susp_fliper;
      ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_g[i] =
          ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_g[i] * g_susp_fliper;
    }

    // flip and exchange
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_lngwhlctrdisp_front()->mutable_y0_axis());
    flipAndExchangeAxisData(*g_susp.mutable_bounce()->mutable_lngwhlctrdisp_rear()->mutable_y0_axis());

    // longitudinal displacement vs compression
    wheel2Map1dFiller(g_susp.bounce().lngwhlctrdisp_front(), ChassisExt::Chassis_P.CoreSubsys.XYFL_bp01Data_o,
                      ChassisExt::Chassis_P.CoreSubsys.XYFL_tableData_o5,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_bp01Data_o,
                      ChassisExt::Chassis_P.CoreSubsys.XYFR_tableData_o, m_lngdisp_vs_compress_maxIndex);
    wheel2Map1dFiller(g_susp.bounce().lngwhlctrdisp_rear(), ChassisExt::Chassis_P.CoreSubsys.XYRL_bp01Data_o,
                      ChassisExt::Chassis_P.CoreSubsys.XYRL_tableData_o,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_bp01Data_o,
                      ChassisExt::Chassis_P.CoreSubsys.XYRR_tableData_o, m_lngdisp_vs_compress_maxIndex_rear);

    // normal wheel rates
    {
      ChassisExt::Chassis_P.NrmlWhlRates[0] = g_susp.bounce().nrmlwhlrates_front().val();
      ChassisExt::Chassis_P.NrmlWhlRates[1] = g_susp.bounce().nrmlwhlrates_front().val();
      ChassisExt::Chassis_P.NrmlWhlRates[2] = g_susp.bounce().nrmlwhlrates_rear().val();
      ChassisExt::Chassis_P.NrmlWhlRates[3] = g_susp.bounce().nrmlwhlrates_rear().val();
    }

    // normal wheel rates offset
    {
      real_T sprung_mass = ChassisExt::Chassis_P.VEH.SprungMass;     // g_sprung.sprungmass().val();
      real_T mass = ChassisExt::Chassis_P.VEH.Mass;                  // g_sprung.sprungmass().val();
      real_T wheel_base = ChassisExt::Chassis_P.VEH.WheelBase;       // g_sprung.wheelbase().val();
      real_T a = ChassisExt::Chassis_P.VEH.FrontAxlePositionfromCG;  // g_sprung.frontaxlepositionfromcg().val();
      real_T b = ChassisExt::Chassis_P.VEH.RearAxlePositionfromCG;   // g_sprung.rearaxlepositionfromcg().val();
      const real_T g = 9.8;

      ChassisExt::Chassis_P.NrmlWhlFrcOff[0] = g * b / wheel_base * sprung_mass / 2;
      ChassisExt::Chassis_P.NrmlWhlFrcOff[1] = ChassisExt::Chassis_P.NrmlWhlFrcOff[0];
      ChassisExt::Chassis_P.NrmlWhlFrcOff[2] = g * a / wheel_base * sprung_mass / 2;
      ChassisExt::Chassis_P.NrmlWhlFrcOff[3] = ChassisExt::Chassis_P.NrmlWhlFrcOff[2];
    }
  }

  // roll
  {
    ChassisExt::Chassis_P.RollStiffArb[0] = g_susp.roll().rollstiff_front().val();
    ChassisExt::Chassis_P.RollStiffArb[1] = g_susp.roll().rollstiff_rear().val();

    ChassisExt::Chassis_P.RollStiffNoArb[0] = 0.0;
    ChassisExt::Chassis_P.RollStiffNoArb[1] = 0.0;
  }

  // steer
  {
    ChassisExt::Chassis_P.CambVsSteerAng[0] = 0;  // 0.1;
    ChassisExt::Chassis_P.CambVsSteerAng[1] = 0;  // 0.1;
    ChassisExt::Chassis_P.CambVsSteerAng[2] = 0;  // 0.1;
    ChassisExt::Chassis_P.CambVsSteerAng[3] = 0;  // 0.1;
  }

  // compliance longitudinal
  {
    auto lngComplianceFiller = [](real_T front_val, real_T rear_val, real_T arr1[], real_T arr2[], uint32_t size = 4) {
      arr1[0] = front_val;
      arr1[1] = front_val;
      arr1[2] = rear_val;
      arr1[3] = rear_val;

      for (auto i = 0; i < 4; ++i) {
        arr2[i] = arr1[i];
      }
    };

    auto lngComplianceSymmetricalFiller = [](real_T front_val, real_T rear_val, real_T arr1[], real_T arr2[],
                                             uint32_t size = 4) {
      arr1[0] = front_val;
      arr1[1] = front_val * g_susp_fliper;
      arr1[2] = rear_val;
      arr1[3] = rear_val * g_susp_fliper;

      for (auto i = 0; i < 4; ++i) {
        arr2[i] = arr1[i];
      }
    };

    // toe deflection by Fx
    lngComplianceSymmetricalFiller(
        g_susp.longitudinal().toecomplianceperfx_front().val(), g_susp.longitudinal().toecomplianceperfx_rear().val(),
        ChassisExt::Chassis_P.CoreSubsys.Constant3_Value, ChassisExt::Chassis_P.CoreSubsys.Constant6_Value);

    // camber deflection by Fx
    lngComplianceFiller(g_susp.longitudinal().cambercomplianceperfx_front().val(),
                        g_susp.longitudinal().cambercomplianceperfx_rear().val(),
                        ChassisExt::Chassis_P.CoreSubsys.Constant1_Value_j,
                        ChassisExt::Chassis_P.CoreSubsys.Constant6_Value_j);

    // caster deflection by Fx
    lngComplianceFiller(
        g_susp.longitudinal().castercompliancefx_front().val(), g_susp.longitudinal().castercompliancefx_rear().val(),
        ChassisExt::Chassis_P.CoreSubsys.Constant2_Value, ChassisExt::Chassis_P.CoreSubsys.Constant4_Value);

    // wheel center longitidunal displacement deflection by Fx
    lngComplianceFiller(g_susp.longitudinal().whldispcomplianceperfx_front().val(),
                        g_susp.longitudinal().whldispcomplianceperfx_rear().val(),
                        ChassisExt::Chassis_P.CoreSubsys.Constant1_Value_m,
                        ChassisExt::Chassis_P.CoreSubsys.Constant2_Value_m);
  }

  // compliance lateral
  {
    auto latComplianceFiller = [](real_T front_val, real_T rear_val, real_T arr1[]) {
      arr1[0] = front_val;
      arr1[1] = front_val;
      arr1[2] = rear_val;
      arr1[3] = rear_val;
    };
    auto latComplianceSymmetricalFiller = [](real_T front_val, real_T rear_val, real_T arr1[]) {
      arr1[0] = front_val;
      arr1[1] = front_val * g_susp_fliper;
      arr1[2] = rear_val;
      arr1[3] = rear_val * g_susp_fliper;
    };

    // toe, confirmed
    latComplianceFiller(g_susp.lateral().toecomplianceperfy_front().val(),
                        g_susp.lateral().toecomplianceperfy_rear().val(), ChassisExt::Chassis_P.LatSteerCompl);

    // camber, confirmed
    latComplianceSymmetricalFiller(g_susp.lateral().cambercomplianceperfy_front().val(),
                                   g_susp.lateral().cambercomplianceperfy_rear().val(),
                                   ChassisExt::Chassis_P.LatCambCompl);
    for (auto i = 0; i < 4; ++i) {
      ChassisExt::Chassis_P.LatCambCompl[i] = ChassisExt::Chassis_P.LatCambCompl[i] * g_susp_fliper;
    }

    // wheel center lateral displacement
    latComplianceFiller(g_susp.lateral().whldispcomplianceperfy_front().val(),
                        g_susp.lateral().whldispcomplianceperfy_rear().val(), ChassisExt::Chassis_P.LatWhlCtrComplLat);
    for (auto i = 0; i < 4; ++i) {
      ChassisExt::Chassis_P.LatWhlCtrComplLat[i] = ChassisExt::Chassis_P.LatWhlCtrComplLat[i] * g_susp_fliper;
    }
  }

  // align
  {
    auto alignComplianceFiller = [](real_T front_val, real_T rear_val, real_T arr1[]) {
      arr1[0] = front_val;
      arr1[1] = front_val;
      arr1[2] = rear_val;
      arr1[3] = rear_val;
    };
    auto alignComplianceSymmetricalFiller = [](real_T front_val, real_T rear_val, real_T arr1[]) {
      arr1[0] = front_val;
      arr1[1] = front_val * g_susp_fliper;
      arr1[2] = rear_val;
      arr1[3] = rear_val * g_susp_fliper;
    };
    // toe aligment
    alignComplianceFiller(g_susp.align().aligntrqsteer_front().val(), g_susp.align().aligntrqsteer_rear().val(),
                          ChassisExt::Chassis_P.AlgnTrqSteerCompl);

    // camber aligment
    alignComplianceSymmetricalFiller(g_susp.align().aligntrqcamber_front().val(),
                                     g_susp.align().aligntrqcamber_rear().val(),
                                     ChassisExt::Chassis_P.AlgnTrqCambCompl);
  }

  // static
  {
    auto staticFiller = [](real_T front_val, real_T rear_val, real_T arr1[]) {
      arr1[0] = front_val;
      arr1[1] = front_val;
      arr1[2] = rear_val;
      arr1[3] = rear_val;
    };
    // toe aligment
    staticFiller(g_susp.static_().statictoe_front().val(), g_susp.static_().statictoe_rear().val(),
                 ChassisExt::Chassis_P.IndependentKandCSuspension_StatToe);
    // camber aligment
    staticFiller(g_susp.static_().staticcamber_front().val(), g_susp.static_().staticcamber_rear().val(),
                 ChassisExt::Chassis_P.IndependentKandCSuspension_StatCamber);
  }

  LOG_2 << "end of init susp parameter.\n";

  return true;
}

/* load parameter from json file and init model */
bool ChassisExt::initMDL() {
  m_err_info = {tx_car::TxCarStateCode::no_error, "no error"};

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
}  // namespace chassis
}  // namespace tx_car

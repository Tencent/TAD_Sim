/*
 * softecu.cpp
 *
 * Code generation for model "softecu".
 *
 * Model version              : 1.169
 * Simulink Coder version : 9.1 (R2019a) 23-Nov-2018
 * C++ source code generated on : Thu May 21 14:36:09 2020
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Linux 64)
 * Code generation objective: Execution efficiency
 * Validation result: Not run
 */

#include "softecu.h"
#include "softecu_private.h"

/* Model step function */
void softecuModelClass::step() {
  int32_T rtb_gearLeverOut;
  real_T rtb_Memory;
  real_T rtb_Memory1;
  real_T rtb_Memory2;
  real_T rtb_Saturation;
  real_T rtb_Abs;
  real_T rtb_Product;
  boolean_T rtb_LogicalOperator3;
  boolean_T rtb_LogicalOperator1;
  real_T rtb_Product_k;
  boolean_T rtb_LogicalOperator_p;
  real_T rtb_steer_system_transfer_ratio;
  int32_T rtb_gearLeverOut_0;

  /* Outputs for Atomic SubSystem: '<Root>/SoftACC' */
  /* Memory: '<S1>/Memory' */
  rtb_Memory = softecu_DW.Memory_PreviousInput;

  /* Memory: '<S1>/Memory1' */
  rtb_Memory1 = softecu_DW.Memory1_PreviousInput;

  /* Memory: '<S1>/Memory2' */
  rtb_Memory2 = softecu_DW.Memory2_PreviousInput;

  /* Abs: '<S4>/Abs' incorporates:
   *  Abs: '<S2>/Abs'
   *  Inport: '<Root>/vx[m|s]'
   */
  rtb_Product_k = std::abs(softecu_U.vxms);

  /* Product: '<S4>/Product' incorporates:
   *  Abs: '<S4>/Abs'
   *  Constant: '<S4>/Constant'
   *  Constant: '<S4>/Constant1'
   *  Constant: '<S4>/ax_max'
   *  Product: '<S4>/Divide'
   *  Sum: '<S4>/Add'
   */
  rtb_Product = (1.2 - rtb_Product_k / 1.4) * 0.8;

  /* Chart: '<S4>/Chart' incorporates:
   *  Inport: '<Root>/target_acc[m//s2]'
   */
  if (softecu_DW.is_active_c3_softecu_lib == 0U) {
    softecu_DW.is_active_c3_softecu_lib = 1U;

    /* :  tar_acc_out = tar_acc_in; */
    /* :  if (tar_acc_in < tar_acc_ori*0.5) */
    if (rtb_Product < softecu_U.target_accms2 * 0.5) {
      /* :  tar_acc_out =  tar_acc_ori*0.5; */
      rtb_Product = softecu_U.target_accms2 * 0.5;
    }
  } else {
    /* :  tar_acc_out = tar_acc_in; */
    /* :  if (tar_acc_in < tar_acc_ori*0.5) */
    if (rtb_Product < softecu_U.target_accms2 * 0.5) {
      /* :  tar_acc_out =  tar_acc_ori*0.5; */
      rtb_Product = softecu_U.target_accms2 * 0.5;
    }
  }

  /* End of Chart: '<S4>/Chart' */

  /* Chart: '<S5>/dummy_gear' incorporates:
   *  Inport: '<Root>/gear_lever_from_controller[]'
   */
  /* :  sf_internal_predicateOutput = gearLeverIn == 0; */
  if (softecu_U.gear_lever_from_controller == 0) {
    /* :  gearLeverOut=0; */
    rtb_gearLeverOut = 0;

    /* Switch: '<S4>/Switch1' incorporates:
     *  Inport: '<Root>/target_acc[m//s2]'
     */
    rtb_Abs = softecu_U.target_accms2;
  } else {
    /* :  sf_internal_predicateOutput = gearLeverIn == 1; */
    if (softecu_U.gear_lever_from_controller == 1) {
      /* :  gearLeverOut=0; */
      rtb_gearLeverOut = 0;

      /* Switch: '<S4>/Switch1' incorporates:
       *  Inport: '<Root>/target_acc[m//s2]'
       */
      rtb_Abs = softecu_U.target_accms2;
    } else {
      /* :  sf_internal_predicateOutput = gearLeverIn == 2; */
      if (softecu_U.gear_lever_from_controller == 2) {
        /* :  gearLeverOut=-1; */
        rtb_gearLeverOut = -1;

        /* Switch: '<S4>/Switch1' incorporates:
         *  Inport: '<Root>/target_acc[m//s2]'
         */
        rtb_Abs = softecu_U.target_accms2;
      } else {
        /* :  sf_internal_predicateOutput = gearLeverIn == 3; */
        if (softecu_U.gear_lever_from_controller == 3) {
          /* :  gearLeverOut=0; */
          rtb_gearLeverOut = 0;

          /* Switch: '<S4>/Switch1' incorporates:
           *  Inport: '<Root>/target_acc[m//s2]'
           */
          rtb_Abs = softecu_U.target_accms2;
        } else {
          /* :  sf_internal_predicateOutput = gearLeverIn == 4; */
          if (softecu_U.gear_lever_from_controller == 4) {
            /* :  gearLeverOut=6; */
            rtb_gearLeverOut = 6;

            /* Switch: '<S4>/Switch' incorporates:
             *  Abs: '<S4>/Abs'
             *  Constant: '<S20>/Constant'
             *  Constant: '<S22>/Constant'
             *  Inport: '<Root>/target_acc[m//s2]'
             *  Logic: '<S4>/Logical Operator'
             *  RelationalOperator: '<S20>/Compare'
             *  RelationalOperator: '<S22>/Compare'
             *  Switch: '<S4>/Switch1'
             */
            if ((rtb_Product_k <= 1.4) && (softecu_U.target_accms2 >= 0.0)) {
              rtb_Abs = rtb_Product;
            } else {
              rtb_Abs = softecu_U.target_accms2;
            }

            /* End of Switch: '<S4>/Switch' */
          } else {
            /* :  gearLeverOut=0; */
            rtb_gearLeverOut = 0;

            /* Switch: '<S4>/Switch1' incorporates:
             *  Inport: '<Root>/target_acc[m//s2]'
             */
            rtb_Abs = softecu_U.target_accms2;
          }
        }
      }
    }
  }

  /* End of Chart: '<S5>/dummy_gear' */

  /* RelationalOperator: '<S2>/Relational Operator' incorporates:
   *  Constant: '<S2>/120km|h'
   *  Constant: '<S2>/Constant'
   *  Product: '<S2>/Divide'
   *  Product: '<S2>/Product1'
   */
  rtb_LogicalOperator3 = (rtb_Abs >= rtb_Product_k / 33.3 * -0.3);

  /* Logic: '<S2>/Logical Operator1' incorporates:
   *  Inport: '<Root>/reset'
   *  Logic: '<S2>/Logical Operator'
   */
  rtb_LogicalOperator1 = ((!rtb_LogicalOperator3) || softecu_U.reset);

  /* Switch: '<S6>/Switch' incorporates:
   *  Constant: '<S24>/Constant'
   *  Constant: '<S6>/Constant'
   *  Constant: '<S6>/Constant1'
   *  RelationalOperator: '<S24>/Compare'
   */
  if (rtb_gearLeverOut == -1) {
    rtb_gearLeverOut_0 = -1;
  } else {
    rtb_gearLeverOut_0 = 1;
  }

  /* End of Switch: '<S6>/Switch' */

  /* Product: '<S6>/Product' incorporates:
   *  Inport: '<Root>/cur_acc[m//s2]'
   */
  rtb_Product_k = static_cast<real_T>(rtb_gearLeverOut_0) * softecu_U.cur_accms2;

  /* Outputs for Atomic SubSystem: '<S3>/brakepedal' */
  /* Outputs for Atomic SubSystem: '<S2>/accpedal' */
  /* Sum: '<S8>/Add' incorporates:
   *  Sum: '<S15>/Add'
   */
  rtb_steer_system_transfer_ratio = rtb_Abs - rtb_Product_k;

  /* End of Outputs for SubSystem: '<S3>/brakepedal' */

  /* Product: '<S8>/Divide' incorporates:
   *  Constant: '<S8>/Constant'
   *  Sum: '<S8>/Add'
   */
  rtb_Product = rtb_steer_system_transfer_ratio / 4.0;

  /* Outputs for Atomic SubSystem: '<S8>/PID' */
  softecu_PID(rtb_Product, rtb_LogicalOperator1, &softecu_B.PID_f, &softecu_DW.PID_f, 1.8, 0.027, 0.6, -0.15, 0.35);

  /* End of Outputs for SubSystem: '<S8>/PID' */

  /* Sum: '<S10>/Add1' incorporates:
   *  Gain: '<S10>/Gain'
   *  Gain: '<S8>/Gain1'
   *  Sum: '<S10>/Add'
   */
  rtb_Saturation = (100.0 * softecu_B.PID_f.pid_out - rtb_Memory) * 0.066666666666666666 + rtb_Memory;

  /* Saturate: '<S8>/Saturation' */
  if (rtb_Saturation > 100.0) {
    rtb_Saturation = 100.0;
  } else {
    if (rtb_Saturation < 0.0) {
      rtb_Saturation = 0.0;
    }
  }

  /* End of Saturate: '<S8>/Saturation' */
  /* End of Outputs for SubSystem: '<S2>/accpedal' */

  /* Product: '<S2>/Product' */
  rtb_Memory = static_cast<real_T>(rtb_LogicalOperator3) * rtb_Saturation;

  /* Logic: '<S3>/Logical Operator3' incorporates:
   *  Constant: '<S12>/Constant'
   *  Constant: '<S13>/Constant'
   *  Constant: '<S14>/Constant'
   *  Logic: '<S3>/Logical Operator2'
   *  RelationalOperator: '<S12>/Compare'
   *  RelationalOperator: '<S13>/Compare'
   *  RelationalOperator: '<S14>/Compare'
   */
  rtb_LogicalOperator3 = (((rtb_Abs == 0.0) && (rtb_Product_k >= 0.05)) || (rtb_Abs < 0.0));

  /* Logic: '<S3>/Logical Operator' incorporates:
   *  Inport: '<Root>/reset'
   *  Logic: '<S3>/Logical Operator1'
   */
  rtb_LogicalOperator_p = ((!rtb_LogicalOperator3) || softecu_U.reset);

  /* Outputs for Atomic SubSystem: '<S3>/brakepedal' */
  /* Product: '<S15>/Divide' incorporates:
   *  Constant: '<S15>/Constant'
   */
  rtb_Product_k = rtb_steer_system_transfer_ratio / -4.0;

  /* Outputs for Atomic SubSystem: '<S15>/PID' */
  softecu_PID(rtb_Product_k, rtb_LogicalOperator_p, &softecu_B.PID_d, &softecu_DW.PID_d, 0.32, 0.015, 0.6, -0.15, 0.23);

  /* End of Outputs for SubSystem: '<S15>/PID' */

  /* Sum: '<S17>/Add1' incorporates:
   *  Gain: '<S15>/Gain'
   *  Gain: '<S17>/Gain'
   *  Sum: '<S17>/Add'
   */
  rtb_Saturation = (100.0 * softecu_B.PID_d.pid_out - rtb_Memory1) * 0.01 + rtb_Memory1;

  /* Saturate: '<S15>/Saturation' */
  if (rtb_Saturation > 100.0) {
    rtb_Saturation = 100.0;
  } else {
    if (rtb_Saturation < 0.0) {
      rtb_Saturation = 0.0;
    }
  }

  /* End of Saturate: '<S15>/Saturation' */
  /* End of Outputs for SubSystem: '<S3>/brakepedal' */

  /* Product: '<S3>/Product' */
  rtb_Memory1 = static_cast<real_T>(rtb_LogicalOperator3) * rtb_Saturation;

  /* Gain: '<S27>/Gain' incorporates:
   *  Inport: '<Root>/target_front_wheel_angle[rad]'
   */
  rtb_Saturation = 57.295780490442965 * softecu_U.target_front_wheel_anglerad;

  /* Gain: '<S26>/steer_system_transfer_ratio' incorporates:
   *  Gain: '<S28>/Gain'
   *  Inport: '<Root>/cur_front_wheel_angle[rad]'
   *  Sum: '<S26>/Add'
   */
  rtb_steer_system_transfer_ratio = (rtb_Saturation - 57.295780490442965 * softecu_U.cur_front_wheel_anglerad) * 16.0;

  /* Outputs for Atomic SubSystem: '<S26>/PID' */
  /* Inport: '<Root>/reset' */
  softecu_PID(rtb_steer_system_transfer_ratio, softecu_U.reset, &softecu_B.PID, &softecu_DW.PID, 0.0, 0.001, 100.0,
              -100.0, 0.0);

  /* End of Outputs for SubSystem: '<S26>/PID' */

  /* Sum: '<S26>/Add1' incorporates:
   *  Gain: '<S26>/Gain'
   */
  rtb_Saturation = 16.0 * rtb_Saturation + softecu_B.PID.pid_out;

  /* Saturate: '<S26>/Saturation' */
  if (rtb_Saturation > 900.0) {
    rtb_Saturation = 900.0;
  } else {
    if (rtb_Saturation < -900.0) {
      rtb_Saturation = -900.0;
    }
  }

  /* End of Saturate: '<S26>/Saturation' */

  /* Sum: '<S25>/Add' */
  rtb_Saturation -= rtb_Memory2;

  /* Saturate: '<S25>/Saturation' */
  if (rtb_Saturation > 50.0) {
    rtb_Saturation = 50.0;
  } else {
    if (rtb_Saturation < -50.0) {
      rtb_Saturation = -50.0;
    }
  }

  /* End of Saturate: '<S25>/Saturation' */

  /* Sum: '<S25>/Add1' */
  rtb_Memory2 += rtb_Saturation;

  /* Update for Memory: '<S1>/Memory' */
  softecu_DW.Memory_PreviousInput = rtb_Memory;

  /* Update for Memory: '<S1>/Memory1' */
  softecu_DW.Memory1_PreviousInput = rtb_Memory1;

  /* Update for Memory: '<S1>/Memory2' */
  softecu_DW.Memory2_PreviousInput = rtb_Memory2;

  /* End of Outputs for SubSystem: '<Root>/SoftACC' */

  /* Outport: '<Root>/gear_lever_out[-1:reverse,0:netural,>0:drive]' */
  softecu_Y.gear_lever_out1reverse0netural0drive = rtb_gearLeverOut;

  /* Outport: '<Root>/accpedal[%]' */
  softecu_Y.accpedal = rtb_Memory;

  /* Outport: '<Root>/brakepedal[%]' */
  softecu_Y.brakepedal = rtb_Memory1;

  /* Outport: '<Root>/steer[deg]' */
  softecu_Y.steerdeg = rtb_Memory2;

  /* Outport: '<Root>/softecu_signals' incorporates:
   *  DataTypeConversion: '<S2>/Data Type Conversion'
   *  DataTypeConversion: '<S3>/Data Type Conversion'
   *  DataTypeConversion: '<S7>/Data Type Conversion'
   *  Inport: '<Root>/reset'
   */
  softecu_Y.softecu_signals[0] = rtb_Memory;
  softecu_Y.softecu_signals[1] = rtb_Product;
  softecu_Y.softecu_signals[2] = rtb_Memory1;
  softecu_Y.softecu_signals[3] = rtb_Product_k;
  softecu_Y.softecu_signals[4] = rtb_Memory2;
  softecu_Y.softecu_signals[5] = rtb_steer_system_transfer_ratio;
  softecu_Y.softecu_signals[6] = rtb_Abs;

  /* Outputs for Atomic SubSystem: '<Root>/SoftACC' */
  softecu_Y.softecu_signals[7] = rtb_LogicalOperator1;
  softecu_Y.softecu_signals[8] = rtb_LogicalOperator_p;
  softecu_Y.softecu_signals[9] = softecu_U.reset;

  /* End of Outputs for SubSystem: '<Root>/SoftACC' */
}

/* Model initialize function */
void softecuModelClass::initialize() {
  /* Registration code */

  /* initialize error status */
  rtmSetErrorStatus((&softecu_M), (NULL));

  /* block I/O */
  (void)memset(((void *)&softecu_B), 0, sizeof(B_softecu_T));

  /* states (dwork) */
  (void)memset((void *)&softecu_DW, 0, sizeof(DW_softecu_T));

  /* external inputs */
  (void)memset(&softecu_U, 0, sizeof(ExtU_softecu_T));

  /* external outputs */
  (void)memset((void *)&softecu_Y, 0, sizeof(ExtY_softecu_T));

  /* SystemInitialize for Atomic SubSystem: '<Root>/SoftACC' */
  /* InitializeConditions for Memory: '<S1>/Memory' */
  softecu_DW.Memory_PreviousInput = 0.0;

  /* InitializeConditions for Memory: '<S1>/Memory1' */
  softecu_DW.Memory1_PreviousInput = 0.0;

  /* InitializeConditions for Memory: '<S1>/Memory2' */
  softecu_DW.Memory2_PreviousInput = 0.0;

  /* SystemInitialize for Chart: '<S4>/Chart' */
  softecu_DW.is_active_c3_softecu_lib = 0U;

  /* SystemInitialize for Atomic SubSystem: '<S2>/accpedal' */
  /* SystemInitialize for Atomic SubSystem: '<S8>/PID' */
  softecu_PID_Init(&softecu_DW.PID_f);

  /* End of SystemInitialize for SubSystem: '<S8>/PID' */
  /* End of SystemInitialize for SubSystem: '<S2>/accpedal' */

  /* SystemInitialize for Atomic SubSystem: '<S3>/brakepedal' */
  /* SystemInitialize for Atomic SubSystem: '<S15>/PID' */
  softecu_PID_Init(&softecu_DW.PID_d);

  /* End of SystemInitialize for SubSystem: '<S15>/PID' */
  /* End of SystemInitialize for SubSystem: '<S3>/brakepedal' */

  /* SystemInitialize for Atomic SubSystem: '<S26>/PID' */
  softecu_PID_Init(&softecu_DW.PID);

  /* End of SystemInitialize for SubSystem: '<S26>/PID' */
  /* End of SystemInitialize for SubSystem: '<Root>/SoftACC' */
}

/* Model terminate function */
void softecuModelClass::terminate() { /* (no terminate code required) */
}

/* Constructor */
softecuModelClass::softecuModelClass() { /* Currently there is no constructor body generated.*/
}

/* Destructor */
softecuModelClass::~softecuModelClass() { /* Currently there is no destructor body generated.*/
}

/* Real-Time Model get method */
RT_MODEL_softecu_T *softecuModelClass::getRTM() { return (&softecu_M); }

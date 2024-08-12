/*
 * Code generation for system system '<S8>/PID'
 * For more details, see corresponding source file softecu_PID.c
 *
 */

#ifndef RTW_HEADER_softecu_PID_h_
#define RTW_HEADER_softecu_PID_h_
#ifndef softecu_COMMON_INCLUDES_
#  define softecu_COMMON_INCLUDES_
#  include "rtwtypes.h"
#  include "rtw_continuous.h"
#  include "rtw_solver.h"


#endif /* softecu_COMMON_INCLUDES_ */

#include "softecu_types.h"

/* Shared type includes */
#include "multiword_types.h"

/* Block signals for system '<S8>/PID' */
typedef struct {
  real_T pid_out; /* '<S9>/Add1' */
} B_PID_softecu_T;

/* Block states (default storage) for system '<S8>/PID' */
typedef struct {
  real_T last_error_PreviousInput; /* '<S9>/last_error' */
  real_T Memory_PreviousInput;     /* '<S11>/Memory' */
} DW_PID_softecu_T;

extern void softecu_PID_Init(DW_PID_softecu_T *localDW);
extern void softecu_PID(real_T rtu_error, boolean_T rtu_reset, B_PID_softecu_T *localB, DW_PID_softecu_T *localDW,
                        real_T rtp_kp, real_T rtp_ki, real_T rtp_iemax, real_T rtp_iemin, real_T rtp_kd);

#endif /* RTW_HEADER_softecu_PID_h_ */

// Copyright 2024 Tencent Inc. All rights reserved.
/**
 * @file EvalCommon.h
 * @author DongYuanHu (dongyuanhu@tencent.com)
 * @brief evaluation common header
 * @version 0.1
 * @date 2023-04-23
 *
 *
 */
#pragma once

#include "utils/eval_constants.h"
#include "utils/eval_fwd.h"
#include "utils/eval_log.h"
#include "utils/eval_types.h"
#include "visibility.h"

#ifndef _MATH_DEFINES_DEFINED
#  define _MATH_DEFINES_DEFINED 1
#endif

namespace eval {
extern "C" {
/**
 * @brief Set the EvalParams object
 *
 * @param evalParams
 */
TXSIM_API void setEvalParams(const EvalParams& evalParams);

/**
 * @brief set module step time
 * @param t_s, unit second
 */
TXSIM_API void setModuleStepTime(double t_s = 0.01);

/**
 * @brief set ego number of this scene
 * @param ego_num
 */
TXSIM_API void setTotalEgoNum(size_t ego_num);
}  // extern "C"

/**
 * @brief Get the EvalParams object
 *
 * @return const EvalParams&
 */
const EvalParams& getEvalParams();

/**
 * @brief disable report function in simcity mode
 *
 * @return true
 * @return false
 */
bool isReportEnabled();

/**
 * @brief return module step time
 * @return step time, unit second
 */
double getModuleStepTime();

/**
 * @brief return ego number of this scene
 * @return ego number
 */
size_t getTotalEgoNum();

/**
 * @brief return my ego groupname
 * @return groupname, such as "Ego_001"
 */
std::string getMyEgoGroupName();
}  // namespace eval

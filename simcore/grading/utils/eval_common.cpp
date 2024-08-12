// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_common.h"

namespace eval {
EvalParams globalEvalParams;
double g_stepTime = 0.01;
size_t total_ego_num = 0;

extern "C" {
void setEvalParams(const EvalParams& evalParams) { globalEvalParams = evalParams; }
void setModuleStepTime(double t_s) { g_stepTime = t_s; }
void setTotalEgoNum(size_t ego_num) { total_ego_num = ego_num; }
}  // extern "C"

const EvalParams& getEvalParams() { return globalEvalParams; }
bool isReportEnabled() { return !(getEvalParams().m_is_simcity); }
double getModuleStepTime() { return g_stepTime; }
size_t getTotalEgoNum() { return total_ego_num; }
std::string getMyEgoGroupName() { return getEvalParams().m_ego_groupname; }
}  // namespace eval

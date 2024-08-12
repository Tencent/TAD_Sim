// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>
#include "utils/eval_common.h"
#include "utils/eval_math.h"

namespace eval {
// simulation time, use second
class CSimTime {
 private:
  double _t_s;

 public:
  explicit CSimTime(double t_ms = 0.0) { FromMilliseond(t_ms); }
  inline void FromMilliseond(double t_ms) { _t_s = t_ms / 1000.0; }
  inline void FromSecond(double t_s) { _t_s = t_s; }
  inline double GetSecond() const { return _t_s; }
  inline double GetMilliseond() const { return _t_s * 1000.0; }
  static double Diff(const CSimTime &t1, const CSimTime &t2) { return t1.GetSecond() - t2.GetSecond(); }
};

// speed
class CSpeed : public EVector3d {
 public:
  explicit CSpeed(double x = 0.0, double y = 0.0, double z = 0.0) : EVector3d(x, y, z) {}
};

// CAcceleration
class CAcceleration : public EVector3d {
 public:
  explicit CAcceleration(double x = 0.0, double y = 0.0, double z = 0.0) : EVector3d(x, y, z) {}
};

// CJerk
class CJerk : public EVector3d {
 public:
  explicit CJerk(double x = 0.0, double y = 0.0, double z = 0.0) : EVector3d(x, y, z) {}
};

// CAngularVelocity
class CAngularVelocity : public EVector3d {
 public:
  explicit CAngularVelocity(double x = 0.0, double y = 0.0, double z = 0.0) : EVector3d(x, y, z) {}
};

// protobuf message
class EvalMsg {
 private:
  CSimTime _sim_time;
  std::string _topic;
  std::string _payload;

 public:
  explicit EvalMsg(double t_ms = 0.0, const std::string &topic = "", const std::string &payload = "");
  const CSimTime &GetSimTime() const { return _sim_time; }
  const std::string &GetTopic() const { return _topic; }
  const std::string &GetPayload() const { return _payload; }
};

using EvalMsgPtr = CSharedPtr<EvalMsg>;
}  // namespace eval

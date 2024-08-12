// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_scene_loader.h"
TX_NAMESPACE_OPEN(Base)
struct SegmentKinectParam {
  using ControlPathGear = Base::ISceneLoader::IRouteViewer::control_path_node::ControlPathGear;

  txFloat mU; /*initial velocity*/
  txFloat mV; /*final velocity*/
  txFloat mS; /*displacement*/
  txFloat mStartS, mEndS;
  txFloat mA; /*acceleration*/
  txFloat mT; /*time*/
  txFloat mStartTime;
  txFloat EndTime() const TX_NOEXCEPT { return mStartTime + mT; }
  ControlPathGear mGear = _plus_(ControlPathGear::drive);

  friend std::ostream& operator<<(std::ostream& os, const SegmentKinectParam& v) TX_NOEXCEPT {
    os << "{" << TX_VARS_NAME(u, v.mU) << TX_VARS_NAME(v, v.mV) << TX_VARS_NAME(s, v.mS)
       << TX_VARS_NAME(start_s, v.mStartS) << TX_VARS_NAME(end_s, v.mEndS) << TX_VARS_NAME(a, v.mA)
       << TX_VARS_NAME(t, v.mT) << TX_VARS_NAME(startTime, v.mStartTime) << TX_VARS_NAME(endTime, v.EndTime())
       << TX_VARS_NAME(gear, __enum2lpsz__(ControlPathGear, v.mGear)) << "}";
    return os;
  }
};
using SegmentKinectParamList = std::list<SegmentKinectParam>;
TX_NAMESPACE_CLOSE(Base)

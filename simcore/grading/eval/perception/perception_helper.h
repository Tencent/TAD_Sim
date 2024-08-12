// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "osi_sensordata.pb.h"

#include "actors/c_actor_base.h"
#include "actors/c_actor_vehicle.h"
#include "utils/eval_math.h"

namespace eval {
// return truth actor of tracked object
template <class FellowTyPtr>
CActorBase *GetTruthOfTrackObj(const eval::ActorList<FellowTyPtr> fellows, const osi3::DetectedMovingObject &tracked) {
  CPosition tracked_pos(tracked.base().position().x(), tracked.base().position().y(), tracked.base().position().z());
  double min_dist = INT32_MAX;

  FellowTyPtr truth_ptr = nullptr;

  for (auto i = 0; i < fellows.size(); ++i) {
    // one fellow
    const CActorBase &fellow = *(fellows.at(i));

    // get dist
    double dist = CEvalMath::Distance2D(fellow.GetLocation().GetPosition(), tracked_pos);
    double base_length =
        (tracked.base().dimension().length() + tracked.base().dimension().width() + fellow.GetBaseLength());
    if (dist < min_dist && dist <= base_length) {
      truth_ptr = fellows.at(i);
      dist = min_dist;
    }
  }

  return truth_ptr;
}
}  // namespace eval

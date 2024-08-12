// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval/prediction/prediction_base.h"

namespace eval {
void FellowPredManager::Reset() {
  while (!m_fellow_info_list.empty()) m_fellow_info_list.clear();
}
std::vector<FellowTrajInfoPtr> FellowPredManager::FilterFellows(double t_s, double t_thresh) {
  std::vector<FellowTrajInfoPtr> popped_fellow;
  popped_fellow.reserve(eval::const_max_actor_size);

  if (m_fellow_info_list.size() == 0) return popped_fellow;

  FellowTrajInfoPtr front = m_fellow_info_list.front();

  // pop if fellow is too old or has enough trajectory
  while (front) {
    bool shall_pop_fellow = false;

    // if fellow has enough trajectory
    if (front->m_actual_traj.size() >= 2) {
      auto first = front->m_actual_traj.front();
      auto last = front->m_actual_traj.back();
      shall_pop_fellow = last->GetSimTime().GetSecond() - first->GetSimTime().GetSecond() >= t_thresh;
      popped_fellow.push_back(front);
    }

    // if fellow is too old
    shall_pop_fellow = shall_pop_fellow || (t_s - front->m_t) >= t_thresh;

    if (shall_pop_fellow) {
      m_fellow_info_list.pop_front();
      front = m_fellow_info_list.front();
    } else {
      break;
    }
  }

  return popped_fellow;
}
void FellowPredManager::Add(FellowTrajInfoPtr fellow_info_ptr) {
  if (fellow_info_ptr.get() != nullptr) m_fellow_info_list.push_back(fellow_info_ptr);
}

void FellowPredManager::Add(int64_t id, FellowTrajActualPtr fellow) {
  if (fellow.get() == nullptr) return;

  for (auto iter = m_fellow_info_list.begin(); iter != m_fellow_info_list.end(); ++iter) {
    FellowTrajInfoPtr &fellow_info = *iter;
    if (fellow_info->m_id == id) {
      fellow_info->m_actual_traj.push_back(fellow);
    }
  }
}

size_t FellowPredManager::Size() { return m_fellow_info_list.size(); }
}  // namespace eval

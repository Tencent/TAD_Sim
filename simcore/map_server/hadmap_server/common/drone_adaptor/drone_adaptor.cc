/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "drone_adaptor.h"

#include "traffic.pb.h"

namespace drone_adaptor {

using RepeatedPtrTraffic = google::protobuf::RepeatedPtrField<sim_msg::Traffic>;

template <typename T>
void PartialSerializeImpl(const DetailType& detail, T* proto) {
  proto->set_id(detail.ID);
  proto->set_x(detail.CenterX);
  proto->set_y(detail.CenterY);
  proto->set_z(detail.CenterZ);
  proto->set_heading(detail.Heading);
  proto->set_pitch(detail.Pitch);
  proto->set_roll(detail.Roll);
  proto->set_v(detail.Speed);
  proto->set_tx_airway_id(detail.Airway);
}

void PartialSerializeImpl(const DroneT& obj, int t, sim_msg::Traffic* tfc) {
  const auto& detail = obj.Tracs.front();

  sim_msg::Drone* one = tfc->add_drones();
  one->set_t(t);
  one->set_priorty(obj.Priority);
  one->set_name(obj.Name);
  one->set_owner(obj.Owner);

  PartialSerializeImpl(detail, one);
}

void PartialSerializeImpl(const CapsuleT& obj, int t, sim_msg::Traffic* tfc) {
  const auto& detail = obj.Capsules.front();

  sim_msg::Capsule* one = tfc->add_capsules();
  one->set_t(t);
  one->set_priorty(obj.Priority);
  one->set_drone_id(obj.DroneID);
  one->set_head_a(detail.Head_a);
  one->set_head_b(detail.Head_b);
  one->set_body_l(detail.Body_L);
  one->set_tail_a(detail.Tail_a);
  one->set_tail_b(detail.Tail_b);
  one->set_height(detail.Height);

  PartialSerializeImpl(detail, one);
}

template <typename T>
void PartialSerialize(const T& obj, int t, std::vector<DetailType>& detail, RepeatedPtrTraffic* tfcs,
                      sim_msg::Traffic* tfc) {
  if (detail.empty() || detail.front().Time > t) {
    return;
  }
  if (!tfc) {
    tfc = tfcs->Add();
  }

  PartialSerializeImpl(obj, t, tfc);

  detail.erase(detail.begin());
}

template <typename T>
void ParseDroneOrCapsuleJson(const std::string& json, const std::string& detail_key, std::vector<T>& output) {
  try {
    for (auto& obj : nlohmann::json::parse(json)) {
      nlohmann::json _obj = TypeDefaultJson<T>();
      _obj.update(obj);
      obj = _obj;
      if (obj[detail_key].empty()) {
        continue;
      }
      for (auto& detail : obj[detail_key]) {
        nlohmann::json _detail = TypeDefaultJson<DetailType>();
        _detail.update(detail);
        detail = _detail;
      }
      output.emplace_back(T());
      from_json(obj, output.back());
    }
  } catch (const std::exception& ex) {
    // std::cerr << "ParseDroneOrCapsuleJson error: " << ex.what() << std::endl;
  } catch (...) {
  }
}

std::string TransDroneToSimrec(const std::string& drone_json, const std::string& capsule_json, int interval) {
  std::vector<DroneT> drones;
  std::vector<CapsuleT> capsules;

  ParseDroneOrCapsuleJson(drone_json, kDroneDetailKey, drones);

  ParseDroneOrCapsuleJson(capsule_json, kCapsuleDetailKey, capsules);

  int f_time = INT_MAX, t_time = INT_MIN;
  auto update_f_t_time = [&f_time, &t_time](const auto& detail) {
    if (detail.empty()) {
      return;
    }
    f_time = std::min(f_time, detail.front().Time);
    t_time = std::max(t_time, detail.back().Time);
  };

  for (auto& drone : drones) {
    update_f_t_time(drone.Tracs);
  }
  for (auto& capsule : capsules) {
    update_f_t_time(capsule.Capsules);
  }
  sim_msg::TrafficRecords4Logsim simrec;
  RepeatedPtrTraffic* tfcs = simrec.mutable_traffic_record();

  if (interval <= 0) {
    interval = kDefaultFrameInterval;
  }

  for (int t = 0; t < t_time; t += interval) {
    if (t < f_time) {
      continue;
    }

    sim_msg::Traffic* tfc = nullptr;

    for (auto& drone : drones) {
      PartialSerialize(drone, t, drone.Tracs, tfcs, tfc);
    }

    for (auto& capsule : capsules) {
      PartialSerialize(capsule, t, capsule.Capsules, tfcs, tfc);
    }
  }

  std::string res;
  simrec.SerializeToString(&res);
  return res;
}

}  // namespace drone_adaptor

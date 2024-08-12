/**
 * @file sim_conflict_alg.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "sim_conflict_alg.h"
#include <chrono>
#include <ctime>
#include <random>
#include "../sensors.h"

std::random_device g_rd;
std::mt19937 g_gen(g_rd());
std::uniform_real_distribution<> g_dis(0, 1);

auto g_start_time = std::chrono::high_resolution_clock::now();
auto g_end_time = std::chrono::high_resolution_clock::now();
auto g_duration = std::chrono::duration_cast<std::chrono::microseconds>(g_end_time - g_start_time).count();

SimConfilctAlg::SimConfilctAlg() {
  // std::srand(std::time(0));
  // std::srand(1);
  m_system_inited = false;
  m_obu_loaded = false;
  m_rsu_loaded = false;
  m_obu_total_num = 0;
  m_rsu_total_num = 0;
  m_interval = 0.0;
  m_sub_step_equal_ms = 0.0;
  m_step_include_sub_step = 50;  // how many sub steps of each step
  m_fixed_delay_ms = 10;
  m_attenuation_model_type = 0;
  m_base_number = 2;
  m_queue_length_limit = 10;                         // how many sub package can one OBU hold
  m_obu_consider_range = 200;                        // how far an OBU consider when try to obtain CSMA resource
  m_package_life_substep = m_step_include_sub_step;  // How long could One Package Remain in Waiting List
  PartialClear();
}

void SimConfilctAlg::SetFrequency(double interval) {
  m_interval = interval;
  if (interval > 0) {
    m_sub_step_equal_ms = interval / m_step_include_sub_step;
  } else {
    m_sub_step_equal_ms = 2;
  }
}

void SimConfilctAlg::PartialClear() {
  m_init_obu_device_id_list.clear();
  EgoCarPosition = Position();
  m_step = 0;            // step count
  m_sub_step = 0;        // sub step count of current step
  m_sub_step_sleep = 0;  // sub step intrval
  m_sucess = 0.0;        // After Every Step, count All sucess received package
  m_fail = 0.0;          // After Every Step, count All fail received package
  m_try_too_much = 0.0;  // After Every Step, count All packages that tried more than MaxTryTimes
  m_total_delay = 0;     // Keep Add Final Delay of All sucess received package
  m_generated_pack_count = 0;
  m_obu_master_package_id.clear();
  m_obu_position.clear();
  m_obu_subpack_to_be_sent.clear();
  m_obu_sent_in_current_sub_step.clear();
  g_obu_subpackID_pack.clear();
  g_receive_packID_sub_count.clear();
  g_receive_packID_sub_res_status.clear();
  m_subpack_received_in_current_sub_step.clear();
  m_obu_last_pack_sucess_time.clear();
  m_obu_pack_sucess_count.clear();
  m_obu_pack_try_too_much.clear();
  m_obu_pack_queue_too_long.clear();
  m_obu_pack_distance_drop.clear();
  m_obu_pack_collision.clear();
  m_obu_consider_rage.clear();
  m_obu_in_ego_car_range.clear();
  m_obu_delay_sum.clear();
  m_obu_success_sum.clear();
  m_device_init_step.clear();
  m_device_receives_queue.clear();
  m_device_receives_five_delay_queue.clear();
}

void SimConfilctAlg::Init(int device_id) {
  m_obu_master_package_id[device_id] = 0;
  m_obu_position[device_id] = Position(device_id);
  m_obu_sent_in_current_sub_step[device_id] = false;
  m_obu_last_pack_sucess_time[device_id] = 0.0;
  m_obu_pack_sucess_count[device_id] = 0.0;
  m_obu_pack_try_too_much[device_id] = 0.0;
  m_obu_pack_queue_too_long[device_id] = 0.0;
  m_obu_pack_distance_drop[device_id] = 0.0;
  m_obu_pack_collision[device_id] = 0.0;
  m_obu_consider_rage[device_id] = m_obu_consider_range;
  m_obu_subpack_to_be_sent[device_id] = std::queue<int>{};
  g_obu_subpackID_pack[device_id] = std::map<int, std::shared_ptr<SubPackage>>{};
  g_receive_packID_sub_count[device_id] = std::map<int, int>{};
  g_receive_packID_sub_res_status[device_id] = std::map<int, int>{};
  m_subpack_received_in_current_sub_step[device_id] = std::vector<int>{};
  m_device_receives_queue[device_id] = std::queue<std::shared_ptr<SubPackage>>{};
  m_device_receives_five_delay_queue[device_id] = std::vector<int>{};
  m_obu_in_ego_car_range[device_id] = 5000;
  m_obu_delay_sum[device_id] = 0.0;
  m_obu_success_sum[device_id] = 0;
  m_device_init_step[device_id] = std::rand() % 5;
}

SimConfilctAlg& SimConfilctAlg::getInstance() {
  static SimConfilctAlg sim_conflict_alg;
  return sim_conflict_alg;
}

void SimConfilctAlg::GenerateNewPackAndLoadMap(SimObuSensors& sim, int type) {
  m_sub_step = 0;
  for (auto& bsm : sim.sim_obus) {
    ++m_generated_pack_count;
    int device_id = bsm.first;
    int bsm_size = sizeof(bsm.second.CarObu);
    // int sub_package_num = (bsm_size + 149) / 150;
    int sub_package_num = 1;
    if (sub_package_num > 9) {
      sub_package_num = 9;
    }
    for (int index = 0; index < sub_package_num; ++index) {
      int global_subpack_id = m_obu_master_package_id[device_id] * 10 + index;
      std::shared_ptr<BSMSubpackage> bsm_sub_package = std::make_shared<BSMSubpackage>();
      bsm_sub_package->DeviceID = device_id;
      bsm_sub_package->GenerateTime = GetNowSubStepTime() + m_device_init_step[device_id] * m_step_include_sub_step;
      bsm_sub_package->MsgType = type;
      bsm_sub_package->MasterPackageID = m_obu_master_package_id[device_id];
      bsm_sub_package->SubPackageID = index;
      bsm_sub_package->SubPackageCount = sub_package_num;
      bsm_sub_package->simObuBsm.ObuId = bsm.second.ObuId;
      bsm_sub_package->simObuBsm.CarObu = bsm.second.CarObu;
      m_obu_subpack_to_be_sent[device_id].push(global_subpack_id);
      g_obu_subpackID_pack[device_id][global_subpack_id] = bsm_sub_package;
    }
    CheckListLength(device_id);

    // Update the position of this Device
    m_obu_position[device_id].lat = bsm.second.CarObu.x();
    m_obu_position[device_id].lng = bsm.second.CarObu.y();
  }
}

// 将每个device的消息分包然后放入到发送队列中
bool SimConfilctAlg::PushObuMessage(SimObuSensors& sim) {
  for (auto& bsm : sim.sim_obus) {
    if (traffic_obu_sensors.find(static_cast<std::int64_t>(bsm.first)) == traffic_obu_sensors.end()) {
      continue;
    }
    if (std::find(m_init_obu_device_id_list.begin(), m_init_obu_device_id_list.end(), bsm.first) ==
        m_init_obu_device_id_list.end()) {
      m_init_obu_device_id_list.push_back(bsm.first);
      Init(bsm.first);
      ++m_obu_total_num;
    }
  }
  // std::cout << "m_obu_total_num = " << m_obu_total_num << std::endl;
  if (m_obu_total_num != 0) {
    GenerateNewPackAndLoadMap(sim, MSG_TYPE_BSM);
  }
  return true;
}

void SimConfilctAlg::LoadRSM(SimRsuSensors& sim) {
  for (auto& rsm : sim.sim_sensor_objects) {
    int device_id = rsm.first + 10000;
    CheckRsuExited(device_id);
    m_obu_position[device_id].lat = sim.sim_rsu_posotion[rsm.first].lat;
    m_obu_position[device_id].lng = sim.sim_rsu_posotion[rsm.first].lng;
  }
}

void SimConfilctAlg::LoadRSI(SimRsuSensors& sim) {
  for (auto& rsi : sim.sim_traffic_events) {
    int device_id = rsi.first + 10000;
    CheckRsuExited(device_id);
    m_obu_position[device_id].lat = sim.sim_rsu_posotion[rsi.first].lat;
    m_obu_position[device_id].lng = sim.sim_rsu_posotion[rsi.first].lng;
  }
}

void SimConfilctAlg::LoadSPAT(SimRsuSensors& sim) {
  for (auto& spat : sim.sim_traffic_lights) {
    int device_id = spat.first + 10000;
    CheckRsuExited(device_id);
    m_obu_position[device_id].lat = sim.sim_rsu_posotion[spat.first].lat;
    m_obu_position[device_id].lng = sim.sim_rsu_posotion[spat.first].lng;
  }
}

void SimConfilctAlg::LoadMAP(SimRsuSensors& sim) {
  for (auto& map : sim.sim_traffic_maps) {
    int device_id = map.first + 10000;
    CheckRsuExited(device_id);
    m_obu_position[device_id].lat = sim.sim_rsu_posotion[map.first].lat;
    m_obu_position[device_id].lng = sim.sim_rsu_posotion[map.first].lng;
  }
}

void SimConfilctAlg::CheckRsuExited(int device_id) {
  if (std::find(m_init_obu_device_id_list.begin(), m_init_obu_device_id_list.end(), device_id) ==
      m_init_obu_device_id_list.end()) {
    m_init_obu_device_id_list.push_back(device_id);
    Init(device_id);
    ++m_rsu_total_num;
  }
}

bool SimConfilctAlg::PushRsuMessage(SimRsuSensors& sim) {
  LoadRSM(sim);
  LoadRSI(sim);
  LoadSPAT(sim);
  LoadMAP(sim);
  if (m_rsu_total_num != 0) {
    m_sub_step = 0;
    GenerateRSM(sim);
    GenerateRSI(sim);
    GenerateSPAT(sim);
    GenerateMAP(sim);
  }
  return true;
}

void SimConfilctAlg::CheckListLength(int device_id) {
  ++m_obu_master_package_id[device_id];
  while (m_obu_subpack_to_be_sent[device_id].size() > m_queue_length_limit) {
    int subpack_id = m_obu_subpack_to_be_sent[device_id].front();
    MarkTransResult(device_id, subpack_id / 10, subpack_id, 4);
    m_obu_subpack_to_be_sent[device_id].pop();
    if (m_obu_subpack_to_be_sent[device_id].size() != 0) {
      ReadyNextSubPack(device_id);
    }
  }
}

void SimConfilctAlg::GenerateRSM(SimRsuSensors& sim) {
  for (auto& rsm : sim.sim_sensor_objects) {
    ++m_generated_pack_count;
    int device_id = rsm.first + 10000;
    int sub_package_num = 1;
    if (sub_package_num > 9) {
      sub_package_num = 9;
    }
    for (int index = 0; index < sub_package_num; ++index) {
      int global_subpack_id = m_obu_master_package_id[device_id] * 10 + index;
      std::shared_ptr<RSMSubpackage> rsm_sub_package = std::make_shared<RSMSubpackage>();
      rsm_sub_package->DeviceID = device_id;
      rsm_sub_package->GenerateTime = GetNowSubStepTime() + m_device_init_step[device_id] * m_step_include_sub_step;
      rsm_sub_package->MsgType = MSG_TYPE_RSM;
      rsm_sub_package->MasterPackageID = m_obu_master_package_id[device_id];
      rsm_sub_package->SubPackageID = index;
      rsm_sub_package->SubPackageCount = sub_package_num;
      rsm_sub_package->simRsuRsm = rsm.second;
      m_obu_subpack_to_be_sent[device_id].push(global_subpack_id);
      g_obu_subpackID_pack[device_id][global_subpack_id] = rsm_sub_package;
    }
    CheckListLength(device_id);
  }
}

void SimConfilctAlg::GenerateRSI(SimRsuSensors& sim) {
  for (auto& rsi : sim.sim_traffic_events) {
    ++m_generated_pack_count;
    int device_id = rsi.first + 10000;
    int sub_package_num = 1;
    if (sub_package_num > 9) {
      sub_package_num = 9;
    }
    for (int index = 0; index < sub_package_num; ++index) {
      int global_subpack_id = m_obu_master_package_id[device_id] * 10 + index;
      std::shared_ptr<RSISubpackage> rsi_sub_package = std::make_shared<RSISubpackage>();
      rsi_sub_package->DeviceID = device_id;
      rsi_sub_package->GenerateTime = GetNowSubStepTime() + m_device_init_step[device_id] * m_step_include_sub_step;
      rsi_sub_package->MsgType = MSG_TYPE_RSI;
      rsi_sub_package->MasterPackageID = m_obu_master_package_id[device_id];
      rsi_sub_package->SubPackageID = index;
      rsi_sub_package->SubPackageCount = sub_package_num;
      rsi_sub_package->simRsuRsi = rsi.second;
      m_obu_subpack_to_be_sent[device_id].push(global_subpack_id);
      g_obu_subpackID_pack[device_id][global_subpack_id] = rsi_sub_package;
    }
    CheckListLength(device_id);
  }
}

void SimConfilctAlg::GenerateSPAT(SimRsuSensors& sim) {
  for (auto& spat : sim.sim_traffic_lights) {
    ++m_generated_pack_count;
    int device_id = spat.first + 10000;
    int sub_package_num = 1;
    if (sub_package_num > 9) {
      sub_package_num = 9;
    }
    for (int index = 0; index < sub_package_num; ++index) {
      int global_subpack_id = m_obu_master_package_id[device_id] * 10 + index;
      std::shared_ptr<SPATSubpackage> spat_sub_package = std::make_shared<SPATSubpackage>();
      spat_sub_package->DeviceID = device_id;
      spat_sub_package->GenerateTime = GetNowSubStepTime() + m_device_init_step[device_id] * m_step_include_sub_step;
      spat_sub_package->MsgType = MSG_TYPE_SPAT;
      spat_sub_package->MasterPackageID = m_obu_master_package_id[device_id];
      spat_sub_package->SubPackageID = index;
      spat_sub_package->SubPackageCount = sub_package_num;
      spat_sub_package->simRsuSpat = spat.second;
      m_obu_subpack_to_be_sent[device_id].push(global_subpack_id);
      g_obu_subpackID_pack[device_id][global_subpack_id] = spat_sub_package;
    }
    CheckListLength(device_id);
  }
}

void SimConfilctAlg::GenerateMAP(SimRsuSensors& sim) {
  for (auto& map : sim.sim_traffic_maps) {
    ++m_generated_pack_count;
    int device_id = map.first + 10000;
    int sub_package_num = 1;
    if (sub_package_num > 9) {
      sub_package_num = 9;
    }
    for (int index = 0; index < sub_package_num; ++index) {
      int global_subpack_id = m_obu_master_package_id[device_id] * 10 + index;
      std::shared_ptr<MAPSubpackage> map_sub_package = std::make_shared<MAPSubpackage>();
      map_sub_package->DeviceID = device_id;
      map_sub_package->GenerateTime = GetNowSubStepTime() + m_device_init_step[device_id] * m_step_include_sub_step;
      map_sub_package->MsgType = MSG_TYPE_MAP;
      map_sub_package->MasterPackageID = m_obu_master_package_id[device_id];
      map_sub_package->SubPackageID = index;
      map_sub_package->SubPackageCount = sub_package_num;
      map_sub_package->simRsuMap = map.second;
      m_obu_subpack_to_be_sent[device_id].push(global_subpack_id);
      g_obu_subpackID_pack[device_id][global_subpack_id] = map_sub_package;
    }
    CheckListLength(device_id);
  }
}

// 调用冲突算法进行step计算
void SimConfilctAlg::RunSim() {
  // if (!m_system_inited) { return; }
  // std::cout << "\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Now Step = " << m_step << " &&&&&&&&&&&&&&&&&&&&&&&&&&" << std::endl;
  for (m_sub_step = 0; m_sub_step < m_step_include_sub_step; ++m_sub_step) {
    // Every Sub Step, set all OBU sent mark to false firstly, cause no obu has sent by now.
    std::map<int, bool>::iterator it;
    for (it = m_obu_sent_in_current_sub_step.begin(); it != m_obu_sent_in_current_sub_step.end(); ++it)
      it->second = false;

    // Clear m_subpack_received_in_current_sub_step, cause no obu has sent by now.
    m_subpack_received_in_current_sub_step.clear();
    std::map<int, std::vector<int>>::iterator ittt;
    for (ittt = m_subpack_received_in_current_sub_step.begin(); ittt != m_subpack_received_in_current_sub_step.end();
         ++ittt) {
      int obu_id = ittt->first;
      m_subpack_received_in_current_sub_step[obu_id] = std::vector<int>{};
    }

    // Run each obu, the one comes earlier will get chance to send package.
    int obu_id = 0;
    int distance = 0;
    // Each sub_step, regroup the list of all obus to make their competion fairly.
    std::shuffle(m_init_obu_device_id_list.begin(), m_init_obu_device_id_list.end(), std::default_random_engine());
    for (int index = 0; index < m_init_obu_device_id_list.size(); ++index) {
      obu_id = m_init_obu_device_id_list[index];
      distance =
          Distance(m_obu_position[obu_id].lat, m_obu_position[obu_id].lng, EgoCarPosition.lat, EgoCarPosition.lng);
      m_obu_in_ego_car_range[obu_id] = distance;
      if (distance > m_obu_consider_range) {
        continue;
      }
      TrafficSend(obu_id);
    }

    EgoCarReceive(GetReceivedSize());
  }
  if (m_step % 20 == 0) {
    ShowInfo();
  }
  ++m_step;
}

void SimConfilctAlg::ShowInfo() {
  std::cout
      //<< "|   Step\tObu\tRemain\tFirstDelay\tSuccessPassed       Success  Collision  TryTooMuch  QueueDroped  Queue"
      << "|   Step\tObu\tRemain\tDelay  SuccessPassed   Success  Collision  Try  Queue  Distance  Range  Delay"
      << std::endl;
  std::map<int, std::map<int, std::shared_ptr<SubPackage>>>::iterator itt;
  for (itt = g_obu_subpackID_pack.begin(); itt != g_obu_subpackID_pack.end(); ++itt) {
    std::cout << "|    " << m_step << "  \t" << itt->first;
    std::cout << "\t" << itt->second.size();
    if (!m_obu_subpack_to_be_sent[itt->first].empty()) {
      std::cout << "   " << itt->second[m_obu_subpack_to_be_sent[itt->first].front()]->Delay << "  \t";
    } else {
      std::cout << "   --\t\t";
    }
    std::cout << (GetNowSubStepTime() - m_obu_last_pack_sucess_time[itt->first]) / m_step_include_sub_step << "  steps";
    std::cout << "\t " << m_obu_pack_sucess_count[itt->first];
    std::cout << "\t " << m_obu_pack_collision[itt->first];
    std::cout << "\t" << m_obu_pack_try_too_much[itt->first];
    std::cout << "\t" << m_obu_pack_queue_too_long[itt->first];
    std::cout << "\t" << m_obu_pack_distance_drop[itt->first];
    // std::map<int, std::shared_ptr<SubPackage>>::iterator it2;
    // for (it2 = itt->second.begin(); it2 != itt->second.end(); ++it2) {
    //   std::cout << " " << it2->first;
    //   if (it2->second == nullptr) std::cout << "!";
    //   if (it2->second.get() == nullptr) std::cout << "!";
    // }
    std::cout << "\t " << m_obu_in_ego_car_range[itt->first];
    if (m_obu_success_sum[itt->first] != 0) {
      std::cout << "\t" << m_obu_delay_sum[itt->first] / m_obu_success_sum[itt->first];
    }
    std::cout << "" << std::endl;
  }

  std::cout << "| Generated packages: " << m_generated_pack_count;
  // if (m_try_too_much != 0) {
  //    std::cout << "\t\t\t\t\t\t| Try too much (sub pack): " << m_try_too_much << std::endl;
  // }
  if (m_fail != 0) {
    std::cout << "  | Fail (pack): " << m_fail << "/" << m_fail + m_sucess;
  }
  if (m_sucess != 0) {
    std::cout << "  | success (pack): " << m_sucess << "/" << m_fail + m_sucess;
    std::cout << "  | average delay : " << m_total_delay / m_sucess;
    std::cout << " " << (m_total_delay / m_sucess) / m_step_include_sub_step << " steps" << std::endl;
  }
}

// 冲突计算完毕，调用返回相应的结果
bool SimConfilctAlg::PopReceivedObuMessage(SimObuSensors& sim) {
  for (auto& receiver : m_device_receives_queue) {
    if (receiver.first < 10000) {
      while (!receiver.second.empty()) {
        auto& sub_package = receiver.second.front();
        if (sub_package->MasterPackageID >= 0) {
          BSMSubpackage* ptr = static_cast<BSMSubpackage*>(sub_package.get());
          ptr->simObuBsm.timestamp = ptr->GenerateTime * m_sub_step_equal_ms - m_interval - m_fixed_delay_ms;
          // std::cout << "=================================== ptr->simObuBsm.timestamp = " << ptr->simObuBsm.timestamp
          // << std::endl;
          sim.sim_obus[receiver.first] = ptr->simObuBsm;
        }
        receiver.second.pop();
      }
    }
  }
  return true;
}

bool SimConfilctAlg::PopReceivedRsuMessage(SimRsuSensors& sim) {
  for (auto& receiver : m_device_receives_queue) {
    if (receiver.first >= 10000) {
      while (!receiver.second.empty()) {
        auto& sub_package = receiver.second.front();
        if (sub_package->MasterPackageID >= 0) {
          if (sub_package->MsgType == MSG_TYPE_RSM) {
            RSMSubpackage* ptr = static_cast<RSMSubpackage*>(sub_package.get());
            ptr->simRsuRsm.timestamp = ptr->GenerateTime * m_sub_step_equal_ms - m_interval - m_fixed_delay_ms;
            sim.sim_sensor_objects[receiver.first] = ptr->simRsuRsm;
          }
          if (sub_package->MsgType == MSG_TYPE_SPAT) {
            SPATSubpackage* ptr = static_cast<SPATSubpackage*>(sub_package.get());
            ptr->simRsuSpat.timestamp = ptr->GenerateTime * m_sub_step_equal_ms - m_interval - m_fixed_delay_ms;
            sim.sim_traffic_lights[receiver.first] = ptr->simRsuSpat;
          }
          if (sub_package->MsgType == MSG_TYPE_RSI) {
            RSISubpackage* ptr = static_cast<RSISubpackage*>(sub_package.get());
            ptr->simRsuRsi.timestamp = ptr->GenerateTime * m_sub_step_equal_ms - m_interval - m_fixed_delay_ms;
            sim.sim_traffic_events[receiver.first] = ptr->simRsuRsi;
          }
          if (sub_package->MsgType == MSG_TYPE_MAP) {
            MAPSubpackage* ptr = static_cast<MAPSubpackage*>(sub_package.get());
            ptr->simRsuMap.timestamp = ptr->GenerateTime * m_sub_step_equal_ms - m_interval - m_fixed_delay_ms;
            sim.sim_traffic_maps[receiver.first] = ptr->simRsuMap;
          }
        }
        receiver.second.pop();
      }
    }
  }
  return true;
}

int SimConfilctAlg::GetNowSubStepTime() { return m_step * m_step_include_sub_step + m_sub_step; }

int SimConfilctAlg::GetNextStepTime() {
  int res = (1 + m_step) * m_step_include_sub_step;
  return res;
}

int SimConfilctAlg::CheckFinalResult(int obu_id, int subpack_id) {
  int pack_id = subpack_id / 10;
  // 0: The entire package is not all received.
  // std::cout << "Try to check obu_id = " << obu_id << " subpack_id = " << subpack_id << std::endl;
  // std::cout << "g_receive_packID_sub_count[obu_id][pack_id] = " << g_receive_packID_sub_count[obu_id][pack_id]
  //          << std::endl;
  // std::cout << "g_obu_subpackID_pack[obu_id][subpack_id] == nullptr = " << (g_obu_subpackID_pack[obu_id][subpack_id]
  // == nullptr) << std::endl;
  if (g_receive_packID_sub_count[obu_id][pack_id] != g_obu_subpackID_pack[obu_id][subpack_id]->SubPackageCount) {
    return 0;
  }

  // 1: If the entire package has failed subpackage, the package is deleted and no more process.
  if (1 != g_receive_packID_sub_res_status[obu_id][pack_id]) {
    g_obu_subpackID_pack[obu_id][subpack_id]->ReceivedTime = GetNowSubStepTime();
    // std::cout << m_step << "-" << m_sub_step << " ";
    // std::cout << " --- Ego Car receive from oub No." << obu_id << " pack_id = " << pack_id;
    // std::cout << " Package Generated at " << g_obu_subpackID_pack[obu_id][subpack_id]->GenerateTime;
    // std::cout << " Received at " << g_obu_subpackID_pack[obu_id][subpack_id]->ReceivedTime;
    // std::cout << " Delay is " << g_obu_subpackID_pack[obu_id][subpack_id]->Delay << " ***** Failed :";
    if (g_receive_packID_sub_res_status[obu_id][pack_id] == 2) {
      m_obu_pack_collision[obu_id] += 1;
      // std::cout << " Collision";
    }
    if (g_receive_packID_sub_res_status[obu_id][pack_id] == 3) {
      m_obu_pack_try_too_much[obu_id] += 1;
      // std::cout << " TryTooMuch";
    }
    if (g_receive_packID_sub_res_status[obu_id][pack_id] == 4) {
      m_obu_pack_queue_too_long[obu_id] += 1;
      // std::cout << " QueueDroped";
    }
    if (g_receive_packID_sub_res_status[obu_id][pack_id] == 5) {
      m_obu_pack_distance_drop[obu_id] += 1;
      // std::cout << " DistanceDroped";
    }
    // std::cout << " " << std::endl;
    g_receive_packID_sub_res_status[obu_id].erase(pack_id);
    g_receive_packID_sub_count[obu_id].erase(pack_id);
    m_fail += 1;
    return 1;
  }

  ManageSuccessPackage(obu_id, pack_id, subpack_id);
  return 2;
}

void SimConfilctAlg::ManageSuccessPackage(int obu_id, int pack_id, int subpack_id) {
  g_obu_subpackID_pack[obu_id][subpack_id]->ReceivedTime = GetNextStepTime();
  g_obu_subpackID_pack[obu_id][subpack_id]->GenerateTime =
      g_obu_subpackID_pack[obu_id][subpack_id]->ReceivedTime - g_obu_subpackID_pack[obu_id][subpack_id]->Delay;

  // std::cout << m_step << "-" << m_sub_step << " ";
  // std::cout << " --- Ego Car receive from oub No." << obu_id << " pack_id = " << pack_id;
  // std::cout << " Package Generated at " << g_obu_subpackID_pack[obu_id][subpack_id]->GenerateTime;
  // std::cout << " Received at " << g_obu_subpackID_pack[obu_id][subpack_id]->ReceivedTime;
  // std::cout << " Delay is " << g_obu_subpackID_pack[obu_id][subpack_id]->Delay << " ***** Success *****" <<
  // std::endl;

  m_obu_delay_sum[obu_id] += g_obu_subpackID_pack[obu_id][subpack_id]->Delay;
  m_obu_success_sum[obu_id] += 1;

  m_device_receives_queue[obu_id].push(g_obu_subpackID_pack[obu_id][subpack_id]);
  // printf("m_device_receives_queue[%d].size() = %d\n", obu_id, m_device_receives_queue[obu_id].size());
  m_total_delay += g_obu_subpackID_pack[obu_id][subpack_id]->Delay;

  m_device_receives_five_delay_queue[obu_id].push_back(g_obu_subpackID_pack[obu_id][subpack_id]->Delay);
  while (m_device_receives_five_delay_queue[obu_id].size() > 5) {
    m_device_receives_five_delay_queue[obu_id].erase(m_device_receives_five_delay_queue[obu_id].begin());
  }

  // if (g_obu_subpackID_pack[obu_id][subpack_id]->MsgType == MSG_TYPE_BSM) {
  //     g_obu_subpackID_pack[obu_id][subpack_id]->simObuBsm.timestamp =
  //       g_obu_subpackID_pack[obu_id][subpack_id]->GenerateTime * m_sub_step_equal_ms;
  // }
  g_receive_packID_sub_res_status[obu_id].erase(pack_id);
  g_receive_packID_sub_count[obu_id].erase(pack_id);
  m_sucess += 1;
  m_obu_last_pack_sucess_time[obu_id] = GetNowSubStepTime();
  m_obu_pack_sucess_count[obu_id] += 1;
  // std::cout << " " << std::endl;
}

int SimConfilctAlg::DistanceDrop(int obu_id) {
  int res = 1;
  double distance =
      Distance(m_obu_position[obu_id].lat, m_obu_position[obu_id].lng, EgoCarPosition.lat, EgoCarPosition.lng);
  double drop_rate = 0.0;
  if (m_attenuation_model_type == 1) {
    drop_rate = (log(distance + 1) / log(m_base_number)) / m_obu_consider_range;
    // std::cout << "drop_rate = " << drop_rate << std::endl;
  } else {
    drop_rate = (distance / m_obu_consider_range) / 20;
  }
  if (g_dis(g_gen) < drop_rate) {
    res = 5;
  }
  return res;
}

void SimConfilctAlg::MarkTransResult(int obu_id, int pack_id, int subpack_id, int trans_res) {
  if (trans_res == 1) {
    trans_res = DistanceDrop(obu_id);
  }

  // If this is the first sub package to arrive in the entire package.
  if (g_receive_packID_sub_res_status[obu_id].count(pack_id) == 0) {
    // std::cout << m_step << "-" << m_sub_step << " ";
    // std::cout << "First Time receive subpackage of Obu No." << obu_id << " pack_id " << pack_id << std::endl;
    g_receive_packID_sub_res_status[obu_id][pack_id] = trans_res;
    g_receive_packID_sub_count[obu_id][pack_id] = 1;
    CheckFinalResult(obu_id, subpack_id);
  } else {
    // If this is not the first sub package to arrive in the entire package.
    // std::cout << m_step << "-" << m_sub_step << " ";
    // std::cout << "Second Time receive subpackage of Obu No." << obu_id << " pack_id " << pack_id << std::endl;
    //  Collisions have the highest priority of all failure cases others can not overwrite.
    if (1 != trans_res && 2 != g_receive_packID_sub_res_status[obu_id][pack_id]) {
      g_receive_packID_sub_res_status[obu_id][pack_id] = trans_res;
    }
    g_receive_packID_sub_count[obu_id][pack_id] += 1;
    CheckFinalResult(obu_id, subpack_id);
  }
  g_obu_subpackID_pack[obu_id].erase(subpack_id);
  // std::cout << "######## All Received -- g_obu_subpackID_pack[" << obu_id << "].earse(" << subpack_id << ")" <<
  // std::endl;
}

// If one sub package is dealt with (sent or discarded), update the next sub package's Delay.
void SimConfilctAlg::ReadyNextSubPack(int obu_id) {
  int next_sub_pack_id = m_obu_subpack_to_be_sent[obu_id].front();
  if (g_obu_subpackID_pack[obu_id][next_sub_pack_id]->Delay == 0) {
    g_obu_subpackID_pack[obu_id][next_sub_pack_id]->Delay = 1;
  }
  if (GetNowSubStepTime() <= g_obu_subpackID_pack[obu_id][next_sub_pack_id]->GenerateTime) {
    return;
  }
  g_obu_subpackID_pack[obu_id][next_sub_pack_id]->Delay +=
      (GetNowSubStepTime() - g_obu_subpackID_pack[obu_id][next_sub_pack_id]->GenerateTime);
}

void SimConfilctAlg::TrafficSend(int obu_id) {
  // std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$ TrafficSend(" << obu_id << ") run $$$$$$$$$$$$$$$$$$$$$$$$$$" <<
  // std::endl;
  if (m_obu_subpack_to_be_sent[obu_id].size() == 0) return;
  int subpack_sent_in_current_sub_step = -1;
  int now_time = GetNowSubStepTime();
  {
    // Step.1 Check if there are subpackages need to be sent in current sub_step.
    int subpack_id = m_obu_subpack_to_be_sent[obu_id].front();
    int send_time =
        g_obu_subpackID_pack[obu_id][subpack_id]->GenerateTime + g_obu_subpackID_pack[obu_id][subpack_id]->Delay;
    if (send_time == now_time) {
      subpack_sent_in_current_sub_step = subpack_id;
    }

    if (subpack_sent_in_current_sub_step == -1) return;
    // std::cout << "\t" << m_step << "-" << m_sub_step << " ";
    // std::cout << " ============ Obu No." << obu_id;
    //  std::cout << " ============ Obu No." << obu_id << " want to send subpackage No." <<
    //  subpack_sent_in_current_sub_step << std::endl;
  }

  {
    // Step.2 If there are subpackages to be sent, find all OBUs in the range of the considering distance.
    std::vector<int> ConsiderObuList{};
    std::map<int, Position>::iterator it;
    for (it = m_obu_position.begin(); it != m_obu_position.end(); it++) {
      if (it->first == obu_id) continue;
      int distance = Distance(m_obu_position[obu_id].lat, m_obu_position[obu_id].lng, it->second.lat, it->second.lng);
      // std::cout << "distance = " << distance << std::endl;
      if (distance > m_obu_consider_rage[obu_id]) continue;
      ConsiderObuList.push_back(it->first);
    }
    // std::cout << "\t" << m_step << "-" << m_sub_step << " ";
    // std::cout << "              Obu No." << obu_id << " need to check Obu list: ";
    // for (int j = 0; j < ConsiderObuList.size(); ++j) std::cout << ConsiderObuList[j] << " ";
    // std::cout << "" << std::endl;

    // Step.3 Check all OBUs if they has sent already.
    m_obu_sent_in_current_sub_step[obu_id] = true;  // Assume this obu can send msg in current sub step.
    for (int index = 0; index < ConsiderObuList.size(); ++index) {
      if (m_obu_sent_in_current_sub_step[ConsiderObuList[index]]) {
        m_obu_sent_in_current_sub_step[obu_id] = false;  // If other obu has sent, this obu will wait.
        break;
      }
    }
    // std::cout << "\t" << m_step << "-" << m_sub_step << " ";
    // if (m_obu_sent_in_current_sub_step[obu_id]) {
    //    std::cout << "              Obu No." << obu_id << " can send package.";
    //}
    // else {
    //    std::cout << "              Obu No." << obu_id << " can not send package." << std::endl;
    //}
  }

  {
    // Step.4 If it can send a msg, send the first one.
    int subpack_id = subpack_sent_in_current_sub_step;
    if (m_obu_sent_in_current_sub_step[obu_id]) {
      m_subpack_received_in_current_sub_step[obu_id].push_back(subpack_id);
      // std::cout << " Send subpackage, sub_pack_id = " << subpack_id << std::endl;

      // Only if the subpackage marked with subpack_sent_in_current_sub_step is truely sent,
      // it will be delete from the list waiting to be sent, and update the Delay of the next object in the queue.
      m_obu_subpack_to_be_sent[obu_id].pop();
      if (m_obu_subpack_to_be_sent[obu_id].size() != 0) {
        ReadyNextSubPack(obu_id);
      }
    } else {
      // Step.5 If it can't send, it should push all msgs back and set new delays
      ++g_obu_subpackID_pack[obu_id][subpack_id]->TryTimes;
      if (g_obu_subpackID_pack[obu_id][subpack_id]->TryTimes >= g_obu_subpackID_pack[obu_id][subpack_id]->MaxTryTimes) {
        // g_obu_subpackID_pack[obu_id].erase(subpack_id);
        // std::cout << "######## Try Times too much -- g_obu_subpackID_pack[" << obu_id << "].earse(" << subpack_id <<
        // ")" << std::endl;
        int pack_id = subpack_id / 10;
        MarkTransResult(obu_id, pack_id, subpack_id, 3);
        m_obu_subpack_to_be_sent[obu_id].pop();
        if (m_obu_subpack_to_be_sent[obu_id].size() != 0) {
          ReadyNextSubPack(obu_id);
        }
        m_try_too_much += 1;
        return;
      }
      g_obu_subpackID_pack[obu_id][subpack_id]->MaxDelay *= 2;
      // In the next line, "1 + rand": the current sub step has passed send stage, so the wait start in the next
      // substep.
      g_obu_subpackID_pack[obu_id][subpack_id]->Delay +=
          (1 + std::rand() % g_obu_subpackID_pack[obu_id][subpack_id]->MaxDelay);
      // std::cout << "\t" << m_step << "-" << m_sub_step << " ";
      // std::cout << " Re Delay sub_pack_id = " << subpack_sent_in_current_sub_step;
      // std::cout << " ----------------- New Delay = " << g_obu_subpackID_pack[obu_id][subpack_id]->Delay << std::endl;
    }
  }
}

int SimConfilctAlg::GetReceivedSize() {
  int res = 0;
  std::map<int, std::vector<int>>::iterator it;
  for (it = m_subpack_received_in_current_sub_step.begin(); it != m_subpack_received_in_current_sub_step.end(); ++it) {
    if (it->second.size() > 0) res += 1;
  }
  return res;
}

void SimConfilctAlg::EgoCarReceive(int total_sub_pack_num) {
  if (total_sub_pack_num == 0) return;

  // Ego Car received more than one sub package, all should be marked as failed.
  if (total_sub_pack_num > 1) {
    int min_distance = INT_MAX;
    std::map<int, int> m_distance_to_obu_id_;
    std::map<int, std::vector<int>>::iterator it;
    for (it = m_subpack_received_in_current_sub_step.begin(); it != m_subpack_received_in_current_sub_step.end();
         ++it) {
      if (it->second.size() == 0) {
        continue;
      }
      int obu_id = it->first;
      int pack_id = m_subpack_received_in_current_sub_step[obu_id].front() / 10;
      int subpack_id = m_subpack_received_in_current_sub_step[obu_id].front();
      int tmp_distance =
          Distance(m_obu_position[obu_id].lat, m_obu_position[obu_id].lng, EgoCarPosition.lat, EgoCarPosition.lng);
      m_distance_to_obu_id_[tmp_distance] = obu_id;
      if (tmp_distance < min_distance) {
        min_distance = tmp_distance;
      }
    }
    int success_obu_id = m_distance_to_obu_id_[min_distance];

    for (it = m_subpack_received_in_current_sub_step.begin(); it != m_subpack_received_in_current_sub_step.end();
         ++it) {
      if (it->second.size() == 0) {
        continue;
      }
      int obu_id = it->first;
      int pack_id = m_subpack_received_in_current_sub_step[obu_id].front() / 10;
      int subpack_id = m_subpack_received_in_current_sub_step[obu_id].front();
      if (obu_id == success_obu_id) {
        MarkTransResult(obu_id, pack_id, subpack_id, 1);
      } else {
        MarkTransResult(obu_id, pack_id, subpack_id, 2);
      }
    }
    // // std::cout << "\t\t\t↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ collision" << std::endl;
    return;
  }

  // Ego Car received only one sub package, this sub package can be marked as success
  std::map<int, std::vector<int>>::iterator it;
  for (it = m_subpack_received_in_current_sub_step.begin(); it != m_subpack_received_in_current_sub_step.end(); ++it) {
    if (it->second.size() == 0) {
      continue;
    }
    int obu_id = it->first;
    int pack_id = m_subpack_received_in_current_sub_step[obu_id].front() / 10;
    int subpack_id = m_subpack_received_in_current_sub_step[obu_id].front();
    MarkTransResult(obu_id, pack_id, subpack_id, 1);
  }
}

void SimConfilctAlg::UpdateEgoCarPosition(sim_msg::Location& loc) {
  EgoCarPosition.lat = loc.position().x();
  EgoCarPosition.lng = loc.position().y();
}

int SimConfilctAlg::GetSubStep() { return m_step_include_sub_step; }

void SimConfilctAlg::ReGroupVector(std::vector<int>& source) {
  std::shuffle(source.begin(), source.end(), std::default_random_engine());
}

void SimConfilctAlg::SetAttenuationModelType(int type) { m_attenuation_model_type = type; }
void SimConfilctAlg::SetBaseNumber(int base_number) {
  m_base_number = base_number;
  if (base_number < 2) {
    m_base_number = 2;
  }
}

double SimConfilctAlg::GetAvgDelay(int obu_id) {
  int res = 0;
  for (int i = 0; i < m_device_receives_five_delay_queue[obu_id].size(); ++i) {
    res += m_device_receives_five_delay_queue[obu_id][i];
  }
  return res * m_sub_step_equal_ms / m_device_receives_five_delay_queue[obu_id].size() + m_fixed_delay_ms;
}

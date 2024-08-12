// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include <math.h>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <vector>
#include "../utils/utils.h"
#include "location.pb.h"
#include "sim_base.h"

#define MSG_TYPE_BSM 1
#define MSG_TYPE_RSM 2
#define MSG_TYPE_RSI 3
#define MSG_TYPE_SPAT 4
#define MSG_TYPE_MAP 5

struct SubpackageConfig {
  int FrequencyBSM = 10;
  double DistanceOpen = 800;
  double DistanceCity = 300;
  int Band = 2;  // 5905-5915, 5915-5925, 5905-5925
  double Mbps = 10;
  double SystemDelay = 10;
  double CommuDelay = 20;
  bool TriggerImmediately = true;
};

class SubPackage {
 public:
  SubpackageConfig package_config;

  int32_t GenerateTime = 0;  // step * g_step_include_sub_step
  int32_t ReceivedTime = 0;  // step * g_step_include_sub_step + g_sub_step

  int32_t DeviceID = 0;  // OBU ID
  int16_t MsgType = 0;
  int32_t MasterPackageID = 0;  // the master package ID which this subpack belongs to
  int32_t SubPackageID = 1;     // sub package ID
  int32_t SubPackageCount = 2;  // the number of sub pakages this master package cantained

  // Delay: The number of sub_steps to wait before sending, superimposed for each additional wait.
  //        Combined with GenerateTime to obtain the absolute time of sending
  int32_t Delay;
  int32_t TryTimes = 0;     // tried times
  int32_t MaxTryTimes = 4;  // Max Try Times Limit
  // MaxDelay: In the binary backoff algorithm, the initial value of the maximum value of the waiting time range,
  //           multiplied by 2 after each additional attempt
  int32_t MaxDelay = 50;

  SubPackage() { Delay = std::rand() % MaxDelay; }
  SubPackage(int Time, int Car, int MasterPackage, int SubPackage, int Count) {
    GenerateTime = Time;
    DeviceID = Car;
    MasterPackageID = MasterPackage;
    SubPackageID = SubPackage;
    SubPackageCount = Count;
    Delay = std::rand() % MaxDelay;
  }
};

// struct Position {
//   double lat;
//   double lng;
//   Position() {
//     lat = 118.5043414;
//     lng = -0.0028034;
//   }
//   Position(int index) {
//     lat = 118.5043414 + 0.00005 * index;
//     lng = -0.0028034 - 0.003 * index;
//   }
// };

class BSMSubpackage : public SubPackage {
 public:
  SimObus simObuBsm;
};

class RSMSubpackage : public SubPackage {
 public:
  SimSensorObjects simRsuRsm;
};

class RSISubpackage : public SubPackage {
 public:
  SimTrafficEvents simRsuRsi;
};

class SPATSubpackage : public SubPackage {
 public:
  SimTrafficLights simRsuSpat;
};

class MAPSubpackage : public SubPackage {
 public:
  SimTrafficMaps simRsuMap;
};

class SimConfilctAlg : public SimConfictBase {
 public:
  SimConfilctAlg();
  ~SimConfilctAlg() {}
  void SetFrequency(double frequency);

  static SimConfilctAlg& getInstance();
  void PartialClear();

  bool PushObuMessage(SimObuSensors& sim);
  bool PushRsuMessage(SimRsuSensors& sim);

  bool PopReceivedObuMessage(SimObuSensors& sim);
  bool PopReceivedRsuMessage(SimRsuSensors& sim);

  void RunSim() override;
  void UpdateEgoCarPosition(sim_msg::Location& loc);
  int GetSubStep();
  void SetAttenuationModelType(int type);
  void SetBaseNumber(int base_number);
  double GetAvgDelay(int obu_id);

 private:
  void Init(int device_id);
  int GetNowSubStepTime();
  int GetNextStepTime();
  int CheckFinalResult(int obu_id, int subpack_id);
  void GenerateNewPackAndLoadMap(SimObuSensors& sim, int type);
  void ManageSuccessPackage(int obu_id, int pack_id, int subpack_id);
  void MarkTransResult(int obu_id, int pack_id, int subpack_id, int trans_res);
  void ReadyNextSubPack(int obu_id);
  void TrafficSend(int obu_id);
  int GetReceivedSize();
  void EgoCarReceive(int total_sub_pack_num);
  void ShowInfo();
  void ReGroupVector(std::vector<int>& source);
  void LoadRSM(SimRsuSensors& sim);
  void LoadRSI(SimRsuSensors& sim);
  void LoadSPAT(SimRsuSensors& sim);
  void LoadMAP(SimRsuSensors& sim);
  void CheckRsuExited(int device_id);
  void CheckListLength(int device_id);
  void GenerateRSM(SimRsuSensors& sim);
  void GenerateRSI(SimRsuSensors& sim);
  void GenerateSPAT(SimRsuSensors& sim);
  void GenerateMAP(SimRsuSensors& sim);
  int DistanceDrop(int obu_id);

 private:
  Position EgoCarPosition;
  int m_runSim_call_flag;
  int m_generated_pack_count;
  int m_step_include_sub_step;  // how many sub steps of each step
  int m_queue_length_limit;     // how many sub package can one OBU hold
  int m_obu_consider_range;     // how far an OBU consider when try to obtain CSMA resource
  int m_step;                   // step count
  int m_sub_step;               // sub step count of current step
  int m_sub_step_sleep;         // sub step intrval
  double m_sucess;              // After Every Step, count All sucess received package
  double m_fail;                // After Every Step, count All fail received package
  double m_try_too_much;        // After Every Step, count All packages that tried more than MaxTryTimes
  int m_total_delay;            // Keep Add Final Delay of All sucess received package
  int m_package_life_substep;   // How long could One Package Remain in Waiting List
  bool m_system_inited;
  bool m_obu_loaded;
  bool m_rsu_loaded;
  int m_obu_total_num;
  int m_rsu_total_num;
  double m_interval;
  double m_sub_step_equal_ms;
  int m_fixed_delay_ms;
  int m_attenuation_model_type;
  int m_base_number;

  // Only clear once at the system init.
  std::map<int, int>
      m_obu_master_package_id;  // how many package this obu generated (If generate in each step, equals to g_step)
  std::map<int, Position> m_obu_position;             // Obu Location
  std::map<int, double> m_obu_last_pack_sucess_time;  // mark the receive time of the newest success received package
  std::map<int, double> m_obu_pack_sucess_count;      // mark the number of all success received package
  std::map<int, double> m_obu_pack_try_too_much;      // mark the number of all packages droped for MaxTryTimes
  std::map<int, double> m_obu_pack_queue_too_long;    // mark the number of all packages droped for Queue Max Length
  std::map<int, double> m_obu_pack_distance_drop;     // mark the number of all packages droped for distance
  std::map<int, double> m_obu_pack_collision;         // mark the number of all received but collided package
  std::map<int, int>
      m_obu_consider_rage;  // mark the range of the current OBU consider about when it try to gat a sub step
  std::map<int, double> m_obu_delay_sum;  // mark all delay of each success received pack for every obu
  std::map<int, int> m_obu_success_sum;   // mark success number for every obu
  std::map<int, int> m_device_init_step;  // mark which step this device generate the first pack

  // Clear before every sub step.
  std::map<int, bool> m_obu_sent_in_current_sub_step;  // Mark if this OBU send subpackage in current sub step
  std::map<int, std::vector<int>>
      m_subpack_received_in_current_sub_step;  // the sub package ID received by Ego Car in current sub step

  // This list restore all the subpackages of every obu, a subpackage is pushed into it when generated.
  // A subpackage is erased when it is sent.
  std::map<int, std::queue<int>> m_obu_subpack_to_be_sent;  // sub package ids of this obu remain to send
  // std::map<int, std::queue<std::shared_ptr<SubPackage>>> m_device_senders_queue;
  std::map<int, std::queue<std::shared_ptr<SubPackage>>> m_device_receives_queue;
  std::map<int, std::vector<int>> m_device_receives_five_delay_queue;

  // The following is not cleared at any time, only when the system inited.
  //   Any time a new package is generated, it will be push into g_obu_subpackID_pack,
  //   Only when a package is all transported, it will be remove from the list no matter success or fail.
  //          Store all auboackages of all OBU, inter map: <subpack_id, object>
  std::map<int, std::map<int, std::shared_ptr<SubPackage>>> g_obu_subpackID_pack;
  //          Use the following two maps to store the number of received subpackages and whether there are any failures
  //          before all subpackages are received. If res = 1, all of them are successful. Others means at least one
  //          subpackage failed for some reason.
  std::map<int, std::map<int, int>> g_receive_packID_sub_count;
  std::map<int, std::map<int, int>>
      g_receive_packID_sub_res_status;  // 1-success 2-collision 3-TryTooMuch 4-QueueTooLong

  // Store all device id, please make sure no device share a same id
  std::vector<int> m_init_obu_device_id_list;

  std::map<int, int> m_obu_in_ego_car_range;  // Mark if this OBU inside the connection range of the Ego Car
};

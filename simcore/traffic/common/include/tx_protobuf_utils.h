// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "structs/hadmap_lanelink.h"
#include "structs/hadmap_predef.h"
#include "tx_header.h"
#ifdef max
#  undef max
#endif  // max

#ifdef min
#  undef min
#endif  // min
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>
#include "ditw_event.pb.h"
#include "its_sim.pb.h"
#include "location.pb.h"
#include "moduleService.pb.h"
#include "traffic.pb.h"
TX_NAMESPACE_OPEN(Utils)

extern Base::txString ProtobufDebugString(sim_msg::Car* _carMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugString(const sim_msg::Car& _carMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugJson(sim_msg::Car* _carMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugLogString(const sim_msg::Car& refCar) TX_NOEXCEPT;
extern Base::txString ProtobufDebugLogString(const its::txVehicle& _vehicleMsg) TX_NOEXCEPT;

extern Base::txString ProtobufDebugString(sim_msg::StaticObstacle* _obsMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugJson(sim_msg::StaticObstacle* _obsMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugLogString(const sim_msg::StaticObstacle& refObs) TX_NOEXCEPT;

extern Base::txString ProtobufDebugString(sim_msg::DynamicObstacle* _obsMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugJson(sim_msg::DynamicObstacle* _obsMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugLogString(const sim_msg::DynamicObstacle& refPed) TX_NOEXCEPT;

extern Base::txString ProtobufDebugString(sim_msg::TrafficLight* _tlMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugJson(sim_msg::TrafficLight* _tlMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugLogString(const sim_msg::TrafficLight& refSignal) TX_NOEXCEPT;

extern Base::txString ProtobufDebugString(sim_msg::Location* _egoMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugJson(sim_msg::Location* _egoMsg) TX_NOEXCEPT;

extern Base::txString ProtobufDebugString(sim_msg::SimMessage* _outputMsg) TX_NOEXCEPT;

#if __TX_Mark__("temp functions")
extern Base::txString txLaneLinksToString(const hadmap::txLaneLinks& refLaneLinks) TX_NOEXCEPT;
#endif

extern Base::txString ProtobufDebugString(sim_msg::DITW_Event* _egoMsg) TX_NOEXCEPT;
extern Base::txString ProtobufDebugJson(sim_msg::DITW_Event* _egoMsg) TX_NOEXCEPT;

extern Base::txBool IsEqual(const sim_msg::Car& leftVehicle, const sim_msg::Car& rightVehicle) TX_NOEXCEPT;
TX_NAMESPACE_CLOSE(Utils)

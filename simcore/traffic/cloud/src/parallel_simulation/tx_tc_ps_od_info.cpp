// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_od_info.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include "nlohmann/json.hpp"
TX_NAMESPACE_OPEN(TrafficFlow)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(od_route_vector_in::route_node_in, route_id, dest_id, origin_desc, dest_desc,
                                   road_id, route_cnt, origin_id, length);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(od_route_vector_in, route, vehicle_cnt);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(od_route_vector_out::route_node_out, route_id, dest_id, origin_desc, dest_desc,
                                   road_id, route_cnt, origin_id, length, dbAverageVelocityFromMicroSim,
                                   dbCongestionIndexFromMicroSim, nVehicleCountFromMicroSim);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(od_route_vector_out, route, vehicle_cnt);

Base::txBool od_info::Import(const Base::txString& inJsonStr) TX_NOEXCEPT {
  try {
    using namespace nlohmann;
    std::stringstream ss;
    ss << inJsonStr;
    json in_json;
    ss >> in_json;
    od_route_vector_in inOdRouteVector;
    from_json(in_json, inOdRouteVector);
    outOdRouteVector = inOdRouteVector;
    return true;
  } catch (std::exception& e) {
    LOG(WARNING) << "od param json parser failure. " << e.what() << TX_VARS(inJsonStr);
    return false;
  }
}

Base::txBool od_info::Export(Base::txString& outJsonStr) TX_NOEXCEPT {
  using namespace nlohmann;
  json out_json;
  to_json(out_json, outOdRouteVector);
  outJsonStr = out_json.dump(4);
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)

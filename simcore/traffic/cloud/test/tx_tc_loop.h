// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <fstream>
#include "google/protobuf/map.h"
#include "tx_tc_type_def.h"
#include "tx_tc_marco.h"
#include "txsim_module.h"

#include "boost/dll/import.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "tx_tc_plugin_micro.h"

#define __cloud_ditw__ (1)
TXST_NAMESPACE_OPEN(TrafficFlow)

class TrafficLoop4CloudDev : public tx_sim::SimModule {
  using TrafficManager = TrafficCloud::TrafficManager;

 public:
  TXST_CLASSNAME(TrafficLoop4CloudDev);
  TrafficLoop4CloudDev();
  virtual ~TrafficLoop4CloudDev() DEFAULT;

  virtual void Init(tx_sim::InitHelper& helper) OVERRIDE;
  virtual void Reset(tx_sim::ResetHelper& helper) OVERRIDE;
  virtual void Step(tx_sim::StepHelper& helper) OVERRIDE;
  virtual void Stop(tx_sim::StopHelper& helper) OVERRIDE;

 protected:
  virtual Base::txBool IsValid() const NOEXCEPT { return NonNull_Pointer(mPlugin); }
  virtual void resetVars() NOEXCEPT;
  virtual Base::txBool GetRoutingInfo(tx_sim::ResetHelper& helper, sim_msg::Location& refEgoData) const NOEXCEPT;
  virtual void SendTraffic(tx_sim::StepHelper& helper, const int ego_id, const double ego_dist) NOEXCEPT;
  virtual void SendTraffic(tx_sim::StepHelper& helper, sim_msg::MapPosition pos) NOEXCEPT;

  Base::txSize max_step_count_ = 0;
  Base::txFloat m_lastStepTime = 0.0;
  Base::txFloat m_passTime = 0.0;
  Base::txInt step_count_ = 0;
  Base::txString payload_;
  Base::txBool m_bFirstFrame = true;
  sim_msg::Traffic outputTraffic;

  typedef boost::shared_ptr<TrafficManager>(PluginCreate)();
  boost::function<PluginCreate> pluginCreator;
  boost::shared_ptr<TrafficManager> mPlugin = nullptr;
#if __cloud_ditw__
  int64_t m_debug_data_start_stamp;
  int m_debug_data_index = 0;
#endif /*__cloud_ditw__*/
  sim_msg::Location m_start_location;
};
TXST_NAMESPACE_CLOSE(TrafficFlow)

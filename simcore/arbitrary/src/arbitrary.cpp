#include "arbitrary.h"

#include <iomanip>
#include <iostream>

#include "arbitrary_imp.h"
#include "glog/logging.h"

#include "control.pb.h"

#define SPLIT_LINE "======================================="

#ifdef _WIN32
#  pragma message("building windows dll now, tell compiler use glog.lib and gflags.lib")
#  pragma comment(lib, "glog.lib")
#  pragma comment(lib, "gflags.lib")
#endif  // _WIN32

namespace arbitrary {
CArbitrary::CArbitrary() {
  name_ = "arbirtary_for_ADAS";
  LOG(INFO) << name_ << " constructed."
            << "\n";
}

CArbitrary::~CArbitrary() {
  LOG(INFO) << name_ << " destroyed."
            << "\n";
}

void CArbitrary::Init(tx_sim::InitHelper &helper) {
  LOG(INFO) << SPLIT_LINE << "\n";

  helper.Subscribe(topic::CONTROL_DRIVER);
  helper.Subscribe(topic::CONTROL_ALGORITHM);
  helper.Publish(topic::CONTROL);

  FLAGS_v = 0;
  std::string argv = helper.GetParameter("_log_level");
  if (argv.size() > 0) {
    try {
      FLAGS_v = std::atoi(argv.c_str());
    } catch (const char *msg) {
      FLAGS_v = 0;
      LOG(ERROR) << "err:" << msg;
    }
  }
  LOG(INFO) << "FLAGS_v:" << FLAGS_v << "\n";
  LOG(INFO) << name_ << " init done."
            << "\n";
};

void CArbitrary::Reset(tx_sim::ResetHelper &helper) {
  LOG(INFO) << SPLIT_LINE << "\n";
  m_arbitrary = std::make_shared<CArbitraryImp>();
  LOG(INFO) << name_ << " reset done."
            << "\n";
};

void CArbitrary::Step(tx_sim::StepHelper &helper) {
  if (m_arbitrary.get() != nullptr) {
    // get payload from tadsim
    std::string driver_payload, alg_payload;
    helper.GetSubscribedMessage(topic::CONTROL_DRIVER, driver_payload);
    helper.GetSubscribedMessage(topic::CONTROL_ALGORITHM, alg_payload);

    // do arbitrary
    m_arbitrary->DoArbitrary_Control(alg_payload, driver_payload);

    // publish
    helper.PublishMessage(topic::CONTROL, m_arbitrary->GetArbitrary_ControlPayload());
  } else {
    helper.StopScenario("m_arbitrary.get() is nullptr.");
  }
};

void CArbitrary::Stop(tx_sim::StopHelper &helper) {
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << name_ << " module stopped."
            << "\n";
  m_arbitrary.reset();
};
}  // namespace arbitrary
TXSIM_MODULE(arbitrary::CArbitrary)

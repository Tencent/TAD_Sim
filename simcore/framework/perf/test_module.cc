#include <limits>
#include <stdexcept>

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"

#include "txsim_module.h"
#include "utils/time.h"


using namespace tx_sim;


class PerfModule final : public SimModule {
 public:
  void Init(InitHelper& h) override {
    std::string v = h.GetParameter("sub_topics");
    if (!v.empty()) {
      boost::split(subs_, v, boost::is_any_of(","));
      for (const std::string& s : subs_) h.Subscribe(s);
    }
    v = h.GetParameter("pub_topics");
    if (!v.empty()) {
      boost::split(pubs_, v, boost::is_any_of(","));
      for (const std::string& p : pubs_) h.Publish(p);
    }
    if (subs_.empty() && pubs_.empty()) throw std::invalid_argument("at least one sub_topics/pub_topics provided.");

    msg_size_ = std::stoi(h.GetParameter("msg_size"));
    pub_payload_.assign(msg_size_, 's');
  }

  void Reset(ResetHelper& h) override {
    last_step_time_ = total_step_calling_period_ = max_step_calling_period_ = total_step_execution_time_ =
        max_step_execution_time_ = step_count_ = 0;
    min_step_calling_period_ = std::numeric_limits<uint32_t>::max();
    min_step_execution_time_ = std::numeric_limits<uint32_t>::max();
  }

  void Step(StepHelper& h) override {
    int64_t t = tx_sim::utils::GetCpuTimeInMicros();
    if (last_step_time_ != 0) {  // step 0 doesn't counts.
      uint32_t step_period = t - last_step_time_;
      total_step_calling_period_ += step_period;
      if (step_period > max_step_calling_period_) max_step_calling_period_ = step_period;
      if (step_period < min_step_calling_period_) min_step_calling_period_ = step_period;
      ++step_count_;
    }
    last_step_time_ = t;

    for (const std::string& t : subs_) h.GetSubscribedMessage(t, sub_payload_);
    for (const std::string& t : pubs_) h.PublishMessage(t, pub_payload_);

    uint32_t elapsed = tx_sim::utils::GetCpuTimeInMicros() - t;
    if (elapsed > max_step_execution_time_) max_step_execution_time_ = elapsed;
    if (elapsed < min_step_execution_time_) min_step_execution_time_ = elapsed;
    total_step_execution_time_ += elapsed;
  }

  void Stop(StopHelper& h) override {
    h.set_feedback("mean_step_period", std::to_string(total_step_calling_period_ / step_count_));
    h.set_feedback("min_step_period", std::to_string(min_step_calling_period_));
    h.set_feedback("max_step_period", std::to_string(max_step_calling_period_));
    h.set_feedback("mean_step_exec_time", std::to_string(total_step_execution_time_ / step_count_));
    h.set_feedback("min_step_exec_time", std::to_string(min_step_execution_time_));
    h.set_feedback("max_step_exec_time", std::to_string(max_step_execution_time_));
  }

 private:
  std::vector<std::string> subs_, pubs_;
  int64_t last_step_time_;
  uint32_t total_step_calling_period_, min_step_calling_period_, max_step_calling_period_, total_step_execution_time_,
      min_step_execution_time_, max_step_execution_time_, step_count_;
  size_t msg_size_;
  std::string pub_payload_, sub_payload_;
};


TXSIM_MODULE(PerfModule)

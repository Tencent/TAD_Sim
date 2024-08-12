// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "stepper.h"

#include <algorithm>
#include <cassert>
#include <sstream>

#include "boost/algorithm/string/join.hpp"
#include "boost/integer/common_factor_rt.hpp"
#include "glog/logging.h"

#include "utils/defs.h"


namespace tx_sim {
namespace coordinator {

//! @brief 函数名：AddModuleNodes
//! @details 函数功能：添加模块节点
//!
//! @param[in] name 模块名称
//! @param[in] exec_period 模块执行周期
//! @param[in] response_time 模块响应时间
void SyncStepper::AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                                 const std::string& group) {
  if (exec_period == 0) throw std::invalid_argument(name + " module exec period cannot be 0.");
  nodes_.emplace_back(name, exec_period);
  if (step_time_ == 0) step_time_ = exec_period;
  step_time_ = boost::integer::gcd(step_time_, exec_period);
}

//! @brief 函数名：SetTriggerModule
//! @details 函数功能：设置触发模块
//!
//! @param[in] name 模块名称
void SyncStepper::SetTriggerModule(const std::string& name) {
  for (const auto& m : nodes_)
    if (m.name == name) trigger_module_.reset(new ModuleNode(m));
  assert(trigger_module_ != nullptr);
}

//! @brief 函数名：TriggerAllModules
//! @details 函数功能：触发所有模块
void SyncStepper::TriggerAllModules() {
  all_module_triggered_ = true;
}

//! @brief 函数名：RemoveModule
//! @details 函数功能：移除模块
//!
//! @param[in] name 模块名称
void SyncStepper::RemoveModule(const std::string& name) {
  for (auto i = nodes_.cbegin(); i != nodes_.cend(); ++i)
    if (i->name == name) {
      nodes_.erase(i);
      break;
    }
}

//! @brief 函数名：ResetModuleTimes
//! @details 函数功能：重置模块时间
void SyncStepper::ResetModuleTimes() {
  assert(!nodes_.empty());
  current_time_ = 0;
  for (ModuleNode& n : nodes_) n.passed_time = 0;
  all_module_triggered_ = (trigger_module_ == nullptr);
  LOG(INFO) << "coordination mode set to SYNC.";
}

//! @brief 函数名：GetModuleEvent
//! @details 函数功能：获取模块事件
//!
//! @param[out] events 存储模块事件的向量
//! @return 模块事件数量
uint32_t SyncStepper::GetModuleEvent(std::vector<ModuleEvent>& events) {
  if (TXSIM_UNLIKELY(!all_module_triggered_)) {
    events.emplace_back(trigger_module_->name, kModuleEventExecutionTriggered);
    events.emplace_back(trigger_module_->name, kModuleEventExecutionFinished);
    return trigger_module_->step_time;
  }
  uint64_t last_event_time = current_time_, next_event_time = 0;
  for (ModuleNode& n : nodes_) {
    if (n.passed_time <= current_time_) {
      events.emplace_back(n.name, kModuleEventExecutionTriggered);
      events.emplace_back(n.name, kModuleEventExecutionFinished);
      n.passed_time += n.step_time;
    }
    next_event_time = next_event_time == 0 ? n.passed_time : std::min(next_event_time, n.passed_time);
  }
  current_time_ = next_event_time;
  return next_event_time - last_event_time;
}

uint32_t AsyncGroupModuleNode::GetGroupModuleEvent(std::vector<ModuleEvent>& events, bool& finished) {
  if (sub_nodes_.size() == 0) {
    events.emplace_back(name, kModuleEventExecutionTriggered);
    events.emplace_back(name, kModuleEventExecutionFinished);
    finished = true;
    return step_time;
  } else {
    uint64_t current_time = future_events_.top().time_point;
    std::string module_name;
    ModuleEventType event_type;
    while (!future_events_.empty() && future_events_.top().time_point == current_time) {
      future_events_.top().UnpackEvent(module_name, event_type);
      future_events_.pop();
      if (sub_nodes_.find(module_name) != sub_nodes_.end()) {
        events.emplace_back(module_name, event_type);
        if (event_type == kModuleEventExecutionTriggered) {
          const SubModuleNode& node = sub_nodes_[module_name];
          future_events_.emplace(module_name, current_time + node.response_time, kModuleEventExecutionFinished);
          //          future_events_.emplace(module_name, current_time + step_time, kModuleEventExecutionTriggered);
        }
      }
    }
    finished = future_events_.empty();
    if (finished) return step_time;
    else
      return 0;
    //    finished = (future_events_.top().event.type == kModuleEventExecutionTriggered);
    //    return future_events_.top().time_point - current_time;
  }
}

void AsyncGroupModuleNode::TriggerGroup() {
  for (auto& n : sub_nodes_) future_events_.emplace(n.first, passed_time, kModuleEventExecutionTriggered);
}

void IntergroupSyncStepper::AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                                           const std::string& group) {
  if (exec_period == 0) throw std::invalid_argument(name + " module exec period cannot be 0.");
  if (group.empty()) {
    AsyncGroupModuleNode group_node;
    group_node.name = name;
    group_node.step_time = exec_period;
    group_nodes_.emplace_back(group_node);
  } else {
    auto iter = std::find_if(group_nodes_.begin(), group_nodes_.end(),
                             [group](const AsyncGroupModuleNode& node) { return node.name == group; });
    if (iter == group_nodes_.end()) {
      AsyncGroupModuleNode group_node;
      group_node.name = group;
      group_node.step_time = exec_period;
      group_node.sub_nodes_[name] = AsyncGroupModuleNode::SubModuleNode(name, group);
      group_node.sub_nodes_[name].exec_period = exec_period;
      group_node.sub_nodes_[name].response_time = response_time;
      group_nodes_.emplace_back(group_node);
    } else {
      iter->step_time = std::max(exec_period, iter->step_time);
      iter->sub_nodes_[name] = AsyncGroupModuleNode::SubModuleNode(name, group);
      iter->sub_nodes_[name].exec_period = exec_period;
      iter->sub_nodes_[name].response_time = response_time;
    }
  }
}


void IntergroupSyncStepper::SetTriggerModule(const std::string& name) {}


void IntergroupSyncStepper::TriggerAllModules() {}

void IntergroupSyncStepper::RemoveModule(const std::string& name) {
  for (int gi = group_nodes_.size() - 1; gi != -1; --gi) {
    if (group_nodes_[gi].name == name && group_nodes_[gi].sub_nodes_.size() == 0) {
      group_nodes_.erase(group_nodes_.begin() + gi);
      break;
    } else {
      for (auto i = group_nodes_[gi].sub_nodes_.cbegin(); i != group_nodes_[gi].sub_nodes_.cend(); ++i) {
        if (i->second.name == name) {
          group_nodes_[gi].sub_nodes_.erase(i);
          break;
        }
      }
      if (group_nodes_[gi].sub_nodes_.size() == 0) {
        group_nodes_.erase(group_nodes_.begin() + gi);
        break;
      }
    }
  }
}


void IntergroupSyncStepper::ResetModuleTimes() {
  assert(!group_nodes_.empty());
  current_time_ = 0;
  for (auto& gn : group_nodes_) {
    gn.passed_time = 0;
    gn.future_events_ = decltype(gn.future_events_)();
    gn.TriggerGroup();
  }
  LOG(INFO) << "coordination mode set to Intergroup SYNC.";
}


uint32_t IntergroupSyncStepper::GetModuleEvent(std::vector<ModuleEvent>& events) {
  uint64_t last_event_time = current_time_, next_event_time = 0;
  for (AsyncGroupModuleNode& gn : group_nodes_) {
    if (gn.passed_time <= current_time_) {
      bool group_finished = true;
      uint32_t time_to_next_event = gn.GetGroupModuleEvent(events, group_finished);
      if (!group_finished) {
        // wait group finished
        return 0;
      } else {
        gn.passed_time += gn.step_time;
        gn.TriggerGroup();
      }
    }
  }
  next_event_time = 0;
  for (AsyncGroupModuleNode& gn : group_nodes_) {
    next_event_time = next_event_time == 0 ? gn.passed_time : std::min(next_event_time, gn.passed_time);
  }
  current_time_ = next_event_time;
  return next_event_time - last_event_time;
}


uint32_t SyncGroupModuleNode::GetGroupModuleEvent(std::vector<ModuleEvent>& events, ModuleEventType event_type) {
  if (sub_nodes_.size() == 0) {
    events.emplace_back(name, event_type);
    return 0;
  } else {
    return 0;
  }
}

void SyncGroupModuleNode::TriggerGroup() {}

void IntragroupSyncStepper::AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                                           const std::string& group) {
  if (exec_period == 0) throw std::invalid_argument(name + " module exec period cannot be 0.");

  if (group.empty()) {
    SyncGroupModuleNode& node = group_nodes_[name];
    node.name = name;
    if (response_time == 0) {  // response_time == 0 indicates synchronous mode. in asynchronous mode, response_time
                               // defaults to exec_period.
      response_time = exec_period;
    }
    node.exec_period = exec_period;
    node.response_time = std::min(response_time, exec_period);  // ensure response_time <= exec_period.
  } else {
    SyncGroupModuleNode& node = group_nodes_[name];
    node.name = name;
    if (response_time == 0) {  // response_time == 0 indicates synchronous mode. in asynchronous mode, response_time
                               // defaults to exec_period.
      response_time = exec_period;
    }
    /*
    if (node.sub_nodes_.size() == 0) {
        node.exec_period = exec_period;  // ensure response_time <= exec_period.
    } else {
        node.exec_period = std::gcd(node.exec_period,exec_period);  // ensure response_time <= exec_period.
    }
    */
    node.exec_period = exec_period;
    node.response_time = std::min(response_time, exec_period);
    SyncGroupModuleNode::SubModuleNode sub_node(name, exec_period);
    node.sub_nodes_.emplace_back(sub_node);
  }
}

void IntragroupSyncStepper::SetTriggerModule(const std::string& name) {}


void IntragroupSyncStepper::TriggerAllModules() {
  const FutureEvent& e = future_events_.top();
  uint64_t t = e.time_point;
  for (auto& n : group_nodes_) future_events_.emplace(n.first, t, kModuleEventExecutionTriggered);
}


void IntragroupSyncStepper::RemoveModule(const std::string& name) {
  for (auto iter = group_nodes_.begin(); iter != group_nodes_.end(); ++iter) {
    if (iter->first == name && iter->second.sub_nodes_.size() == 0) {
      group_nodes_.erase(name);
      break;
    } else {
      for (int i = iter->second.sub_nodes_.size() - 1; i != -1; --i) {
        if (iter->second.sub_nodes_[i].name == name) {
          iter->second.sub_nodes_.erase(iter->second.sub_nodes_.begin() + i);
          break;
        }
      }
      if (iter->second.sub_nodes_.size() == 0) {
        group_nodes_.erase(iter->first);
        break;
      }
    }
  }
}


void IntragroupSyncStepper::ResetModuleTimes() {
  assert(!group_nodes_.empty());
  future_events_ = decltype(future_events_)();  // clear the queue.
  for (auto& n : group_nodes_) future_events_.emplace(n.first, 0, kModuleEventExecutionTriggered);
  LOG(INFO) << "coordination mode set to IntragroupSyncStepper.";
}


uint32_t IntragroupSyncStepper::GetModuleEvent(std::vector<ModuleEvent>& events) {
  uint64_t current_time = future_events_.top().time_point;
  std::string group_name;
  ModuleEventType event_type;
  while (future_events_.top().time_point == current_time) {
    future_events_.top().UnpackEvent(group_name, event_type);
    future_events_.pop();
    if (TXSIM_LIKELY(group_nodes_.find(group_name) != group_nodes_.end())) {
      group_nodes_[group_name].GetGroupModuleEvent(events, event_type);
      // events.emplace_back(group_name, event_type);
      if (event_type == kModuleEventExecutionTriggered) {
        const SyncGroupModuleNode& node = group_nodes_[group_name];
        future_events_.emplace(group_name, current_time + node.response_time, kModuleEventExecutionFinished);
        future_events_.emplace(group_name, current_time + node.exec_period, kModuleEventExecutionTriggered);
      }
    }
  }
  return future_events_.top().time_point - current_time;
}


void AsyncStepper::AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                                  const std::string& group) {
  if (exec_period == 0) throw std::invalid_argument(name + " module exec period cannot be 0.");
  ModuleNode& node = nodes_[name];
  node.exec_period = exec_period;
  if (response_time == 0) {  // response_time == 0 indicates synchronous mode. in asynchronous mode, response_time
                             // defaults to exec_period.
    response_time = exec_period;
  }
  node.response_time = std::min(response_time, exec_period);  // ensure response_time <= exec_period.
}

//! @brief 函数名：RemoveModule
//! @details 函数功能：移除模块
//!
//! @param[in] name 模块名称
void AsyncStepper::RemoveModule(const std::string& name) {
  nodes_.erase(name);
}

//! @brief 函数名：SetTriggerModule
//! @details 函数功能：设置触发模块
//!
//! @param[in] name 模块名称
void AsyncStepper::SetTriggerModule(const std::string& name) {
  assert(nodes_.find(name) != nodes_.end());
  trigger_module_ = name;
}

//! @brief 函数名：TriggerAllModules
//! @details 函数功能：触发所有模块
void AsyncStepper::TriggerAllModules() {
  const FutureEvent& e = future_events_.top();
  uint64_t t = (e.event.type == kModuleEventExecutionTriggered)
                   ? e.time_point
                   : (e.time_point - nodes_[trigger_module_].response_time);
  for (auto& n : nodes_)
    if (n.first != trigger_module_) future_events_.emplace(n.first, t, kModuleEventExecutionTriggered);
}

//! @brief 函数名：ResetModuleTimes
//! @details 函数功能：重置模块时间
void AsyncStepper::ResetModuleTimes() {
  assert(!nodes_.empty());
  future_events_ = decltype(future_events_)();  // clear the queue.
  if (trigger_module_.empty())
    for (auto& n : nodes_) future_events_.emplace(n.first, 0, kModuleEventExecutionTriggered);
  else
    future_events_.emplace(trigger_module_, 0, kModuleEventExecutionTriggered);
  LOG(INFO) << "coordination mode set to ASYNC.";
}

//! @brief 函数名：GetModuleEvent
//! @details 函数功能：获取模块事件
//!
//! @param[out] events 存储模块事件的向量
//! @return 模块事件数量
uint32_t AsyncStepper::GetModuleEvent(std::vector<ModuleEvent>& events) {
  uint64_t current_time = future_events_.top().time_point;
  std::string module_name;
  ModuleEventType event_type;
  while (future_events_.top().time_point == current_time) {
    future_events_.top().UnpackEvent(module_name, event_type);
    future_events_.pop();
    if (TXSIM_LIKELY(nodes_.find(module_name) != nodes_.end())) {
      events.emplace_back(module_name, event_type);
      if (event_type == kModuleEventExecutionTriggered) {
        const ModuleNode& node = nodes_[module_name];
        future_events_.emplace(module_name, current_time + node.response_time, kModuleEventExecutionFinished);
        future_events_.emplace(module_name, current_time + node.exec_period, kModuleEventExecutionTriggered);
      }
    }
  }
  return future_events_.top().time_point - current_time;
}


//! @brief 函数名：AddModuleNodes
//! @details 函数功能：添加模块节点
//!
//! @param[in] name 模块名称
//! @param[in] exec_period 模块执行周期
//! @param[in] response_time 模块响应时间
void PriorityStepper::AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                                     const std::string& group) {
  if (exec_period == 0) throw std::invalid_argument(name + " module exec period cannot be 0.");
  ModuleNode& node = nodes_[name];
  node.exec_period = exec_period;
  node.response_time = response_time;
}

//! @brief 函数名：RemoveModule
//! @details 函数功能：移除模块
//!
//! @param[in] name 模块名称
void PriorityStepper::RemoveModule(const std::string& name) {
  nodes_.erase(name);
}

//! @brief 函数名：SetTriggerModule
//! @details 函数功能：设置触发模块
//!
//! @param[in] name 模块名称
void PriorityStepper::SetTriggerModule(const std::string& name) {
  assert(nodes_.find(name) != nodes_.end());
  trigger_module_ = name;
}

//! @brief 函数名：TriggerAllModules
//! @details 函数功能：触发所有模块
void PriorityStepper::TriggerAllModules() {
  const FutureEvent& e = future_events_.top();
  uint64_t t = (e.event.type == kModuleEventExecutionTriggered) ? e.time_point
                                                                : (e.time_point - nodes_[trigger_module_].exec_period);
  for (auto& n : nodes_)
    if (n.first != trigger_module_) future_events_.emplace(n.first, t, kModuleEventExecutionTriggered);
}

//! @brief 函数名：ResetModuleTimes
//! @details 函数功能：重置模块时间
void PriorityStepper::ResetModuleTimes() {
  assert(!nodes_.empty());
  future_events_ = decltype(future_events_)();  // clear the queue.
  if (trigger_module_.empty())
    for (auto& n : nodes_) future_events_.emplace(n.first, 0, kModuleEventExecutionTriggered);
  else
    future_events_.emplace(trigger_module_, 0, kModuleEventExecutionTriggered);
  LOG(INFO) << "coordination mode set to ASYNC.";
}
//! @brief 函数名：GetModuleEvent
//! @details 函数功能：获取模块事件
//!
//! @param[out] events 存储模块事件的向量
//! @return 模块事件数量
uint32_t PriorityStepper::GetModuleEvent(std::vector<ModuleEvent>& events) {
  uint64_t current_time = future_events_.top().time_point;
  std::string module_name;
  ModuleEventType event_type;
  std::map<int16_t, std::vector<std::string>> mapPriority;
  while (future_events_.top().time_point == current_time) {
    future_events_.top().UnpackEvent(module_name, event_type);
    future_events_.pop();
    if (TXSIM_LIKELY(nodes_.find(module_name) != nodes_.end())) {
      const ModuleNode& node = nodes_[module_name];
      std::vector<std::string>& vecName = mapPriority[node.response_time];
      vecName.emplace_back(module_name);
      future_events_.emplace(module_name, current_time + node.exec_period, kModuleEventExecutionFinished);
    }
  }

  for (auto mapIt = mapPriority.begin(); mapIt != mapPriority.end(); mapIt++) {
    std::vector<std::string>& vecValue = mapIt->second;
    for (auto vecItem : vecValue) { events.emplace_back(vecItem, kModuleEventExecutionTriggered); }

    for (auto vecItem : vecValue) { events.emplace_back(vecItem, kModuleEventExecutionFinished); }
  }

  return future_events_.top().time_point - current_time;
}


}  // namespace coordinator
}  // namespace tx_sim

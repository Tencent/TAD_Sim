// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdint.h>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "status.h"
#include "utils/constant.h"

namespace tx_sim {
namespace coordinator {

//! @brief 枚举类型：ModuleEventType
//! @details 枚举类型功能：定义模块事件类型
enum ModuleEventType {
  //! @brief 枚举值：kModuleEventExecutionTriggered
  //! @details 枚举值功能：表示模块执行触发事件
  kModuleEventExecutionTriggered,

  //! @brief 枚举值：kModuleEventExecutionFinished
  //! @details 枚举值功能：表示模块执行完成事件
  kModuleEventExecutionFinished,
};

//! @brief 结构体：ModuleEvent
//! @details 结构体功能：定义模块事件信息
struct ModuleEvent {
  //! @brief 成员变量：module_name
  //! @details 成员变量功能：存储模块名称
  std::string module_name;

  //! @brief 成员变量：type
  //! @details 成员变量功能：存储模块事件类型
  ModuleEventType type;

  //! @brief 构造函数：ModuleEvent
  //! @details 构造函数功能：初始化模块事件信息
  //!
  //! @param[in] name 模块名称
  //! @param[in] event_type 模块事件类型
  ModuleEvent(const std::string& name, ModuleEventType event_type) : module_name(name), type(event_type) {}
};

//! @brief 类名：SimStepper
//! @details 类功能：定义模拟步进器接口
class SimStepper {
 public:
  //! @brief 函数名：AddModuleNodes
  //! @details 函数功能：添加模块节点
  //!
  //! @param[in] name 模块名称
  //! @param[in] exec_period 模块执行周期
  //! @param[in] response_time 模块响应时间
  virtual void AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                              const std::string& group = "") = 0;

  //! @brief 函数名：SetTriggerModule
  //! @details 函数功能：设置触发模块
  //!
  //! @param[in] name 模块名称
  virtual void SetTriggerModule(const std::string& name) = 0;

  //! @brief 函数名：TriggerAllModules
  //! @details 函数功能：触发所有模块
  virtual void TriggerAllModules() = 0;

  //! @brief 函数名：RemoveModule
  //! @details 函数功能：移除模块
  //!
  //! @param[in] name 模块名称
  virtual void RemoveModule(const std::string& name) = 0;

  //! @brief 函数名：ResetModuleTimes
  //! @details 函数功能：重置模块时间
  virtual void ResetModuleTimes() = 0;

  //! @brief 函数名：GetModuleEvent
  //! @details 函数功能：获取模块事件
  //!
  //! @param[out] events 存储模块事件的向量
  //! @return 模块事件数量
  virtual uint32_t GetModuleEvent(std::vector<ModuleEvent>& events) = 0;
};

//! @brief 类名：SyncStepper
//! @details 类功能：定义同步模拟步进器实现
class SyncStepper final : public SimStepper {
 public:
  //! @brief 函数名：AddModuleNodes
  //! @details 函数功能：添加模块节点
  //!
  //! @param[in] name 模块名称
  //! @param[in] exec_period 模块执行周期
  //! @param[in] response_time 模块响应时间
  void AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                      const std::string& group) override;
  //! @brief 函数名：SetTriggerModule
  //! @details 函数功能：设置触发模块
  //!
  //! @param[in] name 模块名称
  void SetTriggerModule(const std::string& name);

  //! @brief 函数名：TriggerAllModules
  //! @details 函数功能：触发所有模块
  void TriggerAllModules();

  //! @brief 函数名：RemoveModule
  //! @details 函数功能：移除模块
  //!
  //! @param[in] name 模块名称
  void RemoveModule(const std::string& name) override;

  //! @brief 函数名：ResetModuleTimes
  //! @details 函数功能：重置模块时间
  void ResetModuleTimes() override;

  //! @brief 函数名：GetModuleEvent
  //! @details 函数功能：获取模块事件
  //!
  //! @param[out] events 存储模块事件的向量
  //! @return 模块事件数量
  uint32_t GetModuleEvent(std::vector<ModuleEvent>& events) override;

 private:
  //! @brief 结构体：ModuleNode
  //! @details 结构体功能：定义模块节点信息
  struct ModuleNode {
    std::string name;
    uint32_t step_time;
    uint64_t passed_time;
    ModuleNode(const std::string& n, uint32_t t) : name(n), step_time(t), passed_time(0) {}
    ModuleNode(const ModuleNode& o) : name(o.name), step_time(o.step_time), passed_time(o.passed_time) {}
  };

  std::vector<ModuleNode> nodes_;
  uint64_t current_time_;
  uint32_t step_time_{0};  // gcd of step_time of each ModuleNode.
  std::unique_ptr<ModuleNode> trigger_module_;
  bool all_module_triggered_;
};

struct AsyncGroupModuleNode {
  AsyncGroupModuleNode() : passed_time(0) {}

  // GroupModuleNode(const std::string& n, uint32_t t) : name(n), step_time(t), passed_time(0) {}
  AsyncGroupModuleNode(const AsyncGroupModuleNode& o)
      : name(o.name), step_time(o.step_time), passed_time(o.passed_time) {
    sub_nodes_.insert(o.sub_nodes_.begin(), o.sub_nodes_.end());
  }

  void TriggerGroup();
  uint32_t GetGroupModuleEvent(std::vector<ModuleEvent>& events, bool& finished);

  struct SubModuleNode {
    std::string name;
    std::string group;
    uint32_t exec_period;
    uint32_t response_time;
    SubModuleNode() {}
    SubModuleNode(const std::string& n, const std::string& g) : name(n), exec_period(0), response_time(0), group(g) {}
    SubModuleNode(const SubModuleNode& o)
        : name(o.name), exec_period(o.exec_period), response_time(o.response_time), group(o.group) {}
  };

  struct FutureEvent {
    uint64_t time_point;
    ModuleEvent event;
    FutureEvent(const std::string& name, uint64_t next_time_point, ModuleEventType type)
        : time_point(next_time_point), event(name, type) {}
    void UnpackEvent(std::string& name, ModuleEventType& event_type) const {
      name = event.module_name;
      event_type = event.type;
    }
    // in a determined context, if there are modules which are subscribing and publishing on the same topics at a same
    // time point, we always assuming that the currently published messages are visible to others to ensuring the
    // reproducible-scenario invariant.
    // the Compare function used in priority queue returns true if the lhs pops after the rhs.
    bool operator<(const FutureEvent& rhs) const {
      if (time_point == rhs.time_point) {
        if (event.type == rhs.event.type)
          return event.module_name < rhs.event.module_name;
        else
          return (event.type == kModuleEventExecutionTriggered);
      } else
        return time_point > rhs.time_point;
    }
  };

  std::string name;
  uint32_t step_time;
  uint64_t passed_time;
  std::map<std::string, SubModuleNode> sub_nodes_;
  std::priority_queue<FutureEvent> future_events_;
};

// 组间同步 组内异步 完成
class IntergroupSyncStepper final : public SimStepper {
 public:
  void AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                      const std::string& group) override;
  void SetTriggerModule(const std::string& name);
  void TriggerAllModules();
  void RemoveModule(const std::string& name) override;
  void ResetModuleTimes() override;
  uint32_t GetModuleEvent(std::vector<ModuleEvent>& events) override;

 private:
  std::vector<AsyncGroupModuleNode> group_nodes_;
  uint64_t current_time_;
};

struct SyncGroupModuleNode {
  SyncGroupModuleNode() {}

  // GroupModuleNode(const std::string& n, uint32_t t) : name(n), step_time(t), passed_time(0) {}
  SyncGroupModuleNode(const SyncGroupModuleNode& o)
      : name(o.name), response_time(o.response_time), exec_period(o.exec_period) {
    sub_nodes_.insert(sub_nodes_.end(), o.sub_nodes_.begin(), o.sub_nodes_.end());
  }

  void TriggerGroup();
  uint32_t GetGroupModuleEvent(std::vector<ModuleEvent>& events, ModuleEventType event_type);

  struct SubModuleNode {
    std::string name;
    std::string group;
    uint32_t step_time;
    uint64_t passed_time;
    SubModuleNode() {}
    SubModuleNode(const std::string& n, uint32_t t) : name(n), step_time(t), passed_time(0) {}
    SubModuleNode(const SubModuleNode& o)
        : name(o.name), passed_time(o.passed_time), step_time(o.step_time), group(o.group) {}
  };

  std::string name;
  uint32_t response_time;
  uint64_t exec_period;
  uint64_t current_time_;
  std::vector<SubModuleNode> sub_nodes_;
};

// 组间异步 组内同步 todo
class IntragroupSyncStepper final : public SimStepper {
 public:
  void AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                      const std::string& group) override;
  void SetTriggerModule(const std::string& name);
  void TriggerAllModules();
  void RemoveModule(const std::string& name) override;
  void ResetModuleTimes() override;
  uint32_t GetModuleEvent(std::vector<ModuleEvent>& events) override;

 private:
  struct FutureEvent {
    uint64_t time_point;
    ModuleEvent event;
    FutureEvent(const std::string& name, uint64_t next_time_point, ModuleEventType type)
        : time_point(next_time_point), event(name, type) {}
    void UnpackEvent(std::string& name, ModuleEventType& event_type) const {
      name = event.module_name;
      event_type = event.type;
    }
    // in a determined context, if there are modules which are subscribing and publishing on the same topics at a same
    // time point, we always assuming that the currently published messages are visible to others to ensuring the
    // reproducible-scenario invariant.
    // the Compare function used in priority queue returns true if the lhs pops after the rhs.
    bool operator<(const FutureEvent& rhs) const {
      if (time_point == rhs.time_point) {
        if (event.type == rhs.event.type)
          return event.module_name < rhs.event.module_name;
        else
          return (event.type == kModuleEventExecutionTriggered);
      } else
        return time_point > rhs.time_point;
    }
  };

  std::map<std::string, SyncGroupModuleNode> group_nodes_;
  std::priority_queue<FutureEvent> future_events_;
};

class AsyncStepper final : public SimStepper {
 public:
  void AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                      const std::string& group) override;
  void SetTriggerModule(const std::string& name);
  void TriggerAllModules();
  void RemoveModule(const std::string& name) override;
  void ResetModuleTimes() override;
  uint32_t GetModuleEvent(std::vector<ModuleEvent>& events) override;

 private:
  //! @brief 结构体：ModuleNode
  //! @details 结构体功能：定义模块节点信息
  struct ModuleNode {
    uint32_t exec_period;
    uint32_t response_time;
  };

  //! @brief 结构体：FutureEvent
  //! @details 结构体功能：定义未来事件信息
  struct FutureEvent {
    uint64_t time_point;
    ModuleEvent event;
    FutureEvent(const std::string& name, uint64_t next_time_point, ModuleEventType type)
        : time_point(next_time_point), event(name, type) {}
    void UnpackEvent(std::string& name, ModuleEventType& event_type) const {
      name = event.module_name;
      event_type = event.type;
    }
    // in a determined context, if there are modules which are subscribing and publishing on the same topics at a same
    // time point, we always assuming that the currently published messages are visible to others to ensuring the
    // reproducible-scenario invariant.
    // the Compare function used in priority queue returns true if the lhs pops after the rhs.
    bool operator<(const FutureEvent& rhs) const {
      if (time_point == rhs.time_point)
        return event.type == kModuleEventExecutionTriggered;
      else
        return time_point > rhs.time_point;
    }
  };

  //! @brief 成员变量：nodes_
  //! @details 成员变量功能：存储模块节点信息
  std::map<std::string, ModuleNode> nodes_;

  //! @brief 成员变量：future_events_
  //! @details 成员变量功能：存储未来事件信息
  std::priority_queue<FutureEvent> future_events_;

  //! @brief 成员变量：trigger_module_
  //! @details 成员变量功能：存储触发模块信息
  std::string trigger_module_;
};

//! @brief 类名：PriorityStepper
//! @details 类功能：定义优先级模拟步进器实现
class PriorityStepper final : public SimStepper {
 public:
  //! @brief 函数名：AddModuleNodes
  //! @details 函数功能：添加模块节点
  //!
  //! @param[in] name 模块名称
  //! @param[in] exec_period 模块执行周期
  //! @param[in] response_time 模块响应时间
  void AddModuleNodes(const std::string& name, uint32_t exec_period, uint32_t response_time,
                      const std::string& group) override;
  //! @brief 函数名：SetTriggerModule
  //! @details 函数功能：设置触发模块
  //!
  //! @param[in] name 模块名称
  void SetTriggerModule(const std::string& name);

  //! @brief 函数名：TriggerAllModules
  //! @details 函数功能：触发所有模块
  void TriggerAllModules();

  //! @brief 函数名：RemoveModule
  //! @details 函数功能：移除模块
  //!
  //! @param[in] name 模块名称
  void RemoveModule(const std::string& name) override;

  //! @brief 函数名：ResetModuleTimes
  //! @details 函数功能：重置模块时间
  void ResetModuleTimes() override;

  //! @brief 函数名：GetModuleEvent
  //! @details 函数功能：获取模块事件
  //!
  //! @param[out] events 存储模块事件的向量
  //! @return 模块事件数量
  uint32_t GetModuleEvent(std::vector<ModuleEvent>& events) override;

 private:
  //! @brief 结构体：ModuleNode
  //! @details 结构体功能：定义模块节点信息
  struct ModuleNode {
    uint32_t exec_period;
    uint32_t response_time;
  };

  //! @brief 结构体：FutureEvent
  //! @details 结构体功能：定义未来事件信息
  struct FutureEvent {
    uint64_t time_point;
    ModuleEvent event;
    FutureEvent(const std::string& name, uint64_t next_time_point, ModuleEventType type)
        : time_point(next_time_point), event(name, type) {}
    void UnpackEvent(std::string& name, ModuleEventType& event_type) const {
      name = event.module_name;
      event_type = event.type;
    }
    // in a determined context, if there are modules which are subscribing and publishing on the same topics at a same
    // time point, we always assuming that the currently published messages are visible to others to ensuring the
    // reproducible-scenario invariant.
    // the Compare function used in priority queue returns true if the lhs pops after the rhs.
    bool operator<(const FutureEvent& rhs) const { return time_point > rhs.time_point; }
  };

  //! @brief 成员变量：nodes_
  //! @details 成员变量功能：存储模块节点信息
  std::map<std::string, ModuleNode> nodes_;

  //! @brief 成员变量：future_events_
  //! @details 成员变量功能：存储未来事件信息
  std::priority_queue<FutureEvent> future_events_;

  //! @brief 成员变量：trigger_module_
  //! @details 成员变量功能：存储触发模块信息
  std::string trigger_module_;
};

inline std::unique_ptr<SimStepper> CreateSimStepper(tx_sim::impl::CoordinationMode mode) {
  switch (mode) {
    case tx_sim::impl::kCoordinateSync:
      return std::unique_ptr<SimStepper>(new SyncStepper);
    case tx_sim::impl::kCoordinateAsync:
      return std::unique_ptr<SimStepper>(new AsyncStepper);
    case tx_sim::impl::kCoordinatePriority:
      return std ::unique_ptr<SimStepper>(new PriorityStepper);
    default:
      return nullptr;  // already checked in LoadCoordinatorConfig().
  }
}

}  // namespace coordinator
}  // namespace tx_sim

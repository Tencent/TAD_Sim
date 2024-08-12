#pragma once

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "txsim_module.h"

class MyModule final : public tx_sim::SimModule {
 public:
  MyModule();
  virtual ~MyModule();

  void Init(tx_sim::InitHelper& helper) override;
  void Reset(tx_sim::ResetHelper& helper) override;
  void Step(tx_sim::StepHelper& helper) override;
  void Stop(tx_sim::StopHelper& helper) override;

 private:
  void chatterCallback(const std_msgs::String::ConstPtr& msg);
  std::string name_;
  size_t step_count_ = 0, max_step_count_ = 100;
  double start_x_ = 0, start_y_ = 0, cur_x_ = 0, cur_y_ = 0, step_velocity_ = 0.00000001, last_timestamp_ = 0;
  std::string payload_;
  bool puber_;
  ros::NodeHandle m_ros_node_handle;
  std_msgs::String::ConstPtr m_ros_msg;
};

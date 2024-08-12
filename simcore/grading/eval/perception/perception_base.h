// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "eval/eval_base.h"

#include <memory>

#define Perception_Connect(a, b) a##b
#define Perception_ToString(a) #a
#define Define_Perception_Attribute(prefix, suffix)                                      \
  const std::string g_const##prefix##_##suffix = Perception_ToString(prefix##_##suffix); \
  const std::string g_const##prefix##_##suffix##_error = Perception_ToString(prefix##_##suffix##_error);

namespace eval {
// plot for one tracked object
struct TrackedObjPlot;
using TrackedObjPlotPtr = std::shared_ptr<TrackedObjPlot>;

// capture of perception objects
struct TrackedObjPlot {
 public:
  const float const_time_duration_thresh = 1.0;  // above this thresh will be added to report
  uint64_t tracked_id;
  sim_msg::TestReport_XYPlot x_plot;
  sim_msg::TestReport_XYPlot x_error_plot;
  sim_msg::TestReport_XYPlot y_plot;
  sim_msg::TestReport_XYPlot y_error_plot;
  sim_msg::TestReport_XYPlot vx_plot;
  sim_msg::TestReport_XYPlot vx_error_plot;
  sim_msg::TestReport_XYPlot vy_plot;
  sim_msg::TestReport_XYPlot vy_error_plot;
  sim_msg::TestReport_XYPlot yaw_plot;
  sim_msg::TestReport_XYPlot yaw_error_plot;
  sim_msg::TestReport_XYPlot ax_plot;
  sim_msg::TestReport_XYPlot ax_error_plot;
  sim_msg::TestReport_XYPlot ay_plot;
  sim_msg::TestReport_XYPlot ay_error_plot;

  explicit TrackedObjPlot(uint64_t id = 0) : tracked_id(id) {
    ReportHelper::ConfigXYPlot(x_plot, "x_dist", "x dist in flu of tracked object and truth", "time", "s",
                               {"x tracked", "x truth"}, {"m", "m"}, 2);
    ReportHelper::ConfigXYPlot(x_error_plot, "x_dist_error", "x dist error in flu of tracked object and truth", "time",
                               "s", {"x_dist_error"}, {"%"});

    ReportHelper::ConfigXYPlot(y_plot, "y_dist", "y dist in flu of tracked object and truth", "time", "s",
                               {"y tracked", "y truth"}, {"m", "m"}, 2);
    ReportHelper::ConfigXYPlot(y_error_plot, "y_dist_error", "y dist error in flu of tracked object and truth", "time",
                               "s", {"y_dist_error"}, {"%"});

    ReportHelper::ConfigXYPlot(vx_plot, "vx_dist", "vx in flu of tracked object and truth", "time", "s",
                               {"vx tracked", "vx truth"}, {"m|s", "m|s"}, 2);
    ReportHelper::ConfigXYPlot(vx_error_plot, "vx_error", "vx error in flu of tracked object and truth", "time", "s",
                               {"vx_error"}, {"%"});

    ReportHelper::ConfigXYPlot(vy_plot, "vy_dist", "vy in flu of tracked object and truth", "time", "s",
                               {"vy tracked", "vy truth"}, {"m|s", "m|s"}, 2);
    ReportHelper::ConfigXYPlot(vy_error_plot, "vy_error", "vy error in flu of tracked object and truth", "time", "s",
                               {"vy_error"}, {"%"});

    ReportHelper::ConfigXYPlot(yaw_plot, "yaw", "yaw in flu of tracked object and truth", "time", "s",
                               {"yaw tracked", "yaw truth"}, {"rad", "rad"}, 2);
    ReportHelper::ConfigXYPlot(yaw_error_plot, "yaw_error", "yaw error in flu of tracked object and truth", "time", "s",
                               {"yaw_error"}, {"%"});

    ReportHelper::ConfigXYPlot(ax_plot, "ax_dist", "ax in flu of tracked object and truth", "time", "s",
                               {"ax tracked", "ax truth"}, {"m|s2", "m|s2"}, 2);
    ReportHelper::ConfigXYPlot(ax_error_plot, "ax_error", "ax error in flu of tracked object and truth", "time", "s",
                               {"ax_error"}, {"%"});

    ReportHelper::ConfigXYPlot(ay_plot, "ay_dist", "ay in flu of tracked object and truth", "time", "s",
                               {"ay tracked", "ay truth"}, {"m|s2", "m|s2"}, 2);
    ReportHelper::ConfigXYPlot(ay_error_plot, "ay_error", "ay error in flu of tracked object and truth", "time", "s",
                               {"ay_error"}, {"%"});
  }
  virtual ~TrackedObjPlot() {}

  void AddDataToPlot(double t_s, double x, double x_truth, double y, double y_truth, double vx, double vx_truth,
                     double vy, double vy_truth, double yaw, double yaw_truth, double ax = 0.0, double ax_truth = 0.0,
                     double ay = 0.0, double ay_truth = 0.0) {
    double error_ratio = 0.0;

    x_plot.mutable_x_axis()->add_axis_data(t_s);
    x_plot.mutable_y_axis()->at(0).add_axis_data(x);
    x_plot.mutable_y_axis()->at(1).add_axis_data(x_truth);

    x_error_plot.mutable_x_axis()->add_axis_data(t_s);
    error_ratio = std::abs(x_truth) < 1e-3 ? 0.0 : (x_truth - x) / x_truth * 100.0;
    x_error_plot.mutable_y_axis()->at(0).add_axis_data(error_ratio);

    y_plot.mutable_x_axis()->add_axis_data(t_s);
    y_plot.mutable_y_axis()->at(0).add_axis_data(y);
    y_plot.mutable_y_axis()->at(1).add_axis_data(y_truth);

    error_ratio = std::abs(y_truth) < 1e-3 ? 0.0 : (y_truth - y) / y_truth * 100.0;
    y_error_plot.mutable_y_axis()->at(0).add_axis_data(error_ratio);

    vx_plot.mutable_x_axis()->add_axis_data(t_s);
    vx_plot.mutable_y_axis()->at(0).add_axis_data(vx);
    vx_plot.mutable_y_axis()->at(1).add_axis_data(vx_truth);

    vx_error_plot.mutable_x_axis()->add_axis_data(t_s);
    error_ratio = std::abs(vx_truth) < 1e-3 ? 0.0 : (vx_truth - vx) / vx_truth * 100.0;
    vx_error_plot.mutable_y_axis()->at(0).add_axis_data(error_ratio);

    vy_plot.mutable_x_axis()->add_axis_data(t_s);
    vy_plot.mutable_y_axis()->at(0).add_axis_data(vy);
    vy_plot.mutable_y_axis()->at(1).add_axis_data(vy_truth);

    error_ratio = std::abs(vy_truth) < 1e-3 ? 0.0 : (vy_truth - vy) / vy_truth * 100.0;
    vy_error_plot.mutable_y_axis()->at(0).add_axis_data(error_ratio);

    yaw_plot.mutable_x_axis()->add_axis_data(t_s);
    yaw_plot.mutable_y_axis()->at(0).add_axis_data(yaw);
    yaw_plot.mutable_y_axis()->at(1).add_axis_data(yaw_truth);

    error_ratio = std::abs(yaw_truth) < 1e-3 ? 0.0 : (yaw_truth - yaw) / yaw_truth * 100.0;
    yaw_error_plot.mutable_y_axis()->at(0).add_axis_data(error_ratio);

    ax_plot.mutable_x_axis()->add_axis_data(t_s);
    ax_plot.mutable_y_axis()->at(0).add_axis_data(ax);
    ax_plot.mutable_y_axis()->at(1).add_axis_data(ax_truth);

    ax_error_plot.mutable_x_axis()->add_axis_data(t_s);
    error_ratio = std::abs(ax_truth) < 1e-3 ? 0.0 : (ax_truth - ax) / ax_truth * 100.0;
    ax_error_plot.mutable_y_axis()->at(0).add_axis_data(error_ratio);

    ay_plot.mutable_x_axis()->add_axis_data(t_s);
    ay_plot.mutable_y_axis()->at(0).add_axis_data(ay);
    ay_plot.mutable_y_axis()->at(1).add_axis_data(ay_truth);

    error_ratio = std::abs(ay_truth) < 1e-3 ? 0.0 : (ay_truth - ay) / ay_truth * 100.0;
    ay_error_plot.mutable_y_axis()->at(0).add_axis_data(error_ratio);
  }
};

// capture of perception objects
struct TrackedTrafficLightPlot {
 public:
  uint64_t tracked_id;
  sim_msg::TestReport_XYPlot tl_plot;

  explicit TrackedTrafficLightPlot(uint64_t id = 0) : tracked_id(id) {
    ReportHelper::ConfigXYPlot(tl_plot, "traffic light", "tracked traffic light", "time", "s", {"color"},
                               {"U-RGB[0-123]"}, 1);
  }
  virtual ~TrackedTrafficLightPlot() {}

  void AddDataToPlot(double t_s, uint32_t color = 0) {
    tl_plot.mutable_x_axis()->add_axis_data(t_s);
    tl_plot.mutable_y_axis()->at(0).add_axis_data(color);
  }
};

using TrackedTrafficLightPlotPtr = std::shared_ptr<TrackedTrafficLightPlot>;
}  // namespace eval

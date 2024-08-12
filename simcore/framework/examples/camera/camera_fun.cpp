#include "camera_fun.h"
#include <assert.h>
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <thread>

void CameraFun::Init(tx_sim::InitHelper& helper) {
  // get user defined initiation parameters.
  // if we defined the parameters in TADSim UI, override the default values here.

  // by subscribe our interested topics, we expect that the two corresponding messages which defined by
  // traffic.proto and location.proto would received in every Step callback.
  std::cout << "Init...\n";

  std::string value = helper.GetParameter("SourceSim");
  if (value.empty()) {
    std::cout << "Need sim camera address" << std::endl;
    return;
  }
  std::cout << "SourceSim = " << value << std::endl;
  src_sim.init(value, 10);

  value = helper.GetParameter("Width");
  if (value.empty()) {
    std::cout << "Need image width" << std::endl;
    return;
  }
  w = std::atoi(value.c_str());
  std::cout << "Width = " << w << std::endl;

  value = helper.GetParameter("Height");
  if (value.empty()) {
    std::cout << "Need image height" << std::endl;
    return;
  }
  h = std::atoi(value.c_str());
  std::cout << "Height = " << h << std::endl;

  cvImg = cv::Mat(h, w, CV_8UC4);
  cv::namedWindow("camera", 0);
  std::cout << "Init ok.\n";
}

void CameraFun::Reset(tx_sim::ResetHelper& helper) {
  std::cout << "Reseting...\n";
  // here we could get some information(e.g. the map which the scenario runs on, suggested local origin coordinate
  // and the destination of the ego car) of the current scenario.

  src_sim.reset();

  std::cout << "Reset ok.\n";
}

void CameraFun::Step(tx_sim::StepHelper& helper) {
  std::vector<uint8_t> buffer_sim;
  int64_t timestamp_sim;
  if (!src_sim.read(buffer_sim, timestamp_sim)) {
    std::cout << "Cannot read buffer" << std::endl;
    return;
  }
  std::cout << "Timestamp[" << helper.timestamp() << "]: " << timestamp_sim << std::endl;
  memcpy(cvImg.data, buffer_sim.data(), w * h * 4);

  cv::imshow("camera", cvImg);
  cv::waitKey(2);
}

void CameraFun::Stop(tx_sim::StopHelper& helper) { std::cout << "CameraFun stopped." << std::endl; }
TXSIM_MODULE(CameraFun)

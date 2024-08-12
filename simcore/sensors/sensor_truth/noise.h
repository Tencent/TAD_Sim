// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <map>
#include <random>
#include <string>
#include <vector>
#include "traffic.pb.h"

class NoiseDrop {
 public:
  NoiseDrop() = default;
  ~NoiseDrop() = default;
  void init(double a, double b, double c, double d, double sigma = 1, double pmax = 1);
  bool is_drop(double dis);

 private:
  double a = 0, b = 0, c = 0, d = 0;
  double sigma = 0, pmax = 0;
  std::default_random_engine generator;
  std::normal_distribution<double> gauss;
  std::uniform_real_distribution<double> normal;
};

class NoiseJitter {
 public:
  NoiseJitter() = default;
  ~NoiseJitter() = default;
  void init(double a, double b, double c, double d);
  double jitter(double dis);

 private:
  double a = 0, b = 0, c = 0, d = 0;
  std::default_random_engine generator;
};

class Noise {
 public:
  struct Data {
    int64_t id = 0;
    double time = 0;
    double distance = 0;
    double x = 0, y = 0, z = 0;
    double wid = 0, hei = 0, len = 0;
    double vx = 0, vy = 0, vz = 0;
  };

  Noise() = default;
  ~Noise() = default;
  bool good() const { return hasDrop > 0 || hasSize > 0 || hasStay > 0 || hasPos > 0 || hasVeo > 0; }

  bool initDrop(const std::string &str);
  bool initStay(const std::string &str);
  bool initPosition(const std::string &str);
  bool initSize(const std::string &str);
  bool initVeolity(const std::string &str);
  void setAge(double a) { age = a; }

  /// @brief add noise on data
  /// @param data : out with noise
  /// @return false if droped
  bool noise(Data &data);

 private:
  std::vector<double> ParseABCD(const std::string &str);
  double age = 100;
  std::map<int64_t, Data> cache;
  double hasDrop = 0, hasStay = 0, hasPos = 0, hasSize = 0, hasVeo = 0;
  NoiseDrop drop;
  NoiseDrop stay;
  NoiseJitter position;
  NoiseJitter size;
  NoiseJitter veolity;
};

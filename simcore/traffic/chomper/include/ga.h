// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <math.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
namespace gautils {
using IndividualDataPtr = std::shared_ptr<void>;
template <typename T>
T *getRawData(const IndividualDataPtr &data) {
  return static_cast<T *>(data.get());
}
class Individual {
 public:
  IndividualDataPtr data_;
  double fitness_;
};

class Population {
 public:
  std::vector<Individual> datas_;
  double total_fitness_;
};

class GAHolder {
 public:
  virtual void init_population(Population &population, int size) = 0;
  virtual void mutate(Individual &d, double rate) = 0;
  virtual double fitness_function(const Individual &d) = 0;
  virtual bool covergence(const Individual &d) = 0;
  virtual void cross(Individual &da, Individual &db) = 0;
  virtual void deep_copy(const Individual &src, Individual &dst) = 0;

 public:
  virtual void init(const std::string &file) {}
  virtual int best(const Population &population) {
    int idx = 0;
    double max = 0;
    for (size_t i = 0; i < population.datas_.size(); i++) {
      if (population.datas_[i].fitness_ > max) {
        idx = i;
        max = population.datas_[i].fitness_;
      }
    }
    return idx;
  }

  virtual std::string tostring(const Individual &d) { return ""; }
  virtual void deep_copy(const Population &population, Population &population_copy) {
    population_copy.datas_.clear();
    population_copy.total_fitness_ = population.total_fitness_;
    size_t pop_size = population.datas_.size();
    for (size_t i = 0; i < pop_size; i++) {
      Individual dst;
      deep_copy(population.datas_[i], dst);
      population_copy.datas_.emplace_back(dst);
    }
  }
};

class GA {
 public:
  int init(const std::string &file, GAHolder *ptr);

  void update_population(Population &population);

  void init_population(Population &population);
  void evolve();

  double fitness_function(const Individual &a);

  Individual select(Population &population);

  void cross(Individual &a, Individual &b);

  void mutate(Individual &a);

 protected:
  int generations_;
  int population_size_;
  double cross_rate_;
  double mutate_rate_;
  GAHolder *holder_;
};
}  // namespace gautils

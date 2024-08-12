// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "ga.h"
#include <glog/logging.h>
#include "world_manager.h"

namespace gautils {
int GA::init(const std::string &file, GAHolder *ptr) {
  holder_ = ptr;
  generations_ = 201;
  population_size_ = 100;
  cross_rate_ = 0.8;
  mutate_rate_ = 0.003;
  return 0;
}

void GA::update_population(Population &population) {
  Population population_new;
  population_new.total_fitness_ = 0;
  size_t pop_size = population.datas_.size();
  for (size_t i = 0; i < pop_size; i++) {
    auto a = select(population);
    auto b = select(population);
    cross(a, b);
    mutate(a);
    mutate(b);
    a.fitness_ = fitness_function(a);
    b.fitness_ = fitness_function(b);
    population_new.datas_.emplace_back(a);
    population_new.total_fitness_ += a.fitness_;
    if (population_new.datas_.size() >= pop_size) break;
    population_new.datas_.emplace_back(b);
    population_new.total_fitness_ += b.fitness_;
    if (population_new.datas_.size() >= pop_size) break;
  }
  population = population_new;
}

void GA::init_population(Population &population) { holder_->init_population(population, population_size_); }

double GA::fitness_function(const Individual &a) { return holder_->fitness_function(a); }

Individual GA::select(Population &population) {
  std::vector<double> fitness;
  double fitness_accumulate = 0;
  for (auto &i : population.datas_) {
    fitness.emplace_back(i.fitness_ / population.total_fitness_);
    fitness_accumulate += fitness.back();
  }
  for (auto &i : fitness) {
    i /= fitness_accumulate;
  }
  double p = static_cast<double>(rand()) / RAND_MAX;
  int idx = 0;
  while (p > 0) {
    p -= fitness[idx];
    idx++;
  }
  idx--;
  Individual dst;
  holder_->deep_copy(population.datas_[idx], dst);
  return dst;
}

void GA::cross(Individual &a, Individual &b) {
  auto prob = static_cast<double>(rand()) / RAND_MAX;
  if (prob > cross_rate_) {
    return;
  }
  holder_->cross(a, b);
}

void GA::mutate(Individual &a) {
  holder_->mutate(a, mutate_rate_);
  return;
  auto prob = static_cast<double>(rand()) / RAND_MAX;
  if (prob > mutate_rate_) {
    return;
  }
  holder_->mutate(a, mutate_rate_);
}
void GA::evolve() {
  Population population;
  init_population(population);
  int idx = 0;
  bool bcovergence = false;
  int iter = 0;
  for (int i = 0; i < generations_; i++) {
    iter = i;

    idx = holder_->best(population);
    // if (i % 10 == 0) {
    auto str = holder_->tostring(population.datas_[idx]);
    LOG(INFO) << "iter=" << iter << " best idx=" << idx << " fitness=" << population.datas_[idx].fitness_
              << " tostring=" << str << std::endl;
    std::stringstream ss;
    ss << "ga_args_" << i;
    WorldManager::instance().save_ga_args(str, ss.str());
    // }
    if (holder_->covergence(population.datas_[idx])) {
      bcovergence = true;
      break;
    }
    update_population(population);
  }

  if (bcovergence) {
    LOG(INFO) << "iter=" << iter << " pop_idx=" << idx << " tostring=" << holder_->tostring(population.datas_[idx])
              << std::endl;
  } else {
    LOG(INFO) << "iter=" << iter << " best_idx=" << idx << " tostring=" << holder_->tostring(population.datas_[idx])
              << std::endl;
  }
  auto str = holder_->tostring(population.datas_[idx]);

  WorldManager::instance().save_ga_args(str, "ga_args.txt");
}
};  // namespace gautils

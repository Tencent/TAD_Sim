// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <algorithm>
#include <iterator>
#include <random>
#include <sstream>
#include <vector>
#include "tx_header.h"

TX_NAMESPACE_OPEN(Base)

class PseudoRandomDiscreteDistribution {
  enum { random_seed = 55 };
  enum { ditw_probability_sample_size = 10000 };

 public:
  /**
   * @brief Generate a probability vector with the given mean and variance
   *
   * Generate a vector of txFloat values that follow a normal distribution with the given mean and variance.
   *
   * @param mean The mean of the normal distribution
   * @param variance The variance of the normal distribution
   * @return A vector of txFloat values generated from the normal distribution
   */
  static std::vector<txFloat> GenerateProbabilityVector(const txFloat mean, const txFloat variance) TX_NOEXCEPT {
    std::mt19937 rng(random_seed);
    // values near the mean are the most likely
    // standard deviation affects the dispersion of generated values from the mean
    std::normal_distribution<> distribution{mean, variance};

    std::vector<txFloat> retVec;
    retVec.resize(ditw_probability_sample_size);
    for (txInt i = 0; i < ditw_probability_sample_size; ++i) {
      retVec[i] = distribution(rng);
    }
    /*std::ostringstream oss;
    std::copy(retVec.begin(), retVec.end(), std::ostream_iterator< txInt>(oss, ","));
    LOG(INFO) << "Generate Probability : " << oss.str();*/ /*txPAUSE;*/
    return std::move(retVec);
  }

  /**
   * @brief Generate a probability vector with the given mean and variance
   *
   * This function generates a vector of txInt values that follow a normal distribution with the given mean and
   * variance.
   *
   * @param inpute_distribution The input distribution values
   * @param nSeed The seed for the random number generator
   * @return A vector of txInt values generated from the normal distribution
   */
  static std::vector<txInt> GenerateProbabilityVector(const std::vector<txInt>& inpute_distribution,
                                                      const txInt nSeed) TX_NOEXCEPT {
    std::mt19937 rng(nSeed);
    std::discrete_distribution<txInt> distribution(inpute_distribution.begin(), inpute_distribution.end());

    std::vector<txInt> retVec;
    retVec.resize(ditw_probability_sample_size);
    for (txInt i = 0; i < ditw_probability_sample_size; ++i) {
      retVec[i] = distribution(rng);
    }
    /*std::ostringstream oss;
    std::copy(retVec.begin(), retVec.end(), std::ostream_iterator< txInt>(oss, ","));
    LOG(INFO) << "Generate Probability : " << oss.str();*/ /*txPAUSE;*/
    return std::move(retVec);
  }

  /**
   * @brief Generate a probability vector with the given input distribution
   *
   * This function generates a vector of txInt values that follow a probability distribution specified by the
   * inpute_distribution vector.
   *
   * @param inpute_distribution The input probability distribution vector
   * @param nSeed The seed for the random number generator
   * @return A vector of txInt values generated from the probability distribution
   */
  static std::vector<txInt> GenerateProbabilityVector(const std::vector<txInt>& inpute_distribution) TX_NOEXCEPT {
    return GenerateProbabilityVector(inpute_distribution, random_seed);
  }

  /**
   * @brief 根据输入概率分布生成新的随机概率分布
   *
   * 此函数根据输入的概率分布，生成一个新的概率分布。其中，所有相邻的概率数值不相同。
   *
   * @param inpute_distribution 输入的概率分布向量
   * @param nSeed 伪随机数生成器的种子
   * @return 返回一个新的随机概率分布向量
   */
  static std::vector<txInt> GenerateProbabilityVectorWithoutNeighborSame(const std::vector<txInt>& inpute_distribution,
                                                                         const Base::txInt nSeed) TX_NOEXCEPT {
    if (1 == inpute_distribution.size()) {
      return GenerateProbabilityVector(inpute_distribution, nSeed);
    } else if (inpute_distribution.size() > 1) {
      std::mt19937 rng(nSeed);
      std::discrete_distribution<txInt> distribution(inpute_distribution.begin(), inpute_distribution.end());

      std::vector<txInt> retVec;
      retVec.reserve(ditw_probability_sample_size);
      retVec.push_back(distribution(rng));
      while (retVec.size() < ditw_probability_sample_size) {
        txInt res = distribution(rng);
        if (res != retVec.back()) {
          retVec.push_back(res);
        }
      }
      return std::move(retVec);
    } else {
      LOG(FATAL) << TX_VARS(inpute_distribution.size());
      return std::vector<txInt>();
    }
  }
};

TX_NAMESPACE_CLOSE(Base)

// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <algorithm>
#include <iterator>
#include <random>
#include <sstream>
#include <vector>
#include "tx_header.h"
#include "tx_tc_gflags.h"

TX_NAMESPACE_OPEN(Base)

class DiscreteDistribution {
 public:
  /**
   * @brief 根据输入的分布生成指定大小的随机概率向量
   *
   * 该函数使用输入的分布生成指定大小的随机概率向量。通过STL库中的“random_device”和“default_random_engine”
   * 生成随机数，使用“discrete_distribution”根据输入的概率分布生成随机样本。
   *
   * @param inpute_distribution 输入的概率分布向量
   * @return std::vector< txInt > 返回生成的随机概率向量
   */
  static std::vector<txInt> GenerateProbabilityVector(const std::vector<txInt>& inpute_distribution) TX_NOEXCEPT {
    // LOG(INFO) << TXST_TRACE_VARIABLES(FLAGS_tc_probability_sample_size) <<
    // TXST_TRACE_VARIABLES_NAME(input_distribution_size, inpute_distribution.size());
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::discrete_distribution<txInt> distribution(inpute_distribution.begin(), inpute_distribution.end());

    std::vector<txInt> retVec;
    retVec.resize(FLAGS_tc_probability_sample_size);
    for (txInt i = 0; i < FLAGS_tc_probability_sample_size; ++i) {
      retVec[i] = distribution(generator);
    }
    /*std::ostringstream oss;
    std::copy(retVec.begin(), retVec.end(), std::ostream_iterator< txInt>(oss, ","));
    LOG(INFO) << "Generate Probability : " << oss.str(); txPAUSE;*/
    return retVec;
  }
};

TX_NAMESPACE_CLOSE(Base)

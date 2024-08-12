// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <string.h>
#include <time.h>
#include <sstream>
#include <string>
#include <vector>
#include "defines.h"
#define MAX_V_OBS_COUNT 500
#define TRAFFIC_GA_POP_SIZE 11  // population size,最后一个永远存这代的最优解，其他1,2,3,4....99再去crossover mutation

struct Chrome_Data {
  char value[12];
};
struct TT_Feature {
  float tt_obs_;
  float tt_obs_distance;
  std::vector<float> v_obs_;

  TT_Feature() { clear(); }
  void clear() {
    tt_obs_distance = 0.0f;
    tt_obs_ = 0.0f;
    v_obs_.clear();
  }
  std::string tostring() {
    std::stringstream ss;
    ss << "{TT_Feature|tt_obs_:" << tt_obs_ << ",tt_obs_distance:" << tt_obs_distance << ",count:" << v_obs_.size();
    for (auto &item : v_obs_) {
      ss << " " << item;
    }
    ss << "}";
    return ss.str();
  }

  void fromfile(const std::string &filename);
  void tofile(const std::string &filename) const;
};

struct DrivingData {
  float cc0_;
  float cc1_;
  float cc2_;
  std::string tostring() {
    std::stringstream ss;
    ss << "{DrivingData|cc0_:" << cc0_ << ",cc1_:" << cc1_ << ",cc2_:" << cc2_ << "}";
    return ss.str();
  }

  void fromfile(const std::string &filename) {
    FILE *pFile = fopen(filename.c_str(), "r+");
    if (pFile) {
      fscanf(pFile, "%f, %f, %f\n", &cc0_, &cc1_, &cc2_);
      fclose(pFile);
    }
  }
  void tofile(const std::string &filename) const {
    FILE *pFile = fopen(filename.c_str(), "w+");
    if (pFile) {
      fprintf(pFile, "%f, %f, %f\n", cc0_, cc1_, cc2_);
      fclose(pFile);
    }
  }
};
class WorldManager;
class TrafficGA {
 public:
  TrafficGA();
  /*!
  @brief   通过仿真器进行更新。
  */
  bool Update(const float *V_Obs, const unsigned int &V_ObsCount, float TT_Obs, float TT_ObsDistance,
              WorldManager *pWorldManager);
  void GetTrainningData(float &cc0, float &cc1, float &cc2) { cc0 = m_CC0, cc1 = m_CC1, cc2 = m_CC2; }
  unsigned int DecodeChrome(Chrome_Data data, unsigned int startIndex, unsigned int length);
  /*!
  @brief    损失函数。
  */
  float FitnessFunction(float TT_Obs, float TT_Output, const float *V_Obs, unsigned int V_ObsCount, float *V_Output);

 private:
  /*!
  @brief    选择操作。从旧群体中以一定概率选择优良个体组成新的种群，以繁殖得到下一代个体。
  */
  unsigned int Roulette_Wheel_Selection();

  /*!
  @brief
  交叉操作是指从种群中随机选择两个个体，通过两个染色体的交换组合，把父串的优秀特征遗传给子串，从而产生新的优秀个体。
  @param    Parents1  父基因.
  @param    Parents2  母基因.
  @param    Children  子基因.
  */
  void CrossOver(Chrome_Data *Parents1, Chrome_Data *Parents2, Chrome_Data *Children);

  /*!
  @brief    为了防止遗传算法在优化过程中陷入局部最优解，在搜索过程中，需要对个体进行变异。
  @param    Population  群体.
  */
  void Mutation(Chrome_Data *Population);

  /*!
  @brief    单体变异
  */
  void MutationOne(Chrome_Data *ChromeTarget, unsigned int length);

  /*!
  @brief    获取参数
  @param    cc0  参数0.
  @param    cc1  参数1.
  @param    cc2  参数2.
  */
  void GetResult(float &cc0, float &cc1, float &cc2);

 private:
  double F_Total;
  double F_Max, F_Min;
  unsigned int Max_F[3];
  unsigned int Max_F_Current[3];
  unsigned int Min_F[3];
  double F[TRAFFIC_GA_POP_SIZE];
  Chrome_Data Chrome[TRAFFIC_GA_POP_SIZE];
  Chrome_Data Parents[TRAFFIC_GA_POP_SIZE];
  Chrome_Data Children[TRAFFIC_GA_POP_SIZE];
  float cc0[TRAFFIC_GA_POP_SIZE];
  float cc1[TRAFFIC_GA_POP_SIZE];
  float cc2[TRAFFIC_GA_POP_SIZE];
  unsigned int ChromeLength;
  unsigned int ChromeLength1;
  unsigned int ChromeLength2;
  unsigned int ChromeLength3;
  unsigned int Children_Index;
  double MAX_LastIteration;
  unsigned int Convergence;
  unsigned int Father_Index;
  unsigned int Mother_Index;
  float m_CC0, m_CC1, m_CC2;
};
#include "ga.h"

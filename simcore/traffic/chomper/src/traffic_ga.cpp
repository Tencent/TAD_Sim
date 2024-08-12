// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "traffic_ga.h"
#include "world_manager.h"
#include "float.h"
#include "tx_math.h"
#include "visualizer.pb.h"

#include <iostream>
#include <sstream>
#include "boost/dll/import.hpp"
#include "boost/function.hpp"
#include "boost/shared_ptr.hpp"
#include "defines.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

extern float g_Driving_Parameters[10];
float LowerBound[3] = {1.2, 0.8, 2.0};   // 定义cc0-cc2取值的下限
float UpperBound[3] = {2.0, 1.5, 11.0};  // 定义cc0-cc2取值的上限
static constexpr const char *kYellowColor = "\033[33m";
static constexpr const char *kGreenColor = "\033[32m";
static constexpr const char *kRedColor = "\033[31m";
static constexpr const char *kResetColor = "\033[0m";

char g_szMessageText[4096] = "";

void MessagePrintf(char *pstrText, std::ostream *os = nullptr) {
  if (os) {
    (*os) << pstrText;
  } else {
    LOG(INFO) << kGreenColor << pstrText << kResetColor;
  }
}

TrafficGA::TrafficGA() {
  ChromeLength1 = 4;  // cc0 [1.2, 2]
  ChromeLength2 = 4;  // cc1 [0.8, 1.5]
  ChromeLength3 = 4;  // cc2 [2, 11]
  ChromeLength = ChromeLength1 + ChromeLength2 + ChromeLength3;
}

bool doSimulate(WorldManager *pWorldManager, float cc0, float cc1, float cc2, float &TT_Output, float TT_V,
                unsigned int TT_Vcount, float *V_Output, float TT_AVG) {
  // Connect to txTrafficSim
  using FilePath = boost::filesystem::path;
  if (exists(FilePath(pWorldManager->sceneFile_))) {
    sim_msg::Visualizer2TrafficCommand v2tcmd;
    v2tcmd.set_cmdno(1);
    v2tcmd.set_taskid(1);
    v2tcmd.set_timestep(pWorldManager->_time_step);
    v2tcmd.set_max_step(pWorldManager->_max_step);
    // v2tcmd.set_timestep(20.0);
    // v2tcmd.set_max_step(120);
    v2tcmd.set_max_displacement(TT_AVG);
    v2tcmd.set_driving_param_0(cc0);
    v2tcmd.set_driving_param_1(cc1);
    v2tcmd.set_driving_param_2(cc2);
    v2tcmd.set_tt_v(TT_V);
    v2tcmd.set_tt_obscount(TT_Vcount);
    v2tcmd.set_scenario_file_path(pWorldManager->sceneFile_);

    pWorldManager->txSimTraffic->connect(v2tcmd);
    // LOG(INFO) << kYellowColor << "connect txTrafficSim with cmd : " << v2tcmd.DebugString() << kResetColor <<
    // std::endl; step to calculate
    do {
      sim_msg::Traffic2VisualizerCommand *t2vcmdPtr = nullptr;

      bool finished = false;
      ChomperGAInterface::ErrCode _code = pWorldManager->txSimTraffic->step(&t2vcmdPtr, finished);
      auto &t2vcmd = *t2vcmdPtr;
      if (2 == t2vcmd.cmdno()) {
        std::stringstream oss;
        oss << " [v_output]: ";
        const auto sampleCount = t2vcmd.v_output_size();
        for (int i = 0; i < sampleCount; ++i) {
          oss << t2vcmd.v_output(i) << ", ";
          V_Output[i] = t2vcmd.v_output(i);
        }
        // LOG(INFO) << kYellowColor
        //      << "[Finish] "
        //      << "cc0 = " << t2vcmd.param0()
        //      << "; cc1 = " << t2vcmd.param1()
        //      << "; cc2 = " << t2vcmd.param2() << "; TT_Output = "
        //      << t2vcmd.tt_output()<< "; V_Output.size = "
        //      << t2vcmd.v_output().size()
        //      //<< oss.str()
        //      << kResetColor
        //      << std::endl;
        TT_Output = t2vcmd.tt_output();
        break;
      } else {
        // system("pause");
      }

      if (!t2vcmdPtr) delete t2vcmdPtr;
    } while (true);
    return true;
  } else {
    std::cout << "[Error] file do not exists, " << std::endl
              << ".sim File Full Path : "
              << ", " << ((exists(FilePath(pWorldManager->sceneFile_))) ? ("exist") : ("do not exist.")) << std::endl;
    return false;
  }
}

// - `TT_Obs`: 输入json文件中从开始到结束一共的时间，对应`g_TT_Obs`
// - `TT_ObsDistance`: 每段观察路段的长度，默认为100
// - `TT_V_Output`: 每一个路段速度的平均速度，读入到`g_V_Obs`中
// - `TT_V_OutputCount`：一共多少个路段，以`TT_ObsDistance`为一个路段，对应`g_TT_ObsCount`
bool TrafficGA::Update(const float *V_Obs, const unsigned int &V_ObsCount, float TT_Obs, float TT_ObsDistance,
                       WorldManager *pWorldManager) {
  Max_F[0] = 0;          // 存适应度最大的值
  Max_F[1] = 0;          // 存适应度最大值时的下标值，对应第几个DNA
  Max_F[2] = 0;          // 存适应度最大值时的代(generation)，对应第几代
  Max_F_Current[0] = 0;  // 存适应度最大的值(当前循环的，不包括最后保留的那个精英）
  Max_F_Current[1] = 0;  // 存适应度最大值时的下标值，对应第几个DNA(当前循环的，不包括最后保留的那个精英）
  Max_F_Current[2] = 0;  // 存适应度最大值时的代(generation)，对应第几代(当前循环的，不包括最后保留的那个精英）
  Min_F[0] = 10000;      // 存适应度最小的值
  Min_F[1] = 10000;      // 存适应度最小值时的下标值，对应第几个DNA
  Min_F[2] = 10000;      // 存适应度最小值时的代(generation)，对应第几代
  F_Max = 0.0;
  F_Min = 10000.0;
  MAX_LastIteration = 10000;  // 保存上一代最大值
  Convergence = 0;            // 1证明收敛条件达成
  ChromeLength1 = 4;          // cc0 [1.2, 2]
  ChromeLength2 = 4;          // cc1 [0.8, 1.5]
  ChromeLength3 = 4;          // cc2 [2, 11]
  Children_Index = 0;         // Husan Added

#ifdef TRAFFIC_LIB_FOR_VISULAIZER
  char szFileName[1024];
  sprintf(szFileName, "%s/x64/Release/TrafficOutputData.txt", g_szCurrentExePath);
  FILE *pFile = fopen(szFileName, "w+");
#endif
  srand(time(NULL));
  // Init Chrome
  for (int i = 0; i < TRAFFIC_GA_POP_SIZE; i++) {
    for (int j = 0; j < 12; j++) {
      Chrome[i].value[j] = ((rand() % 10000) > 5000) ? '1' : '0';
    }
  }

  // pWorldManager->StartPercentBarProcess(0, N_GENERATIONS * TRAFFIC_GA_POP_SIZE);
  for (unsigned int iteration = 0; iteration < trafficga::FLAGS_N_GENERATIONS; iteration++) {  // 每代的主循环
    F_Total = 0;
    // sprintf(g_szMessageText, "[iteration]%d\n", iteration);
    MessagePrintf(g_szMessageText);
    FLAGS_minloglevel = 0;
    LOG(WARNING) << kYellowColor << "#############################  [Training iteration] = " << iteration + 1
                 << " ##########################" << kResetColor << std::endl;

    double MAX_CurIteration = 0.0;
    double res_log[TRAFFIC_GA_POP_SIZE][2] = {0};
    int ga_pop_size = TRAFFIC_GA_POP_SIZE;
    for (unsigned int i = 0; i < TRAFFIC_GA_POP_SIZE; i++) {  // population中循环每个DNA
      // 下面解码成仿真可用的cc0,cc1,cc2
      unsigned int temp1 = DecodeChrome(Chrome[i], 0, ChromeLength1);              // 1,2,3,4   Chrome[i]是第i条DNA
      unsigned int temp2 = DecodeChrome(Chrome[i], ChromeLength1, ChromeLength2);  // 5,6,7,8
      unsigned int temp3 = DecodeChrome(Chrome[i], ChromeLength1 + ChromeLength2, ChromeLength3);  // 9,10,11,12
      cc0[i] = LowerBound[0] + temp1 * (UpperBound[0] - LowerBound[0]) / (Power(2, ChromeLength1) - 1);
      cc1[i] = LowerBound[1] + temp2 * (UpperBound[1] - LowerBound[1]) / (Power(2, ChromeLength2) - 1);
      cc2[i] = LowerBound[2] + temp3 * (UpperBound[2] - LowerBound[2]) / (Power(2, ChromeLength3) - 1);
      if (iteration == 0 && i == 0) {
        cc0[i] = g_Driving_Parameters[0];
        cc1[i] = g_Driving_Parameters[1];
        cc2[i] = g_Driving_Parameters[2];
      }
      if (iteration != 0) ga_pop_size = TRAFFIC_GA_POP_SIZE - 1;
      if ((i == TRAFFIC_GA_POP_SIZE - 1) &&
          (iteration > 0)) {  // 第一代之后的，最后一个DNA存储的是最大适应度对应的DNA，所以不用再重新运行这个
        F[i] = MAX_LastIteration;
      } else {  // 其他情况下都要运行这个仿真已得到新的的适应度数值
        float TT_Output = TT_Obs;
        float V_Output[MAX_V_OBS_COUNT];
        // sprintf(g_szMessageText, "Iteration: %d, DNA: %d", iteration, i);
        auto beg = time(NULL);
        // doSimulate(pWorldManager, 5.5, 1.3, 4.0, TT_Output, V_Obs[0], V_ObsCount, V_Output);
        // LOG(INFO) << "[Model Params]  "
        //  << "cc0 = " << cc0[i]
        //  << "; cc1 = " << cc1[i]
        //  << "; cc2 = " << cc2[i]
        //  << "; TT_Output = " << TT_Output
        //  << std::endl;
        doSimulate(pWorldManager, cc0[i], cc1[i], cc2[i], TT_Output, V_Obs[0], V_ObsCount, V_Output,
                   TT_ObsDistance);  // 在此调用仿真模块，参数是cc0-cc2,并产生输出 [TT_Output] [V_Output]。
        auto end = time(NULL);
        {
          DrivingData drivingData;
          drivingData.cc0_ = cc0[i];
          drivingData.cc1_ = cc1[i];
          drivingData.cc2_ = cc2[i];
          std::stringstream ss;
          ss << "cc/TrafficData" << i + 1 << ".txt";
          std::string drivingfile = ss.str();
          drivingData.tofile(drivingfile);
        }
        // 基于观察到的数据（TT_Obs, V_Obs）和仿真输出结果（TT_Output, V_Output）计算适应度值
        /*for (int i = 0; i < V_ObsCount; i++) {
                LOG(INFO) << kYellowColor << "v-" << i << ": " << V_Obs[i] << "," << V_Output[i] << kResetColor << "\n";
        }*/
        TT_Output = TT_Output / 1000;
        F[i] = FitnessFunction(TT_Obs, TT_Output, V_Obs, V_ObsCount, V_Output);  // 第i组DNA的适应度函数值
        /*LOG(INFO) << kGreenColor << "iterater:" << iteration + 1 << "/" << trafficga::FLAGS_N_GENERATIONS << " DNA: "
           << i + 1 << "/" << ga_pop_size << "\n"
                << "cc0: " << cc0[i] << ", cc1: " << cc1[i] << ", cc2: " << cc2[i] << ", tt_out: " << TT_Output << ", F:
           " << F[i] <<
                "\ndoSimulate cost = " << end - beg << "s" << kResetColor << "\n";*/
        sprintf(
            g_szMessageText,
            "iterater:%d/%d, DNA: %d/%d\ncc0: %.2f, cc1: %.2f, cc2: %.2f, tt_out: %.2f, F: %.6f\ndoSimulate cost= %ds",
            iteration + 1, trafficga::FLAGS_N_GENERATIONS, i + 1, ga_pop_size, cc0[i], cc1[i], cc2[i], TT_Output, F[i],
            end - beg);
        MessagePrintf(g_szMessageText);
        // LOG(INFO) << kYellowColor << "TT_OBS: " << TT_Obs << kResetColor << "\n";
        res_log[i][0] = TT_Output;
        res_log[i][1] = F[i];
        // sprintf(g_szMessageText, "<DNA %d>%.2f, %.2f, %.2f, %.1f, %.6f", i, cc0[i], cc1[i], cc2[i], TT_Output, F[i]);
        // char szTemp[256];
        // for (unsigned int m = 0; m < V_ObsCount; m++)
        // {
        //   sprintf(szTemp, "%.4f,", V_Output[m]);
        //   strcat(g_szMessageText, szTemp);
        // }

        // sprintf(szTemp, ")\n");
        // strcat(g_szMessageText, szTemp);
        // MessagePrintf(g_szMessageText);
        // fflush(pFile);
      }

      F_Total = F_Total + F[i];
      if (((iteration > 0) && (i != TRAFFIC_GA_POP_SIZE - 1) && F[i] > MAX_CurIteration) ||
          ((iteration == 0) && F[i] > MAX_CurIteration)) {
        MAX_CurIteration = F[i];
        Max_F_Current[0] = F[i];
        Max_F_Current[1] = i;
        Max_F_Current[2] = iteration;
      }

      if (((iteration > 0) && (i != TRAFFIC_GA_POP_SIZE - 1) && F_Max < F[i]) || ((iteration == 0) && F_Max < F[i])) {
        F_Max = F[i];
        // LOG(INFO) << kYellowColor << "before CC : " << m_CC0 << "," << m_CC1 << "," << m_CC2 << "," << cc0[i] << ","
        // << cc1[i] << "," << cc2[i] << kResetColor << "\n";
        m_CC0 = cc0[i];
        m_CC1 = cc1[i];
        m_CC2 = cc2[i];
        // LOG(INFO) << kYellowColor << "after CC : " << m_CC0 << "," << m_CC1 << "," << m_CC2 << kResetColor << "\n";
        Max_F[0] = F[i];
        Max_F[1] = i;
        Max_F[2] = iteration;
        // BestChrome=Chrome[i];
      }
      if ((i != TRAFFIC_GA_POP_SIZE - 1) && F_Min > F[i]) {
        F_Min = F[i];
        Min_F[0] = F[i];
        Min_F[1] = i;
        Min_F[2] = iteration;
        // WorstChrome=Chrome[i];
      }

      // pWorldManager->AddPercentBarProcess(1);
    }
    LOG(INFO) << kYellowColor << "*************** STATISTICAL RESULTS FOR " << iteration + 1 << " ***************"
              << kResetColor << "\n";
    for (int i = 0; i < ga_pop_size; i++) {
      // LOG(INFO) << kGreenColor << "DNA-" << i + 1 << ": cc0= " << cc0[i] << ", cc1= " << cc1[i] << ", cc2= " <<
      // cc2[i] << ",tt_out= " << res_log[i][0] << ", F= " << res_log[i][1] << kResetColor << "\n";
      sprintf(g_szMessageText, "DNA-%d: cc0= %.2f, cc1= %.2f, cc2= %.2f, tt_out= %.2f, F= %.6f", i + 1, cc0[i], cc1[i],
              cc2[i], res_log[i][0], res_log[i][1]);
      MessagePrintf(g_szMessageText);
    }

    // sprintf(g_szMessageText, "Maximum fitness for iteration %d: %.4f @DNA %d;Last iteration MAX: %.4f\n", iteration,
    // MAX_CurIteration, Max_F[1],MAX_LastIteration);
    sprintf(g_szMessageText, "Maximum fitness for iteration %d: %.4f @DNA %d;Last iteration MAX: %.4f\n", iteration,
            MAX_CurIteration, Max_F_Current[1], MAX_LastIteration);
    sprintf(g_szMessageText, "Maximum fitness for iteration %d: %.4f @DNA %d;Last iteration MAX: %.4f\n", iteration,
            MAX_CurIteration, Max_F_Current[1], MAX_LastIteration);
    MessagePrintf(g_szMessageText);
    LOG(WARNING) << "MAX_CurIteration is: [" << MAX_CurIteration << "] and the MAX_LastIteration is: ["
                 << MAX_LastIteration << "]\n";
    if (fabs(MAX_CurIteration - MAX_LastIteration) <=
        trafficga::FLAGS_e) {  // 收敛条件满足：当前的最大适应度和上一代的最大适应度之差小于给定阈值（FLAGS_e）
      sprintf(g_szMessageText,
              "Maximum F: %.4f, Best Chrome: %d. Stop due to convergence. Reach best at iteration %d. Last Maximum: "
              "%.4f. Total Iteration: %d\n",
              F_Max, Max_F[1], Max_F[2], MAX_LastIteration, iteration + 1);
      MessagePrintf(g_szMessageText);
      // LOG(INFO) << kYellowColor << "after CC : " << m_CC0 << "," << m_CC1 << "," << m_CC2 << kResetColor <<"\n";
      sprintf(g_szMessageText, "Best cc: %.2f, %.2f, %.2f\n", m_CC0, m_CC1, m_CC2);
      MessagePrintf(g_szMessageText);
      Convergence = 1;  // 如果已经收敛，即可跳出主循环
      break;
    }

    // 对每一代，产生一个Parents池，根据适应度函数的轮盘赌结果选择POP_Size个进入Parents池
    for (unsigned int j = 0;
         j < TRAFFIC_GA_POP_SIZE - 2;) {  // 只对前面POP_SIZE-1（偶数）个DNA执行这个操作，最后一个直接赋值为最大适应度的
      Father_Index = Roulette_Wheel_Selection();                        // 轮盘赌来选择父的下标
      Mother_Index = Roulette_Wheel_Selection();                        // 轮盘赌来选择母的下标
      memcpy(&Parents[j], &Chrome[Father_Index], sizeof(Chrome_Data));  // Parents和Chrome结构相同
      memcpy(&Parents[j + 1], &Chrome[Mother_Index], sizeof(Chrome_Data));
      CrossOver(&Parents[j], &Parents[j + 1],
                Children);  // 交叉过程，其中交叉概率部分在交叉的函数里面做，每次更新Children里面的两个：
                            // 或者是父母的交叉（概率满足），或者就是父母本身（概率不满足）,直到产生所有的population
      j = j + 2;  // 每次选择两个进入这个Parents池 ChildIndex在CrossOver里同步更新
    }

    // 突变：经过上面那步，Children已经形成一个完整(除了最后一个要做精英保留)的population,
    // 其中有crossover出来的，也有直接选出来但没有crossover的parents
    Mutation(Children);
    if ((iteration == 0) || (MAX_LastIteration < MAX_CurIteration)) {
      MAX_LastIteration = MAX_CurIteration;
      memcpy(&Children[TRAFFIC_GA_POP_SIZE - 1], &Chrome[Max_F_Current[1]], sizeof(Chrome_Data));
    } else {
      memcpy(&Children[TRAFFIC_GA_POP_SIZE - 1], &Chrome[TRAFFIC_GA_POP_SIZE - 1], sizeof(Chrome_Data));
    }

    // 将和当前最大适应值相同的DNA进行突变，保证精英唯一
    for (int t = 0; t < TRAFFIC_GA_POP_SIZE - 1; t++) {
      if (memcmp(&Children[t], &Children[TRAFFIC_GA_POP_SIZE - 1], 12) == 0) {  // &Children[TRAFFIC_GA_POP_SIZE - 1]
        MutationOne(&Children[t], 12);
      }
    }
    memcpy(Chrome, Children, sizeof(Chrome_Data) * TRAFFIC_GA_POP_SIZE);
    Children_Index = 0;  // 重置下标
  }

  // GetResult(m_CC0, m_CC1, m_CC2);
  // pWorldManager->StopPercentBarProcess();

  Chrome_Data BestChrome;
  if (Convergence != 1) {  // generation循环次数到达造成的终止
    BestChrome = Chrome[TRAFFIC_GA_POP_SIZE - 1];
    sprintf(g_szMessageText,
            "Maximum F: %.4f, Best Chrome is %d at iteration %d. Stop due to reaching maximum iteration.\n", F_Max,
            Max_F[1], Max_F[2]);
    MessagePrintf(g_szMessageText);
  }
  unsigned int temp1 = DecodeChrome(BestChrome, 0, ChromeLength1);              // 1,2,3,4   Chrome[i]是第i条DNA
  unsigned int temp2 = DecodeChrome(BestChrome, ChromeLength1, ChromeLength2);  // 5,6,7,8
  unsigned int temp3 = DecodeChrome(BestChrome, ChromeLength1 + ChromeLength2, ChromeLength3);  // 9,10,11,12
  float cc0 = LowerBound[0] + temp1 * (UpperBound[0] - LowerBound[0]) / (Power(2, ChromeLength1) - 1);
  float cc1 = LowerBound[1] + temp2 * (UpperBound[1] - LowerBound[1]) / (Power(2, ChromeLength2) - 1);
  float cc2 = LowerBound[2] + temp3 * (UpperBound[2] - LowerBound[2]) / (Power(2, ChromeLength3) - 1);

#ifdef TRAFFIC_LIB_FOR_VISULAIZER
  if (pFile) fclose(pFile);
#endif
  return true;
}

// 轮盘赌算法 按设定的概率，随机选中一个个体 P[i]表示第i个个体被选中的概率/
unsigned int TrafficGA::Roulette_Wheel_Selection() {
  float m = 0;
  float r = RandomRange(0, 1);  // r为0至1的随机数
  for (unsigned int i = 0; i < TRAFFIC_GA_POP_SIZE; i++) {
    // 产生的随机数在m~m+P[i]间则认为选中了i，因此i被选中的概率是P[i]
    // P[i]=F[i]/F_Total;  //计算每组DNA的被选中的概率
    // m = m + P[i];
    m = m + F[i] / F_Total;
    if (r <= m) return i;
  }

  return 0;
}

unsigned int TrafficGA::DecodeChrome(Chrome_Data data, unsigned int startIndex, unsigned int length) {
  unsigned int i, j;
  int mask = 0;
  for (i = startIndex, j = 0; j < length; i++) {
    if (data.value[i] == '1') mask |= (1 << (length - 1 - j));

    j++;
  }
  return mask;
}

// 交叉
void TrafficGA::CrossOver(Chrome_Data *Parents1, Chrome_Data *Parents2,
                          Chrome_Data *Children) {  // Parent1和Parent2是两个DNA(向量)，格式是Parents1=100111000110
  int CrossOver_Location;
  CrossOver_Location = floor(RandomRange(0, 1) * ChromeLength);  // 随机取一个位置，让这个位置之前的父亲和母亲基因置换
  for (unsigned int i = 0; i < ChromeLength; i++) {  // 遍历父母的每个基因，在CrossOver_Location前面的互换
    if (i < CrossOver_Location) {
      Children[Children_Index].value[i] = Parents1->value[i];
      Children[Children_Index + 1].value[i] = Parents2->value[i];
    } else if (RandomRange(0, 1) <
               trafficga::FLAGS_CROSS_RATE) {  // 如果随机数小于交叉概率，则在CrossOver_Location后面互换
      Children[Children_Index].value[i] = Parents2->value[i];
      Children[Children_Index + 1].value[i] = Parents1->value[i];
    } else {  // 如果随机数不小于交叉概率，则不在CrossOver_Location后面互换，此时,Children应该和Parents相同
      Children[Children_Index].value[i] = Parents1->value[i];
      Children[Children_Index + 1].value[i] = Parents2->value[i];
    }
  }
  Children_Index =
      Children_Index + 2;  // 这个是记录在Childern这一代中哪个DNA在更新中，每次调用crossover更新两个children
}

// 突变
void TrafficGA::Mutation(Chrome_Data *Population) {
  for (unsigned int i = 0; i < TRAFFIC_GA_POP_SIZE - 1;
       i++) {  // 遍历这一代中除最后一个（预留给无变化的最大适应度对应的DNA）外的每个DNA
    for (unsigned int j = 0; j < ChromeLength; j++) {  // 遍历这个DNA中的每个基因
      if (RandomRange(0, 1) < trafficga::FLAGS_MUTATION_RATE)
        Population[i].value[j] = (Population[i].value[j] == '1') ? '0' : '1';
    }
  }
}

void TrafficGA::MutationOne(
    Chrome_Data *ChromeTarget,
    unsigned int Length) {  // ChromeTarget是1个DNA(向量)，格式是ChromeTarget=100111000110,其中挑选一个突变
  int Mutation_Index = rand() % Length;
  ChromeTarget->value[Mutation_Index] = (ChromeTarget->value[Mutation_Index] == '1') ? '0' : '1';
}

// 目标函数，观测值和仿真输出之间的差越小，F越大
float TrafficGA::FitnessFunction(float TT_Obs, float TT_Output, const float *V_Obs, unsigned int V_ObsCount,
                                 float *V_Output) {
  // 适应度函数
  float F = 0.0, E1 = 0.0, E2 = 0.0;
  float WeightDelta = 0.5;                 // 两个E各占多大比重，暂时取一半一半
  E1 = fabs(TT_Output - TT_Obs) / TT_Obs;  // 只有一个Travel Time的观测点,计算绝对值
  // sprintf(g_szMessageText, "V_Obs, V_Output\n");
  // MessagePrintf(f, g_szMessageText);
  for (unsigned int i = 0; i < V_ObsCount; i++) {  // n是一共有多少速度的观测点,按i遍历所有观测点
    double value = (V_Output[i] - V_Obs[i]) / V_Obs[i];
    E2 = E2 + Power(value, 2);
  }
  E2 = sqrt(E2 / V_ObsCount);  // RMASE，取平方根
  F = 1.0 / (WeightDelta * E1 + (1 - WeightDelta) * E2);
  return F;
}

void TrafficGA::GetResult(float &cc0, float &cc1, float &cc2) {
  // 算法已经终止，解码出最好（后）的参数
  Chrome_Data BestChrome;
  if (Convergence == 1) {  // 如果已经收敛造成的终止
    BestChrome = Chrome[Max_F_Current[1]];
  } else {  // generation循环次数到达造成的终止
    BestChrome = Chrome[TRAFFIC_GA_POP_SIZE - 1];
  }

  // BestChrome的格式为=100111000110
  // 下面解码成仿真可用的cc0,cc1,cc2.
  unsigned int temp1 = DecodeChrome(BestChrome, 0, ChromeLength1);              // 1,2,3,4   Chrome[i]是第i条DNA
  unsigned int temp2 = DecodeChrome(BestChrome, ChromeLength1, ChromeLength2);  // 5,6,7,8
  unsigned int temp3 = DecodeChrome(BestChrome, ChromeLength1 + ChromeLength2, ChromeLength3);  // 9,10,11,12
  cc0 = LowerBound[0] + temp1 * (UpperBound[0] - LowerBound[0]) / (Power(2, ChromeLength1) - 1);
  cc1 = LowerBound[1] + temp2 * (UpperBound[1] - LowerBound[1]) / (Power(2, ChromeLength2) - 1);
  cc2 = LowerBound[2] + temp3 * (UpperBound[2] - LowerBound[2]) / (Power(2, ChromeLength3) - 1);
}

void TT_Feature::fromfile(const std::string &filename) {
  FILE *pFile = fopen(filename.c_str(), "r+");
  if (pFile) {
    int count = 0;
    fscanf(pFile, "%f,%f,%d\n", &tt_obs_, &tt_obs_distance, &count);

    for (unsigned int i = 0; i < count; i++) {
      float d = 0.0f;
      fscanf(pFile, "%f,\n", &d);
      v_obs_.emplace_back(d);
    }
    fclose(pFile);
  }
  std::cout << tostring() << std::endl;
}
void TT_Feature::tofile(const std::string &filename) const {
  FILE *pFile = fopen(filename.c_str(), "w+");
  if (!pFile) return;
  fprintf(pFile, "%.1f,%.5f,%d\n", tt_obs_, tt_obs_distance, static_cast<int>(v_obs_.size()));
  for (auto &item : v_obs_) fprintf(pFile, "%.1f,", item);
  fclose(pFile);
  // std::cout  << filename << " saved!";
}

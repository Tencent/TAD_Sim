// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_route_group_agent.h"
#include "tx_probability_generator.h"
TX_NAMESPACE_OPEN(Scene)

#if __TX_Mark__("TAD_Cloud_ElementGenerator::RouteGroupAgent")

Base::txBool TAD_RouteGroupAgent::Init(RouteGroupPtr xsdPtr) TX_NOEXCEPT {
  mRawXSDRouteGroupPtr = xsdPtr;
  if (NonNull_Pointer(mRawXSDRouteGroupPtr) && (InvalidId != mRawXSDRouteGroupPtr->start())) {
    // 获取原始XSD路径组指针的ID
    const auto nRouteId = mRawXSDRouteGroupPtr->id();
    // 输出路径组ID
    LOG(INFO) << TX_VARS(mRawXSDRouteGroupPtr->id());
    // 根据起始位置ID获取起始位置代理对象
    LocationAgentPtr startLocationPtr = Base::LocationAgent::GetLocationAgentById(mRawXSDRouteGroupPtr->start());
    // 根据终点位置ID获取终点位置代理对象
    LocationAgentPtr end1_LocationPtr = Base::LocationAgent::GetLocationAgentById(mRawXSDRouteGroupPtr->end1());
    LocationAgentPtr end2_LocationPtr = Base::LocationAgent::GetLocationAgentById(mRawXSDRouteGroupPtr->end2());
    LocationAgentPtr end3_LocationPtr = Base::LocationAgent::GetLocationAgentById(mRawXSDRouteGroupPtr->end3());
    // 若起始位置指针不为空
    if (NonNull_Pointer(startLocationPtr)) {
      // 初始化概率向量
      std::vector<Base::txInt> vecPercentages{0, 0, 0};
      // 调整候选向量的大小
      mCandidateVector.resize(3);
      // 如果路径组的第一个路径的概率大于0，且终点位置ID有效且终点位置代理对象非空
      if ((mRawXSDRouteGroupPtr->percentage1()) > 0 && (InvalidId != mRawXSDRouteGroupPtr->end1()) &&
          NonNull_Pointer(end1_LocationPtr)) {
        // 创建一个包含起始位置、终点位置和路径的元组
        auto curRoute = std::make_tuple(startLocationPtr, end1_LocationPtr, TrafficFlow::Component::RouteAI());
        // 初始化中间点向量
        std::vector<hadmap::txPoint> midVec;
        // 如果路径组的第一个路径的中间点ID大于0
        if ((mRawXSDRouteGroupPtr->mid1()) > 0) {
          // 根据中间点ID获取中间点位置代理对象
          LocationAgentPtr mid1_LocationPtr = Base::LocationAgent::GetLocationAgentById(mRawXSDRouteGroupPtr->mid1());
          if (NonNull_Pointer(mid1_LocationPtr)) {
            midVec.push_back(mid1_LocationPtr->GPS());
          }
        }
        // 初始化路径
        std::get<2>(curRoute).Initialize(nRouteId, 0, -1, (startLocationPtr->GPS()), midVec, (end1_LocationPtr->GPS()));
        // 初始化伪随机数生成器
        Base::Component::Pseudorandom pr;
        pr.Initialize(nRouteId, 1);
        LOG_IF(INFO, FLAGS_LogLevel_Routing) << TX_VARS(nRouteId) << TX_VARS_NAME(subRoute, 0) << " start...";
        std::get<2>(curRoute).ComputeRoute(pr);
        LOG_IF(INFO, FLAGS_LogLevel_Routing) << TX_VARS(nRouteId) << TX_VARS_NAME(subRoute, 0) << " finish.";
        // 将当前路径添加到候选向量中
        mCandidateVector[0] = curRoute;
        // 设置概率向量的第一个元素为路径组的第一个路径的概率
        vecPercentages[0] = mRawXSDRouteGroupPtr->percentage1();
      }

      // 如果路径组的第二个路径的概率大于0，且终点位置ID有效且终点位置代理对象非空
      if ((mRawXSDRouteGroupPtr->percentage2()) > 0 && (InvalidId != mRawXSDRouteGroupPtr->end2()) &&
          NonNull_Pointer(end2_LocationPtr)) {
        // 创建一个包含起始位置、终点位置和路径的元组
        auto curRoute = std::make_tuple(startLocationPtr, end2_LocationPtr, TrafficFlow::Component::RouteAI());
        // 初始化中间点向量
        std::vector<hadmap::txPoint> midVec;
        // 如果路径组的第二个路径的中间点ID大于0
        if ((mRawXSDRouteGroupPtr->mid2()) > 0) {
          // 根据中间点ID获取中间点位置代理对象
          LocationAgentPtr mid2_LocationPtr = Base::LocationAgent::GetLocationAgentById(mRawXSDRouteGroupPtr->mid2());
          // 如果中间点位置代理对象非空
          if (NonNull_Pointer(mid2_LocationPtr)) {
            // 将中间点位置添加到中间点向量中
            midVec.push_back(mid2_LocationPtr->GPS());
          }
        }
        std::get<2>(curRoute).Initialize(nRouteId, 1, -1, (startLocationPtr->GPS()), midVec, (end2_LocationPtr->GPS()));
        // 初始化伪随机数生成器
        Base::Component::Pseudorandom pr;
        pr.Initialize(nRouteId, 2);
        LOG_IF(INFO, FLAGS_LogLevel_Routing) << TX_VARS(nRouteId) << TX_VARS_NAME(subRoute, 1) << " start...";
        // 计算路径
        std::get<2>(curRoute).ComputeRoute(pr);
        LOG_IF(INFO, FLAGS_LogLevel_Routing) << TX_VARS(nRouteId) << TX_VARS_NAME(subRoute, 1) << " finish.";
        // 将当前路径添加到候选向量中
        mCandidateVector[1] = curRoute;
        // 设置概率向量的第二个元素为路径组的第二个路径的概率
        vecPercentages[1] = mRawXSDRouteGroupPtr->percentage2();
      }

      // 如果路径组的第三个路径的概率大于0，且终点位置ID有效且终点位置代理对象非空
      if ((mRawXSDRouteGroupPtr->percentage3()) > 0 && (InvalidId != mRawXSDRouteGroupPtr->end3()) &&
          NonNull_Pointer(end3_LocationPtr)) {
        // 创建一个包含起始位置、终点位置和路径的元组
        auto curRoute = std::make_tuple(startLocationPtr, end3_LocationPtr, TrafficFlow::Component::RouteAI());
        // 初始化中间点向量
        std::vector<hadmap::txPoint> midVec;
        // 如果路径组的第三个路径的中间点ID大于0
        if ((mRawXSDRouteGroupPtr->mid3()) > 0) {
          // 根据中间点ID获取中间点位置代理对象
          LocationAgentPtr mid3_LocationPtr = Base::LocationAgent::GetLocationAgentById(mRawXSDRouteGroupPtr->mid3());
          // 如果中间点位置代理对象非空
          if (NonNull_Pointer(mid3_LocationPtr)) {
            midVec.push_back(mid3_LocationPtr->GPS());
          }
        }
        // 初始化路径
        std::get<2>(curRoute).Initialize(nRouteId, 2, -1, (startLocationPtr->GPS()), midVec, (end3_LocationPtr->GPS()));
        // 初始化伪随机数生成器
        Base::Component::Pseudorandom pr;
        pr.Initialize(nRouteId, 3);
        LOG_IF(INFO, FLAGS_LogLevel_Routing) << TX_VARS(nRouteId) << TX_VARS_NAME(subRoute, 2) << " start...";
        std::get<2>(curRoute).ComputeRoute(pr);
        LOG_IF(INFO, FLAGS_LogLevel_Routing) << TX_VARS(nRouteId) << TX_VARS_NAME(subRoute, 2) << " finish.";
        // 将当前路径添加到候选向量中
        mCandidateVector[2] = curRoute;
        // 设置概率向量的第三个元素为路径组的第三个路径的概率
        vecPercentages[2] = mRawXSDRouteGroupPtr->percentage3();
      }

      // 如果集合不为空
      if (CallFail(vecPercentages.empty())) {
        // 初始化起始位置的哈希区域
        if (CallSucc(mStartHashArea.Initialize(nRouteId, (startLocationPtr->vPos())))) {
          // 设置路径组代理对象有效
          mValid = true;
          // 设置索引为0
          mIndex = 0;
          // 根据概率向量生成概率向量
          mProbability = Base::PseudoRandomDiscreteDistribution::GenerateProbabilityVector(vecPercentages, nRouteId);
          const auto RouteGroupId = mRawXSDRouteGroupPtr->id();
          return true;
        } else {
          // 输出日志
          LOG(WARNING) << "Route Start Line Area Init Failure. "
                       << TX_VARS_NAME(RouteGroupId, (mRawXSDRouteGroupPtr)->id())
                       << TX_VARS_NAME(StartPos, (startLocationPtr->vPos()));
        }
      }
    }
  }

  mValid = false;
  mIndex = 0;
  return false;
}

// 将路径组信息转换为字符串
Base::txString TAD_RouteGroupAgent::Str() const TX_NOEXCEPT {
  // 创建一个字符串流，用于存储概率向量的字符串表示
  std::ostringstream oss_probability;
  // 将概率向量中的元素复制到字符串流中，用逗号分隔
  std::copy(mProbability.begin(), mProbability.end(), std::ostream_iterator<Base::txInt>(oss_probability, ","));
  // 创建一个字符串流，用于存储路径组信息
  std::ostringstream oss;
  oss << "{" << TX_VARS_NAME(RouteGroupId, RouteGroupId()) << TX_COND_NAME(Valid, mValid)
      << TX_VARS_NAME(Probability, oss_probability.str());

  for (const auto& refRoute : mCandidateVector) {
    oss << std::endl
        << "#############################################################" << std::endl
        << "[Start Location : " << (std::get<0>(refRoute)->Str()) << "], "
        << "[End Location : " << (std::get<1>(refRoute)->Str()) << "], "
        << "[Route : " << (std::get<2>(refRoute).Str()) << "]. ";
  }
  return oss.str();
}

Base::txInt TAD_RouteGroupAgent::RouteGroupId() const TX_NOEXCEPT {
  // 如果有效
  if (IsValid()) {
    // 返回原始XSD路径组指针的ID
    return mRawXSDRouteGroupPtr->id();
  } else {
    return InvalidId;
  }
}

void TAD_RouteGroupAgent::ResetRoute() TX_NOEXCEPT { mIndex = 0; }

// 获取下一个概率路径
std::tuple<TAD_RouteGroupAgent::LocationAgentPtr, TAD_RouteGroupAgent::LocationAgentPtr, TAD_RouteGroupAgent::RouteAI>
TAD_RouteGroupAgent::GetNextProbabilityRoute(Base::txInt& subRouteId) TX_NOEXCEPT {
  // 如果有效
  if (IsValid()) {
    // 增加索引
    mIndex++;
    // 如果索引大于等于概率向量的大小
    if (mIndex >= mProbability.size()) {
      mIndex = 0;
    }
    // 设置子路径ID为概率向量中对应索引的值
    subRouteId = mProbability[mIndex];
    // 返回候选向量中对应子路径ID的元素
    return mCandidateVector[subRouteId];
  } else {
    // 返回空指针和默认的RouteAI对象
    return std::make_tuple(nullptr, nullptr, TrafficFlow::Component::RouteAI());
  }
}

// 获取特定子路径的概率路径
std::tuple<TAD_RouteGroupAgent::LocationAgentPtr, TAD_RouteGroupAgent::LocationAgentPtr, TAD_RouteGroupAgent::RouteAI>
TAD_RouteGroupAgent::GetSpecialRoute(const Base::txInt subRouteId) TX_NOEXCEPT {
  // 如果有效且子路径ID在候选向量的范围内
  if (IsValid() && subRouteId < mCandidateVector.size() && subRouteId >= 0) {
    // 返回候选向量中对应子路径ID的元素
    return mCandidateVector[subRouteId];
  } else {
    // 返回空指针和默认的RouteAI对象
    return std::make_tuple(nullptr, nullptr, TrafficFlow::Component::RouteAI());
  }
}

Base::txBool TAD_RouteGroupAgent::QueryVehicles(VehicleContainer& refVehicleContainer) TX_NOEXCEPT {
  // 清空车辆容器
  refVehicleContainer.clear();
  // 查询起始哈希区域的车辆
  mStartHashArea.QueryVehicles(refVehicleContainer);
  // 如果车辆容器为空，返回失败，否则返回成功
  return CallFail(refVehicleContainer.empty());
}

#endif /*__TX_Mark__("TAD_Cloud_ElementGenerator::RouteGroupAgent")*/

TX_NAMESPACE_CLOSE(Scene)

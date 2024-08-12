// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_element_generator.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include "HdMap/tx_hashed_road.h"
#include "tx_component.h"
#include "tx_probability_generator.h"
#include "tx_spatial_query.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneLoader)
TX_NAMESPACE_OPEN(Base)

#if __TX_Mark__("DITW_ElementGenerator::LocationAgent")

std::unordered_map<Base::txInt /* RouteGroup id*/, LocationAgent::LocationAgentPtr> LocationAgent::sMapLocation;

Base::txBool LocationAgent::Init(Base::ISceneLoader::ILocationViewerPtr xsdPtr) TX_NOEXCEPT {
  mRawXSDLocationPtr = xsdPtr;
  if (NonNull_Pointer(mRawXSDLocationPtr)) {
    mValid = true;
    mPos.FromWGS84(mRawXSDLocationPtr->PosGPS());
    return Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(mPos, mInfoLane,
                                                                                                      mST.x(), mST.y());
  } else {
    return false;
  }
}

Base::txBool LocationAgent::Init(const hadmap::txPoint& globalPos TX_MARK("__virtual_city__"),
                                 const Base::Info_Lane_t& _locInfo) TX_NOEXCEPT {
  mValid = true;
  mPos.FromWGS84(globalPos);
  mInfoLane = _locInfo;

  auto geomPtr = HdMap::HadmapCacheConCurrent::GetGeomInfoById(mInfoLane);
  if (NonNull_Pointer(geomPtr)) {
    return geomPtr->xy2sl(mPos.ToENU().ENU2D(), mST.x(), mST.y());
  } else {
    mST.setZero();
    return false;
  }
}

Base::txBool LocationAgent::LocationAgent::Init(const hadmap::txPoint& globalPos) TX_NOEXCEPT {
  mValid = true;
  mPos.FromWGS84(globalPos);
  return Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(mPos, mInfoLane,
                                                                                                    mST.x(), mST.y());
}

Base::txString LocationAgent::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS(LocationId()) << TX_COND_NAME(Valid, mValid) << TX_VARS_NAME(vPos, mPos)
      << TX_VARS_NAME(LaneInfo, mInfoLane) << TX_VARS_NAME(ST, Utils::ToString(mST)) << "}";
  return oss.str();
}

void LocationAgent::AddLocationAgent(LocationAgent::LocationAgentPtr locAgentPtr) TX_NOEXCEPT {
  if (locAgentPtr) {
    sMapLocation[locAgentPtr->LocationId()] = locAgentPtr;
  }
}

#endif /*__TX_Mark__("DITW_ElementGenerator::LocationAgent")*/

#if __TX_Mark__("DITW_ElementGenerator::VehicleExitAgent")
Base::txBool VehicleExitAgent::Init(Base::ISceneLoaderPtr _loader,
                                    Base::ISceneLoader::ITrafficFlowViewer::VehExitPtr xsdPtr) TX_NOEXCEPT {
  mRawXSDVehExitPtr = xsdPtr;
  if (NonNull_Pointer(mRawXSDVehExitPtr)) {
    LocationAgent::LocationAgentPtr locPtr = LocationAgent::GetLocationAgentById(mRawXSDVehExitPtr->location());
    if (NonNull_Pointer(locPtr)) {
      if (mDeadlineArea.Initialize(locPtr->vPos())) {
        mValid = true;
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

Base::txInt VehicleExitAgent::VehInputId() const TX_NOEXCEPT {
  if (IsValid() && NonNull_Pointer(mRawXSDVehExitPtr)) {
    return mRawXSDVehExitPtr->id();
  }
  return InvalidId;
}

Base::txBool VehicleExitAgent::ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid, const Base::txVec3& elemDir,
                                                    const Coord::txENU& elementPt) const TX_NOEXCEPT {
  if (IsValid()) {
    return mDeadlineArea.ArriveAtDeadlineArea(elemLaneUid, elemDir, elementPt);
  }
  return false;
}

#endif /*__TX_Mark__("DITW_ElementGenerator::VehicleExitAgent")*/

#if __TX_Mark__("DITW_ElementGenerator::VehicleInputeAgent")

Base::txBool VehicleInputeAgent::Init(Base::ISceneLoaderPtr _loader,
                                      Base::ISceneLoader::ITrafficFlowViewer::VehInputPtr xsdPtr) TX_NOEXCEPT {
  mCurPeriod = 0.0;
  if (NonNull_Pointer(xsdPtr) && NonNull_Pointer(_loader)) {
    mRawXSDVehInputPtr = xsdPtr;
    const Base::txInt viId = xsdPtr->id();
    const Base::txInt locId = xsdPtr->location();
    const Base::txInt comId = xsdPtr->composition();
    Base::ISceneLoader::ITrafficFlowViewer::VehCompPtr curCompPtr = _loader->GetTrafficFlow()->GetVehComp(comId);
    mInputLocationPtr = LocationAgent::GetLocationAgentById(locId);
    const Base::txFloat half_s = 0.0;

    if (NonNull_Pointer(mInputLocationPtr) && CallFail(mInputLocationPtr->GetLaneInfo().isOnLaneLink) &&
        (mInputLocationPtr->GetDistanceOnCurve() > half_s /*20220629 car overlap*/)) {
      const std::set<Base::txLaneID>& refValidLaneId = mRawXSDVehInputPtr->cover();
      std::ostringstream oss;
      std::copy(refValidLaneId.begin(), refValidLaneId.end(), std::ostream_iterator<Base::txLaneID>(oss, ","));
      LogInfo << TX_VARS(viId) << " valid lane : " << oss.str();
      if (NonNull_Pointer(curCompPtr)) {
        auto& vecVehInputParams = mCandidateVehicleInitParamVector;
        const Base::txFloat StartV = mRawXSDVehInputPtr->start_v();
        const Base::txFloat MaxV = mRawXSDVehInputPtr->max_v();
        const Base::txFloat HalfRange = mRawXSDVehInputPtr->halfRange_v();
#  if 1
        const txInt nPropCnt = curCompPtr->PropCnt();
        for (txInt idx = 0; idx < nPropCnt; ++idx) {
          if (curCompPtr->IsValid(idx)) {
            Base::ISceneLoader::ITrafficFlowViewer::BehPtr behPtr =
                _loader->GetTrafficFlow()->GetBeh(curCompPtr->Behavior(idx));

            Base::txString typeStr = curCompPtr->TypeStr(idx);

            std::vector<Base::txString> typeVector;
            boost::algorithm::split(typeVector, typeStr, boost::is_any_of(","));

            Base::txInt typeCnt = typeVector.size();
            if (typeCnt > 0 && behPtr && (curCompPtr->Percentage(idx) > 0) && (curCompPtr->Aggress(idx) >= 0.0)) {
              // Base::ISceneLoader::ITrafficFlowViewer::VehTypePtr vehType1Ptr =
              //     _loader->GetTrafficFlow()->GetVehType(curCompPtr->Type(idx));

              for (auto& typeId : typeVector) {
                Base::ISceneLoader::ITrafficFlowViewer::VehTypePtr vehTypePtr =
                    _loader->GetTrafficFlow()->GetVehType(stoi(typeId));
                if (vehTypePtr) {
                  VehicleInitParamWithRandomKinect tmpNode;
                  tmpNode.mVehTypePtr = vehTypePtr;
                  tmpNode.mBehPtr = behPtr;
                  tmpNode.mPercentage = std::floor(curCompPtr->Percentage(idx) / typeCnt);
                  tmpNode.mAggress = curCompPtr->Aggress(idx);

                  tmpNode.mStartV_native = StartV;
                  tmpNode.mMaxV_native = MaxV;
                  tmpNode.mHalfRange_native = HalfRange;
                  tmpNode.GenerateRndKinect(tmpNode.mStartV_native, tmpNode.mMaxV_native, tmpNode.mHalfRange_native);
                  tmpNode.mLocationPtr = mInputLocationPtr;
                  tmpNode.mVehCompPtr = curCompPtr;

                  vecVehInputParams.emplace_back(tmpNode);
                }
              }
            } else {
              LOG(WARNING) << "Vehicle Input Init Param Error. " << TX_VARS(viId) << TX_VARS(locId) << TX_VARS(comId)
                           << TX_VARS_NAME(VehTypeId, curCompPtr->TypeStr(idx))
                           << TX_VARS_NAME(BehId, curCompPtr->Behavior(idx))
                           << TX_VARS_NAME(Percentage, curCompPtr->Percentage(idx))
                           << TX_VARS_NAME(Aggress, curCompPtr->Aggress(idx));
            }
          } else {
            LOG(WARNING) << "comp " << idx << " is invalid.";
          }
        }
#  endif
        if (CallFail(vecVehInputParams.empty())) {
          std::vector<Base::txInt> vecPercentages;
          for (const auto& refParam : vecVehInputParams) {
            vecPercentages.push_back(refParam.mPercentage);
          }
          mValid = true;
          mIndex = 0;
          mProbability[VehParams] =
              Base::PseudoRandomDiscreteDistribution::GenerateProbabilityVector(vecPercentages, viId);

          hadmap::txLanes resLanesWithUnuseLane;
          hadmap::getLanes(txMapHdr,
                           hadmap::txLaneId(mInputLocationPtr->GetLaneInfo().onLaneUid.roadId,
                                            mInputLocationPtr->GetLaneInfo().onLaneUid.sectionId, LANE_PKID_INVALID),
                           resLanesWithUnuseLane);

          hadmap::txLanes resLanes;
          for (Base::txInt i = 0; i < resLanesWithUnuseLane.size(); ++i) {
            const auto laneType = resLanesWithUnuseLane[i]->getLaneType();
            if ((Utils::IsDrivingLane(laneType)) && (refValidLaneId.count(resLanesWithUnuseLane[i]->getId()) > 0)) {
              resLanes.push_back(resLanesWithUnuseLane[i]);
            } else {
              LOG(WARNING) << "unsupport laneType " << laneType
                           << TX_VARS_NAME(srcLaneUid, resLanesWithUnuseLane[i]->getTxLaneId());
            }
          }
          const Base::txInt nLaneSize = resLanes.size();
          if (0 == nLaneSize) {
            mValid = false;
            mIndex = 0;
            LOG(WARNING) << " vehicle inpute init failure."
                         << "valid lane size is zero.";
            return false;
          }
          std::vector<Base::txInt> vecLanePercetage(nLaneSize, 1);
          mProbability[AppearLaneId] =
              Base::PseudoRandomDiscreteDistribution::GenerateProbabilityVectorWithoutNeighborSame(vecLanePercetage,
                                                                                                   viId);

          mCandidateLaneIdVector.reserve(nLaneSize);
          m_vec_HashedRoadInfoCache.reserve(nLaneSize);
          for (Base::txInt i = 0; i < nLaneSize; ++i) {
            const auto laneUid = resLanes[i]->getTxLaneId();
            HdMap::txLaneInfoInterfacePtr geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(laneUid);
            if (NonNull_Pointer(geom_ptr)) {
              mCandidateLaneIdVector.emplace_back(laneUid.laneId);
              TX_MARK("mInputLocationPtr->GetDistanceOnCurve() must be > 1.0, for car overlap.");
              HashedLaneInfo inputHashedInfoPerLane = Geometry::SpatialQuery::GenerateHashedLaneInfo(
                  geom_ptr, (mInputLocationPtr->GetDistanceOnCurve() - half_s), Base::Info_Lane_t(laneUid));
              HashedLaneInfoOrthogonalListPtr input_node_ptr =
                  HdMap::HashedRoadCacheConCurrent::Query_OrthogonalList(inputHashedInfoPerLane);
              m_vec_HashedRoadInfoCache.emplace_back(std::make_tuple(inputHashedInfoPerLane, input_node_ptr));
            } else {
              LOG(WARNING) << "GetLaneInfoByUid failure. " << TX_VARS(laneUid);
            }
          }

          if (mProbability[AppearLaneId].size() != mProbability[VehParams].size()) {
            mValid = false;
            mIndex = 0;
            LOG(WARNING) << " vehicle inpute init failure."
                         << "mProbability[AppearLaneId].size() != mProbability[VehParams].size()";
            return false;
          }

          mSrcTimeHeadway = mRawXSDVehInputPtr->timeHeadway();
          mTimeDistribution = mRawXSDVehInputPtr->distribution();
          mSrcDuration = mRawXSDVehInputPtr->duration();
          mSrcTimeVaryDistributionParams = mRawXSDVehInputPtr->TimeVaryingDistribution();
          mCurTimeVaryDistribution.clear_value();
          mCurTimeHeadway = UpdateTimeHeadway();
          std::ostringstream oss;
          for (const auto& refParam : mSrcTimeVaryDistributionParams) {
            oss << refParam.Str() << ";";
          }
          LogInfo << "[TimeVarying]" << TX_VARS(VehInputId())
                  << TX_VARS_NAME(TimeVarySize, mSrcTimeVaryDistributionParams.size()) << oss.str();
          return true;
        }
      } else {
        LOG(WARNING) << "unknown Composition id : " << comId;
      }
    } else {
      LOG(WARNING) << "unknown Location id : " << locId;
    }
  }

  mValid = false;
  mIndex = 0;
  LOG(WARNING) << " vehicle inpute init failure.";
  return false;
}

Base::txBool VehicleInputeAgent::ResetInputAgent() TX_NOEXCEPT {
  mCurDudation = 0.0;
  mCurPeriod = 0.0;
  mCurTimeHeadway = UpdateTimeHeadway();
  mIndex = 0;
  return true;
}

Base::txBool VehicleInputeAgent::ResetPeriod() TX_NOEXCEPT {
  mCurPeriod = mCurTimeHeadway;
  return true;
}

Base::txBool VehicleInputeAgent::UpdatePeriod(const Base::txFloat timeStepInSecond) TX_NOEXCEPT {
  if (Distribution::TimeVarying == mTimeDistribution) {
    for (auto& refDis : mSrcTimeVaryDistributionParams) {
      if (refDis.IsValid() && mCurDudation > refDis.start_time_s) {
        mCurTimeVaryDistribution = refDis;
        refDis.valid = false;
        mCurTimeHeadway = UpdateTimeHeadway();
        LOG(INFO) << "[TimeVarying]" << TX_VARS(VehInputId()) << " reset input distribution, " << TX_VARS(mCurDudation)
                  << TX_VARS(refDis.start_time_s) << TX_VARS(mCurTimeHeadway) << TX_VARS(refDis.Str());
        break;
      }
    }
  }
  LOG_IF(INFO, 0) << TX_VARS(mCurPeriod) << TX_VARS(mCurTimeHeadway) << TX_VARS(mCurDudation) << TX_VARS(mSrcDuration);
  if (mCurPeriod < (mCurTimeHeadway - Math::EPSILON)) {
    mCurDudation += timeStepInSecond;
    mCurPeriod += timeStepInSecond;
    return false;
  } else {
    mCurDudation += timeStepInSecond;
    mCurTimeHeadway = UpdateTimeHeadway();
    mCurPeriod = timeStepInSecond;
    return true;
  }
}

Base::txInt VehicleInputeAgent::VehInputId() const TX_NOEXCEPT {
  if (IsValid() && NonNull_Pointer(mRawXSDVehInputPtr)) {
    return (mRawXSDVehInputPtr->id());
  }
  return InvalidId;
}

std::tuple<VehicleInputeAgent::VehicleInitParam_t, Base::txLaneID, VehicleInputeAgent::HashedLaneInfo,
           VehicleInputeAgent::HashedLaneInfoOrthogonalListPtr>
VehicleInputeAgent::GetNextProbabilityVehicleInitParam() TX_NOEXCEPT {
  if (IsValid()) {
    mIndex++;
    if (mIndex >= mProbability[0].size()) {
      mIndex = 0;
    }
    mCandidateVehicleInitParamVector[mProbability[VehParams][mIndex]].UpateRndKinect();
    return std::make_tuple(mCandidateVehicleInitParamVector[mProbability[VehParams][mIndex]],
                           mCandidateLaneIdVector[mProbability[AppearLaneId][mIndex]],
                           std::get<0>(m_vec_HashedRoadInfoCache[mProbability[AppearLaneId][mIndex]]),
                           std::get<1>(m_vec_HashedRoadInfoCache[mProbability[AppearLaneId][mIndex]]));
  } else {
    return std::make_tuple(VehicleInputeAgent::VehicleInitParam_t(), 0, HashedLaneInfo(), nullptr);
  }
}

Base::txFloat VehicleInputeAgent::UpdateTimeHeadway() TX_NOEXCEPT {
  if (Distribution::Fixed == mTimeDistribution) {
    if (FLAGS_EnableCrashJam) {
      // LOG(INFO) << TX_VARS(FLAGS_Active_TimeHead);
      return FLAGS_Active_TimeHead;
    } else {
      return mSrcTimeHeadway;
    }
  } else if (Distribution::Uniform == mTimeDistribution) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(mSrcTimeHeadway * 0.8, mSrcTimeHeadway * 1.2);
    return dis(gen);
  } else if (Distribution::Exponential == mTimeDistribution) {
    Base::txFloat pv = 0.0;
    pv = (Base::txFloat)(rand() % 100) / 100;
    while (pv == 0) {
      pv = (Base::txFloat)(rand() % 100) / 100;
    }
    pv = (-1.0 * mSrcTimeHeadway) * log(1 - pv);
    return pv;
  } else if (Distribution::TimeVarying == mTimeDistribution) {
    if (mCurTimeVaryDistribution.has_value()) {
      const Base::txFloat new_time_headway = (*mCurTimeVaryDistribution).NextValue();
      LOG(INFO) << "[TimeVarying]" << TX_VARS(VehInputId()) << " update TimeHeadway "
                << TX_VARS_NAME(from, mCurTimeHeadway) << TX_VARS_NAME(to, new_time_headway);
      return new_time_headway;
    } else {
      return FLT_MAX;
    }
  }
  return mSrcTimeHeadway;
}

Base::txString VehicleInputeAgent::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "VehicleInputeAgent " << TX_VARS(VehInputId()) << TX_VARS(mCurDudation) << TX_VARS(mCurPeriod)
      << TX_VARS(mCurTimeHeadway) << TX_COND(mCurPeriod < mCurTimeHeadway);
  return oss.str();
}

#endif /*__TX_Mark__("DITW_ElementGenerator::VehicleInputeAgent")*/

txBool ISceneElementGenerator::Release() TX_NOEXCEPT {
  LocationAgent::ClearLocationAgent();
  return true;
}

void VehicleInputeAgent::VehicleInitParamWithRandomKinect::UpateRndKinect() TX_NOEXCEPT {
  if (_NonEmpty_(mVecRandomKinectInfo)) {
    mRndKinectIdx++;
    if (mRndKinectIdx < 0 || mRndKinectIdx >= mVecRandomKinectInfo.size()) {
      mRndKinectIdx = 0;
    }
    mRndKinectInfo = mVecRandomKinectInfo[mRndKinectIdx];
  } else {
    mRndKinectInfo = NativeKinect();
  }
}

void VehicleInputeAgent::VehicleInitParamWithRandomKinect::GenerateRndKinect(const txFloat v, const txFloat max_v,
                                                                             const txFloat variance) TX_NOEXCEPT {
  mVecRandomKinectInfo.clear();
  mRndKinectIdx = 0;
  const std::vector<txFloat> v_list = Base::PseudoRandomDiscreteDistribution::GenerateProbabilityVector(v, variance);
  const std::vector<txFloat> max_v_list =
      Base::PseudoRandomDiscreteDistribution::GenerateProbabilityVector(max_v, variance);

  if (v_list.size() == max_v_list.size() && _NonEmpty_(v_list)) {
    for (Base::txSize idx = 0; idx < v_list.size(); ++idx) {
      mVecRandomKinectInfo.emplace_back(RandomKinectInfo_t{v_list[idx], max_v_list[idx]});
    }
  }
}

TX_NAMESPACE_CLOSE(Base)
#undef LogInfo

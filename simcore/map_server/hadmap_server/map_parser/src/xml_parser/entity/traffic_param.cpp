/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "traffic_param.h"
#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "batch_scene_param.h"
#include "common/engine/constant.h"
#include "common/log/xml_logger.h"
#include "common/xml_parser/entity/parser_tools.h"
#include "single_vehicle_info.h"

CTrafficParam::CTrafficParam() {}

CTrafficParam::CTrafficParam(CTraffic& traffic) { (*this) = traffic; }

CTrafficParam& CTrafficParam::operator=(CTraffic& traffic) {
  if (this == (&traffic)) {
    return (*this);
  }

  m_mapRoutes = traffic.Routes();
  m_mapAccs = traffic.Accs();
  m_mapMerges = traffic.Merges();
  m_mapVelocities = traffic.Velocities();
  m_mapPedestrianVelocities = traffic.PedestrianVelocities();
  m_mapVehicles = traffic.Vehicles();
  m_mapObstacles = traffic.Obstacles();
  m_mapPedestrians = traffic.Pedestrians();
  m_mapPedestriansV2 = traffic.PedestriansV2();
  m_mapSignlights = traffic.Signlights();

  m_RandomSeed = traffic.RandomSeed();
  m_dAggress = traffic.Aggress();
  m_nTrafficFileType = traffic.FileType();

  return (*this);
}

void CTrafficParam::SetToDefault() {
  m_mapVelocities.clear();

  AccelerationMap::iterator apItr = m_mapAccs.begin();
  for (; apItr != m_mapAccs.end(); ++apItr) {
    apItr->second.SetToDefault();
  }

  MergeMap::iterator mpItr = m_mapMerges.begin();
  for (; mpItr != m_mapMerges.end(); ++mpItr) {
    mpItr->second.SetToDefault();
  }
}

void CTrafficParam::GenerateAllSinglsFactors() {
  std::map<std::string, CVehicleParam>::iterator itr = m_mapVehicleParams.begin();
  for (; itr != m_mapVehicleParams.end(); ++itr) {
    itr->second.m_GeneratedVehicles.clear();
    itr->second.Generate(itr->second.m_GeneratedVehicles);
  }

  VelocityParamMap::iterator vpIter = m_mapVelocityParams.begin();
  for (; vpIter != m_mapVelocityParams.end(); ++vpIter) {
    vpIter->second.m_generatedVelocities.clear();
    vpIter->second.Generate(vpIter->second.m_generatedVelocities);
  }

  AccelerationParamMap::iterator apItr = m_mapAccelerationParams.begin();
  for (; apItr != m_mapAccelerationParams.end(); ++apItr) {
    apItr->second.m_generatedAccs.clear();
    apItr->second.Generate(apItr->second.m_generatedAccs);
  }

  MergeParamMap::iterator mpItr = m_mapMergeParams.begin();
  for (; mpItr != m_mapMergeParams.end(); ++mpItr) {
    mpItr->second.m_generatedMerges.clear();
    mpItr->second.Generate(mpItr->second.m_generatedMerges);
  }
}

void CTrafficParam::CalculateAllSinglesFactors(sTagBatchSceneParam& param, uint64_t& ullCount) {
  uint64_t ullVehilceCount = 0;
  uint64_t ullVelocityCount = 1;
  uint64_t ullMergeCount = 1;
  uint64_t ullAccCount = 1;

  uint64_t ullVehicleParamedCount = m_mapVehicleParams.size();
  if (ullVehicleParamedCount == 0) {
    ullVehicleParamedCount = 1;
  }

  std::map<std::string, CVehicleParam>::iterator itr = m_mapVehicleParams.begin();
  for (; itr != m_mapVehicleParams.end(); ++itr) {
    uint64_t ullSinglVehicleCount = 0;
    itr->second.m_GeneratedVehicles.clear();
    ullSinglVehicleCount = itr->second.Generate(itr->second.m_GeneratedVehicles, true);
    ullVehilceCount += ullSinglVehicleCount;
  }
  if (ullVehilceCount == 0) {
    ullVehilceCount = 1;
  }

  VelocityParamMap::iterator vpIter = m_mapVelocityParams.begin();
  for (; vpIter != m_mapVelocityParams.end(); ++vpIter) {
    uint64_t ullSinglVelocityCount = 0;
    CVelocityParam::SequencedParamizedVelocityTimeNodes nodes;
    ullSinglVelocityCount = vpIter->second.ListVariations(nodes, true);
    ullVelocityCount *= ullSinglVelocityCount;
  }
  if (ullVelocityCount == 0) {
    ullVelocityCount = 1;
  }

  AccelerationParamMap::iterator apItr = m_mapAccelerationParams.begin();
  for (; apItr != m_mapAccelerationParams.end(); ++apItr) {
    uint64_t ullSinglAccCount = 0;
    CAccelerationParam::SequencedParamizedTimeNodes nodes;
    ullSinglAccCount = apItr->second.ListVariations(nodes, true);
    ullAccCount *= ullSinglAccCount;
  }
  if (ullAccCount == 0) {
    ullAccCount = 1;
  }

  MergeParamMap::iterator mpItr = m_mapMergeParams.begin();
  for (; mpItr != m_mapMergeParams.end(); ++mpItr) {
    uint64_t ullSinglMergeCount = 0;
    CMergeParam::SequencedParamizedMergeNodes nodes;
    ullSinglMergeCount = mpItr->second.ListVariations(nodes, true);
    ullMergeCount *= ullSinglMergeCount;
  }
  if (ullMergeCount == 0) {
    ullMergeCount = 1;
  }

  if (param.m_type == SCENE_BATCH_GEN_TYPE_SINGLE) {
    ullCount = ullVehilceCount * ullAccCount * ullMergeCount * ullVelocityCount;
  } else {
    ullCount = ullVehilceCount * (ullVehicleParamedCount * ullAccCount) * (ullVehicleParamedCount * ullMergeCount) *
               (ullVehicleParamedCount * ullVelocityCount);
  }
}

void CTrafficParam::CalculateAllSinglesFactorsV2(sTagBatchSceneParamV2& param, uint64_t& ullCount) {
  uint64_t ullVehilceCount = 1;
  uint64_t ullVelocityCount = 1;
  uint64_t ullMergeCount = 1;
  uint64_t ullAccCount = 1;

  /*
  uint64_t ullVehicleParamedCount = m_mapVehicleParams.size();
  if (ullVehicleParamedCount == 0)
  {
          ullVehicleParamedCount = 1;
  }
  */

  std::map<std::string, CVehicleParam>::iterator itr = m_mapVehicleParams.begin();
  for (; itr != m_mapVehicleParams.end(); ++itr) {
    uint64_t ullSinglVehicleCount = 0;
    itr->second.m_GeneratedVehicles.clear();
    ullSinglVehicleCount = itr->second.Generate(itr->second.m_GeneratedVehicles, true);
    ullVehilceCount *= (ullSinglVehicleCount);
  }

  if (ullVehilceCount == 0) {
    ullVehilceCount = 1;
  }

  VelocityParamMap::iterator vpIter = m_mapVelocityParams.begin();
  for (; vpIter != m_mapVelocityParams.end(); ++vpIter) {
    uint64_t ullSinglVelocityCount = 0;
    CVelocityParam::SequencedParamizedVelocityTimeNodes nodes;
    ullSinglVelocityCount = vpIter->second.ListVariations(nodes, true);
    ullVelocityCount *= ullSinglVelocityCount;
  }
  if (ullVelocityCount == 0) {
    ullVelocityCount = 1;
  }

  AccelerationParamMap::iterator apItr = m_mapAccelerationParams.begin();
  for (; apItr != m_mapAccelerationParams.end(); ++apItr) {
    uint64_t ullSinglAccCount = 0;
    CAccelerationParam::SequencedParamizedTimeNodes nodes;
    ullSinglAccCount = apItr->second.ListVariations(nodes, true);
    ullAccCount *= ullSinglAccCount;
  }
  if (ullAccCount == 0) {
    ullAccCount = 1;
  }

  MergeParamMap::iterator mpItr = m_mapMergeParams.begin();
  for (; mpItr != m_mapMergeParams.end(); ++mpItr) {
    uint64_t ullSinglMergeCount = 0;
    CMergeParam::SequencedParamizedMergeNodes nodes;
    ullSinglMergeCount = mpItr->second.ListVariations(nodes, true);
    ullMergeCount *= ullSinglMergeCount;
  }
  if (ullMergeCount == 0) {
    ullMergeCount = 1;
  }

  ullCount = ullVehilceCount * ullAccCount * ullMergeCount * ullVelocityCount;
}

void CTrafficParam::Generate() {
  XML_LOGGER_INFO("start generate from template!");

  uint64_t nFile = 0;
  m_mapTraffics.clear();

  CVehicleParam::Vehicles allVehicles;
  CVelocityParam::Velocitys allVelocities;
  CAccelerationParam::Accelerations allAccs;
  CMergeParam::Merges allMerges;

  std::map<std::string, CVehicleParam>::iterator itr = m_mapVehicleParams.begin();
  for (; itr != m_mapVehicleParams.end(); ++itr) {
    CVehicleParam vp = itr->second;

    CVehicleParam::Vehicles& vs = vp.GeneratedVehicles();
    CVehicleParam::Vehicles::iterator vitr = vs.begin();
    for (; vitr != vs.end(); ++vitr) {
      allVehicles.push_back(*vitr);
    }
  }

  VelocityParamMap::iterator vpIter = m_mapVelocityParams.begin();
  for (; vpIter != m_mapVelocityParams.end(); ++vpIter) {
    CVelocityParam::Velocitys& velocities = vpIter->second.GeneratedVelocitys();
    CVelocityParam::Velocitys::iterator velitr = velocities.begin();
    for (; velitr != velocities.end(); ++velitr) {
      allVelocities.push_back(*velitr);
    }
  }

  AccelerationParamMap::iterator apItr = m_mapAccelerationParams.begin();
  for (; apItr != m_mapAccelerationParams.end(); ++apItr) {
    CAccelerationParam::Accelerations& accs = apItr->second.GeneratedAccelerations();
    CAccelerationParam::Accelerations::iterator aItr = accs.begin();
    for (; aItr != accs.end(); ++aItr) {
      allAccs.push_back(*aItr);
    }
  }

  MergeParamMap::iterator mpItr = m_mapMergeParams.begin();
  for (; mpItr != m_mapMergeParams.end(); ++mpItr) {
    CMergeParam::Merges& merges = mpItr->second.GeneratedMerges();
    CMergeParam::Merges::iterator mItr = merges.begin();
    for (; mItr != merges.end(); ++mItr) {
      allMerges.push_back(*mItr);
    }
  }

  if (allVehicles.size() > 0) {
    CVehicleParam::Vehicles::iterator vitr = allVehicles.begin();
    for (; vitr != allVehicles.end(); ++vitr) {
      if (allAccs.size() > 0) {
        CAccelerationParam::Accelerations::iterator aItr = allAccs.begin();
        for (; aItr != allAccs.end(); ++aItr) {
          if (allMerges.size() > 0) {
            CMergeParam::Merges::iterator mItr = allMerges.begin();
            for (; mItr != allMerges.end(); ++mItr) {
              if (allVelocities.size() > 0) {
                CVelocityParam::Velocitys::iterator velitr = allVelocities.begin();
                for (; velitr != allVelocities.end(); ++velitr) {
                  CTraffic tr(*this);
                  (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);
                  (*tr.AccsMap())[aItr->m_strID] = (*aItr);
                  (*tr.MergesMap())[mItr->m_strID] = (*mItr);
                  (*tr.VelocitiesMap())[velitr->m_strID] = (*velitr);

                  nFile++;
                  std::string strID = boost::lexical_cast<std::string>(nFile);
                  m_mapTraffics.insert(std::make_pair(strID, tr));
                }
              } else {
                CTraffic tr(*this);
                (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);
                (*tr.AccsMap())[aItr->m_strID] = (*aItr);
                (*tr.MergesMap())[mItr->m_strID] = (*mItr);

                nFile++;
                std::string strID = boost::lexical_cast<std::string>(nFile);
                m_mapTraffics.insert(std::make_pair(strID, tr));
              }
            }
          } else {
            if (allVelocities.size() > 0) {
              CVelocityParam::Velocitys::iterator velitr = allVelocities.begin();
              for (; velitr != allVelocities.end(); ++velitr) {
                CTraffic tr(*this);
                (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);
                (*tr.AccsMap())[aItr->m_strID] = (*aItr);
                (*tr.VelocitiesMap())[velitr->m_strID] = (*velitr);

                nFile++;
                std::string strID = boost::lexical_cast<std::string>(nFile);
                m_mapTraffics.insert(std::make_pair(strID, tr));
              }
            } else {
              CTraffic tr(*this);
              (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);
              (*tr.AccsMap())[aItr->m_strID] = (*aItr);

              CTraffic::AccelerationMap& accs = tr.Accs();
              accs[aItr->m_strID] = (*aItr);

              nFile++;
              std::string strID = boost::lexical_cast<std::string>(nFile);
              m_mapTraffics.insert(std::make_pair(strID, tr));
            }
          }
        }
      } else {
        if (allMerges.size() > 0) {
          CMergeParam::Merges::iterator mItr = allMerges.begin();
          for (; mItr != allMerges.end(); ++mItr) {
            if (allVelocities.size() > 0) {
              CVelocityParam::Velocitys::iterator velitr = allVelocities.begin();
              for (; velitr != allVelocities.end(); ++velitr) {
                CTraffic tr(*this);
                (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);
                (*tr.MergesMap())[mItr->m_strID] = (*mItr);
                (*tr.VelocitiesMap())[velitr->m_strID] = (*velitr);

                nFile++;
                std::string strID = boost::lexical_cast<std::string>(nFile);
                m_mapTraffics.insert(std::make_pair(strID, tr));
              }
            } else {
              CTraffic tr(*this);
              (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);
              (*tr.MergesMap())[mItr->m_strID] = (*mItr);

              nFile++;
              std::string strID = boost::lexical_cast<std::string>(nFile);
              m_mapTraffics.insert(std::make_pair(strID, tr));
            }
          }
        } else {
          if (allVelocities.size() > 0) {
            CVelocityParam::Velocitys::iterator velitr = allVelocities.begin();
            for (; velitr != allVelocities.end(); ++velitr) {
              CTraffic tr(*this);
              (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);
              (*tr.VelocitiesMap())[velitr->m_strID] = (*velitr);

              nFile++;
              std::string strID = boost::lexical_cast<std::string>(nFile);
              m_mapTraffics.insert(std::make_pair(strID, tr));
            }
          } else {
            CTraffic tr(*this);
            (*tr.VehiclesMap())[vitr->m_strID] = (*vitr);

            nFile++;
            std::string strID = boost::lexical_cast<std::string>(nFile);
            m_mapTraffics.insert(std::make_pair(strID, tr));
          }
        }
      }
    }
  } else {
    CTraffic tr(*this);

    nFile++;
    std::string strID = boost::lexical_cast<std::string>(nFile);
    m_mapTraffics.insert(std::make_pair(strID, tr));
  }
}

int CTrafficParam::ComposeTraffic(AllVehiclesRelatedInfos& allVehilclesInfos, TrafficList& outTraffics) {
  if (allVehilclesInfos.size() == 0) {
    return 0;
  }

  SingleVehicleRelatedInfos vs = allVehilclesInfos.front();
  allVehilclesInfos.pop_front();

  if (allVehilclesInfos.size() == 0) {
    for (auto sc : vs) {
      CTraffic tr(*this);
      (*tr.VehiclesMap())[sc.m_vehicle.m_strID] = sc.m_vehicle;
      (*tr.AccsMap())[sc.m_accs.m_strID] = (sc.m_accs);
      (*tr.MergesMap())[sc.m_merge.m_strID] = (sc.m_merge);
      (*tr.VelocitiesMap())[sc.m_velocity.m_strID] = (sc.m_velocity);
      outTraffics.push_back(tr);
    }
  } else {
    ComposeTraffic(allVehilclesInfos, outTraffics);

    TrafficList tl;
    for (auto sc : vs) {
      for (auto t : outTraffics) {
        CTraffic tr(t);
        (*tr.VehiclesMap())[sc.m_vehicle.m_strID] = sc.m_vehicle;
        (*tr.AccsMap())[sc.m_accs.m_strID] = (sc.m_accs);
        (*tr.MergesMap())[sc.m_merge.m_strID] = (sc.m_merge);
        (*tr.VelocitiesMap())[sc.m_velocity.m_strID] = (sc.m_velocity);
        tl.push_back(tr);
      }
    }

    outTraffics.clear();
    outTraffics = tl;
  }

  return 0;
}

void CTrafficParam::GenerateV2() {
  XML_LOGGER_INFO("start generate from template!");

  m_mapVehiclesRelatedInfo.clear();

  AllVehiclesRelatedInfos allVehiclesInfos;

  std::map<std::string, CVehicleParam>::iterator itr = m_mapVehicleParams.begin();
  for (; itr != m_mapVehicleParams.end(); ++itr) {
    CVehicleParam vp = itr->second;

    CVehicleParam::Vehicles& vs = vp.GeneratedVehicles();
    CVehicleParam::Vehicles::iterator vitr = vs.begin();

    CVelocityParam::Velocitys velocities;
    CAccelerationParam::Accelerations accs;
    CMergeParam::Merges merges;

    std::string strVID = vitr->m_strID;
    // velocity
    auto veItr = m_mapVelocityParams.find(vitr->m_strID);
    if (veItr == m_mapVelocityParams.end()) {
      CVelocity v;
      v.m_strID = vitr->m_strID;
      velocities.push_back(v);
    } else {
      CVelocityParam::Velocitys& generatedVelocities = veItr->second.GeneratedVelocitys();
      if (generatedVelocities.size() == 0) {
        CVelocity v;
        v.m_strID = vitr->m_strID;
        velocities.push_back(v);
      } else {
        for (auto gv : generatedVelocities) {
          velocities.push_back(gv);
        }
      }
    }

    // acc
    auto apItr = m_mapAccelerationParams.find(vitr->m_strAccID);
    if (apItr == m_mapAccelerationParams.end()) {
      CAcceleration acc;
      acc.m_strID = vitr->m_strAccID;
      accs.push_back(acc);
    } else {
      CAccelerationParam::Accelerations& generatedAccs = apItr->second.GeneratedAccelerations();
      if (generatedAccs.size() == 0) {
        CAcceleration acc;
        acc.m_strID = vitr->m_strAccID;
        accs.push_back(acc);
      } else {
        for (auto ga : generatedAccs) {
          accs.push_back(ga);
        }
      }
    }

    // merge
    auto mpItr = m_mapMergeParams.find(vitr->m_strMergeID);
    if (mpItr == m_mapMergeParams.end()) {
      CMerge m;
      m.m_strID = vitr->m_strMergeID;
      merges.push_back(m);
    } else {
      CMergeParam::Merges& generatedMerges = mpItr->second.GeneratedMerges();
      if (generatedMerges.size() == 0) {
        CMerge m;
        m.m_strID = vitr->m_strMergeID;
        merges.push_back(m);
      } else {
        for (auto gm : generatedMerges) {
          merges.push_back(gm);
        }
      }
    }

    std::list<CSingleVehicleInfo> allvehicles;

    for (; vitr != vs.end(); ++vitr) {
      auto aaItr = accs.begin();
      for (; aaItr != accs.end(); ++aaItr) {
        auto mmItr = merges.begin();
        for (; mmItr != merges.end(); ++mmItr) {
          auto vvItr = velocities.begin();
          for (; vvItr != velocities.end(); ++vvItr) {
            CSingleVehicleInfo v;
            v.m_vehicle = (*vitr);
            v.m_accs = (*aaItr);
            v.m_merge = (*mmItr);
            v.m_velocity = (*vvItr);
            allvehicles.push_back(v);
          }
        }
      }
    }

    m_mapVehiclesRelatedInfo.insert(std::make_pair(strVID, allvehicles));
    allVehiclesInfos.push_back(allvehicles);
  }

  if (allVehiclesInfos.size() > 0) {
    ComposeTraffic(allVehiclesInfos, m_generatedAllTraffics);
  }
}

int CTrafficParam::CalculateGeneratedTraffCount(int64_t& count, int64_t& size) {
  count = 0;
  size = 0;

  int64_t vehicleCount = 0;
  std::map<std::string, CVehicleParam>::iterator itr = m_mapVehicleParams.begin();
  for (; itr != m_mapVehicleParams.end(); ++itr) {
    vehicleCount += itr->second.m_GeneratedVehicles.size();
  }
  if (vehicleCount == 0) {
    vehicleCount = 1;
  }

  int64_t velocityCount = 1;
  VelocityParamMap::iterator vpIter = m_mapVelocityParams.begin();
  for (; vpIter != m_mapVelocityParams.end(); ++vpIter) {
    velocityCount *= vpIter->second.m_generatedVelocities.size();
  }
  if (velocityCount == 0) {
    velocityCount = 1;
  }

  int64_t accCount = 1;
  AccelerationParamMap::iterator apItr = m_mapAccelerationParams.begin();
  for (; apItr != m_mapAccelerationParams.end(); ++apItr) {
    accCount *= apItr->second.m_generatedAccs.size();
  }
  if (accCount == 0) {
    accCount = 1;
  }

  int64_t mergeCount = 1;
  MergeParamMap::iterator mpItr = m_mapMergeParams.begin();
  for (; mpItr != m_mapMergeParams.end(); ++mpItr) {
    mergeCount *= mpItr->second.m_generatedMerges.size();
  }
  if (mergeCount == 0) {
    mergeCount = 1;
  }
  count = mergeCount * accCount * velocityCount * vehicleCount;

  return 0;
}

int CTrafficParam::Parse(const char* strFile) {
  int nRet = CTraffic::Parse(strFile);
  if (nRet != 0) {
    return nRet;
  }

  AccelerationMap& accs = Accs();
  AccelerationMap::iterator aitr = accs.begin();
  for (; aitr != accs.end(); ++aitr) {
    aitr->second.ConvertToEventValue();
    aitr->second.ConvertToTimeValue();
  }

  MergeMap& merges = Merges();
  MergeMap::iterator mitr = merges.begin();
  for (; mitr != merges.end(); ++mitr) {
    mitr->second.ConvertToEventValue();
    mitr->second.ConvertToTimeValue();
  }

  VehicleMap& vehicles = Vehicles();
  VehicleMap::iterator itr = vehicles.begin();
  for (; itr != vehicles.end(); itr++) {
    itr->second.ConvertToValue();
  }

  ObstacleMap& obstacles = Obstacles();
  ObstacleMap::iterator oitr = obstacles.begin();
  for (; oitr != obstacles.end(); oitr++) {
    oitr->second.ConvertToValue();
  }

  PedestrianMap& pedestrians = Pedestrians();
  PedestrianMap::iterator pitr = pedestrians.begin();
  for (; pitr != pedestrians.end(); pitr++) {
    pitr->second.ConvertToValue();
  }

  PedestrianV2Map& pedestriansV2 = PedestriansV2();
  PedestrianV2Map::iterator pitrV2 = pedestriansV2.begin();
  for (; pitrV2 != pedestriansV2.end(); pitrV2++) {
    pitrV2->second.ConvertToValue();
  }

  SignLightMap& signlights = Signlights();
  SignLightMap::iterator sitr = signlights.begin();
  for (; sitr != signlights.end(); sitr++) {
    sitr->second.ConvertToValue();
  }

  return 0;
}

int CTrafficParam::GetOneAccID() {
  int nOneAccID = CTraffic::GetOneAccID();

  AccelerationParamMap::iterator itr = m_mapAccelerationParams.begin();
  for (; itr != m_mapAccelerationParams.end(); ++itr) {
    std::string strID = itr->first;
    int nID = atoi(strID.c_str());
    if (nID > nOneAccID) {
      nOneAccID = nID;
    }
  }

  nOneAccID++;

  return nOneAccID;
}

int CTrafficParam::GetOneMergeID() {
  int nOneMergeID = CTraffic::GetOneMergeID();

  MergeParamMap::iterator itr = m_mapMergeParams.begin();
  for (; itr != m_mapMergeParams.end(); ++itr) {
    std::string strID = itr->first;
    int nID = atoi(strID.c_str());
    if (nID > nOneMergeID) {
      nOneMergeID = nID;
    }
  }

  nOneMergeID++;

  return nOneMergeID;
}

void CTrafficParam::SetParam(sTagBatchSceneParam& param) {
  VehicleMap& vehicles = Vehicles();

  std::string strDefaultAccID = GetDefaultAccStrID();
  std::string strDefaultMergeID = GetDefaultMergeStrID();

  if (param.m_type == SCENE_BATCH_GEN_TYPE_SINGLE) {
    std::string strID = std::to_string(param.m_vehicle_id);

    VehicleMap::iterator itr = vehicles.find(strID);
    if (itr != vehicles.end()) {
      CVehicleParam vp(itr->second);
      vp.m_fStartShiftStart = param.m_vehicle_pos_start;
      vp.m_fStartShiftEnd = param.m_vehicle_pos_end;
      vp.m_fStartShiftSep = param.m_vehicle_pos_sep;
      vp.m_fStartOffsetStart = param.m_vehicle_offset_start;
      vp.m_fStartOffsetEnd = param.m_vehicle_offset_end;
      vp.m_fStartOffsetSep = param.m_vehicle_offset_sep;
      vp.m_fStartVelocityStart = param.m_vehicle_velocity_start;
      vp.m_fStartVelocityEnd = param.m_vehicle_velocity_end;
      vp.m_fStartVelocitySep = param.m_vehicle_velocity_sep;
      m_mapVehicleParams.insert(std::make_pair(vp.m_strID, vp));

      if (boost::iequals("UserDefine", itr->second.m_strBehavior)) {
        if (boost::iequals(strDefaultAccID, vp.m_strAccID)) {
          std::string strAccID = GetOneAccStrID();
          vp.m_strAccID = strAccID;
          param.m_acceleration_param.m_strID = strAccID;
          param.m_acceleration_param.m_ID = atoi(strAccID.c_str());
        } else {
          param.m_acceleration_param.m_strID = vp.m_strAccID;
          param.m_acceleration_param.m_ID = atoi(vp.m_strAccID.c_str());
        }
        m_mapAccelerationParams.insert(std::make_pair(vp.m_strAccID, param.m_acceleration_param));

        if (boost::iequals(strDefaultMergeID, vp.m_strMergeID)) {
          std::string strMergeID = GetOneMergeStrID();
          vp.m_strMergeID = strMergeID;
          param.m_merge_param.m_strID = strMergeID;
          param.m_merge_param.m_ID = atoi(strMergeID.c_str());

        } else {
          param.m_merge_param.m_strID = vp.m_strMergeID;
          param.m_merge_param.m_ID = atoi(vp.m_strMergeID.c_str());
        }
        m_mapMergeParams.insert(std::make_pair(vp.m_strMergeID, param.m_merge_param));

        param.m_velocity_param.m_strID = vp.m_strID;
        param.m_velocity_param.m_ID = atoi(vp.m_strID.c_str());
        m_mapVelocityParams.insert(std::make_pair(vp.m_strID, param.m_velocity_param));
      }
    }

  } else {
    VehicleMap::iterator itr = vehicles.begin();
    for (; itr != vehicles.end(); ++itr) {
      CVehicleParam vp(itr->second);
      vp.m_fStartShiftStart = param.m_vehicle_pos_start;
      vp.m_fStartShiftEnd = param.m_vehicle_pos_end;
      vp.m_fStartShiftSep = param.m_vehicle_pos_sep;
      vp.m_fStartOffsetStart = param.m_vehicle_offset_start;
      vp.m_fStartOffsetEnd = param.m_vehicle_offset_end;
      vp.m_fStartOffsetSep = param.m_vehicle_offset_sep;
      vp.m_fStartVelocityStart = param.m_vehicle_velocity_start;
      vp.m_fStartVelocityEnd = param.m_vehicle_velocity_end;
      vp.m_fStartVelocitySep = param.m_vehicle_velocity_sep;
      m_mapVehicleParams.insert(std::make_pair(vp.m_strID, vp));

      if (boost::iequals("TrafficVehicle", itr->second.m_strBehavior)) {
        continue;
      }

      if (boost::iequals(strDefaultAccID, vp.m_strAccID)) {
        std::string strAccID = GetOneAccStrID();
        vp.m_strAccID = strAccID;
        param.m_acceleration_param.m_strID = strAccID;
        param.m_acceleration_param.m_ID = atoi(strAccID.c_str());
      } else {
        param.m_acceleration_param.m_strID = vp.m_strAccID;
        param.m_acceleration_param.m_ID = atoi(vp.m_strAccID.c_str());
      }
      m_mapAccelerationParams.insert(std::make_pair(vp.m_strAccID, param.m_acceleration_param));

      if (boost::iequals(strDefaultMergeID, vp.m_strMergeID)) {
        std::string strMergeID = GetOneMergeStrID();
        vp.m_strMergeID = strMergeID;
        param.m_merge_param.m_strID = strMergeID;
        param.m_merge_param.m_ID = atoi(strMergeID.c_str());

      } else {
        param.m_merge_param.m_strID = vp.m_strMergeID;
        param.m_merge_param.m_ID = atoi(vp.m_strMergeID.c_str());
      }
      m_mapMergeParams.insert(std::make_pair(vp.m_strMergeID, param.m_merge_param));

      param.m_velocity_param.m_strID = vp.m_strID;
      param.m_velocity_param.m_ID = atoi(vp.m_strID.c_str());
      m_mapVelocityParams.insert(std::make_pair(vp.m_strID, param.m_velocity_param));
    }
  }
}

void CTrafficParam::SetParamV2(sTagBatchSceneParamV2& param) {
  VehicleMap& vehicles = Vehicles();

  std::string strDefaultAccID = GetDefaultAccStrID();
  std::string strDefaultMergeID = GetDefaultMergeStrID();

  auto vItr = param.m_vehicles.begin();
  for (; vItr != param.m_vehicles.end(); ++vItr) {
    std::string strID = std::to_string(vItr->m_vehicle_id);
    VehicleMap::iterator itr = vehicles.find(strID);
    if (itr != vehicles.end()) {
      CVehicleParam vp(itr->second);
      vp.m_fStartShiftStart = vItr->m_vehicle_pos_start;
      vp.m_fStartShiftEnd = vItr->m_vehicle_pos_end;
      vp.m_fStartShiftSep = vItr->m_vehicle_pos_sep;
      vp.m_fStartOffsetStart = vItr->m_vehicle_offset_start;
      vp.m_fStartOffsetEnd = vItr->m_vehicle_offset_end;
      vp.m_fStartOffsetSep = vItr->m_vehicle_offset_sep;
      vp.m_fStartVelocityStart = vItr->m_vehicle_velocity_start;
      vp.m_fStartVelocityEnd = vItr->m_vehicle_velocity_end;
      vp.m_fStartVelocitySep = vItr->m_vehicle_velocity_sep;

      // if (boost::iequals("UserDefine", itr->second.m_strBehavior))
      {
        if (boost::iequals(strDefaultAccID, vp.m_strAccID)) {
          std::string strAccID = GetOneAccStrID();
          vp.m_strAccID = strAccID;
          vItr->m_acceleration_param.m_strID = strAccID;
          vItr->m_acceleration_param.m_ID = atoi(strAccID.c_str());
        } else {
          vItr->m_acceleration_param.m_strID = vp.m_strAccID;
          vItr->m_acceleration_param.m_ID = atoi(vp.m_strAccID.c_str());
        }
        m_mapAccelerationParams.insert(std::make_pair(vp.m_strAccID, vItr->m_acceleration_param));

        if (boost::iequals(strDefaultMergeID, vp.m_strMergeID)) {
          std::string strMergeID = GetOneMergeStrID();
          vp.m_strMergeID = strMergeID;
          vItr->m_merge_param.m_strID = strMergeID;
          vItr->m_merge_param.m_ID = atoi(strMergeID.c_str());

        } else {
          vItr->m_merge_param.m_strID = vp.m_strMergeID;
          vItr->m_merge_param.m_ID = atoi(vp.m_strMergeID.c_str());
        }
        m_mapMergeParams.insert(std::make_pair(vp.m_strMergeID, vItr->m_merge_param));

        vItr->m_velocity_param.m_strID = vp.m_strID;
        vItr->m_velocity_param.m_ID = atoi(vp.m_strID.c_str());
        m_mapVelocityParams.insert(std::make_pair(vp.m_strID, vItr->m_velocity_param));
      }

      m_mapVehicleParams.insert(std::make_pair(vp.m_strID, vp));
    }
  }
}

int CTrafficParam::ParseTemplate(const char* strFile) {
  XML_LOGGER_INFO("start parse template!");

  if (!strFile) return -1;

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strFile);

  if (!bRet) return -1;

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  // if (_stricmp(strName.c_str(), "traffic") != 0)
  if (!boost::algorithm::iequals(strName, "traffic")) return -1;

  /*TiXmlElement* elemData = xmlRoot->FirstChildElement("data");
  if (elemData)
  {
          int nRet = ParseData(elemData, m_mapRoutes);
          if (nRet != 0)
          {
                  return -1;
          }
  }*/

  TiXmlElement* elemRoutes = xmlRoot->FirstChildElement("routes");
  int nRet = ParseRoutes(elemRoutes, m_mapRoutes);
  if (nRet != 0) {
    return -1;
  }

  TiXmlElement* elemAccs = xmlRoot->FirstChildElement("accs");
  nRet = ParseAccs(elemAccs, m_mapAccs);
  if (nRet != 0) {
    return -1;
  }

  TiXmlElement* elemMerges = xmlRoot->FirstChildElement("merges");
  nRet = ParseMerges(elemMerges, m_mapMerges);
  if (nRet != 0) {
    return -1;
  }

  TiXmlElement* elemVehicles = xmlRoot->FirstChildElement("vehicles");
  nRet = ParseVehicles(elemVehicles, m_mapVehicleParams);
  if (nRet != 0) {
    return -1;
  }

  m_strFileName = strFile;
  std::vector<std::string> strs;
  std::string strSep = ",";
  CParseTools::SplitString(m_strFileName, strs, strSep);
  m_strFileName = strs[0];

  return 0;
}

int CTrafficParam::ParseVehicles(TiXmlElement* VehiclesElem, std::map<std::string, CVehicleParam>& mapVehicles) {
  if (!VehiclesElem) return -1;

  mapVehicles.clear();

  TiXmlElement* elemVehicle = VehiclesElem->FirstChildElement("vehicle");
  while (elemVehicle) {
    CVehicleParam v;

    int nRet = ParseOneVehicle(elemVehicle, v);
    if (nRet) {
      std::cout << "vehicle format error!" << std::endl;
      assert(false);
    } else {
      mapVehicles.insert(std::make_pair(v.m_strID, v));
    }

    elemVehicle = elemVehicle->NextSiblingElement("vehicle");
  }

  return 0;
}

int CTrafficParam::ParseOneVehicle(TiXmlElement* elemVehicle, CVehicleParam& v) {
  if (!elemVehicle) return -1;

  const char* p = elemVehicle->Attribute("id");
  if (p) v.m_strID = p;

  p = elemVehicle->Attribute("routeID");
  if (p) v.m_strRouteID = p;

  p = elemVehicle->Attribute("laneID");
  if (p) v.m_strLaneID = p;

  p = elemVehicle->Attribute("start_s");
  if (p) {
    v.m_strStartShift = p;

    std::vector<std::string> strs;
    std::string strSep = ",";
    CParseTools::SplitString(v.m_strStartShift, strs, strSep);

    if (strs.size() != 3) {
      assert(false);
    }

    v.m_fStartShiftStart = atof(strs[0].c_str());
    v.m_fStartShiftEnd = atof(strs[1].c_str());
    v.m_fStartShiftSep = atof(strs[2].c_str());
  }

  p = elemVehicle->Attribute("start_t");
  if (p) v.m_strStartTime = p;

  p = elemVehicle->Attribute("start_v");
  if (p) {
    v.m_strStartVelocity = p;

    std::vector<std::string> strs;
    std::string strSep = ",";
    CParseTools::SplitString(v.m_strStartVelocity, strs, strSep);

    if (strs.size() != 3) {
      assert(false);
    }

    v.m_fStartVelocityStart = atof(strs[0].c_str());
    v.m_fStartVelocityEnd = atof(strs[1].c_str());
    v.m_fStartVelocitySep = atof(strs[2].c_str());
  }

  p = elemVehicle->Attribute("l_offset");
  if (p) v.m_strOffset = p;

  p = elemVehicle->Attribute("length");
  if (p) v.m_boundingBox.m_strLength = p;

  p = elemVehicle->Attribute("width");
  if (p) v.m_boundingBox.m_strWidth = p;

  p = elemVehicle->Attribute("height");
  if (p) v.m_boundingBox.m_strHeight = p;

  p = elemVehicle->Attribute("accID");
  if (p) v.m_strAccID = p;

  p = elemVehicle->Attribute("mergeID");
  if (p) v.m_strMergeID = p;

  p = elemVehicle->Attribute("vehicleType");
  if (p) v.m_strType = p;

  return 0;
}

int CTrafficParam::Save(const char* strFileName) {
  XML_LOGGER_INFO("start save generated files!");

  std::string strSaveName = strFileName;
  std::vector<std::string> strs;
  std::string strSep = ".";
  CParseTools::SplitString(strSaveName, strs, strSep);
  strSaveName = strs[0];

  int nFile = 0;
  std::map<std::string, CTraffic>::iterator itr = m_mapTraffics.begin();
  for (; itr != m_mapTraffics.end(); ++itr) {
    char strID[16] = {0};
    sprintf(strID, "%d", nFile);
    std::string strFileNameNew = strSaveName;
    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append(".xml");

    itr->second.Save(strFileNameNew.c_str());
    nFile++;
  }

  return 0;
}

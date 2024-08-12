// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_cloud_loader.h"
#include <boost/algorithm/string.hpp> /*string splite*/
#include <boost/format.hpp>
#include "tx_path_utils.h"
#include "tx_protobuf_utils.h"
#include "tx_scene_loader.h"
#include "tx_string_utils.h"
#include "tx_tc_gflags.h"
TX_NAMESPACE_OPEN(SceneLoader)

#if __TX_Mark__("TAD_Cloud_LocationViewer")

TAD_Cloud_SceneLoader::TAD_Cloud_LocationViewer::TAD_Cloud_LocationViewer() : m_inited(false) {}

void TAD_Cloud_SceneLoader::TAD_Cloud_LocationViewer::Init(const Location& srcLocation) TX_NOEXCEPT {
  m_location = srcLocation;
  m_inited = true;
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_LocationViewer::id() const TX_NOEXCEPT {
  return (m_inited) ? (stoll(m_location.id)) : (InvalidId);
}

hadmap::txPoint TAD_Cloud_SceneLoader::TAD_Cloud_LocationViewer::PosGPS() const TX_NOEXCEPT {
  if (m_inited) {
    txString raw_str = m_location.Pos;
    Utils::trim(raw_str);
    if (CallFail(raw_str.empty())) {
      std::vector<Base::txString> results_lon_lat;
      boost::algorithm::split(results_lon_lat, raw_str, boost::is_any_of(","));
      if ((results_lon_lat.size() > 1)) {
        Utils::trim(results_lon_lat[0]);
        Utils::trim(results_lon_lat[1]);
        if (CallFail(results_lon_lat[0].empty()) && CallFail(results_lon_lat[1].empty())) {
          return hadmap::txPoint(std::stod(results_lon_lat[0]), std::stod(results_lon_lat[1]), FLAGS_default_altitude);
        }
      }
    }
  }
  LOG(WARNING) << "Get Location Pos Failure.";
  return hadmap::txPoint();
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_LocationViewer::info() const TX_NOEXCEPT {
  return (m_inited) ? (m_location.info) : ("uninited");
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_LocationViewer::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, m_location.id) << TX_VARS_NAME(Pos, m_location.Pos)
      << TX_VARS_NAME(info, m_location.info);
  return oss.str();
}
#endif /*__TX_Mark__("TAD_Cloud_LocationViewer")*/

#if __TX_Mark__("TTAD_Cloud_TrafficFlowViewer")

#  if __TX_Mark__("TAD_Cloud_VehType")

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::TAD_Cloud_VehType() : m_inited(false) {}

void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::Init(const VehType& srcVehType)
    TX_NOEXCEPT {
  m_VehType = srcVehType;
  m_inited = true;
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::id() const TX_NOEXCEPT {
  return (m_inited) ? (stoll(m_VehType.id)) : (InvalidId);
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::vehicleType() const TX_NOEXCEPT {
  if (m_inited) {
    return m_VehType.Type;
  }
  return Base::txString("Sedan");
}

Base::ISceneLoader::VEHICLE_BEHAVIOR TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::behavior()
    const TX_NOEXCEPT {
  if (m_inited) {
    return Base::ISceneLoader::Str2VehicleBehavior(m_VehType.behavior);
  }
  return +Base::ISceneLoader::VEHICLE_BEHAVIOR::undefined;
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::length() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehType.length) : (0.0);
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::width() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehType.width) : (0.0);
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::height() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehType.height) : (0.0);
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehType::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, m_VehType.id)
      << TX_VARS_NAME(behavior, m_VehType.behavior) << TX_VARS_NAME(length, m_VehType.length)
      << TX_VARS_NAME(width, m_VehType.width) << TX_VARS_NAME(height, m_VehType.height);
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Cloud_VehType")*/

#  if __TX_Mark__("TAD_Cloud_VehComp")

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::TAD_Cloud_VehComp() : m_inited(false) {}

void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::Init(const VehComp& srcVehComp)
    TX_NOEXCEPT {
  m_VehComp = srcVehComp;
  m_inited = true;
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.id) : (InvalidId);
}

#    if 1
Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::Type(const txInt idx) const
    TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.Type(idx)) : (IVehComp::Type(idx));
}

// for vitrual city senece editor
Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::TypeStr(const txInt idx) const
    TX_NOEXCEPT {
  return (m_inited) ? m_VehComp.Type(idx) : "";
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::Percentage(const txInt idx) const
    TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.Percentage(idx)) : (IVehComp::Percentage(idx));
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::Behavior(const txInt idx) const
    TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehComp.Behavior(idx)) : (IVehComp::Behavior(idx));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::Aggress(const txInt idx) const
    TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehComp.Aggress(idx)) : (IVehComp::Aggress(idx));
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::IsValid(const txInt idx) const
    TX_NOEXCEPT {
  return (m_inited) && (_NonEmpty_(m_VehComp.Type(idx))) && (_NonEmpty_(m_VehComp.Percentage(idx))) &&
         (_NonEmpty_(m_VehComp.Behavior(idx))) && (_NonEmpty_(m_VehComp.Aggress(idx)));
}
#    endif

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::Info() const TX_NOEXCEPT {
  return (m_inited) ? (m_VehComp.Info) : ("uninited");
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id());
  for (Base::txInt idx = 0; idx < m_VehComp.CompNodeArray.size(); ++idx) {
    oss << TX_VARS(idx) << TX_VARS_NAME(Type, Type(idx)) << TX_VARS_NAME(Percentage, Percentage(idx))
        << TX_VARS_NAME(Behavior, Behavior(idx)) << TX_VARS_NAME(Aggress, Aggress(idx));
  }
  oss << TX_VARS_NAME(Info, Info());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Cloud_VehComp")*/

#  if __TX_Mark__("TAD_Cloud_VehInput")

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::TAD_Cloud_VehInput() : m_inited(false) {}

void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::Init(const VehInput& srcVehInput)
    TX_NOEXCEPT {
  m_VehInput = srcVehInput;
  m_inited = true;
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.id) : (InvalidId);
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::location() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.Location) : (IVehInput::location());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::composition() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.Composition) : (IVehInput::composition());
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::start_v() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehInput.start_v) : (IVehInput::start_v());
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::max_v() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehInput.max_v) : (IVehInput::max_v());
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::halfRange_v() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_VehInput.halfRange_v)) {
    return stod(m_VehInput.halfRange_v);
  }
  return IVehInput::halfRange_v();
  /*return (m_inited) ? stod(m_VehInput.halfRange_v) : (IVehInput::halfRange_v());*/
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::Distribution
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::distribution() const TX_NOEXCEPT {
  return (m_inited) ? (IVehInput::Str2Distribution(m_VehInput.Distribution)) : (IVehInput::distribution());
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::TimeVaryingDistributionParamVec
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::TimeVaryingDistribution() const TX_NOEXCEPT {
  if (Distribution::TimeVarying == distribution()) {
    TimeVaryingDistributionParamVec retV;
    /*profile="startTimeInSecond,Mean,Variance;startTimeInSecond,Mean,Variance;"*/
    auto refResult = Utils::SpliteStringVector(m_VehInput.Profile, ";");
    for (auto strCurDistribution : refResult) {
      Utils::trim(strCurDistribution);
      if (_NonEmpty_(strCurDistribution)) {
        auto refElem = Utils::SpliteStringVector(strCurDistribution, ",");
        if (3 == refElem.size()) {
          TimeVaryingDistributionParam newNode;
          Utils::trim(refElem[0]);
          Utils::trim(refElem[1]);
          Utils::trim(refElem[2]);
          if (_NonEmpty_(refElem[0]) && _NonEmpty_(refElem[1]) && _NonEmpty_(refElem[2])) {
            newNode.start_time_s = std::stod(refElem[0]);
            newNode.mean_value = std::stod(refElem[1]);
            newNode.variance = std::stod(refElem[2]);
            if (CallSucc(newNode.Check())) {
              newNode.Generate();
              retV.emplace_back(std::move(newNode));
            } else {
              LOG(WARNING) << "TimeVaryingDistribution element  invalid : " << TX_VARS(m_VehInput.id) << newNode.Str();
            }
          } else {
            LOG(WARNING) << "TimeVaryingDistribution parse error. " << TX_VARS(m_VehInput.id)
                         << TX_VARS(strCurDistribution) << TX_VARS(refElem[0]) << TX_VARS(refElem[1])
                         << TX_VARS(refElem[2]);
          }
        } else {
          LOG(WARNING) << "TimeVaryingDistribution parse error. " << TX_VARS(m_VehInput.id)
                       << TX_VARS(strCurDistribution);
        }
      }
    }
    return retV;
  } else {
    return IVehInput::TimeVaryingDistribution();
  }
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::timeHeadway() const TX_NOEXCEPT {
  return (m_inited) ? stod(m_VehInput.TimeHeadway) : (IVehInput::timeHeadway());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::duration() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehInput.Duration) : (IVehInput::duration());
}

std::set<Base::txLaneID> TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::cover() const
    TX_NOEXCEPT {
  if (m_inited) {
    std::set<Base::txLaneID> res;
    if (m_VehInput.Cover.empty() || "100" == m_VehInput.Cover) {
      for (int i = 1; i < 20; ++i) {
        res.insert(-1 * i);
      }
    } else {
      const auto refResult = Utils::SpliteStringVector(m_VehInput.Cover, ",");
      for (const auto& refStr : refResult) {
        res.insert(std::atoi(refStr.c_str()));
      }
    }
    return res;
  } else {
    return IVehInput::cover();
  }
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::cover_str() const TX_NOEXCEPT {
  return (m_inited) ? (m_VehInput.Cover) : (IVehInput::cover_str());
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::Info() const TX_NOEXCEPT {
  return "";
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(location, location())
      << TX_VARS_NAME(composition, composition()) << TX_VARS_NAME(start_v, start_v()) << TX_VARS_NAME(max_v, max_v())
      << TX_VARS_NAME(halfRange_v, halfRange_v()) << TX_VARS_NAME(distribution, distribution())
      << TX_VARS_NAME(timeHeadway, timeHeadway()) << TX_VARS_NAME(duration, duration())
      << TX_VARS_NAME(cover, cover_str())

      << TX_VARS_NAME(Info, Info());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Cloud_VehInput")*/

#  if !__TX_Mark__("TAD_Cloud_PedInput")
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::TAD_Cloud_PedInput() : m_inited(false) {}
void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::Init(const PedInput& srcPedInput)
    TX_NOEXCEPT {
  m_PedInput = srcPedInput;
  m_inited = true;
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::id() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.id()) : (IPedInput::id());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::startLocation() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.StartLocation()) : (IPedInput::startLocation());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::ExitLocation() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.ExitLocation()) : (IPedInput::ExitLocation());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::composition() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.Composition()) : (IPedInput::composition());
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::start_v() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.start_v()) : (IPedInput::start_v());
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::max_v() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.max_v()) : (IPedInput::max_v());
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInput::Distribution
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::distribution() const TX_NOEXCEPT {
  return (m_inited) ? (IVehInput::Str2Distribution(m_PedInput.Distribution())) : (IPedInput::distribution());
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::timeHeadway() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.TimeHeadway()) : (IPedInput::timeHeadway());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::duration() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.Duration()) : (IPedInput::duration());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::Resp2Signal() const TX_NOEXCEPT {
  return (m_inited) ? (m_PedInput.Resp2Signal()) : (IPedInput::Resp2Signal());
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::Info() const TX_NOEXCEPT {
  return "";
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInput::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(startLocation, startLocation())
      << TX_VARS_NAME(ExitLocation, ExitLocation()) << TX_VARS_NAME(composition, composition())
      << TX_VARS_NAME(start_v, start_v())

      << TX_VARS_NAME(max_v, max_v()) << TX_VARS_NAME(distribution, distribution())
      << TX_VARS_NAME(timeHeadway, timeHeadway()) << TX_VARS_NAME(duration, duration())
      << TX_VARS_NAME(Resp2Signal, Resp2Signal()) << TX_VARS_NAME(Info, Info());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Cloud_PedInput")*/

#  if __TX_Mark__("TAD_Cloud_Beh")
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::TAD_Cloud_Beh() : m_inited(false) {}

void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::Init(const Beh& srcBeh) TX_NOEXCEPT {
  m_Beh = srcBeh;
  m_inited = true;
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_Beh.id) : (IBeh::id());
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::IBeh::BehType
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::type() const TX_NOEXCEPT {
  return (m_inited) ? (IBeh::Str2BehType(m_Beh.Type)) : (IBeh::type());
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::cc0() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.cc0)) {
    return stod(m_Beh.cc0);
  }
  return IBeh::cc0();
}
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::cc1() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.cc1)) {
    return stod(m_Beh.cc1);
  }
  return IBeh::cc1();
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::cc2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.cc2)) {
    return stod(m_Beh.cc2);
  }
  return IBeh::cc2();
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::AX() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.AX)) {
    return stod(m_Beh.AX);
  }
  return IBeh::AX();
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::BX_Add() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.BX_Add)) {
    return stod(m_Beh.BX_Add);
  }
  return IBeh::BX_Add();
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::LCduration() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.LCduration)) {
    return stod(m_Beh.LCduration);
  }
  return IBeh::LCduration();
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::BX_Mult() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_Beh.BX_Mult)) {
    return stod(m_Beh.BX_Mult);
  }
  return IBeh::BX_Mult();
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_Beh::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(type, type())
      << TX_VARS_NAME(cc0, cc0()) << TX_VARS_NAME(cc1, cc1()) << TX_VARS_NAME(cc2, cc2())

      << TX_VARS_NAME(AX, AX()) << TX_VARS_NAME(BX_Add, BX_Add()) << TX_VARS_NAME(BX_Mult, BX_Mult())
      << TX_VARS_NAME(LCduration, LCduration());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Cloud_Beh")*/

#  if __TX_Mark__("TAD_Cloud_VehExit")
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::TAD_Cloud_VehExit() : m_inited(false) {}

void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::Init(const VehExit& srcVehExit)
    TX_NOEXCEPT {
  m_VehExit = srcVehExit;
  m_inited = true;
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehExit.id) : (IVehExit::id());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::location() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_VehExit.Location) : (IVehExit::location());
}

std::set<Base::txLaneID> TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::cover() const
    TX_NOEXCEPT {
  if (m_inited) {
    std::set<Base::txLaneID> res;
    if (m_VehExit.Cover.empty() || "100" == m_VehExit.Cover) {
      for (int i = 1; i < 20; ++i) {
        res.insert(-1 * i);
      }
    } else {
      const auto refResult = Utils::SpliteStringVector(m_VehExit.Cover, ",");
      for (const auto& refStr : refResult) {
        res.insert(std::atoi(refStr.c_str()));
      }
    }
    return res;
  } else {
    return IVehExit::cover();
  }
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::cover_str() const TX_NOEXCEPT {
  return (m_inited) ? (m_VehExit.Cover) : (IVehExit::cover_str());
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::Info() const TX_NOEXCEPT {
  return "";
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExit::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(location, location())
      << TX_VARS_NAME(cover, cover_str()) << TX_VARS_NAME(Info, Info());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Cloud_VehExit")*/

#  if __TX_Mark__("TAD_Cloud_RouteGroup")

void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::Init(const RouteGroup& srcRouteGroup)
    TX_NOEXCEPT {
  m_RouteGroup = srcRouteGroup;
  m_inited = true;
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::TAD_Cloud_RouteGroup() : m_inited(false) {}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::id() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.id) : (IRouteGroup::id());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::start() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.Start) : (IRouteGroup::start());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::mid1() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.Mid1) : (IRouteGroup::mid1());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::end1() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.End1) : (IRouteGroup::end1());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::percentage1() const TX_NOEXCEPT {
  return (m_inited) ? stoi(m_RouteGroup.Percentage1) : (IRouteGroup::percentage1());
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::mid2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Mid2)) {
    return stoi(m_RouteGroup.Mid2);
  }
  return IRouteGroup::mid2();
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::end2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.End2)) {
    return stoi(m_RouteGroup.End2);
  }
  return IRouteGroup::end2();
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::percentage2() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Percentage2)) {
    return stoi(m_RouteGroup.Percentage2);
  }
  return IRouteGroup::percentage2();
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::mid3() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Mid3)) {
    return stoi(m_RouteGroup.Mid3);
  }
  return IRouteGroup::mid3();
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::end3() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.End3)) {
    return stoi(m_RouteGroup.End3);
  }
  return IRouteGroup::end3();
}

Base::txInt TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::percentage3() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_RouteGroup.Percentage3)) {
    return stoi(m_RouteGroup.Percentage3);
  }
  return IRouteGroup::percentage3();
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroup::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_COND_NAME(m_inited, m_inited) << TX_VARS_NAME(id, id()) << TX_VARS_NAME(start, start())
      << TX_VARS_NAME(mid1, mid1()) << TX_VARS_NAME(end1, end1()) << TX_VARS_NAME(percentage1, percentage1())

      << TX_VARS_NAME(mid2, mid2()) << TX_VARS_NAME(end2, end2()) << TX_VARS_NAME(percentage2, percentage2())

      << TX_VARS_NAME(mid3, mid3()) << TX_VARS_NAME(end3, end3()) << TX_VARS_NAME(percentage3, percentage3());
  return oss.str();
}

#  endif /*__TX_Mark__("TAD_Cloud_RouteGroup")*/

void TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::Init(const TrafficFlowXML& srcTrafficFlow) TX_NOEXCEPT {
  m_trafficflow = srcTrafficFlow;
  m_inited = true;
}

std::unordered_map<Base::txInt, TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::VehInputPtr>
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::GetAllVehInputData() const TX_NOEXCEPT {
  std::unordered_map<Base::txInt, VehInputPtr> retV;
  if (IsInited()) {
    for (const auto& refVehInput : m_trafficflow.VehicleInput) {
      TAD_Cloud_VehInputPtr curPtr = std::make_shared<TAD_Cloud_VehInput>();
      curPtr->Init(refVehInput);
      retV[curPtr->id()] = curPtr;
    }
  }
  return retV;
}
#  if 0
std::unordered_map<Base::txInt, TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::PedInputPtr>
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::GetAllPedInputData() const TX_NOEXCEPT {
    std::unordered_map<Base::txInt, PedInputPtr> retV;
    if (IsInited()) {
        for (const auto& refPedestrianInput : m_trafficflow.PedestrianInput()) {
            for (const auto& refPedInput : refPedestrianInput.PedInput()) {
                TAD_Cloud_PedInputPtr curPtr = std::make_shared<TAD_Cloud_PedInput>();
                curPtr->Init(refPedInput);
                retV[curPtr->id()] = curPtr;
            }
        }
    }
    return retV;
}
#  endif

std::unordered_map<Base::txInt, TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::RouteGroupPtr>
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::GetAllRouteGroupData() const TX_NOEXCEPT {
  std::unordered_map<Base::txInt, RouteGroupPtr> retV;
  if (IsInited()) {
    for (const auto& refRouteGroup : m_trafficflow.RouteGroups) {
      TAD_Cloud_RouteGroupPtr curPtr = std::make_shared<TAD_Cloud_RouteGroup>();
      curPtr->Init(refRouteGroup);
      retV[curPtr->id()] = curPtr;
    }
  }
  return retV;
}

std::unordered_map<Base::txInt, TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::VehExitPtr>
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::GetAllVehExitData() const TX_NOEXCEPT {
  std::unordered_map<Base::txInt, VehExitPtr> retV;
  if (IsInited()) {
    for (const auto& refVehExit : m_trafficflow.VehicleExit) {
      TAD_Cloud_VehExitPtr curPtr = std::make_shared<TAD_Cloud_VehExit>();
      curPtr->Init(refVehExit);
      retV[curPtr->id()] = curPtr;
    }
  }
  return retV;
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::VehCompPtr
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::GetVehComp(const txInt id) const TX_NOEXCEPT {
  if (m_inited) {
    const txString strObjId = std::to_string(id);
    for (const auto& refComp : m_trafficflow.VehicleComposition) {
      if (strObjId == refComp.id) {
        TAD_Cloud_VehCompPtr curPtr = std::make_shared<TAD_Cloud_VehComp>();
        curPtr->Init(refComp);
        return curPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find VehComp, id = " << id;
  return std::make_shared<TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehComp>();
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::VehTypePtr
TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::GetVehType(const Base::txInt id) const TX_NOEXCEPT {
  if (m_inited) {
    const txString strObjId = std::to_string(id);
    for (const auto& refVehType : m_trafficflow.VehicleType) {
      if (strObjId == refVehType.id) {
        TAD_Cloud_VehTypePtr curPtr = std::make_shared<TAD_Cloud_VehType>();
        curPtr->Init(refVehType);
        return curPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find VehType, id = " << id;
  return std::make_shared<TAD_Cloud_VehType>();
}

TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::BehPtr TAD_Cloud_SceneLoader::TAD_Cloud_TrafficFlowViewer::GetBeh(
    const txInt id) const TX_NOEXCEPT {
  if (m_inited) {
    const txString strObjId = std::to_string(id);
    for (const auto& refBeh : m_trafficflow.Behavior) {
      if (strObjId == refBeh.id) {
        TAD_Cloud_BehPtr curPtr = std::make_shared<TAD_Cloud_Beh>();
        curPtr->Init(refBeh);
        return curPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find Beh, id = " << id;
  return std::make_shared<TAD_Cloud_Beh>();
}

#endif /*__TX_Mark__("TTAD_Cloud_TrafficFlowViewer")*/

TAD_Cloud_SceneLoader::TAD_Cloud_SceneLoader() : _class_name(__func__) {}

TAD_Cloud_SceneLoader::ILocationViewerPtr TAD_Cloud_SceneLoader::GetLocationData(const Base::txUInt id) TX_NOEXCEPT {
  if (NonNull_Pointer(m_traffic_ptr)) {
    for (const auto& refLocation : (m_traffic_ptr->Locations)) {
      if (id == std::stoi(refLocation.id)) {
        TAD_Cloud_LocationViewerPtr retPtr = std::make_shared<TAD_Cloud_LocationViewer>();
        retPtr->Init(refLocation);
        return retPtr;
      }
    }
  }
  LOG(WARNING) << "Can not find location, id = " << id;
  return std::make_shared<TAD_Cloud_SceneLoader::ILocationViewer>();
}

TAD_Cloud_SceneLoader::ITrafficFlowViewerPtr TAD_Cloud_SceneLoader::GetTrafficFlow() TX_NOEXCEPT {
  if (NonNull_Pointer(m_traffic_ptr)) {
    const auto& refTrafficFlow = m_traffic_ptr->trafficFlow;
    TAD_Cloud_TrafficFlowViewerPtr retPtr = std::make_shared<TAD_Cloud_TrafficFlowViewer>();
    retPtr->Init(refTrafficFlow);
    return retPtr;
  }
  LOG(WARNING) << "TAD Cloud Traffic Flow Not Inited.";
  return std::make_shared<TAD_Cloud_TrafficFlowViewer>();
}

std::unordered_map<Base::txInt, Base::ISceneLoader::ILocationViewerPtr> TAD_Cloud_SceneLoader::GetAllLocationData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txInt, Base::ISceneLoader::ILocationViewerPtr> retV;
  if (NonNull_Pointer(m_traffic_ptr)) {
    const auto& refLocations = m_traffic_ptr->Locations;
    for (const auto& refLoc : refLocations) {
      TAD_Cloud_LocationViewerPtr tcLocationPtr = std::make_shared<TAD_Cloud_LocationViewer>();
      tcLocationPtr->Init(refLoc);
      retV[tcLocationPtr->id()] = tcLocationPtr;
    }
  }
  LOG(INFO) << TX_VARS_NAME(LocationCount, retV.size());
  return retV;
}

Base::txBool TAD_Cloud_SceneLoader::Load(Base::txString const& strTrafficXMLFilePath) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  _source = strTrafficXMLFilePath;

  if (exists(FilePath(_source))) {
    _status = +EStatus::eLoading;
    LOG(INFO) << "Loading TAD Cloud Traffic XML File : " << _source;
    m_traffic_ptr = SceneLoader::Traffic::load_scene_traffic(_source);

    if (ParseSceneEvent()) {
      for (const auto& ref : _seceneEventVec) {
        LOG(INFO) << (ref->Str());
      }
      LOG(INFO) << "parse scene event success.";
    } else {
      LOG(WARNING) << "parse scene event failure.";
    }

    if (0 != (m_traffic_ptr->rand_pedestrian.valid)) {
      const auto& ref_rand_pedestrian_conf = m_traffic_ptr->rand_pedestrian;
      FLAGS_need_random_obs_pedes = true;
      FLAGS_rnd_obs_row = ref_rand_pedestrian_conf.obs_row;
      FLAGS_rnd_obs_col = ref_rand_pedestrian_conf.obs_col;
      FLAGS_rnd_obs_valid_radius = ref_rand_pedestrian_conf.obs_valid_radius;
      FLAGS_obs_rnd_seed = ref_rand_pedestrian_conf.obs_rnd_seed;
      FLAGS_obs_valid_section_length = ref_rand_pedestrian_conf.obs_valid_section_length;
      FLAGS_pedestrian_valid_section_length = ref_rand_pedestrian_conf.pedestrian_valid_section_length;
      FLAGS_pedestrian_show = (1 == ref_rand_pedestrian_conf.pedestrian_show);
      FLAGS_pedestrian_velocity = ref_rand_pedestrian_conf.pedestrian_velocity;
      FLAGS_pedestrian_occurrence_dist = ref_rand_pedestrian_conf.pedestrian_occurrence_dist;
    } else {
      FLAGS_need_random_obs_pedes = false;
    }

    LOG(INFO) << "[virtual_city_random_element] " << TX_FLAGS(need_random_obs_pedes) << TX_FLAGS(rnd_obs_row)
              << TX_FLAGS(rnd_obs_col) << TX_FLAGS(rnd_obs_valid_radius) << TX_FLAGS(obs_rnd_seed)
              << TX_FLAGS(obs_valid_section_length) << TX_FLAGS(pedestrian_valid_section_length)
              << TX_FLAGS(pedestrian_show) << TX_FLAGS(pedestrian_velocity) << TX_FLAGS(pedestrian_occurrence_dist);
    _status = +EStatus::eReady;
    return NonNull_Pointer(m_traffic_ptr);
  } else {
    LOG(WARNING) << "Scene File is not existing. File : " << _source;
    _status = +EStatus::eError;
    return false;
  }
}

Base::txBool TAD_Cloud_SceneLoader::IsValid() TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  return (+EStatus::eReady == Status()) && (exists(FilePath(_source)));
}

Base::txBool TAD_Cloud_SceneLoader::GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
    TX_NOEXCEPT {
  txMsg("TAD_Cloud_SceneLoader::GetMapManagerInitParams");
  txPAUSE;
  /*refParams.strHdMapFilePath = FLAGS_tc_hadmap_path;
  refParams.SceneOriginGPS = hadmap::txPoint(116.1767915355885, 39.86443026827959, FLAGS_default_altitude);*/
  return true;
}

TAD_Cloud_SceneLoader::IVehiclesViewerPtr TAD_Cloud_SceneLoader::GetEgoData() TX_NOEXCEPT {
#if ON_CLOUD
  TAD_Cloud_EgoVehiclesViewerPtr retPtr = std::make_shared<TAD_Cloud_EgoVehiclesViewer>();
  retPtr->Init(m_EgoData);
  return retPtr;
#else  /*ON_CLOUD*/
  return nullptr;
#endif /*ON_CLOUD*/
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::IsIdValid() const TX_NOEXCEPT { return true; }
Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::routeID() const TX_NOEXCEPT { return -1; }
Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::IsRouteIdValid() const TX_NOEXCEPT { return false; }
Base::txLaneID TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::laneID() const TX_NOEXCEPT { return 0; }
Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::IsLaneIdValid() const TX_NOEXCEPT { return true; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::start_s() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::start_t() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::start_v() const TX_NOEXCEPT { return 16.0; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::max_v() const TX_NOEXCEPT { return 16.0; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::l_offset() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::lon() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().x()) : (0.0));
}
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::lat() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().y()) : (0.0));
}
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::alt() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::length() const TX_NOEXCEPT { return 4.68; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::width() const TX_NOEXCEPT { return 2.18; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::height() const TX_NOEXCEPT { return 1.76; }
Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::accID() const TX_NOEXCEPT { return -1; }
Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::IsAccIdValid() const TX_NOEXCEPT { return false; }
Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::mergeID() const TX_NOEXCEPT { return -1; }
Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::IsMergeIdValid() const TX_NOEXCEPT { return false; }
Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::behavior() const TX_NOEXCEPT {
  return Base::txString("Ego");
}
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::aggress() const TX_NOEXCEPT { return 0.5; }
Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::vehicleType() const TX_NOEXCEPT {
  return Base::txString("SUV");
}
Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::follow() const TX_NOEXCEPT {
  return FLAGS_Topological_Root_Id;
}
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::mergeTime() const TX_NOEXCEPT { return 0.0; }
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_EgoVehiclesViewer::offsetTime() const TX_NOEXCEPT { return 0.0; }

#if __TX_Mark__("TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer")

void TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::Init(SceneLoader::Traffic::Signlight const& srcSignlight)
    TX_NOEXCEPT {
  m_signlight = srcSignlight;
  m_inited = true;
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_signlight.id) : (-1));
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::routeID() const TX_NOEXCEPT {
  return (m_inited ? ((m_signlight.routeID)) : (-1));
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::IsRouteIdValid() const TX_NOEXCEPT {
  return m_inited && (routeID() >= 0);
}

Base::txLaneID TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::laneID() const TX_NOEXCEPT {
  LOG(FATAL) << ", Unsupport Props.";
  return -1;
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.start_s)) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::start_t() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.start_t)) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.l_offset)) : (0.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::time_green() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.time_green)) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::time_yellow() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.time_yellow)) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::time_red() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.time_red)) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::direction() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.direction)) : (0));
}
#  if USE_SignalByLane
Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::compliance() const TX_NOEXCEPT {
  return ((m_inited) ? ((m_signlight.compliance)) : (1.0));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::lane() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.lane) : ("ALL"));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::phase() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.phase) : ("A"));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer::status() const TX_NOEXCEPT {
  return ((m_inited) ? (m_signlight.status) : ("Activated"));
}
#  endif /*USE_SignalByLane*/
#endif   // __TX_Mark__("TAD_Cloud_SceneLoader::TAD_Cloud_SignlightsViewer")

std::unordered_map<Base::txSysId, Base::ISceneLoader::ISignlightsViewerPtr> TAD_Cloud_SceneLoader::GetAllSignlightData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::ISignlightsViewerPtr> retMap;
  if (NonNull_Pointer(m_traffic_ptr)) {
    for (auto& refSignlight : m_traffic_ptr->signlights) {
      auto retPtr = std::make_shared<TAD_Cloud_SignlightsViewer>();
      if (NonNull_Pointer(retPtr)) {
        retPtr->Init(refSignlight);
        retMap[(refSignlight.id)] = retPtr;
      }
    }
  }
  return retMap;
}

Base::ISceneLoader::ISignlightsViewerPtr TAD_Cloud_SceneLoader::GetSignlightData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_Cloud_SignlightsViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_traffic_ptr)) {
    for (auto& refSignlight : m_traffic_ptr->signlights) {
      if (id == (refSignlight.id)) {
        retPtr = std::make_shared<TAD_Cloud_SignlightsViewer>();
        if (NonNull_Pointer(retPtr)) {
          retPtr->Init(refSignlight);
        }
        break;
      }
    }
  }
  return retPtr;
}

Base::txInt TAD_Cloud_SceneLoader::GetRandomSeed() const TX_NOEXCEPT { return FLAGS_tc_random_seed; }

Base::txFloat TAD_Cloud_SceneLoader::GetRuleComplianceProportion() const TX_NOEXCEPT {
  if (NonNull_Pointer(m_traffic_ptr) && (m_traffic_ptr->data.ruleCompliance > 0)) {
    return m_traffic_ptr->data.ruleCompliance;
  } else {
    return FLAGS_DefaultRuleComplianceProportion;
  }
}

Base::ISceneLoader::IRouteViewerPtr TAD_Cloud_SceneLoader::GetRouteData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_Cloud_RouteViewerPtr retPtr = nullptr;
  for (auto& refRoute : (m_traffic_ptr->routes)) {
    if (id == (refRoute.id)) {
      retPtr = std::make_shared<TAD_Cloud_RouteViewer>();
      if (NonNull_Pointer(retPtr)) {
        retPtr->Init(refRoute);
      }
      break;
    }
  }

  return retPtr;
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_RouteViewer::startLon() const TX_NOEXCEPT {
  if (+ROUTE_TYPE::ePos == type()) {
    Base::txString strStart = m_route.start;
    Utils::trim(strStart);
    const Base::txInt elemIdx = 0;
    if (!strStart.empty()) {
      std::vector<Base::txString> results;
      boost::algorithm::split(results, strStart, boost::is_any_of(","));
      if (results.size() > 1) {
        Utils::trim(results[elemIdx]);
        if (!results[elemIdx].empty()) {
          return std::stod(results[elemIdx]);
        } else {
          LOG(WARNING) << " Context's first elements is null.";
          return 0.0;
        }
      } else {
        LOG(WARNING) << " Context do not have two elements.";
        return 0.0;
      }
    } else {
      LOG(WARNING) << " Context is Null.";
      return 0.0;
    }
  } else {
    LOG(WARNING) << "Route Type Error.";
    return 0.0;
  }
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_RouteViewer::startLat() const TX_NOEXCEPT {
  if (+ROUTE_TYPE::ePos == type()) {
    Base::txString strStart = m_route.start;
    Utils::trim(strStart);
    const Base::txInt elemIdx = 1;
    if (!strStart.empty()) {
      std::vector<Base::txString> results;
      boost::algorithm::split(results, strStart, boost::is_any_of(","));
      if (results.size() > 1) {
        Utils::trim(results[elemIdx]);
        if (!results[elemIdx].empty()) {
          return std::stod(results[elemIdx]);
        } else {
          LOG(WARNING) << " Context's second elements is null.";
          return 0.0;
        }
      } else {
        LOG(WARNING) << " Context do not have two elements.";
        return 0.0;
      }
    } else {
      LOG(WARNING) << " Context is Null.";
      return 0.0;
    }
  } else {
    LOG(WARNING) << " Type Error.";
    return 0.0;
  }
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_RouteViewer::endLon() const TX_NOEXCEPT {
  if (+ROUTE_TYPE::ePos == type()) {
    if (_NonEmpty_(m_route.end)) {
      Base::txString strEnd = m_route.end;
      Utils::trim(strEnd);
      const Base::txInt elemIdx = 0;
      if (!strEnd.empty()) {
        std::vector<Base::txString> results;
        boost::algorithm::split(results, strEnd, boost::is_any_of(","));
        if (results.size() > 1) {
          Utils::trim(results[elemIdx]);
          if (!results[elemIdx].empty()) {
            return std::stod(results[elemIdx]);
          } else {
            LOG(WARNING) << " Context's first elements is null.";
            return 0.0;
          }
        } else {
          LOG(WARNING) << " Context do not have two elements.";
          return 0.0;
        }
      } else {
        LOG(WARNING) << " Context is Null.";
        return 0.0;
      }
    } else {
      LOG(WARNING) << " do not have end point.";
      return 0.0;
    }
  } else {
    LOG(WARNING) << " Type Error.";
    return 0.0;
  }
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_RouteViewer::endLat() const TX_NOEXCEPT {
  if (+ROUTE_TYPE::ePos == type()) {
    if (_NonEmpty_(m_route.end)) {
      Base::txString strEnd = m_route.end;
      Utils::trim(strEnd);
      const Base::txInt elemIdx = 1;
      if (!strEnd.empty()) {
        std::vector<Base::txString> results;
        boost::algorithm::split(results, strEnd, boost::is_any_of(","));
        if (results.size() > 1) {
          Utils::trim(results[elemIdx]);
          if (!results[elemIdx].empty()) {
            return std::stod(results[elemIdx]);
          } else {
            LOG(WARNING) << " Context's second elements is null.";
            return 0.0;
          }
        } else {
          LOG(WARNING) << " Context do not have two elements.";
          return 0.0;
        }
      } else {
        LOG(WARNING) << " Context is Null.";
        return 0.0;
      }
    } else {
      LOG(WARNING) << " do not have end point.";
      return 0.0;
    }
  } else {
    LOG(WARNING) << " Type Error.";
    return 0.0;
  }
}

#if __TX_Mark__("TAD_SceneLoader::TAD_VehiclesViewer")

void TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::Init(SceneLoader::Traffic::Vehicle const& srcVehicle)
    TX_NOEXCEPT {
  m_vehicle = srcVehicle;
  m_inited = true;
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::IsMergeIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.mergeID >= 0);
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::IsAccIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.accID >= 0);
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::IsIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.id >= 0);
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::IsRouteIdValid() const TX_NOEXCEPT {
  return m_inited && (m_vehicle.routeID >= 0);
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::IsLaneIdValid() const TX_NOEXCEPT { return m_inited; }

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::id() const TX_NOEXCEPT {
  return (IsIdValid() ? (m_vehicle.id) : (-1));
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::routeID() const TX_NOEXCEPT {
  return (IsRouteIdValid() ? (m_vehicle.routeID) : (-1));
}

Base::txLaneID TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::laneID() const TX_NOEXCEPT {
  return (IsLaneIdValid() ? (m_vehicle.laneID) : (-1));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.start_s) : (10.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::start_t() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.start_t) : (0.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::start_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.start_v) : (8.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::max_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.max_v) : (30.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.l_offset) : (0.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::length() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.length) : (4.5));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::width() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.width) : (1.8));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::height() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.height) : (FLAGS_default_height_of_obstacle_pedestrian));
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::accID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.accID) : (-1));
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::mergeID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.mergeID) : (-1));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::behavior() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.behavior) : (Base::txString("UserDefine")));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::aggress() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.aggress) : (0.5));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::vehicleType() const TX_NOEXCEPT {
  if ("Trunk_1" == m_vehicle.vehicleType) {
    return Base::txString("Truck_1");
  } else {
    return ((m_inited) ? (m_vehicle.vehicleType) : ("UnDefined"));
  }
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::follow() const TX_NOEXCEPT {
  if (m_inited && _NonEmpty_(m_vehicle.follow)) {
    ISceneLoader::ISceneEventViewer::IConditionViewer::ElementType elementType;
    Base::txSysId elementId;
    std::tie(elementType, elementId) =
        ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(m_vehicle.follow);
    return elementId;
  }
  return FLAGS_Topological_Root_Id;
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::mergeTime() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.mergeTime) : (FLAGS_LaneChangeDuration));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::offsetTime() const TX_NOEXCEPT {
  return ((m_inited) ? (m_vehicle.offsetTime) : (FLAGS_OffsetChangeDuration));
}

std::vector<Base::txSysId> TAD_Cloud_SceneLoader::TAD_Cloud_VehiclesViewer::eventId() const TX_NOEXCEPT {
  std::vector<Base::txSysId> ret_event_id;
  if (m_inited) {
    Base::txString strEventId = m_vehicle.eventId;
    Utils::trim(strEventId);
    if (_NonEmpty_(strEventId)) {
      std::vector<Base::txString> results_event_id;
      boost::algorithm::split(results_event_id, strEventId, boost::is_any_of(","));
      for (Base::txString strId : results_event_id) {
        Utils::trim(strId);
        ret_event_id.emplace_back(std::atol(strId.c_str()));
      }
    }
  }
  return ret_event_id;
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_VehiclesViewer")

std::unordered_map<Base::txSysId, Base::ISceneLoader::IVehiclesViewerPtr> TAD_Cloud_SceneLoader::GetAllVehicleData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IVehiclesViewerPtr> retMap;

  if (NonNull_Pointer(m_traffic_ptr)) {
    for (auto& refVehicle : m_traffic_ptr->vehicles) {
      // LOG(INFO) << "[XXX] refVehicle.id() = " << refVehicle.id();
      auto retPtr = std::make_shared<TAD_Cloud_VehiclesViewer>();
      if (retPtr) {
        retPtr->Init(refVehicle);
        retMap[refVehicle.id] = retPtr;
      } else {
        LOG(WARNING) << "[XXX] refVehicle.id() = " << refVehicle.id << ", not found.";
      }
    }
  }
  return retMap;
}

#if __TX_Mark__("TAD_SceneLoader::TAD_ObstacleViewer")
TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::STATIC_ELEMENT_TYPE
TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::str2type(Base::txString const& strType) const TX_NOEXCEPT {
  return __lpsz2enum__(STATIC_ELEMENT_TYPE, strType.c_str());
}

void TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::Init(SceneLoader::Traffic::Obstacle const& srcObstacle)
    TX_NOEXCEPT {
  m_obstacle = srcObstacle;
  m_inited = true;
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_obstacle.id) : (-1));
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::routeID() const TX_NOEXCEPT {
  return (m_inited ? (m_obstacle.routeID) : (-1));
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::IsRouteIdValid() const TX_NOEXCEPT {
  return m_inited && (m_obstacle.routeID >= 0);
}

Base::txLaneID TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::laneID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.laneID) : (-1));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.start_s) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.l_offset) : (0.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::length() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.length) : (3.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::width() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.width) : (3.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::height() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.height) : (FLAGS_default_height_of_obstacle_pedestrian));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::vehicleType() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.vehicleType) : Base::txString("Sedan"));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::type() const TX_NOEXCEPT { return vehicleType(); }

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_ObstacleViewer::direction() const TX_NOEXCEPT {
  return ((m_inited) ? (m_obstacle.direction) : (0.0));
}
#endif  // #if  __TX_Mark__("TAD_SceneLoader::TAD_ObstacleViewer")

#if __TX_Mark__("TAD_SceneLoader::TAD_PedestriansViewer")

void TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::Init(SceneLoader::Traffic::Pedestrian const& srcPedestrian)
    TX_NOEXCEPT {
  m_pedestrian = srcPedestrian;
  m_inited = true;
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::id() const TX_NOEXCEPT {
  return (m_inited ? (m_pedestrian.id) : (-1));
}

Base::txSysId TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::routeID() const TX_NOEXCEPT {
  return (m_inited) ? (m_pedestrian.routeID) : (-1);
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::IsRouteIdValid() const TX_NOEXCEPT {
  return routeID() >= 0;
}

Base::txLaneID TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::laneID() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.laneID) : (-1));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::start_s() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.start_s) : (0.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::start_t() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.start_t) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::end_t() const TX_NOEXCEPT {
  return (m_inited ? (m_pedestrian.end_t) : (0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::l_offset() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.l_offset) : (0.0));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::type() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.type) : ("human"));
}

Base::txBool TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::hadDirection() const TX_NOEXCEPT { return false; }

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::start_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.start_v) : (0.0));
}

Base::txFloat TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::max_v() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.max_v) : (0.0));
}

Base::txString TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::behavior() const TX_NOEXCEPT {
  return ((m_inited) ? (m_pedestrian.behavior) : ("UserDefine"));
}

std::vector<Base::txSysId> TAD_Cloud_SceneLoader::TAD_Cloud_PedestriansViewer::eventId() const TX_NOEXCEPT {
  std::vector<Base::txSysId> ret_event_id;
  if (m_inited) {
    Base::txString strEventId = m_pedestrian.eventId;
    Utils::trim(strEventId);
    if (_NonEmpty_(strEventId)) {
      std::vector<Base::txString> results_event_id;
      boost::algorithm::split(results_event_id, strEventId, boost::is_any_of(","));
      for (Base::txString strId : results_event_id) {
        Utils::trim(strId);
        ret_event_id.emplace_back(std::atol(strId.c_str()));
      }
    }
  }
  return ret_event_id;
}

#endif  // __TX_Mark__("TAD_SceneLoader::TAD_PedestriansViewer")

std::unordered_map<Base::txSysId, Base::ISceneLoader::IObstacleViewerPtr> TAD_Cloud_SceneLoader::GetAllObstacleData()
    TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IObstacleViewerPtr> retMap;
  if (NonNull_Pointer(m_traffic_ptr)) {
    for (auto& refObstacle : m_traffic_ptr->obstacles) {
      auto retPtr = std::make_shared<TAD_Cloud_ObstacleViewer>();
      if (retPtr) {
        retPtr->Init(refObstacle);
        retMap[refObstacle.id] = retPtr;
      }
    }
  }

  return retMap;
}

Base::ISceneLoader::IPedestriansViewerPtr TAD_Cloud_SceneLoader::GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT {
  TAD_Cloud_PedestriansViewerPtr retPtr = nullptr;
  if (NonNull_Pointer(m_traffic_ptr)) {
    for (auto& refPedestrian : m_traffic_ptr->pedestrians) {
      if (id == refPedestrian.id) {
        retPtr = std::make_shared<TAD_Cloud_PedestriansViewer>();
        if (retPtr) {
          retPtr->Init(refPedestrian);
        }
        break;
      }
    }
  }

  return retPtr;
}

std::unordered_map<Base::txSysId, Base::ISceneLoader::IPedestriansViewerPtr>
TAD_Cloud_SceneLoader::GetAllPedestrianData() TX_NOEXCEPT {
  std::unordered_map<Base::txSysId, Base::ISceneLoader::IPedestriansViewerPtr> retMap;
  if (NonNull_Pointer(m_traffic_ptr)) {
    for (auto& refPedestrian : m_traffic_ptr->pedestrians) {
      auto retPtr = std::make_shared<TAD_Cloud_PedestriansViewer>();
      if (retPtr) {
        retPtr->Init(refPedestrian);
        retMap[refPedestrian.id] = retPtr;
      }
    }
  }

  return retMap;
}

#if __TX_Mark__("Scene Event")
Base::txBool TAD_Cloud_SceneLoader::ParseSceneEvent() TX_NOEXCEPT {
#  define LogInfo LOG(INFO)
#  define LogWarn LOG(WARNING)
  _seceneEventVec.clear();
  if (NonNull_Pointer(m_traffic_ptr)) {
    const SceneLoader::Traffic::scene_event& ref_scene_event = m_traffic_ptr->scene_event_array;
    if (boost::equal(txString("1.0.0.0"), ref_scene_event.version)) {
      for (const auto& ref_event : ref_scene_event.event_array) {
        LogInfo << "parse scene event id : " << ref_event.id;
        ISceneEventViewerPtr ptr = std::make_shared<ISceneEventViewer>();
        try {
          const Base::txSysId evId = std::atol(ref_event.id.c_str());
          kvMap conditionKVMap = ISceneEventViewer::generateKVMap(ref_event.condition);
          conditionKVMap["type"] = ref_event.type;
          kvMapVec endConditionKVMapVec = ISceneEventViewer::generateKVMapVec(ref_event.endCondition);
          kvMapVec actionKVMapVec = ISceneEventViewer::generateKVMapVec(ref_event.action);
          if (endConditionKVMapVec.size() == actionKVMapVec.size() && 3 == actionKVMapVec.size()) {
            std::vector<txInt> uncheckedIdxVec;
            for (txInt i = 0; i < actionKVMapVec.size(); ++i) {
              if (_Contain_(actionKVMapVec[i], "checked")) {
                if ("true" != actionKVMapVec[i].at("checked")) {
                  uncheckedIdxVec.emplace_back(i);
                }
              } else {
                uncheckedIdxVec.emplace_back(i);
              }
            }
            for (txInt i = (uncheckedIdxVec.size() - 1); i >= 0; --i) {
              TX_MARK("reverse iterator");
              const txInt deleteIdx = uncheckedIdxVec[i];
              endConditionKVMapVec.erase(endConditionKVMapVec.begin() + deleteIdx);
              actionKVMapVec.erase(actionKVMapVec.begin() + deleteIdx);
              LogWarn << TX_VARS(evId) << " ignore action index " << deleteIdx;
            }
            if (_NonEmpty_(conditionKVMap) && _NonEmpty_(endConditionKVMapVec) && _NonEmpty_(actionKVMapVec) &&
                endConditionKVMapVec.size() == actionKVMapVec.size()) {
              if (CallSucc(ptr->initialize(evId, conditionKVMap, endConditionKVMapVec, actionKVMapVec))) {
                LogInfo << "parse scene event id : " << ref_event.id << " success.";
                _seceneEventVec.push_back(ptr);
              } else {
                LogWarn << "parse scene event id : " << ref_event.id << " failure." << TX_VARS(ref_event);
                /*_seceneEventVec.clear();
                return false;*/
              }
            } else {
              LogWarn << TX_VARS_NAME(ignore_event_id, evId) << TX_VARS(conditionKVMap.size())
                      << TX_VARS(endConditionKVMapVec.size()) << TX_VARS(actionKVMapVec.size())
                      << TX_COND(endConditionKVMapVec.size() == actionKVMapVec.size());
              /*_seceneEventVec.clear();
              return false;*/
            }
          } else {
            LogWarn << "scene event error. " << TX_VARS(endConditionKVMapVec.size()) << TX_VARS(actionKVMapVec.size());
          }
        } catch (const std::exception& e) {
          LOG(WARNING) << e.what();
          _seceneEventVec.clear();
          return false;
        }
      }
      return true;
    } else {
      LogWarn << "un support scene event version " << ref_scene_event.version << ", support version 1.0.0.0";
      return false;
    }
  } else {
    return false;
  }
#  undef LogWarn
#  undef LogInfo
}
#endif /*__TX_Mark__("Scene Event")*/

TX_NAMESPACE_CLOSE(SceneLoader)

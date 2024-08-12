// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_signal_element.h"
#include "tad_signal_phase_period.h"
#include "tx_frame_utils.h"
#include "tx_protobuf_utils.h"
#include "tx_units.h"
#if USE_RTree
#  include "tx_spatial_query.h"
#endif                                /*USE_RTree*/
#include <boost/algorithm/string.hpp> /*string splite*/
#include <queue>
#include "HdMap/hashed_lane_info_orthogonal_list.h"
#include "HdMap/tx_hashed_road.h"
#include "tx_hadmap_utils.h"
#include "tx_signal_utils.h"
#include "tx_tadsim_flags.h"
#include "tx_timer_on_cpu.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Signal)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

TAD_SignalLightElement::~TAD_SignalLightElement() { /*LOG(WARNING) << "[~]TAD_SignalLightElement" << TX_VARS(Id());*/
}

Base::txBool TAD_SignalLightElement::Initialize(Base::ISceneLoader::IViewerPtr _absAttrView,
                                                Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 如果_absAttrView指针不为空且已初始化，且_sceneLoader指针不为空
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    _elemAttrViewPtr = std::dynamic_pointer_cast<ISignlightsViewer>(_absAttrView);
    // 如果元素属性视图指针不为空
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      // 设置信号灯属性，包括信号灯的控制计划、路口信息等
      m_signal_control_plan = _elemAttrViewPtr->plan();
      // 获取信号控制交叉口
      m_signal_control_junction = _elemAttrViewPtr->junction();
      // 获取信号控制相位数量
      m_signal_control_phaseNumber = _elemAttrViewPtr->phaseNumber();
      // 从输入的信号属性视图指针中获取信号头和事件ID
      m_signal_control_signalHead = _elemAttrViewPtr->signalHead();
      m_signal_control_eventId = _elemAttrViewPtr->eventId();
      // 输出信号控制信息
      LogInfo << "[Signal_Control] " << TX_VARS_NAME(Id, _elemAttrViewPtr->id()) << TX_VARS(plan())
              << TX_VARS(junction()) << TX_VARS(phaseNumber()) << TX_VARS(signalHead()) << TX_VARS(eventId());

      // 根据路由ID获取路由数据，并将其转换为IRouteViewer类型的智能指针
      _elemRouteViewPtr =
          std::dynamic_pointer_cast<IRouteViewer>(_sceneLoader->GetRouteData(_elemAttrViewPtr->routeID()));
      // 若元素路由视图指针不为空
      if (NonNull_Pointer(_elemRouteViewPtr)) {
        Base::txLaneUId t_LaneUid;
        Base::txFloat t_start_s = 0;
        // 设置元素身份id
        mIdentity.Id() = _elemAttrViewPtr->id();
        // 设置元素身份系统id
        mIdentity.SysId() = CreateSysId(mIdentity.Id());

        Base::txFloat distancePedalOnInit = 0.0;
        // 获取信号灯所在的车道指针
        hadmap::txLanePtr initLane =
            HdMap::HadmapCacheConCurrent::GetLaneForInit(_elemRouteViewPtr->startLon(), _elemRouteViewPtr->startLat(),
                                                         mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
        {
          const hadmap::txPoint _srcPt(_elemRouteViewPtr->startLon(), _elemRouteViewPtr->startLat(),
                                       FLAGS_default_altitude);
          Coord::txWGS84 srcPt(_srcPt);
          // 定义一个变量用于存储信号灯控制的车道集合
          hadmap::txLanes lanes;
          // 获取信号灯控制的车道集合
          int32_t opCode = hadmap::getLanes(txMapHdr, srcPt.WGS84(), FLAGS_SignalQueryRaidus, lanes);
          lanes.insert(lanes.begin(), initLane);
          // 控制的车道数量 > 2 时
          if (NonNull_Pointer(initLane) && lanes.size() > 2) {
            txFloat dist = 999.0;
            auto road_ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(initLane->getRoadId());
            Coord::txWGS84 left_pt, right_pt;
            // 获取信号灯所在道路的停止线
            if (Utils::GetRoadStopLine(road_ptr, left_pt, right_pt) &&
                (Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(left_pt, right_pt) >
                 (FLAGS_default_lane_width / 2.0))) {
              // 计算信号灯与停止线的距离
              dist = Utils::Point2Line2d(left_pt.ToENU().ENU2D(), right_pt.ToENU().ENU2D(), srcPt.ToENU().ENU2D());
            }

            // 遍历信号灯控制的车道集合
            for (txInt idx = 1; idx < lanes.size(); ++idx) {
              auto refLane = lanes[idx];
              if (NonNull_Pointer(refLane)) {
                auto road_ptr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(refLane->getRoadId());
                // 获取信号灯所在道路的停止线
                if (Utils::GetRoadStopLine(road_ptr, left_pt, right_pt) &&
                    (Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(left_pt, right_pt) >
                     (FLAGS_default_lane_width / 2.0))) {
                  // 计算信号灯与停止线的新距离
                  const txFloat new_dist =
                      Utils::Point2Line2d(left_pt.ToENU().ENU2D(), right_pt.ToENU().ENU2D(), srcPt.ToENU().ENU2D());
                  // 如果新距离小于原距离，更新信号灯所在车道和距离
                  if (new_dist < dist) {
                    LogInfo << "[signal_filter]" << TX_VARS_NAME(signalId, Id()) << TX_VARS_NAME(src_dist, dist)
                            << TX_VARS_NAME(src_RoadId, initLane->getRoadId()) << TX_VARS_NAME(new_dist, new_dist)
                            << TX_VARS_NAME(new_RoadId, refLane->getRoadId());
                    initLane = refLane;
                    dist = new_dist;
                  }
                }
              }
            }
          }
        }

        // 如果信号灯所在车道指针非空
        if (NonNull_Pointer(initLane)) {
          // 设置信号灯的车道唯一标识符
          t_LaneUid = initLane->getTxLaneId();
          t_start_s = _elemAttrViewPtr->start_s() /*+ distanceCurveOnInit*/;
          // 根据车道ID和s坐标获取信号灯的经纬度坐标
          HdMap::HadmapCacheConCurrent::GetLonLatFromSTonLaneWithOffset(
              initLane->getTxLaneId(), GenerateST(t_start_s, mLocation.DistanceAlongCurve()),
              _elemAttrViewPtr->l_offset(), mLocation.GeomCenter());
          // 初始化信号灯的追踪器
          mLocation.InitTracker(mIdentity.Id());
          // 重新定位信号灯的追踪器
          RelocateTracker(initLane, 0.0);
          // 设置信号灯的方向
          m_rot.FromDegree(_elemAttrViewPtr->direction());

          // 初始化信号灯的阶段周期
          Scene::TAD_SignalPhasePeriod::InitParam_t tmpInitParam;
          tmpInitParam._start_t = _elemAttrViewPtr->start_t();
          tmpInitParam._period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eGreen))] = _elemAttrViewPtr->time_green();
          tmpInitParam._period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eYellow))] = _elemAttrViewPtr->time_yellow();
          tmpInitParam._period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eRed))] = _elemAttrViewPtr->time_red();
          m_SignalPhasePeriod.Initialize(tmpInitParam);
          // 输出信号灯的调试信息
          LogInfo << "[SignalDebug] " << TX_VARS(Id()) << (m_SignalPhasePeriod.Str());
          /*pImpl->m_sysId = CreateSysId();*/ TX_MARK("signal is not traffic element");

#if 1
          // 获取信号灯的偏移量
          const Base::txFloat lOffset = _elemAttrViewPtr->l_offset();
          // 获取信号灯的起始经度
          const Base::txFloat lon = _elemRouteViewPtr->startLon();
          // 获取信号灯的起始纬度
          const Base::txFloat lat = _elemRouteViewPtr->startLat();
          LogWarn << "[show_location]" << TX_VARS(Id()) << TX_VARS(lOffset) << TX_VARS(lon) << TX_VARS(lat);
          // 将经纬度坐标转换为WGS84坐标
          Coord::txWGS84 raw_pt;
          raw_pt.FromWGS84(lon, lat);
          // 初始化s坐标
          Base::txFloat s = 0.0;
          // 初始化车道信息
          Base::Info_Lane_t locInfo;
          // 将经纬度坐标转换为WGS84坐标
          m_show_wgs84.FromWGS84(lon, lat);
          // 获取s坐标和车道信息
          if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(raw_pt, locInfo,
                                                                                                        s)) {
            // 如果信号灯所在车道是连接车道
            if (locInfo.IsOnLaneLink()) {
              // 从连接车道获取车道信息
              locInfo.FromLane(locInfo.onLinkFromLaneUid);
            }
            // 计算s坐标
            s = HdMap::HadmapCacheConCurrent::GetLaneLength(locInfo.onLaneUid) - 0.5;
            // 获取车道方向向量
            const Base::txVec3 laneDir = HdMap::HadmapCacheConCurrent::GetLaneDir(locInfo, s);
            // 计算信号灯的显示位置
            m_show_wgs84 = ComputeLaneOffset(raw_pt, laneDir, lOffset);

            // 计算信号灯的旋转角度
            m_rot.FromDegree(Utils::GetLaneAngleFromVectorOnENU(laneDir).GetDegree());
            LogWarn << "[show_location][succ]" << TX_VARS(Id()) << TX_VARS(m_show_wgs84) << TX_VARS(locInfo)
                    << TX_VARS(s) << TX_VARS_NAME(dir, Utils::ToString(laneDir)) << TX_VARS(m_rot.GetDegree());
          } else {
            LogWarn << "[show_location][error]" << TX_VARS(Id()) << TX_VARS(m_show_wgs84);
          }
#endif
          return true;
        } else {
          LogWarn << " GetLane Failure.";
          return false;
        }
      } else {
        LogWarn << " Get Route Failure. Route Id = " << (_elemAttrViewPtr->routeID());
        return false;
      }
    } else {
      LogWarn << "Signal attribute is null. ";
      return false;
    }
  } else {
    LogWarn << "Param Error." << TX_VARS_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_VARS_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_VARS_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

// 生成信号灯控制车道信息列表
Base::txBool TAD_SignalLightElement::GenerateSignalControlLaneInfoList(const RoadId2SignalIdSet &map_roadid2signalId)
    TX_NOEXCEPT {
  // 如果信号灯属性视图指针非空
  if (NonNull_Pointer(_elemAttrViewPtr)) {
    // 初始化有效车道集合
    hadmap::txLanes valid_lanes;
    // 获取信号灯控制车道
    const Base::txString controlLane = _elemAttrViewPtr->lane();
    // 根据信号灯控制车道生成信号灯控制车道缓存
    if (GenerateSignalByLaneCache(controlLane, mLocation.LaneLocInfo().onLaneUid, valid_lanes,
                                  mLocation.GeomCenter())) {
      // 生成信号灯控制车道缓存的内部函数
      GenerateSignalByLaneCache_Internal(valid_lanes, map_roadid2signalId);
      // 获取信号灯控制相位
      const Base::txString controlPhase = _elemAttrViewPtr->phase();
      // 生成信号灯控制相位
      GenerateControlPhases(controlPhase);
      // 生成信号灯控制车道连接
      GenerateControlLaneLinks(valid_lanes);
      LogInfo << "[Signal]" << TX_VARS_NAME(SignalId, Id()) << " initialize success.";
      return true;
    } else {
      // 输出信号灯的调试信息
      LogWarn << "GenerateSignalByLaneCache failure." << TX_VARS_NAME(SignalId, Id());
      return false;
    }
  } else {
    LogWarn << "_elemAttrViewPtr is nullptr.";
    return false;
  }
}

Base::txBool TAD_SignalLightElement::Update(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT {
  // signal更新
  m_SignalPhasePeriod.ComputeColor(timeMgr.PassTime());
  PublicSignals(timeMgr);
  return true;
}

Base::txBool TAD_SignalLightElement::Release() TX_NOEXCEPT { return true; }

// 填充信号灯元素数据
Base::txBool TAD_SignalLightElement::FillingElement(Base::TimeParamManager const &,
                                                    sim_msg::Traffic &TrafficData) TX_NOEXCEPT {
  // 如果信号灯元素存活
  if (IsAlive()) {
    // 如果信号灯控制车道集合非空
    if (m_control_lane_set_from_conf.size() > 0) {
      // 获取信号灯控制车道
      const auto &control_lane = m_control_lane_set_from_conf.front();
      // 初始化交通信号灯数据
      sim_msg::TrafficLight *tmpTrafficLightData = nullptr;
      {
#if USE_TBB
        // 使用TBB互斥锁锁定输出交通数据
        tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#endif
        // 添加交通信号灯数据
        tmpTrafficLightData = TrafficData.add_trafficlights();
      }
      // 设置信号灯id 显示位置 经纬度 颜色等信息
      tmpTrafficLightData->set_id(mIdentity.Id());
      /*const hadmap::txPoint geom_center_gps = GetLocation().WGS84();*/
      const hadmap::txPoint show_gps = m_show_wgs84.WGS84();
      tmpTrafficLightData->set_x(__Lon__(show_gps));
      tmpTrafficLightData->set_y(__Lat__(show_gps));
      tmpTrafficLightData->set_z(Altitude());
      tmpTrafficLightData->set_color(__enum2idx__(m_SignalPhasePeriod.GetPhase()));
      tmpTrafficLightData->set_next_color(__enum2idx__(m_SignalPhasePeriod.GetNextPhase()));
      tmpTrafficLightData->set_heading(m_rot.GetRadian());
      tmpTrafficLightData->set_heading(m_rot.GetDegree());
      tmpTrafficLightData->set_age(m_SignalPhasePeriod.GetAge());
      tmpTrafficLightData->set_next_age(m_SignalPhasePeriod.GetNextAge());

#if __TX_Mark__("signal control configure")
      tmpTrafficLightData->set_plan(plan());
      tmpTrafficLightData->set_junction(junction());
      tmpTrafficLightData->set_phasenumber(phaseNumber());
      tmpTrafficLightData->set_signalhead(signalHead());
#endif /*"signal control configure"*/
#if Use_SignalPhase
      // 遍历信号灯控制车道集合
      for (const auto &control_lane : m_control_lane_set_from_conf) {
        // 添加信号灯控制车道
        sim_msg::LaneUid *cur_control_lane = tmpTrafficLightData->add_control_lanes();
        cur_control_lane->set_tx_road_id(control_lane.first.roadId);
        cur_control_lane->set_tx_section_id(control_lane.first.sectionId);
        cur_control_lane->set_tx_lane_id(control_lane.first.laneId);
      }

      // 遍历信号灯控制相位集合
      for (const auto &phaseEnum : m_vec_control_phases) {
        // 添加信号灯控制相位
        tmpTrafficLightData->add_control_phases(phaseEnum);
      }
#endif /*Use_SignalPhase*/
      // 输出信号灯的调试信息
      LogInfo << "[trafficlight_output][" << (Id()) << "]:" << Utils::ProtobufDebugJson(tmpTrafficLightData)
              << TX_VARS_NAME(age, tmpTrafficLightData->age()) << TX_VARS_NAME(Color, m_SignalPhasePeriod.GetPhase());
    }
  }
  return true;
}

Base::ISignalLightElement::SIGN_LIGHT_COLOR_TYPE TAD_SignalLightElement::GetCurrentSignLightColorType() const
    TX_NOEXCEPT {
  // 如果信号灯当前颜色类型为灰色
  if (_plus_(Base::ISignalLightElement::SIGN_LIGHT_COLOR_TYPE::eGrey) == m_SignalPhasePeriod.GetPhase()) {
    // 返回绿色颜色类型
    return Base::ISignalLightElement::SIGN_LIGHT_COLOR_TYPE::eGreen;
  } else {
    // 返回信号灯当前颜色类型
    return m_SignalPhasePeriod.GetPhase();
  }
}

void TAD_SignalLightElement::GenerateControlPhases(Base::txString control_phase) TX_NOEXCEPT {
#if Use_SignalPhase
  m_vec_control_phases.clear();
  Utils::trim(control_phase);
  // 如果控制相位字符串非空
  if (!control_phase.empty()) {
    // 初始化结果对
    std::vector<Base::txString> results_pairs;
    // 根据分号分割控制相位字符串
    boost::algorithm::split(results_pairs, control_phase, boost::is_any_of(";"));
    // 遍历结果对
    for (auto s : results_pairs) {
      Utils::trim(s);
      // 如果结果对非空
      if (!s.empty()) {
        // 初始化当前相位
        sim_msg::Phase curPhase = sim_msg::Phase::T;
        // 根据结果对设置当前相位
        if ("U" == s) {
          curPhase = sim_msg::Phase::U;
        } else if ("L" == s) {
          curPhase = sim_msg::Phase::L;
        } else if ("T" == s) {
          curPhase = sim_msg::Phase::T;
        } else if ("R" == s) {
          curPhase = sim_msg::Phase::R;
        } else if ("R0" == s) {
          curPhase = sim_msg::Phase::R0;
        } else if ("L0" == s) {
          curPhase = sim_msg::Phase::L0;
        } else if ("A" == s) {
          // 如果结果对为"A"，则添加所有相位
          m_vec_control_phases.push_back(sim_msg::Phase::U);
          m_vec_control_phases.push_back(sim_msg::Phase::L);
          m_vec_control_phases.push_back(sim_msg::Phase::T);
          m_vec_control_phases.push_back(sim_msg::Phase::R);
          m_vec_control_phases.push_back(sim_msg::Phase::L0);
          m_vec_control_phases.push_back(sim_msg::Phase::R0);
          break;
        }
        // 将当前相位添加到信号灯控制相位集合中
        m_vec_control_phases.push_back(curPhase);
      }
    }
  }
#endif /*Use_SignalPhase*/
}

void TAD_SignalLightElement::GenerateControlLaneLinks(const hadmap::txLanes &lanes) TX_NOEXCEPT {
#if Use_SignalPhase
  // 遍历信号灯控制车道集合
  for (const auto &curLanePtr : lanes) {
    // 如果信号灯控制车道非空
    if (NonNull_Pointer(curLanePtr)) {
      // 获取信号灯控制车道ID
      Base::txLaneUId curLaneUId = curLanePtr->getTxLaneId();
      // 获取信号灯控制车道的下一个车道连接集合
      hadmap::txLaneLinks nextLaneLinks = HdMap::HadmapCacheConCurrent::GetLaneNextLinks(curLaneUId);
      // 如果下一个车道连接集合非空
      if (_NonEmpty_(nextLaneLinks)) {
        // 初始化可达车道连接集合
        hadmap::txLaneLinks reachableLinks;
        reachableLinks.reserve(nextLaneLinks.size());
        // 遍历下一个车道连接集合
        for (auto curLaneLinkPtr : nextLaneLinks) {
          //     LOG(WARNING)<< TX_VARS(curLaneLinkPtr->getId()) <<
          //     TX_VARS(HdMap::HadmapCacheConCurrent::GetLaneLinkControlPhase(curLaneLinkPtr));

          // 如果当前车道连接的控制相位在信号灯控制相位集合中
          if (CheckControlPhase(HdMap::HadmapCacheConCurrent::GetLaneLinkControlPhase(curLaneLinkPtr))) {
            // 将当前车道连接添加到可达车道连接集合中
            reachableLinks.emplace_back(curLaneLinkPtr);
          }
        }
        m_map_control_lanelinks[curLaneUId] = reachableLinks;
      }
    }
  }
#endif /*Use_SignalPhase*/
}

Base::txBool TAD_SignalLightElement::CheckControlPhase(Base::txString phase) const TX_NOEXCEPT {
#if Use_SignalPhase
  // 如果信号灯控制相位非空
  if (!phase.empty()) {
    // 初始化当前相位
    sim_msg::Phase curPhase = sim_msg::Phase::T;
    // 根据信号灯控制相位设置当前相位
    if ("L" == phase) {
      curPhase = sim_msg::Phase::L;
    } else if ("T" == phase) {
      curPhase = sim_msg::Phase::T;
    } else if ("R" == phase) {
      curPhase = sim_msg::Phase::R;
    } else if ("R0" == phase) {
      curPhase = sim_msg::Phase::R0;
    } else if ("L0" == phase) {
      curPhase = sim_msg::Phase::L0;
    } else {
      return false;
    }
    // 如果当前相位在信号灯控制相位集合中，则返回true，否则返回false
    return std::find(m_vec_control_phases.begin(), m_vec_control_phases.end(), curPhase) != m_vec_control_phases.end();
  }
#endif /*Use_SignalPhase*/
  return false;
}
TX_NAMESPACE_CLOSE(TrafficFlow)

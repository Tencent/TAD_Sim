// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_relative_trajectory_follow_element.h"
#include "tx_enum_def.h"
#include "tx_spatial_query.h"
#include "tx_string_utils.h"
#include "tx_tadsim_flags.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_RelativeTrajectoryFollow)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool TAD_RelativeTrajectoryFollowElement::RelativeActionItemInfo::Initialize(
    const std::map<txString, txString>& kv_map) TX_NOEXCEPT {
  if (_Contain_(kv_map, "dx")) {
    // 将"dx"对应的值转换为double类型，并赋值给dStep.x()
    dStep.x() = std::stod(kv_map.at("dx"));
  }
  if (_Contain_(kv_map, "dy")) {
    // 将"dy"对应的值转换为double类型，并赋值给dStep.y()
    dStep.y() = std::stod(kv_map.at("dy"));
  }
  if (_Contain_(kv_map, "dz")) {
    // 将"dz"对应的值转换为double类型，并赋值给dStep.z()
    dStep.z() = std::stod(kv_map.at("dz"));
  }
  if (_Contain_(kv_map, "time")) {
    // 将"time"对应的值转换为double类型，并赋值给time
    time = std::stod(kv_map.at("time"));
  }
  if (_Contain_(kv_map, "target_element")) {
    // 将"target_element"对应的值分割为元素类型和元素ID，并赋值给elemType和elemId
    std::tie(elemType, elemId) =
        Base::ISceneLoader::ISceneEventViewer::IConditionViewer::splite_type_id(kv_map.at("target_element"));
  }
  return true;
}

Base::txString TAD_RelativeTrajectoryFollowElement::RelativeActionItemInfo::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  /*
  Base::txVec3 dStep;
  Base::txFloat time;
  Base::txSysId elemId;
  ElementType elemType;
  */
  oss << "{" << TX_VARS_NAME(dx, dStep.x()) << TX_VARS_NAME(dy, dStep.y()) << TX_VARS_NAME(dz, dStep.z())
      << TX_VARS_NAME(time, time) << TX_VARS_NAME(elemType, elemType._to_string()) << TX_VARS(elemId) << "}";
  return oss.str();
}

Base::txBool TAD_RelativeTrajectoryFollowElement::AbsoluteActionItemInfo::Initialize(
    const std::map<txString, txString>& kv_map) TX_NOEXCEPT {
  // 从KV中解析键值对初始化数据
  if (_Contain_(kv_map, "lon")) {
    gps.x = std::stod(kv_map.at("lon"));
  }
  if (_Contain_(kv_map, "lat")) {
    gps.y = std::stod(kv_map.at("lat"));
  }
  if (_Contain_(kv_map, "alt")) {
    gps.z = std::stod(kv_map.at("alt"));
  }
  if (_Contain_(kv_map, "time")) {
    time = std::stod(kv_map.at("time"));
  }
  if (_Contain_(kv_map, "height")) {
    height = std::stod(kv_map.at("height"));
  }
  return true;
}

Base::txString TAD_RelativeTrajectoryFollowElement::AbsoluteActionItemInfo::Str() const TX_NOEXCEPT {
  // 创建一个ostringstream对象
  std::ostringstream oss;
  // 将AbsoluteActionItemInfo对象的成员变量转换为字符串并拼接
  oss << "{" << TX_VARS_NAME(gps, Utils::ToString(gps)) << TX_VARS_NAME(height, height) << TX_VARS_NAME(time, time)
      << "}";
  // 返回拼接后的字符串
  return oss.str();
}

Base::txBool TAD_RelativeTrajectoryFollowElement::Initialize(ISceneLoader::IViewerPtr _absAttrView,
                                                             ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT {
  // 检查_absAttrView、_sceneLoader和_absAttrView->IsInited()是否成功初始化
  if (NonNull_Pointer(_absAttrView) && NonNull_Pointer(_sceneLoader) && CallSucc(_absAttrView->IsInited())) {
    m_vehicle_come_from = Base::IVehicleElement::VehicleSource::scene_cfg;
    Base::ISceneLoader::IObstacleViewerPtr _elemAttrViewPtr =
        std::dynamic_pointer_cast<Base::ISceneLoader::IObstacleViewer>(_absAttrView);
    // 检查_elemAttrViewPtr是否为非空指针
    if (NonNull_Pointer(_elemAttrViewPtr)) {
      // 根据路线id获取元素的路线视图
      Base::ISceneLoader::IRouteViewerPtr _elemRouteViewPtr = _sceneLoader->GetRouteData(_elemAttrViewPtr->routeID());
      LogInfo << "Obstacle Attr :" << (_elemAttrViewPtr->Str());
      // 检查_elemAttrViewPtr是否为非空指针
      if (NonNull_Pointer(_elemRouteViewPtr)) {
        // 输出障碍物ID和路径信息
        LogInfo << "Obstacle Id : " << (_elemAttrViewPtr->id()) << ", Route : " << (_elemRouteViewPtr->Str());
        // 清除场景事件
        ClearSceneEvent();
        // 检查_sceneLoader是否有场景事件
        if (_sceneLoader->HasSceneEvent()) {
          // 设置m_use_scene_event_1_0_0_0为true
          m_use_scene_event_1_0_0_0 = true;
          // 将_elemAccViewPtr、_elemAccEventViewPtr、_elemMergeViewPtr、
          // // _elemMergeEventViewPtr、_elemVelocityViewPtr和_elemVelocityEventViewPtr设置为nullptr
          _elemAccViewPtr = nullptr;
          _elemAccEventViewPtr = nullptr;
          _elemMergeViewPtr = nullptr;
          _elemMergeEventViewPtr = nullptr;
          _elemVelocityViewPtr = nullptr;
          _elemVelocityEventViewPtr = nullptr;

          // 获取_sceneLoader中的所有场景事件
          std::vector<ISceneEventViewerPtr> all_scene_event = _sceneLoader->GetSceneEventVec();
          // 创建一个map，用于存储场景事件ID和对应的指针
          std::map<txSysId, ISceneEventViewerPtr> map_id2ptr;
          // 遍历所有场景事件，将事件ID和对应的指针存入map
          for (auto ptr : all_scene_event) {
            if (NonNull_Pointer(ptr)) {
              map_id2ptr[ptr->id()] = ptr;
            }
          }

          // 获取_elemAttrViewPtr中的事件ID
          const auto event_id_vec = _elemAttrViewPtr->eventId();
          // 遍历事件ID，将找到的场景事件添加到m_vec_scene_event中
          for (const auto evId : event_id_vec) {
            if (_Contain_(map_id2ptr, evId)) {
              m_vec_scene_event.emplace_back(map_id2ptr.at(evId));
            } else {
              // 如果找不到场景事件，输出警告信息
              LogWarn << "can not find scene event " << TX_VARS(evId);
            }
          }
        } else {
          // 如果_sceneLoader没有场景事件，输出警告信息
          LOG(WARNING) << TX_VARS(Id()) << " do not have relative action trigger.";
          return false;
        }

        // 获取_elemRouteViewPtr和_elemAttrViewPtr的引用
        const auto refRoutePtr = (_elemRouteViewPtr);
        const auto refVehiclePtr = (_elemAttrViewPtr);
        // 设置mIdentity.Id()为_elemAttrViewPtr的ID
        mIdentity.Id() = refVehiclePtr->id();
        // 设置mIdentity.SysId()为_elemAttrViewPtr的ID对应的系统ID
        mIdentity.SysId() = CreateSysId(mIdentity.Id());
        // 使用_elemAttrViewPtr的ID和_sceneLoader的随机种子初始化mPRandom
        mPRandom.Initialize(mIdentity.Id(), _sceneLoader->GetRandomSeed());

        // 设置mGeometryData的长度、宽度和高度为_elemAttrViewPtr的长度、宽度和高度
        mGeometryData.Length() = refVehiclePtr->length();
        mGeometryData.Width() = refVehiclePtr->width();
        mGeometryData.Height() = refVehiclePtr->height();
        // 设置mGeometryData的对象类型为车辆
        mGeometryData.ObjectType() = Base::Component::GeometryData::Object_Type::eVehicle;

        // 设置mObstacleType为_elemAttrViewPtr的类型
        mObstacleType = __lpsz2enum__(STATIC_ELEMENT_TYPE, refVehiclePtr->type().c_str());
#if USE_CustomModelImport
        mObstacleTypeStr = refVehiclePtr->type().c_str();
        mObstacleTypeId = Base::CatalogCache::OBSTACLE_TYPE(mObstacleTypeStr);
#endif /*#USE_CustomModelImport*/

        // 使用_elemAttrViewPtr的ID和随机值初始化mLaneKeepMgr
        mLaneKeepMgr.Initialize(Id(), m_aggress + 0.2 * mPRandom.GetRandomValue_NegOne2PosOne());
        // 输出日志信息
        LOG(INFO) << TX_VARS(Id()) << TX_VARS(mLaneKeepMgr.RndFactor());
        // 重置mLaneKeepMgr的计数器
        mLaneKeepMgr.ResetCounter(5.0, m_aggress);

        // 初始化mLocation的追踪器
        mLocation.InitTracker(mIdentity.Id());

        // 设置mObstacleBehaviorType为未知
        mObstacleBehaviorType = ObstacleBehaviorType::Unknown;
        // 获取_elemAttrViewPtr的行为类型字符串
        const Base::txString strBehavior = refVehiclePtr->behavior();
        // 如果行为类型为"RelativeTrajectoryFollow"
        if ("RelativeTrajectoryFollow" == strBehavior) {
          // 设置mObstacleBehaviorType为相对轨迹跟随
          mObstacleBehaviorType = ObstacleBehaviorType::eRelative;
          // 获取_elemRouteViewPtr中的相对路径
          Base::txString relativePath = _elemRouteViewPtr->relativePath();
          // 输出日志信息
          LogInfo << TX_VARS(Id()) << TX_VARS(relativePath);
          // 将相对路径字符串分割为子动作字符串向量
          const auto strResults = Utils::SpliteStringVector(relativePath, ";");

          // 遍历子动作字符串向量
          for (auto subAction : strResults) {
            // 去除子动作字符串的空格
            Utils::trim(subAction);
            // 如果子动作字符串非空
            if (_NonEmpty_(subAction)) {
              // 将子动作字符串分割为子项字符串向量
              const auto strItems = Utils::SpliteStringVector(subAction, ",");
              if (5 == strItems.size()) {
                std::map<txString, txString> cur_kv_map;
                // 遍历子项字符串向量
                for (auto strItem : strItems) {
                  // 去除子项字符串的空格
                  Utils::trim(strItem);
                  // 如果子项字符串非空
                  if (_NonEmpty_(strItem)) {
                    // 将子项字符串分割为键值对字符串向量
                    auto str_kv = Utils::SpliteStringVector(strItem, ":");
                    // 如果键值对字符串向量的大小为2
                    if (2 == str_kv.size()) {
                      // 去除键值对字符串向量中的空格
                      Utils::trim(str_kv[0]);
                      Utils::trim(str_kv[1]);
                      // 将键值对添加到cur_kv_map中
                      cur_kv_map[str_kv[0]] = str_kv[1];
                    }
                  }
                }

                // 如果cur_kv_map的大小为5
                if (5 == cur_kv_map.size()) {
                  // 初始化RelativeActionItemInfo对象
                  RelativeActionItemInfo node;
                  node.Initialize(cur_kv_map);
                  // 将RelativeActionItemInfo对象添加到m_relative_action_vec中
                  m_relative_action_vec.emplace_back(std::move(node));
                } else {
                  // 如果cur_kv_map的大小不为5，输出警告信息
                  LogWarn << "kv parse failure. " << TX_VARS(cur_kv_map.size());
                }
              } else {
                // 如果子项字符串向量的大小不为5，输出警告信息
                LogWarn << "subAction parse error, " << TX_VARS(subAction);
              }
            }
          }
        } else if ("AbsoluteTrajectoryFollow" == strBehavior) {  // 如果行为类型为"AbsoluteTrajectoryFollow"
          // 设置mObstacleBehaviorType为绝对轨迹跟随
          mObstacleBehaviorType = ObstacleBehaviorType::eAbsolute;
          // 获取_elemRouteViewPtr中的绝对路径
          Base::txString absolutePath = _elemRouteViewPtr->absolutePath();
          // 输出日志信息
          LogInfo << TX_VARS(Id()) << TX_VARS(absolutePath);
          // 将绝对路径字符串分割为子动作字符串向量
          const auto strResults = Utils::SpliteStringVector(absolutePath, ";");

          // 遍历子动作字符串向量
          for (auto subAction : strResults) {
            // 去除子动作字符串的空格
            Utils::trim(subAction);
            // 如果子动作字符串非空
            if (_NonEmpty_(subAction)) {
              // 将子动作字符串分割为子项字符串向量
              const auto strItems = Utils::SpliteStringVector(subAction, ",");
              // 如果子项字符串向量的大小为5
              if (5 == strItems.size()) {
                std::map<txString, txString> cur_kv_map;
                // 遍历子项字符串向量
                for (auto strItem : strItems) {
                  // 去除子项字符串的空格
                  Utils::trim(strItem);
                  // 如果子项字符串非空
                  if (_NonEmpty_(strItem)) {
                    // 将子项字符串分割为键值对字符串向量
                    auto str_kv = Utils::SpliteStringVector(strItem, ":");
                    // 如果键值对字符串向量的大小为2
                    if (2 == str_kv.size()) {
                      // 去除键值对字符串向量的元素的空格
                      Utils::trim(str_kv[0]);
                      Utils::trim(str_kv[1]);
                      // 将键值对字符串向量中的元素添加到cur_kv_map中
                      cur_kv_map[str_kv[0]] = str_kv[1];
                    }
                  }
                }

                // 如果cur_kv_map的大小为5
                if (5 == cur_kv_map.size()) {
                  // 初始化AbsoluteActionItemInfo对象
                  AbsoluteActionItemInfo node;
                  node.Initialize(cur_kv_map);
                  // 将AbsoluteActionItemInfo对象添加到m_absolute_action_vec中
                  m_absolute_action_vec.emplace_back(std::move(node));
                } else {
                  // 如果cur_kv_map的大小不为5，输出警告信息
                  LogWarn << "kv parse failure. " << TX_VARS(cur_kv_map.size());
                }
              } else {
                LogWarn << "subAction parse error, " << TX_VARS(subAction);
              }
            }
          }
        }

        // 初始化日志输出流
        std::ostringstream oss;
        oss << TX_VARS(Id());
        // 遍历绝对动作节点集合
        for (const auto& actionNode : m_absolute_action_vec) {
          // 将动作节点的字符串表示添加到日志输出流
          oss << actionNode.Str();
        }
        LOG(INFO) << oss.str();
        // 如果绝对动作节点向量非空
        if (_NonEmpty_(m_absolute_action_vec)) {
          // 设置生命周期开始时间为FLT_MAX
          mLifeCycle.StartTime() = FLT_MAX /*m_absolute_action_vec.front().time*/;
          // 输出日志信息
          LogInfo << TX_VARS(Id()) << " initialize success.";
          // 将m_cur_step设置为0
          m_cur_step.setZero();
          return true;
        } else if (_NonEmpty_(m_relative_action_vec)) {  // 如果相对行为集合不为空
          // 设置生命周期开始时间为FLT_MAX
          mLifeCycle.StartTime() = FLT_MAX /*m_relative_action_vec.front().time*/;
          LogInfo << TX_VARS(Id()) << " initialize success.";
          // 将m_cur_step设置为相对动作节点集合的第一个元素的dStep
          m_cur_step = m_relative_action_vec.front().dStep;
          return true;
        } else {
          LOG(WARNING) << TX_VARS(Id()) << " relative action empty.";
          return false;
        }
      } else {
        LogWarn << "Can not find Route Info.";
        return false;
      }
    } else {
      LogWarn << "Param Cast Error." << TX_COND_NAME(_elemAttrViewPtr, NonNull_Pointer(_elemAttrViewPtr));
      return false;
    }
  } else {
    LogWarn << "Param Error." << TX_COND_NAME(_absAttrView, NonNull_Pointer(_absAttrView))
            << TX_COND_NAME(_sceneLoader, NonNull_Pointer(_sceneLoader))
            << TX_COND_NAME(_absAttrView_IsInited, CallSucc(_absAttrView->IsInited()));
    return false;
  }
}

Base::txFloat interp(const Base::txFloat t1, const Base::txFloat t2, const Base::txFloat step1,
                     const Base::txFloat step2, const Base::txFloat t) TX_NOEXCEPT {
  // 计算并返回插值值
  return step1 + (step2 - step1) * (t - t1) / (t2 - t1);
}

Base::txVec3 interp(const Base::txFloat t1, const Base::txFloat t2, const Base::txVec3 step1, const Base::txVec3 step2,
                    const Base::txFloat t) TX_NOEXCEPT {
  // 计算并返回插值值
  return step1 + (step2 - step1) * (t - t1) / (t2 - t1);
}

Coord::txWGS84 interp(const Base::txFloat t1, const Base::txFloat t2, const hadmap::txPoint pre_gps,
                      const hadmap::txPoint post_gps, const Base::txFloat t) TX_NOEXCEPT {
  // 将pre_gps转换为Coord::txWGS84类型，并赋值给pre
  Coord::txWGS84 pre;
  pre.FromWGS84(pre_gps);
  // 将post_gps转换为Coord::txWGS84类型，并赋值给post
  Coord::txWGS84 post;
  post.FromWGS84(post_gps);
  // 将pre转换为ENU坐标系，并提取其2D分量，赋值给step1_2d
  const Base::txVec2 step1_2d = pre.ToENU().ENU2D();
  // 将post转换为ENU坐标系，并提取其2D分量，赋值给step2_2d
  const Base::txVec2 step2_2d = post.ToENU().ENU2D();
  // 计算插值结果的2D分量，并赋值给res_2d
  const Base::txVec2 res_2d = step1_2d + (step2_2d - step1_2d) * (t - t1) / (t2 - t1);
  // 将res_2d转换为Coord::txWGS84类型，并赋值给res
  Coord::txWGS84 res;
  res.FromENU(res_2d.x(), res_2d.y());
  return res;
}

Base::txBool TAD_RelativeTrajectoryFollowElement::UpdateRelative(TimeParamManager const& cur_timeMgr,
                                                                 const Base::txFloat relativePassTime,
                                                                 Base::ITrafficElementPtr followElemPtr) TX_NOEXCEPT {
  if (NonNull_Pointer(followElemPtr) && _NonEmpty_(m_relative_action_vec)) {
    // 如果相对时间小于相对动作节点集合的最后一个元素的时间
    if (relativePassTime < m_relative_action_vec.back().time) {
      // 遍历相对动作节点向量
      for (Base::txInt idx = 1; idx < m_relative_action_vec.size(); ++idx) {
        // 获取当前动作节点的前一个动作节点
        const auto& preAction = m_relative_action_vec[idx - 1];
        // 获取当前动作节点
        const auto& curAction = m_relative_action_vec[idx + 0];
        // 如果前一个动作节点的时间小于等于相对时间，且当前动作节点的时间大于相对时间
        if (preAction.time <= relativePassTime && curAction.time > relativePassTime) {
          // 计算插值结果，并赋值给m_cur_step
          m_cur_step = interp(preAction.time, curAction.time, preAction.dStep, curAction.dStep, relativePassTime);
          break;
        }
      }
    } else {  // 如果相对时间大于等于相对动作节点集合的最后一个元素的时间
      // 设置生命周期为停止
      mLifeCycle.SetStop();
      LogInfo << TX_VARS(Id()) << " relative action finish." << TX_VARS(m_relative_action_vec.back().time);
    }

    // 获取跟随元素的位置
    Coord::txWGS84 referencePos = followElemPtr->GetLocation();
    // 获取跟随元素的车道方向，并将其赋值给local_coord_x
    const Base::txVec3 local_coord_x /*as same as LocalCoord_AxisY*/ = followElemPtr->GetLaneDir();
    // 计算local_coord_y，使其与local_coord_x正交
    const Base::txVec3 local_coord_y =
        Utils::VetRotVecByDegree(local_coord_x, Unit::txDegree::MakeDegree(RightOnENU), Utils::Axis_Enu_Up());
    // 将local_coord_x转换为2D向量，并乘以m_cur_step.x()
    Base::txVec2 local_coord_x_2d = Utils::Vec3_Vec2(local_coord_x) * m_cur_step.x();
    // 将local_coord_y转换为2D向量，并乘以m_cur_step.y()
    Base::txVec2 local_coord_y_2d = Utils::Vec3_Vec2(local_coord_y) * m_cur_step.y();
    // 计算相对步长的2D分量
    Base::txVec2 relative_step_2d = local_coord_x_2d + local_coord_y_2d;
    // 将relative_step_2d转换为3D向量
    Base::txVec3 relative_step_3d = Base::txVec3(relative_step_2d.x(), relative_step_2d.y(), 0.0);
    // 将相对步长的3D分量应用于跟随元素的位置
    referencePos.TranslateLocalPos(relative_step_3d);

    // 将计算后的位置赋值给mLocation
    mLocation.GeomCenter() = referencePos;
    // 使用RTree2D_NearestReferenceLine查询最近的参考线
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
        mLocation.GeomCenter(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
    // 获取高程信息
    Base::txFloat baseAlt =
        HdMap::HadmapCacheConCurrent::GetHdMapAlt(mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve());
    // 将高程信息与m_cur_step.z()相加，并赋值给mLocation的高度
    mLocation.GeomCenter().Alt() = baseAlt + m_cur_step.z();
    return true;
  } else {
    LogWarn << TX_VARS(Id()) << " follow element ptr is nullptr." << TX_VARS(m_relative_action_vec.size());
    return false;
  }
}

Base::txBool TAD_RelativeTrajectoryFollowElement::UpdateAbsolute(TimeParamManager const& cur_timeMgr,
                                                                 const Base::txFloat relativePassTime,
                                                                 Base::ITrafficElementPtr followElemPtr) TX_NOEXCEPT {
  if (_NonEmpty_(m_absolute_action_vec)) {
    // 如果相对时间小于绝对动作节点向量的最后一个元素的时间
    if (relativePassTime < m_absolute_action_vec.back().time) {
      // 遍历绝对动作节点向量
      for (Base::txInt idx = 1; idx < m_absolute_action_vec.size(); ++idx) {
        // 获取当前动作节点的前一个动作节点
        const auto& preAction = m_absolute_action_vec[idx - 1];
        // 获取当前动作节点
        const auto& curAction = m_absolute_action_vec[idx + 0];
        // 如果前一个动作节点的时间小于等于相对时间，且当前动作节点的时间大于相对时间
        if (preAction.time <= relativePassTime && curAction.time > relativePassTime) {
          // 计算插值结果，并赋值给mLocation的几何中心
          mLocation.GeomCenter() =
              interp(preAction.time, curAction.time, preAction.gps, curAction.gps, relativePassTime);
          // 使用RTree2D_NearestReferenceLine查询最近的参考线
          Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
              mLocation.GeomCenter(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
          // 获取高程信息
          Base::txFloat baseAlt =
              HdMap::HadmapCacheConCurrent::GetHdMapAlt(mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve());
          // 计算高度步长，并与高程信息相加，赋值给mLocation的高度
          Base::txFloat hStep =
              interp(preAction.time, curAction.time, preAction.height, curAction.height, relativePassTime);
          mLocation.GeomCenter().Alt() = baseAlt + hStep;
          break;
        }
      }
      return true;
    } else {  // 如果相对时间大于等于绝对动作节点向量的最后一个元素的时间
      // 设置生命周期为停止
      mLifeCycle.SetStop();
      LogInfo << TX_VARS(Id()) << " relative action finish." << TX_VARS(m_absolute_action_vec.back().time);
      return false;
    }
  } else {
    LogWarn << TX_VARS(Id()) << " m_absolute_action_vec is null." << TX_VARS(m_absolute_action_vec.size());
    return false;
  }
}

Base::txBool TAD_RelativeTrajectoryFollowElement::Update(TimeParamManager const& cur_timeMgr,
                                                         Base::ITrafficElementPtr followElemPtr) TX_NOEXCEPT {
  // 计算相对时间
  const Base::txFloat relativePassTime = cur_timeMgr.PassTime() - m_event_start_passTime;
  // 如果元素存活，且未停止，且相对时间大于等于0
  if (IsAlive() && CallFail(IsStop()) && (relativePassTime >= 0.0)) {
    // 如果是相对跟随
    if (CallSucc(IsRelativeObs())) {
      // 更新相对跟随
      UpdateRelative(cur_timeMgr, relativePassTime, followElemPtr);
    } else if (CallSucc(IsAbsoluteObs())) {  // 如果是绝对跟随
      // 更新绝对跟随
      UpdateAbsolute(cur_timeMgr, relativePassTime, followElemPtr);
    }
    // 使用RTree2D_NearestReferenceLine查询最近的参考线
    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(
        mLocation.GeomCenter(), mLocation.LaneLocInfo(), mLocation.DistanceAlongCurve(), mLocation.LaneOffset());
    hadmap::txLanePtr resLane = nullptr;
    hadmap::txLaneLinkPtr resLaneLink = nullptr;
    // 如果在车道上
    if (mLocation.IsOnLane()) {
      // 获取车道指针
      resLane = HdMap::HadmapCacheConCurrent::GetTxLanePtr(mLocation.LaneLocInfo().onLaneUid);
      // 如果车道指针非空
      if (NonNull_Pointer(resLane)) {
        LogInfo << "hadmap::getLane success.";
        // 重新定位跟踪器
        RelocateTracker(resLane, 0.0);
        // 更新车道位置信息
        mLocation.PosOnLaneCenterLinePos() =
            HdMap::HadmapCacheConCurrent::GetLanePos(resLane, mLocation.DistanceAlongCurve());
        // 更新车道方向信息
        mLocation.vLaneDir() = HdMap::HadmapCacheConCurrent::GetLaneDir(resLane, mLocation.DistanceAlongCurve());
      } else {
        // 设置生命周期为停止
        mLifeCycle.SetStop();
        LogWarn << "GetTxLaneLinkPtr failure. " << mLocation.LaneLocInfo();
        return false;
      }
    } else {  // 如果在车道链接上
      // 获取车道链接指针
      resLaneLink = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(mLocation.LaneLocInfo().onLinkFromLaneUid,
                                                                   mLocation.LaneLocInfo().onLinkToLaneUid);
      // 如果车道链接指针非空
      if (NonNull_Pointer(resLaneLink)) {
        LogInfo << "hadmap::getLaneLink success.";
        // 重新定位跟踪器
        RelocateTracker(resLaneLink, FLT_MAX);
        // 更新车道链接位置信息
        mLocation.PosOnLaneCenterLinePos() =
            HdMap::HadmapCacheConCurrent::GetLaneLinkPos(resLaneLink, mLocation.DistanceAlongCurve());
        // 更新车道链接方向信息
        mLocation.vLaneDir() =
            HdMap::HadmapCacheConCurrent::GetLaneLinkDir(resLaneLink, mLocation.DistanceAlongCurve());
      } else {
        // 设置生命周期为停止
        mLifeCycle.SetStop();
        LogWarn << "GetTxLaneLinkPtr failure. " << mLocation.LaneLocInfo();
        return false;
      }
    }
    UpdateHashedLaneInfo(mLocation);
  }
  return IsAlive();
}

Base::txBool TAD_RelativeTrajectoryFollowElement::Update(TimeParamManager const&) TX_NOEXCEPT { return true; }
Base::txBool TAD_RelativeTrajectoryFollowElement::Release() TX_NOEXCEPT { return true; }
Base::txBool TAD_RelativeTrajectoryFollowElement::FillingElement(Base::TimeParamManager const& timeMgr,
                                                                 sim_msg::Traffic& trafficData) TX_NOEXCEPT {
  // 如果元素存活
  if (IsAlive()) {
    // 创建一个StaticObstacle指针
    sim_msg::StaticObstacle* tmpObstacleData = nullptr;
    {
#if USE_TBB
      // 使用TBB互斥锁锁定s_tbbMutex_outputTraffic
      tbb::mutex::scoped_lock lock(s_tbbMutex_outputTraffic);
#endif
      // 向trafficData的staticobstacles添加一个新的StaticObstacle
      tmpObstacleData = trafficData.add_staticobstacles();
    }
    // 设置StaticObstacle的时间戳
    tmpObstacleData->set_t(timeMgr.TimeStamp());
    // 设置StaticObstacle的ID
    tmpObstacleData->set_id(Id());
#if USE_CustomModelImport
    tmpObstacleData->set_type(mObstacleTypeId);
#else  /*#USE_CustomModelImport*/
    tmpObstacleData->set_type(__enum2int__(STATIC_ELEMENT_TYPE, mObstacleType));
#endif /*#USE_CustomModelImport*/
    tmpObstacleData->set_type(__enum2int__(STATIC_ELEMENT_TYPE, mObstacleType));
    const hadmap::txPoint geom_center_gps = GeomCenter().WGS84();
    // 设置StaticObstacle的经度
    tmpObstacleData->set_x(__Lon__(geom_center_gps));
    // 设置StaticObstacle的纬度
    tmpObstacleData->set_y(__Lat__(geom_center_gps));
    tmpObstacleData->set_z(__Alt__(geom_center_gps));
    // 设置StaticObstacle的长度
    tmpObstacleData->set_length(GetLength());
    // 设置StaticObstacle的宽度
    tmpObstacleData->set_width(GetWidth());
    // 设置StaticObstacle的高度
    tmpObstacleData->set_height(GetHeigth());
    // 计算StaticObstacle的方向（弧度）
    const Base::txFloat _headingRadian = Utils::GetLaneAngleFromVectorOnENU(GetLaneDir()).GetRadian();
    tmpObstacleData->set_heading(_headingRadian);
    SaveStableState();
    FillingSpatialQuery();
  }
  return true;
}
sim_msg::Car* TAD_RelativeTrajectoryFollowElement::FillingElement(txFloat const timeStamp,
                                                                  sim_msg::Car* pSimVehicle) TX_NOEXCEPT {
  return nullptr;
}
Base::txBool TAD_RelativeTrajectoryFollowElement::FillingSpatialQuery() TX_NOEXCEPT {
  // 如果元素存活
  if (IsAlive()) {
    // 获取参考多边形
    const auto& refPolygon = mGeometryData.Polygon();
    // 如果参考多边形的大小等于OBB2D::PolygonSize
    if (Geometry::OBB2D::PolygonSize == refPolygon.size()) {
      /*LOG(INFO) << "[obs] " << mGeometryData.Str();*/
      // 将参考多边形插入到RTree2D中
      Geometry::SpatialQuery::RTree2D::InsertBulk(refPolygon[0], refPolygon[1], refPolygon[2], refPolygon[3],
                                                  GeomCenter().ToENU().ENU2D(), GetBasePtr());
      return true;
    } else {
      // 输出日志信息
      LOG(WARNING) << "m_vecPolygon.size = " << refPolygon.size();
      return false;
    }
  } else {
    return false;
  }
}
Base::txBool TAD_RelativeTrajectoryFollowElement::Pre_Update(
    const TimeParamManager&, std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT {
  return true;
}
Base::txBool TAD_RelativeTrajectoryFollowElement::Post_Update(TimeParamManager const&) TX_NOEXCEPT { return true; }
void TAD_RelativeTrajectoryFollowElement::SaveStableState() TX_NOEXCEPT {
  // 保存上次的几何中心
  mLocation.LastGeomCenter() = mLocation.GeomCenter().ToENU();
  // 保存上次的后轴中心
  mLocation.LastRearAxleCenter() = mLocation.RearAxleCenter().ToENU();
  // 保存上次在车道中心线上的位置
  mLocation.LastOnLanePos() = mLocation.PosOnLaneCenterLinePos();
  // 保存上次车道上方向
  mLocation.LastLaneDir() = mLocation.vLaneDir();
  // 保存上次的车道信息
  mLocation.LastLaneInfo() = mLocation.LaneLocInfo();
  mLocation.LastHeading() = mLocation.heading();
  mKinetics.LastVelocity() = 0.0;
  mKinetics.LastAcceleration() = 0.0;
}

Base::txFloat TAD_RelativeTrajectoryFollowElement::SetStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  // 如果是相对跟随，且相对跟随动作向量非空
  if (CallSucc(IsRelativeObs()) && _NonEmpty_(m_relative_action_vec)) {
    // 设置生命周期的开始时间为当前时间加上相对跟随动作向量的第一个元素的时间
    mLifeCycle.StartTime() = timeMgr.PassTime() + m_relative_action_vec.front().time;
    // 设置事件开始时间为当前时间
    m_event_start_passTime = timeMgr.PassTime();
    LogInfo << TX_VARS(Id()) << " start relative action." << TX_VARS(mLifeCycle.StartTime());
  } else if (CallSucc(IsAbsoluteObs()) &&
             _NonEmpty_(m_absolute_action_vec)) {  // 如果是绝对跟随，且绝对跟随动作集合非空
    // 设置生命周期的开始时间为当前时间加上绝对跟随动作集合的第一个元素的时间
    mLifeCycle.StartTime() = timeMgr.PassTime() + m_absolute_action_vec.front().time;
    // 设置事件开始时间为当前时间
    m_event_start_passTime = timeMgr.PassTime();
    LogInfo << TX_VARS(Id()) << " start absolute action." << TX_VARS(mLifeCycle.StartTime());
  }
  // 返回生命周期的开始时间
  return mLifeCycle.StartTime();
}

Base::txBool TAD_RelativeTrajectoryFollowElement::CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
#if 0
  mLifeCycle.SetStart();
  m_event_start_passTime = timeMgr.PassTime();
  return IsStart();
#else
  // 如果元素未开始
  if (CallFail(IsStart())) {
    // 如果相对时间大于0，且绝对时间大于等于生命周期的开始时间
    if (timeMgr.RelativeTime() > 0.0 && timeMgr.AbsTime() >= mLifeCycle.StartTime()) {
      // 设置生命周期为开始
      mLifeCycle.SetStart();
      // OnStart(timeMgr);
    }
  }
  return IsStart();
#endif
}
Base::txBool TAD_RelativeTrajectoryFollowElement::CheckDeadLine() TX_NOEXCEPT { return IsAlive(); }

std::tuple<Base::Enums::ElementType, Base::txSysId> TAD_RelativeTrajectoryFollowElement::RelativeReferenceElementInfo()
    const TX_NOEXCEPT {
  if (_NonEmpty_(m_relative_action_vec)) {
    return std::make_tuple(m_relative_action_vec.front().elemType, m_relative_action_vec.front().elemId);
  } else {
    return std::make_tuple(_plus_(Base::Enums::ElementType::Unknown), 0);
  }
}

Base::txBool TAD_RelativeTrajectoryFollowElement::HandlerEvent(Base::TimeParamManager const& timeMgr,
                                                               ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT {
  SetStart(timeMgr);
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)

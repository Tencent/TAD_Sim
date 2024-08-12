// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_traffic_element_base.h"
#include "tx_math.h"
#include "tx_hadmap_utils.h"
#include "tx_header.h"
#define VehicleTrajInfo LOG_IF(INFO, FLAGS_LogLevel_Vehicle_Traj)
TX_NAMESPACE_OPEN(Base)

#if __TX_Mark__("IIdentity")

boost::atomic<txSysId> IIdentity::s_sys_id_base(FLAGS_Topological_Root_Id);
IIdentity::ElemId2SysIdMap IIdentity::s_elemId2SysId;

txSysId IIdentity::RegisterElementId(const txSysId _sysId, const txSysId elemId) TX_NOEXCEPT {
  // 使用const_accessor查找elemId是否已经存在于s_elemId2SysId映射中
  ElemId2SysIdMap::const_accessor ca;
  // 如果查找失败，说明elemId不存在于s_elemId2SysId映射中
  if (CallFail(s_elemId2SysId.find(ca, elemId))) {
    ca.release();
    // 使用accessor插入elemId到s_elemId2SysId映射中
    ElemId2SysIdMap::accessor a;
    s_elemId2SysId.insert(a, elemId);
    // 将_sysId与elemId关联
    a->second = _sysId;
    a.release();
  }
  return _sysId;
}

// 根据元素ID获取系统ID
txBool IIdentity::GetSysIdByElemId(const txSysId elemId, txSysId& resSysId) TX_NOEXCEPT {
  ElemId2SysIdMap::const_accessor ca;
  // 如果查找成功，说明elemId存在于s_elemId2SysId映射中
  if (CallSucc(s_elemId2SysId.find(ca, elemId))) {
    // 将找到的系统ID赋值给resSysId
    resSysId = ca->second;
    return true;
  } else {
    return false;
  }
}

Base::txSysId IIdentity::CreateSysId(const txSysId elemId) TX_NOEXCEPT {
  return RegisterElementId((++s_sys_id_base), elemId);
  /*return (++s_sys_id_base);*/
}

txSysId IIdentity::SysIdCount() TX_NOEXCEPT { return (s_sys_id_base + 1 TX_MARK("ego")); }

void IIdentity::ResetSysId() TX_NOEXCEPT {
  // 将s_sys_id_base设置为FLAGS_Topological_Root_Id
  s_sys_id_base.exchange(FLAGS_Topological_Root_Id);
  s_elemId2SysId.clear();
}

txSysId IIdentity::SysRootId() TX_NOEXCEPT { return FLAGS_Topological_Root_Id; }

txBool IIdentity::IsTheSameElement(const IIdentityPtr& leftElemPtr, const IIdentityPtr& rightElemPtr) TX_NOEXCEPT {
  // 如果leftElemPtr和rightElemPtr都不为空
  if (NonNull_Pointer(leftElemPtr) && NonNull_Pointer(rightElemPtr)) {
    // 比较两个元素的系统ID是否相同
    return (leftElemPtr->SysId()) == (rightElemPtr->SysId());
  } else {
    // 如果leftElemPtr和rightElemPtr都为空
    if (Null_Pointer(leftElemPtr) && Null_Pointer(rightElemPtr)) {
      TX_MARK("forward compatbility !!!");
      return true;
    } else {
      // 如果leftElemPtr和rightElemPtr中有一个为空，返回false
      return false;
    }
  }
}

txBool IIdentity::IsNotTheSameElement(const IIdentityPtr& leftElemPtr, const IIdentityPtr& rightElemPtr) TX_NOEXCEPT {
  return CallFail(IsTheSameElement(leftElemPtr, rightElemPtr));
}
#endif /*__TX_Mark__("IIdentity")*/

#if __TX_Mark__("IMapRange")
std::vector<Base::map_range_t> IMapRange::s_vec_map_range;
Base::txBool IMapRange::InRanges(const Base::txFloat dLng, const Base::txFloat dLat) TX_NOEXCEPT {
  // 遍历地图范围列表
  for (const Base::map_range_t& ref_map_range : s_vec_map_range) {
    // 获取地图范围的左下角和右上角坐标
    const hadmap::txPoint& bottom_left = ref_map_range.bottom_left;
    const hadmap::txPoint& top_right = ref_map_range.top_right;
    // 判断给定的经纬度是否在地图范围内
    if (dLng >= __Lon__(bottom_left) && dLng < __Lon__(top_right) && dLat >= __Lat__(bottom_left) &&
        dLat < __Lat__(top_right)) {
      return true;
    }
  }
  // 如果地图范围列表非空，且给定的经纬度不在地图范围内，返回false
  if (_NonEmpty_(s_vec_map_range)) {
    return false;
  } else {
    return true;
  }
}
#endif /*__TX_Mark__("IMapRange")*/

txFloat IMapLocation::GenerateST(const txFloat _s, const txFloat _distanceCurve) TX_NOEXCEPT {
  return _s + _distanceCurve;
}

Coord::txWGS84 IMapLocation::ComputeLaneOffset(Coord::txWGS84 srcPos, const Base::txVec3& vLaneDir,
                                               const Base::txFloat offset) TX_NOEXCEPT {
  return Coord::ComputeLaneOffset(srcPos, vLaneDir, offset);
}

Base::Enums::VehicleInJunction_SelfDrivingDirection GetSelfDrivingDirection(const Base::txVec3& fromLaneDir,
                                                                            const Base::txVec3& toLaneDir) TX_NOEXCEPT {
  const txFloat fromAngle = Math::FixAngle(Utils::GetLaneAngleFromVectorOnENU(fromLaneDir).GetDegree());
  const txFloat toAngle = Math::FixAngle(Utils::GetLaneAngleFromVectorOnENU(toLaneDir).GetDegree());
  // 根据起始车道方向向量和目标车道方向向量的角度，获取自动驾驶方向
  return GetSelfDrivingDirection(fromAngle, toAngle);
}

Base::Enums::VehicleInJunction_SelfDrivingDirection GetSelfDrivingDirection(const Base::txFloat fromAngle,
                                                                            const Base::txFloat toAngle) TX_NOEXCEPT {
  using SelfDrivingDirection = Base::Enums::VehicleInJunction_SelfDrivingDirection;
  // 计算两个角度之间的差值，并将其修正为0到360度之间的值
  Base::txFloat diffRot = Math::FixAngle(toAngle - fromAngle);
  // 根据差值判断自动驾驶方向
  if (diffRot > 315 || diffRot < 45) {
    // 如果差值在315到45度之间，表示直行
    return _plus_(SelfDrivingDirection::eStraight);
  } else if (diffRot > 135 && diffRot < 225) {
    // 如果差值在135到225度之间，表示掉头
    return _plus_(SelfDrivingDirection::eUTurnSide);
  } else if (diffRot > 225) {
    // 如果差值大于225度，表示右转
    return _plus_(SelfDrivingDirection::eRightSide);
  } else {
    return _plus_(SelfDrivingDirection::eLeftSide);
  }
}

Base::Enums::Vehicle2Vehicle_RelativeDirection ComputeVehicleInterchangeRelationship(
    const Base::txVec3& SelfVehicleDir, const Base::txVec3& otherVehicleDir) TX_NOEXCEPT {
  const txFloat SelfVehicleAngle = Math::FixAngle(Utils::GetLaneAngleFromVectorOnENU(SelfVehicleDir).GetDegree());
  const txFloat otherVehicleAngle = Math::FixAngle(Utils::GetLaneAngleFromVectorOnENU(otherVehicleDir).GetDegree());
  // 根据两车方向向量的角度，计算两车之间的相对方向
  return ComputeVehicleInterchangeRelationship(SelfVehicleAngle, otherVehicleAngle);
}

// 根据两车方向向量的角度，计算两车之间的相对方向
Base::Enums::Vehicle2Vehicle_RelativeDirection ComputeVehicleInterchangeRelationship(
    const Base::txFloat SelfVehicleAngle, const Base::txFloat otherVehicleAngle) TX_NOEXCEPT {
  using RelativeDirection = Base::Enums::Vehicle2Vehicle_RelativeDirection;
  // 计算两车方向角度之间的差值，并将其修正为0到360度之间的值
  Base::txFloat diffRot = Math::FixAngle(otherVehicleAngle - SelfVehicleAngle);
  // 根据差值判断两车之间的相对方向
  if (diffRot > 315 || diffRot < 45) {
    // 如果差值在315到45度之间，表示同向
    return _plus_(RelativeDirection::eSameSide);
  } else if (diffRot > 135 && diffRot < 225) {
    return _plus_(RelativeDirection::eOpposite);
  } else if (diffRot > 225) {
    // 如果差值大于225度，表示左转
    return _plus_(RelativeDirection::eLeftSide);
  } else {
    return _plus_(RelativeDirection::eRightSide);
  }
}

Geometry::Topology::txEdgeVec ITrafficElement::GetTopologyRelationShip() const TX_NOEXCEPT {
  Geometry::Topology::txEdgeVec edges;
  // 将当前交通元素的系统根ID和系统ID作为一条边添加到边向量中
  edges.push_back(Geometry::Topology::txEdge(SysRootId(), SysId()));
  txAssert(1 == edges.size());
  return edges;
}

txBool ITrafficElement::Pre_Update(TimeParamManager const&,
                                   std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT {
  // 计算局部坐标系的逆变换矩阵
  ComputeLocalCoordTransMatInv(StableGeomCenter().ENU2D(), StableLaneDir());
#if __Consistency__
  std::ostringstream oss;
  const auto matInv = TransMatInv();
  oss << TX_VARS_NAME(center, Utils::ToString(StableGeomCenter().ENU2D()))
      << TX_VARS_NAME(dir, Utils::ToString(StableLaneDir())) << TX_VARS_NAME(matInv00, matInv.coeffRef(0, 0))
      << TX_VARS_NAME(matInv00, matInv.coeffRef(0, 1)) << TX_VARS_NAME(matInv00, matInv.coeffRef(1, 0))
      << TX_VARS_NAME(matInv00, matInv.coeffRef(1, 1));
  ConsistencyAppend(oss.str());
#endif /*__Consistency__*/
  return true;
}

txBool ITrafficElement::Post_Update(TimeParamManager const&) TX_NOEXCEPT {
  // 获取交通元素的位置（以ENU坐标系表示）
  const Base::txVec2 p0 = GetLocation().ToENU().ENU2D();
  // 获取交通元素的朝向角度（以弧度为单位）
  const Base::txFloat _headingRadian = GetHeadingWithAngle().GetRadian();
  // 计算交通元素的朝向向量
  const Base::txVec2 yawDir(GetHeadingWithAngle().Cos(), GetHeadingWithAngle().Sin());
  ComputeProjectionMat(p0, (p0 + 2.0 * yawDir));
  return true;
}

ITrafficElement::KineticsInfo_t ITrafficElement::GetKineticsInfo(Base::TimeParamManager const& timeMgr) const
    TX_NOEXCEPT {
  KineticsInfo_t retInfo;
  retInfo.m_elemId = Id();
  retInfo.m_compute_velocity = GetVelocity();
  retInfo.m_acc = GetAcc();
  retInfo.m_show_velocity = GetShowVelocity();
  return retInfo;
}

#if __TX_Mark__("ILocalCoord")

Base::txVec2 ILocalCoord::RelativePosition(const Base::txVec2& center2d, const Base::txVec2 obj2d,
                                           const Base::txMat2& tranMatInv) TX_NOEXCEPT {
  return (obj2d - center2d).transpose() * tranMatInv;
}

void ILocalCoord::ComputeProjectionMat(const Base::txVec2& p0, const Base::txVec2& p1) TX_NOEXCEPT {
  // 将输入的两个点分别赋值给局部坐标系的原点和方向向量
  m_p0 = p0;
  m_p1 = p1;
  // 获取局部坐标系的逆变换矩阵
  Base::txMat2& invMat = m_invMat;
  // 标记逆变换矩阵的计算过程
  __TX_Mark__("the inverse of the second order matrix [[a,b],[-b,a]]");
  // 计算两点之间的横坐标差值
  const Base::txFloat _a = p1.x() - p0.x();
  // 计算两点之间的纵坐标差值
  const Base::txFloat _b = p1.y() - p0.y();
  // 计算两点之间的距离平方
  const Base::txFloat _det = _a * _a + _b * _b;
  // 计算逆变换矩阵的系数
  const Base::txFloat _a_det = _a / _det;
  const Base::txFloat _b_det = _b / _det;
  // 填充逆变换矩阵的系数
  invMat.coeffRef(0, 0) = invMat.coeffRef(1, 1) = _a_det;
  invMat.coeffRef(0, 1) = -1.0 * _b_det;
  invMat.coeffRef(1, 0) = _b_det;
  m_rhs_1 = -1.0 * (-1.0 * p0.y() * (_a /*p1.x() - p0.x()*/) + p0.x() * (_b /*p1.y() - p0.y()*/));
}

Base::txVec2 ILocalCoord::ComputeProjectPt(const Base::txVec2& q) const TX_NOEXCEPT {
  // 获取局部坐标系的原点和方向向量
  const Base::txVec2& p0 = m_p0;
  const Base::txVec2& p1 = m_p1;
  // 创建一个向量用于存储右侧项
  Base::txVec2 rhs;
  rhs[1] = m_rhs_1;
  rhs[0] = -1.0 * (-1.0 * q.x() * (p1.x() - p0.x()) - q.y() * (p1.y() - p0.y()));
  // 计算投影点的坐标
  Base::txVec2 projPos = m_invMat * rhs;
  return projPos;
}

Base::txBool ILocalCoord::IsPtInSegment(const Base::txVec2& pt, const Base::txVec2& segHead,
                                        const Base::txVec2& segTrail) const TX_NOEXCEPT {
  // 如果点到线段头部和尾部的向量夹角为0，则点在线段上
  if (!Math::IsSynchronicity(pt - segHead, pt - segTrail)) {
    return true;
  } else {
    return false;
  }
}

Base::txFloat TX_MARK("dist > 0 means obs is front of ego, dist < 0.0 means obs is behind of ego")
    ILocalCoord::ComputeProjectDistance(const Geometry::OBB2D::PolygonArray& target_vecq,
                                        const Geometry::OBB2D::PolygonArray& source_vecq) const TX_NOEXCEPT {
  // 计算目标交通元素的头部和尾部在局部坐标系中的投影点
  const Base::txVec2 egoTrail = ComputeProjectPt(target_vecq[0]);
  const Base::txVec2 egoHead = ComputeProjectPt(target_vecq[1]);

  // 计算源交通元素的头部和尾部在局部坐标系中的投影点
  const Base::txVec2 obsTrail = ComputeProjectPt(source_vecq[0]);
  const Base::txVec2 obsHead = ComputeProjectPt(source_vecq[1]);

  // 如果启用日志记录，则输出投影点的坐标
  if (FLAGS_LogLevel_EventTrigger) {
    Coord::txENU enu_pos;
    enu_pos.FromENU(egoHead.x(), egoHead.y());
    LOG(INFO) << TX_VARS_NAME(egoHead, enu_pos);
    enu_pos.FromENU(egoTrail.x(), egoTrail.y());
    LOG(INFO) << TX_VARS_NAME(egoTrail, enu_pos);

    enu_pos.FromENU(obsHead.x(), obsHead.y());
    LOG(INFO) << TX_VARS_NAME(selfHead, enu_pos);
    enu_pos.FromENU(obsTrail.x(), obsTrail.y());
    LOG(INFO) << TX_VARS_NAME(selfTrail, enu_pos);
  }

  // 判断源交通元素的头部和尾部是否在目标交通元素的线段上
  if (!IsPtInSegment(obsHead, egoHead, egoTrail) && !IsPtInSegment(obsTrail, egoHead, egoTrail) &&
      !IsPtInSegment(egoHead, obsHead, obsTrail) && !IsPtInSegment(egoTrail, obsHead, obsTrail)) {
    // 计算源交通元素头部和尾部与目标交通元素头部和尾部之间的距离
    const Base::txFloat dist_1 = (obsHead - egoHead).norm();
    const Base::txFloat dist_2 = (obsHead - egoTrail).norm();
    const Base::txFloat dist_3 = (obsTrail - egoTrail).norm();
    const Base::txFloat dist_4 = (obsTrail - egoHead).norm();

    // 计算最小距离
    const Base::txFloat minDist = std::min(std::min(dist_1, dist_2), std::min(dist_3, dist_4));
    Base::txFloat sgn = 1.0;
    // 判断源交通元素是否在目标交通元素的前面
    if (Math::IsSynchronicity(obsHead - egoTrail, egoHead - egoTrail)) {
      TX_MARK("obs is front of ego.");
      sgn = 1.0;
    } else {
      sgn = -1.0;
    }
    return sgn * minDist;
  } else {
    TX_MARK("projection overlay");
    return 0.0;
  }
}

void ILocalCoord::ComputeLocalCoordTransMatInv(const Base::txVec2& _geo_center, const Base::txVec3& _dir) TX_NOEXCEPT {
  const Unit::txDegree _headingAngle = Utils::GetLaneAngleFromVectorOnENU(_dir);
  // 根据局部坐标系的原点和方向角计算局部坐标系的变换矩阵
  Utils::CreateLocalCoordTranslationMat(_geo_center, _headingAngle, m_TransMat_UnSerialize);
  m_TransMatInv = m_TransMat_UnSerialize.inverse();
}

#endif /*__TX_Mark__("ILocalCoord")*/

#if __TX_Mark__("ISimulationConsistency")

void ISimulationConsistency::ConsistencyAppend(const txString& str) TX_NOEXCEPT { m_ConsistencyStr << "[*]" << str; }

txString ISimulationConsistency::ConsistencyStr() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS_NAME(Id, ConsistencyId()) << TX_VARS_NAME(Center, StableGeomCenter().StrWGS84())
      << TX_VARS_NAME(LocInfo, StableLaneInfo()) << TX_VARS_NAME(v, StableVelocity()) << TX_VARS_NAME(a, StableAcc())
      << TX_VARS_NAME(S, StableDistanceAlongCurve()) << TX_VARS_NAME(InverS, StableInvertDistanceAlongCurve());
  return oss.str();
}

#endif /*#if __TX_Mark__("ISimulationConsistency")*/
TX_NAMESPACE_CLOSE(Base)

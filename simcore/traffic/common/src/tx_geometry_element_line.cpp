// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_geometry_element_line.h"
#include "tx_frame_utils.h"
#include "tx_math.h"
#include "tx_string_utils.h"
TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(Element)

txGeomElementBase::txString txLine::Str() const TX_NOEXCEPT { return ParentClass::Str(); }

txGeomElementBase::txBool txLine::Initialize(const txVec3& _start_vtx, const txVec3& _end_vtx) TX_NOEXCEPT {
  mVtxStart = _start_vtx;
  mVtxEnd = _end_vtx;
  m_RealLength = m_PolylineLength = (EndVtx() - StartVtx()).norm();
  mIsValid = true;
  return true;
}

txGeomElementBase::txBool txLine::CheckIntersection(txGeomElementBasePtr obj_ptr, txVertex& intersectPt,
                                                    txFloat& distOnTraj, txFloat& distOnObjTraj) const TX_NOEXCEPT {
  if (IsValid() && NonNull_Pointer(obj_ptr) && obj_ptr->IsValid()) {
    if (GeomElemType::eLine == (obj_ptr->ElemType())) {
      Base::txVec2 intersectPt2d;
      if (CallSucc(Math::intersection_between_2d_line(Utils::Vec3_Vec2(StartVtx()), Utils::Vec3_Vec2(EndVtx()),
                                                      Utils::Vec3_Vec2(obj_ptr->StartVtx()),
                                                      Utils::Vec3_Vec2(obj_ptr->EndVtx()), intersectPt2d))) {
        intersectPt = Utils::Vec2_Vec3(intersectPt2d);
        distOnTraj = (intersectPt2d - Utils::Vec3_Vec2(StartVtx())).norm();
        distOnObjTraj = (intersectPt2d - Utils::Vec3_Vec2(obj_ptr->StartVtx())).norm();
        return true;
      }
    } else if (GeomElemType::ePolyline == (obj_ptr->ElemType())) {
      Base::txVec2 intersectPt2d;
      const Base::txVec2 start2d = Utils::Vec3_Vec2(StartVtx());
      const Base::txVec2 end2d = Utils::Vec3_Vec2(EndVtx());
      txPolylinePtr polylinePtr = std::dynamic_pointer_cast<txPolyline>(obj_ptr);
      if (NonNull_Pointer(polylinePtr)) {
        const txSize samplePtSize = polylinePtr->SamplingPtSize();
        txVec2 leftPt2d = Utils::Vec3_Vec2(polylinePtr->StartVtx());
        distOnObjTraj = 0.0;
        for (txSize idx = 1; idx < samplePtSize; ++idx) {
          txVec2 rightPt2d = Utils::Vec3_Vec2(polylinePtr->SamplingPt(idx));
          if (CallSucc(Math::intersection_between_2d_line(start2d, end2d, leftPt2d, rightPt2d, intersectPt2d))) {
            intersectPt = Utils::Vec2_Vec3(intersectPt2d);
            distOnTraj += (intersectPt2d - start2d).norm();
            distOnTraj = (intersectPt2d - start2d).norm();
            return true;
          }
          distOnObjTraj += (rightPt2d - leftPt2d).norm();
          leftPt2d = rightPt2d;
        }
      }
    }
  }
  return false;
}

txGeomElementBase::txBool txPolyline::IsValid() const TX_NOEXCEPT { return (mIsValid) && (mPts.size() > 1); }

void txPolyline::Release() TX_NOEXCEPT {
  ParentClass::Release();
  mPts.clear();
}

txGeomElementBase::txString txPolyline::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{";
  for (const auto& refPt : mPts) {
    oss << Utils::ToString(refPt);
  }
  oss << ParentClass::Str();
  oss << "}";
  return oss.str();
}

txGeomElementBase::txBool txPolyline::Initialize(const std::vector<txVec3>& _vtx_array) TX_NOEXCEPT {
  Release();
  if (_vtx_array.size() > 1) {
    mPts.insert(std::end(mPts), std::begin(_vtx_array), std::end(_vtx_array));
    txFloat resLen = 0.0;
    for (txSize i = 1; i < mPts.size(); ++i) {
      resLen += (SamplingPt(i) - SamplingPt(i - 1)).norm();
    }
    m_RealLength = m_PolylineLength = resLen;
    mIsValid = true;
  }
  return IsValid();
}

txLinePtr txPolyline::SubSegmentLine(const txSize idx) const TX_NOEXCEPT {
  if (idx >= 0 && idx < SegmentLineSize()) {
    txLinePtr resPtr = std::make_shared<txLine>();
    resPtr->Initialize(SamplingPt(idx), SamplingPt(idx + 1));
    return resPtr;
  } else {
    return nullptr;
  }
}

txGeomElementBase::txBool txPolyline::CheckIntersection(txGeomElementBasePtr obj_ptr, txVertex& intersectPt,
                                                        txFloat& distOnTraj, txFloat& distOnObjTraj) const TX_NOEXCEPT {
  if (IsValid() && NonNull_Pointer(obj_ptr) && obj_ptr->IsValid()) {
    distOnTraj = 0.0;
    if (GeomElemType::eLine == (obj_ptr->ElemType())) {
      Base::txVec2 intersectPt2d;
      const Base::txVec2 objStart2d = Utils::Vec3_Vec2(obj_ptr->StartVtx());
      const Base::txVec2 objEnd2d = Utils::Vec3_Vec2(obj_ptr->EndVtx());

      const txSize samplePtSize = SamplingPtSize();
      txVec2 leftPt2d = Utils::Vec3_Vec2(StartVtx());
      for (txSize idx = 1; idx < samplePtSize; ++idx) {
        txVec2 rightPt2d = Utils::Vec3_Vec2(SamplingPt(idx));
        if (CallSucc(Math::intersection_between_2d_line(leftPt2d, rightPt2d, objStart2d, objEnd2d, intersectPt2d))) {
          intersectPt = Utils::Vec2_Vec3(intersectPt2d);
          distOnTraj += (intersectPt2d - leftPt2d).norm();
          distOnObjTraj = (intersectPt2d - Utils::Vec3_Vec2(obj_ptr->StartVtx())).norm();
          return true;
        }
        distOnTraj += (rightPt2d - leftPt2d).norm();
        leftPt2d = rightPt2d;
      }
    } else if (GeomElemType::ePolyline == (obj_ptr->ElemType())) {
      txPolylinePtr polylinePtr = std::dynamic_pointer_cast<txPolyline>(obj_ptr);
      if (NonNull_Pointer(polylinePtr)) {
        Base::txVec2 intersectPt2d;
        const txSize srcSamplePtSize = SamplingPtSize();
        const txSize objSamplePtSize = polylinePtr->SamplingPtSize();

        txVec2 srcLeftPt2d = Utils::Vec3_Vec2(StartVtx());
        for (txSize srcIdx = 1; srcIdx < srcSamplePtSize; ++srcIdx) {
          distOnObjTraj = 0.0;
          txVec2 srcRightPt2d = Utils::Vec3_Vec2(SamplingPt(srcIdx));
          txVec2 objLeftPt2d = Utils::Vec3_Vec2(polylinePtr->StartVtx());
          for (txSize objIdx = 1; objIdx < objSamplePtSize; ++objIdx) {
            txVec2 objRightPt2d = Utils::Vec3_Vec2(polylinePtr->SamplingPt(objIdx));
            if (CallSucc(Math::intersection_between_2d_line(srcLeftPt2d, srcRightPt2d, objLeftPt2d, objRightPt2d,
                                                            intersectPt2d))) {
              intersectPt = Utils::Vec2_Vec3(intersectPt2d);
              distOnTraj += (intersectPt2d - srcLeftPt2d).norm();
              distOnObjTraj += (intersectPt2d - objLeftPt2d).norm();
              return true;
            }
            distOnObjTraj += (objRightPt2d - objLeftPt2d).norm();
            objLeftPt2d = objRightPt2d;
          }
          distOnTraj += (srcRightPt2d - srcLeftPt2d).norm();
          srcLeftPt2d = srcRightPt2d;
        }
      }
    }
  }
  return false;
}

TX_NAMESPACE_CLOSE(Element)
TX_NAMESPACE_CLOSE(Geometry)

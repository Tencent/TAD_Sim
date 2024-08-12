// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_units.h"
#include <boost/format.hpp>
#include "tx_math.h"

TX_NAMESPACE_OPEN(Unit)

#if __TX_Mark__("IAngle")

std::array<Base::txFloat, IAngle::RadianSampleCnt> IAngle::s_array_radian_lookup_table_cos;
std::array<Base::txFloat, IAngle::RadianSampleCnt> IAngle::s_array_radian_lookup_table_sin;
std::array<Base::txFloat, IAngle::DegreeSampleCnt> IAngle::s_array_degree_lookup_table_cos;
std::array<Base::txFloat, IAngle::DegreeSampleCnt> IAngle::s_array_degree_lookup_table_sin;

Base::txBool IAngle::GenerateSinCosLookupTable() TX_NOEXCEPT {
  txAssert(s_array_radian_lookup_table_cos.size() == s_array_radian_lookup_table_sin.size());
  txAssert(s_array_degree_lookup_table_cos.size() == s_array_degree_lookup_table_sin.size());

  /*step 1. radian lookup table*/
  const Base::txFloat dbRadianSampleScalarFactor = RadianSampleScalarFactor;
  const Base::txSize radian_lookup_size = s_array_radian_lookup_table_cos.size();
  for (Base::txSize i = 0; i < radian_lookup_size; ++i) {
    const Base::txFloat rad = (i / dbRadianSampleScalarFactor);
    s_array_radian_lookup_table_cos[i] = std::cos(rad);
    s_array_radian_lookup_table_sin[i] = std::sin(rad);
  }

  /*step 2. degree lookup table*/
  const Base::txFloat dbDegreeSampleScalarFactor = DegreeSampleScalarFactor;
  const Base::txInt degree_lookup_size = s_array_degree_lookup_table_cos.size();
  for (Base::txInt i = 0; i < degree_lookup_size; ++i) {
    const Base::txFloat degree = (i / dbDegreeSampleScalarFactor);
    const Base::txFloat rad = Math::Degrees2Radians(degree);
    s_array_degree_lookup_table_cos[i] = std::cos(rad);
    s_array_degree_lookup_table_sin[i] = std::sin(rad);
  }
  return true;
}

#endif /*__TX_Mark__("IAngle")*/

#if __TX_Mark__("txRadian")
Base::txFloat txRadian::GetDegree() const TX_NOEXCEPT { return Math::Radians2Degrees(m_radian); }

const txRadian& txRadian::FromDegree(const Base::txFloat _d) TX_NOEXCEPT {
  m_radian = Math::Degrees2Radians(Math::DegreeNormalise(_d));
  return *this;
}

txDegree txRadian::ToDegree() const TX_NOEXCEPT {
  txDegree degree;
  degree.FromRadian(m_radian);
  return degree;
}

void txRadian::Normalize(const Base::txFloat _s, const Base::txFloat _e) TX_NOEXCEPT {
  m_radian = Math::RadianNormalise(m_radian, _s, _e);
}

Base::txString txRadian::Str() const TX_NOEXCEPT { return (boost::format("%1% (Radian)") % m_radian).str(); }

Base::txFloat txRadian::Cos() const TX_NOEXCEPT {
#  if USE_TEST
  const Base::txFloat normalRadian = Math::RadianNormalise(m_radian);
  txAssert((normalRadian * RadianSampleScalarFactor) < RadianSampleCnt && (normalRadian) >= 0);
  const auto retV = s_array_radian_lookup_table_cos[normalRadian * RadianSampleScalarFactor];
  if (Math::isEqual(std::cos(m_radian), retV)) {
    return retV;
  } else {
    LOG_IF(WARNING, USE_TEST) << TXST_TRACE_VARIABLES(m_radian)
                              << TXST_TRACE_VARIABLES((int)(normalRadian * RadianSampleScalarFactor))
                              << TXST_TRACE_VARIABLES(std::cos(m_radian)) << TXST_TRACE_VARIABLES(retV);
    txPAUSE;
    return std::cos(m_radian);
  }
#  else  /*USE_TEST*/
  return s_array_radian_lookup_table_cos[Math::RadianNormalise(m_radian) * RadianSampleScalarFactor];
#  endif /*USE_TEST*/
         /*return std::cos(m_radian);*/
}

Base::txFloat txRadian::Sin() const TX_NOEXCEPT {
#  if USE_TEST
  const Base::txFloat normalRadian = Math::RadianNormalise(m_radian);
  txAssert((normalRadian * RadianSampleScalarFactor) < RadianSampleCnt && (normalRadian) >= 0);
  const auto retV = s_array_radian_lookup_table_sin[normalRadian * RadianSampleScalarFactor];
  if (Math::isEqual(std::sin(m_radian), retV)) {
    return retV;
  } else {
    LOG_IF(WARNING, USE_TEST) << TXST_TRACE_VARIABLES(m_radian)
                              << TXST_TRACE_VARIABLES((int)(normalRadian * RadianSampleScalarFactor))
                              << TXST_TRACE_VARIABLES(std::sin(m_radian)) << TXST_TRACE_VARIABLES(retV);
    txPAUSE;
    return std::sin(m_radian);
  }
  /*return std::sin(m_radian);*/
#  else  /*USE_TEST*/
  return s_array_radian_lookup_table_sin[Math::RadianNormalise(m_radian) * RadianSampleScalarFactor];
#  endif /*USE_TEST*/
}

txRadian txRadian::MakeRadian(const Base::txFloat _r) TX_NOEXCEPT {
  txRadian retV;
  retV.FromRadian(_r);
  return retV;
}
#endif  // __TX_Mark__("txRadian")

#if __TX_Mark__("txDegree")
const txDegree& txDegree::FromRadian(const Base::txFloat _r) TX_NOEXCEPT {
  m_degree = Math::Radians2Degrees(Math::RadianNormalise(_r));
  return *this;
}

void txDegree::Normalize(const Base::txFloat _s, const Base::txFloat _e) TX_NOEXCEPT {
  m_degree = Math::DegreeNormalise(m_degree, _s, _e);
}

Base::txFloat txDegree::GetRadian() const TX_NOEXCEPT { return Math::Degrees2Radians(m_degree); }

txRadian txDegree::ToRadian() const TX_NOEXCEPT {
  txRadian radian;
  radian.FromDegree(m_degree);
  return radian;
}

Base::txString txDegree::Str() const TX_NOEXCEPT { return (boost::format("%1% (Degree)") % m_degree).str(); }

Base::txFloat txDegree::Cos() const TX_NOEXCEPT {
#  if USE_TEST
  txAssert((m_degree * DegreeSampleScalarFactor) < DegreeSampleCnt && (m_degree * DegreeSampleScalarFactor) >= 0);
  const auto retV = s_array_degree_lookup_table_cos[m_degree * DegreeSampleScalarFactor];
  if (Math::isEqual(std::cos(GetRadian()), retV)) {
    return retV;
  } else {
    LOG_IF(WARNING, USE_TEST) << TXST_TRACE_VARIABLES(m_degree)
                              << TXST_TRACE_VARIABLES((int)(m_degree * RadianSampleScalarFactor))
                              << TXST_TRACE_VARIABLES(std::cos(GetRadian())) << TXST_TRACE_VARIABLES(retV);
    txPAUSE;
    return std::cos(GetRadian());
  }
#  else  /*USE_TEST*/
  return s_array_degree_lookup_table_cos[m_degree * DegreeSampleScalarFactor];
#  endif /*USE_TEST*/
}

Base::txFloat txDegree::Sin() const TX_NOEXCEPT {
#  if USE_TEST
  txAssert((m_degree * DegreeSampleScalarFactor) < DegreeSampleCnt && (m_degree * DegreeSampleScalarFactor) >= 0);
  const auto retV = s_array_degree_lookup_table_sin[m_degree * DegreeSampleScalarFactor];
  if (Math::isEqual(std::sin(GetRadian()), retV)) {
    return retV;
  } else {
    LOG_IF(WARNING, USE_TEST) << TXST_TRACE_VARIABLES(m_degree)
                              << TXST_TRACE_VARIABLES((int)(m_degree * RadianSampleScalarFactor))
                              << TXST_TRACE_VARIABLES(std::sin(GetRadian())) << TXST_TRACE_VARIABLES(retV);
    txPAUSE;
    return std::sin(GetRadian());
  }
#  else  /*USE_TEST*/
  // LOG(WARNING) << TX_VARS_NAME(Sin_index, m_degree * DegreeSampleScalarFactor);
  return s_array_degree_lookup_table_sin[m_degree * DegreeSampleScalarFactor];
#  endif /*USE_TEST*/
}

txDegree txDegree::MakeDegree(const Base::txFloat _d) TX_NOEXCEPT {
  txDegree retV;
  retV.FromDegree(_d);
  return retV;
}

Base::txBool operator<(const txDegree& lhs, const txDegree& rhs) TX_NOEXCEPT {
  return lhs.GetDegree() < rhs.GetDegree();
}

Base::txBool operator<=(const txDegree& lhs, const txDegree& rhs) TX_NOEXCEPT {
  return lhs.GetDegree() <= rhs.GetDegree();
}

Base::txBool operator>(const txDegree& lhs, const txDegree& rhs) TX_NOEXCEPT {
  return lhs.GetDegree() > rhs.GetDegree();
}

Base::txBool operator>=(const txDegree& lhs, const txDegree& rhs) TX_NOEXCEPT {
  return lhs.GetDegree() >= rhs.GetDegree();
}

txDegree operator-(const txDegree& lhs, const txDegree& rhs) TX_NOEXCEPT {
  return txDegree::MakeDegree(lhs.GetDegree() - rhs.GetDegree());
}

txDegree DegreeBetweenVectors(const Base::txVec3& Vector1, const Base::txVec3& Vector2) TX_NOEXCEPT {
  return txDegree::MakeDegree(Math::VectorDegree3D(Vector1, Vector2));
}
#endif  // __TX_Mark__("txDegree")

TX_NAMESPACE_CLOSE(Unit)

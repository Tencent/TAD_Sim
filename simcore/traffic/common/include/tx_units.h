// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_algebra_defs.h"
#include "tx_marco.h"
#include "tx_math.h"
#include "tx_serialization.h"
TX_NAMESPACE_OPEN(Unit)
class txRadian;
class txDegree;

// @brief 角的基类
class IAngle {
 public:
  enum class AngleType { Radian, Degree };

 public:
  virtual ~IAngle() TX_DEFAULT;
  virtual AngleType Type() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取弧度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetRadian() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取角度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetDegree() const TX_NOEXCEPT = 0;

  /**
   * @brief 从弧度初始化
   *
   * @param _r
   * @return const IAngle&
   */
  virtual const IAngle& FromRadian(const Base::txFloat _r) TX_NOEXCEPT = 0;

  /**
   * @brief 从角度初始化
   *
   * @param _d
   * @return const IAngle&
   */
  virtual const IAngle& FromDegree(const Base::txFloat _d) TX_NOEXCEPT = 0;
  virtual void Normalize(const Base::txFloat, const Base::txFloat) TX_NOEXCEPT = 0;

  /**
   * @brief 返回对象格式化字符串
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT = 0;

  /**
   * @brief 计算cos值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat Cos() const TX_NOEXCEPT = 0;

  /**
   * @brief 计算sin值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat Sin() const TX_NOEXCEPT = 0;
  static Base::txBool GenerateSinCosLookupTable() TX_NOEXCEPT;

 protected:
  enum {
    RadianSampleCnt = 6284,
    RadianSampleScalarFactor = 1000,
    DegreeSampleCnt = 7201,
    DegreeSampleScalarFactor = 20
  };
  static std::array<Base::txFloat, RadianSampleCnt> s_array_radian_lookup_table_cos;
  static std::array<Base::txFloat, RadianSampleCnt> s_array_radian_lookup_table_sin;
  static std::array<Base::txFloat, DegreeSampleCnt> s_array_degree_lookup_table_cos;
  static std::array<Base::txFloat, DegreeSampleCnt> s_array_degree_lookup_table_sin;
};

class txRadian : public IAngle {
 public:
  using self_type = txRadian;

 public:
  virtual ~txRadian() TX_DEFAULT;

  /**
   * @brief 获取角类型
   *
   * @return AngleType
   */
  virtual AngleType Type() const TX_NOEXCEPT TX_OVERRIDE { return AngleType::Radian; }

  /**
   * @brief 获取弧度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetRadian() const TX_NOEXCEPT TX_OVERRIDE { return m_radian; }

  /**
   * @brief 获取角度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetDegree() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 转成弧度
   *
   * @return txRadian
   */
  virtual txRadian ToRadian() const TX_NOEXCEPT { return *this; }

  /**
   * @brief 转成角度
   *
   * @return txDegree
   */
  virtual txDegree ToDegree() const TX_NOEXCEPT;

  /**
   * @brief 根据传入的弧度设置
   *
   * @param _r
   * @return const txRadian&
   */
  virtual const txRadian& FromRadian(const Base::txFloat _r) TX_NOEXCEPT TX_OVERRIDE {
    m_radian = Math::RadianNormalise(_r);
    return *this;
  }

  /**
   * @brief 根据传入的角度设置
   *
   * @param _d
   * @return const txRadian&
   */
  virtual const txRadian& FromDegree(const Base::txFloat _d) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Normalize(const Base::txFloat _s = 0.0, const Base::txFloat _e = Math::TWO_PI) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 输出对象字符串格式
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算cos值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat Cos() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算sin值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat Sin() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回一个txRadian对象
   *
   * @param _r 角大小
   * @return txRadian
   */
  static txRadian MakeRadian(const Base::txFloat _r) TX_NOEXCEPT;

  friend std::ostream& operator<<(std::ostream& os, txRadian const& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << v.GetRadian() << "(Radian)";
    return os;
  }

 protected:
  Base::txFloat m_radian = 0.0;

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("Radian", m_radian));
  }
};

class txDegree : public IAngle {
 public:
  using self_type = txDegree;

 public:
  virtual ~txDegree() TX_DEFAULT;

  /**
   * @brief 获取角类型
   *
   * @return AngleType
   */
  virtual AngleType Type() const TX_NOEXCEPT TX_OVERRIDE { return AngleType::Degree; }

  /**
   * @brief 获取弧度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetRadian() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取角度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetDegree() const TX_NOEXCEPT TX_OVERRIDE { return m_degree; }

  /**
   * @brief 转成弧度
   *
   * @return txRadian
   */
  virtual txRadian ToRadian() const TX_NOEXCEPT;

  /**
   * @brief 转成角度
   *
   * @return txDegree
   */
  virtual txDegree ToDegree() const TX_NOEXCEPT { return *this; }

  /**
   * @brief 从弧度初始化
   *
   * @param _r
   * @return const txDegree&
   */
  virtual const txDegree& FromRadian(const Base::txFloat _r) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 从角度初始化
   *
   * @param _d
   * @return const txDegree&
   */
  virtual const txDegree& FromDegree(const Base::txFloat _d) TX_NOEXCEPT TX_OVERRIDE {
    m_degree = Math::DegreeNormalise(_d);
    return *this;
  }
  virtual void Normalize(const Base::txFloat _s = 0.0, const Base::txFloat _e = 360.0) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 对象格式化字符串
   *
   * @return Base::txString
   */
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief cos值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat Cos() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief sin值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat Sin() const TX_NOEXCEPT TX_OVERRIDE;

  static txDegree MakeDegree(const Base::txFloat _d) TX_NOEXCEPT;

  friend Base::txBool operator<(const self_type& lhs, const self_type& rhs) TX_NOEXCEPT;
  friend Base::txBool operator<=(const self_type& lhs, const self_type& rhs) TX_NOEXCEPT;
  friend Base::txBool operator>(const self_type& lhs, const self_type& rhs) TX_NOEXCEPT;
  friend Base::txBool operator>=(const self_type& lhs, const self_type& rhs) TX_NOEXCEPT;
  friend self_type operator-(const self_type& lhs, const self_type& rhs) TX_NOEXCEPT;
  friend self_type operator+(const self_type& lhs, const self_type& rhs) TX_NOEXCEPT {
    return txDegree::MakeDegree(lhs.GetDegree() + rhs.GetDegree());
  }

  friend self_type DegreeBetweenVectors(const Base::txVec3& Vector1, const Base::txVec3& Vector2) TX_NOEXCEPT;

  friend std::ostream& operator<<(std::ostream& os, txDegree const& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }

 protected:
  Base::txFloat m_degree = 0.0;

 public:
  template <class Archive>
  void serialize(Archive& ar) {
    ar(_MAKE_NVP_("Degree", m_degree));
  }
};

inline Base::txFloat ms_kmh(const Base::txFloat _ms) TX_NOEXCEPT { return _ms * 3.6; }

inline Base::txFloat kmh_ms(const Base::txFloat _kmh) TX_NOEXCEPT { return _kmh * 0.27777777778 /*1.0 / 3.6*/; }

class CircleRadian {
 public:
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;

 public:
  CircleRadian(const txFloat _l, const txFloat _h, const txFloat _z)
      : L(_l), H(_h), Z(_z), R(_h - _l), R_2((_h - _l) / 2.0) {
    m_isValid = (H > L) && (Z >= L) && (Z < H);
  }
  txBool IsValid() const TX_NOEXCEPT { return m_isValid; }
  static txFloat PI() TX_NOEXCEPT { return Math::PI; }
  static txFloat Half_PI() TX_NOEXCEPT { return Math::HALF_PI; }
  static txFloat QUARTER_PI() TX_NOEXCEPT { return Math::QUARTER_PI; }

  /**
   * @brief 是否在范围内
   *
   * @param r
   * @return txBool
   */
  txBool IsInRange(const txFloat r) const TX_NOEXCEPT { return (r >= L && r < H); }

  txFloat Wrap(const txFloat r) const TX_NOEXCEPT {
    if (CallFail(IsValid())) {
      return r;
    }
    // the next lines are for optimization and improved accuracy only
    if (r >= L) {
      if (r < H) {
        return r;
      } else if (r < (H + R)) {
        return r - R;
      }
    } else {
      if (r >= (L - R)) {
        return r + R;
      }
    }
    // general case
    return Mod(r - L, R) + L;
  }

  txFloat Mod(const txFloat x, const txFloat y) const TX_NOEXCEPT {
    if (0 == y) {
      return x;
    }

    const txFloat m = x - y * floor(x / y);

    // handle boundary cases resulting from floating-point limited accuracy:

    if (y > 0) {   // modulo range: [0..y)
      if (m >= y) {  // Mod(-1e-16             , 360.    ): m= 360.
        return 0;
      }

      if (m < 0) {
        if (y + m == y) {
          return 0;  // just in case...
        } else {
          return y + m;  // Mod(106.81415022205296 , _TWO_PI ): m= -1.421e-14
        }
      }
    } else {  // modulo range: (y..0]
      if (m <= y) {  // Mod(1e-16              , -360.   ): m= -360.
        return 0;
      }

      if (m > 0) {
        if (y + m == y) {
          return 0;  // just in case...
        } else {
          return y + m;  // Mod(-106.81415022205296, -_TWO_PI): m= 1.421e-14
        }
      }
    }
    return m;
  }

 public:
  const txFloat L, H, Z, R, R_2;

 protected:
  Base::txBool m_isValid = false;
};

TX_NAMESPACE_CLOSE(Unit)

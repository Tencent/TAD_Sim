// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <algorithm>
#include "manager/map_data_type.h"
#include "utils/eval_common.h"
#include "utils/eval_math.h"
#include "utils/eval_shape.h"
#include "utils/eval_unit.h"

namespace eval {
// riseup, falling detection
template <class T>
class CompareDetection {
 public:
  static inline bool IsAbove(const T &t, const T &thresh) { return t > thresh; }
  static inline bool IsBelow(const T &t, const T &thresh) { return t < thresh; }
  static inline bool IsEqual(const T &t, const T &thresh) { return std::abs(t - thresh) <= const_Zero_Threshold; }
  static inline bool GreaterOrEqual(const T &t, const T &thresh) { return IsAbove(t, thresh) || IsEqual(t, thresh); }
  static inline bool EqualOrBelow(const T &t, const T &thresh) { return IsBelow(t, thresh) || IsEqual(t, thresh); }

 public:
  CompareDetection() = delete;
  explicit CompareDetection(const T &init_val) : _last_val(init_val), _count(0), _latest_state(false) {}
  virtual ~CompareDetection() {}

  virtual bool Detect(const T &t, const T &thresh) { throw("CompareDetection<T>::Detect not implemented."); }
  virtual inline size_t GetCount() { return _count; }
  virtual inline bool GetLatestState() const { return _latest_state; }
  virtual inline void Reset(const T &init_val) {
    _last_val = init_val;
    _latest_state = false;
    _count = 0;
  }

 protected:
  T _last_val;
  size_t _count;
  bool _latest_state;
};

/**
 * @brief use last val and current_val to detect rising edge, and count rising times.
 */
template <class T>
class RiseUpDetection : public CompareDetection<T> {
 public:
  explicit RiseUpDetection(const T &init_val) : CompareDetection<T>(init_val), m_edgeState(false) {}
  virtual ~RiseUpDetection() {}

  virtual bool Detect(const T &t, const T &thresh) {
    // edge
    bool edge = (!this->_latest_state && t >= thresh);
    m_edgeState = edge;
    if (edge) this->_count++;
    this->_last_val = t;

    // state
    this->_latest_state = (t >= thresh);

    return edge;
  }
  virtual bool GetLatestEdgeState() const { return m_edgeState; }

 private:
  bool m_edgeState;
};

/**
 * @brief use last val and current_val to detect falling edge, and count falling times.
 */
template <class T>
class FallingDetection : public CompareDetection<T> {
 public:
  explicit FallingDetection(const T &init_val) : CompareDetection<T>(init_val), m_edgeState(false) {}
  virtual ~FallingDetection() {}

  virtual bool Detect(const T &t, const T &thresh) {
    // edge
    bool edge = (!this->_latest_state && t <= thresh);
    m_edgeState = edge;
    if (edge) this->_count++;
    this->_last_val = t;

    // state
    this->_latest_state = (t <= thresh);

    return edge;
  }
  virtual bool GetLatestEdgeState() const { return m_edgeState; }

 private:
  bool m_edgeState;
};

// counter
class EvalCounter : public NoneCopyable {
 public:
  explicit EvalCounter(int init_counter = 0) : m_counter(init_counter) {}
  virtual ~EvalCounter() {}

  inline void UpdateCounterBy(int step = 1) { m_counter += step; }
  inline void ResetCounter(int reset_counter = 0) { m_counter = reset_counter; }
  inline int GetCounter() { return m_counter; }

 protected:
  int m_counter;
};

// trend check
template <class Ty>
class TrendCheck {
 public:
  explicit TrendCheck(const Ty &init_val) { Reset(init_val); }
  int64_t GetDisOrderCounter() { return m_disorder_counter; }
  int64_t GetOrderCounter() { return m_order_counter; }
  void Reset(const Ty &init_val) {
    m_pre_value = init_val;
    m_disorder_counter = 0;
    m_order_counter = 0;
  }

 protected:
  Ty m_pre_value;
  uint64_t m_disorder_counter = 0;
  uint64_t m_order_counter = 0;
};
// check value is in rising trend
template <class Ty>
class RisingCheck : public TrendCheck<Ty> {
 public:
  explicit RisingCheck(const Ty &init_val) : TrendCheck<Ty>(init_val) {}
  bool IsRising(const Ty &val) {
    bool ret = val >= this->m_pre_value;
    this->m_disorder_counter += !ret;
    this->m_order_counter += ret;
    this->m_pre_value = val;
    return ret;
  }
};
// check value is in falling trend
template <class Ty>
class FallingCheck : public TrendCheck<Ty> {
 public:
  explicit FallingCheck(const Ty &init_val) : TrendCheck<Ty>(init_val) {}
  bool IsFalling(const Ty &val) {
    bool ret = val <= this->m_pre_value;
    this->m_disorder_counter += !ret;
    this->m_order_counter += ret;
    this->m_pre_value = val;
    return ret;
  }
};

template <class T>
RiseUpDetection<T> MakeRiseUpDetection(T init_val) {
  return RiseUpDetection<T>(init_val);
}
template <class T>
FallingDetection<T> MakeFallingDetection(T init_val) {
  return FallingDetection<T>(init_val);
}

/**
 * @brief collision detection
 */
class CollisionDetection {
 public:
  /**
   * @brief calculate the bottom-left and top-right corners of a rectangle
   */
  static void CalAABB(const RectCorners &cornres, CPosition &bl, CPosition &tr);
  /**
   * @brief calculate the center point of a rectangle
   */
  static void CalCenterPt(const RectCorners &corners, CPosition &center);
  /**
   * @brief check if a point is inside a rectangle by bottom-left and top-right corners points
   */
  static bool IsInsideAABB(const CPosition &pt, const CPosition &bl, const CPosition &tr);
  /**
   * @brief check if a point is inside a rectangle by using point-in-polygon algorithm
   */
  static bool IsInsideRect(const CPosition &pt, const RectCorners &corners_enu);
  /**
   * @brief check if is there any intersection between two rectangles by using IsInsideAABB and IsInsideRect.
   */
  static bool IsCollision(CStaticActor *actor1, CStaticActor *actor2);
};
}  // namespace eval

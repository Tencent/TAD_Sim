// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_signal_phase_period.h"
#include <algorithm>
#include <functional>
#include <iomanip>
#include "tx_enum_utils.h"
#include "tx_math.h"

TX_NAMESPACE_OPEN(Scene)
// eGreen = 0, eYellow = 1, eRed = 2, eGrey = 3, eBlack
// 初始化信号灯相位周期
Base::txBool TAD_SignalPhasePeriod::Initialize(InitParam_t const& initParam) TX_NOEXCEPT {
  // 设置信号灯相位周期的开始时间
  _start_t = initParam._start_t;
  // 设置信号灯相位周期的绿灯时间
  _period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eGreen)) /*(+SIGN_LIGHT_COLOR_TYPE::eGreen)._to_index()*/] =
      initParam._period[__enum2idx__(
          _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen)) /*(+SIGN_LIGHT_COLOR_TYPE::eGreen)._to_index()*/];
  // 设置信号灯相位周期的黄灯时间
  _period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eYellow)) /*(+SIGN_LIGHT_COLOR_TYPE::eYellow)._to_index()*/] =
      initParam._period[__enum2idx__(
          _plus_(SIGN_LIGHT_COLOR_TYPE::eYellow)) /*(+SIGN_LIGHT_COLOR_TYPE::eYellow)._to_index()*/];
  // 设置信号灯相位周期的红灯时间
  _period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eRed)) /*(+SIGN_LIGHT_COLOR_TYPE::eRed)._to_index()*/] =
      initParam
          ._period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eRed)) /*(+SIGN_LIGHT_COLOR_TYPE::eRed)._to_index()*/];
  _current_Phase[0] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
  _current_Phase[1] = _plus_(SIGN_LIGHT_COLOR_TYPE::eYellow);

  // 计算信号灯相位周期的总时间
  _totalPeriod =
      _period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eGreen)) /*(+SIGN_LIGHT_COLOR_TYPE::eGreen)._to_index()*/] +
      _period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eYellow)) /*(+SIGN_LIGHT_COLOR_TYPE::eYellow)._to_index()*/] +
      _period[__enum2idx__(_plus_(SIGN_LIGHT_COLOR_TYPE::eRed)) /*(+SIGN_LIGHT_COLOR_TYPE::eRed)._to_index()*/];

  // 如果信号灯相位周期的总时间为0，则将当前信号灯相位设置为灰色，并返回true
  if (Math::isZero(_totalPeriod)) {
    _current_Phase[0] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGrey);
    _current_Phase[1] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGrey);
    return true;
  }

  // 初始化信号灯相位周期的颜色区间
  Base::txInt _current_Phase_index = __enum2idx__(_current_Phase[0]);
  Base::txFloat leftTimeClosed = 0.0;
  Base::txFloat rightTimeOpen = 0.0;
  while (_current_Phase_index < 3) {
    rightTimeOpen = leftTimeClosed + _period[_current_Phase_index];
    if (leftTimeClosed < rightTimeOpen) {
      ColorInterval_t curInterval(leftTimeClosed, rightTimeOpen,
                                  (__idx2enum__(SIGN_LIGHT_COLOR_TYPE, _current_Phase_index)));
      _colorIntervalVec.emplace_back(curInterval);

      leftTimeClosed = rightTimeOpen;
    }
    _current_Phase_index++;
  }

  // 如果信号灯相位周期的颜色区间非空，则返回true，否则返回false
  if (_colorIntervalVec.size() > 0) {
    return true;
  } else {
    LOG(WARNING) << " time interval create failure. set default green light.";
    _current_Phase[0] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    _current_Phase[1] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    return false;
  }
}

Base::txString TAD_SignalPhasePeriod::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  // 遍历信号灯相位周期的颜色区间
  for (const auto interval : _colorIntervalVec) {
    oss << "[" << (interval.leftTimeClosed) << ", " << interval.rightTimeOpen << ", " << interval.color << "],";
  }
  return oss.str();
}

Base::txBool TAD_SignalPhasePeriod::ComputeColor(const Base::txFloat _passTime) TX_NOEXCEPT {
  // 如果当前信号灯相位为灰色，则将当前信号灯相位设置为灰色，并返回true
  if (_plus_(SIGN_LIGHT_COLOR_TYPE::eGrey) == _current_Phase[0]) {
    _current_Phase[1] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGrey);
    _age[0] = 100;
    _age[1] = 100;
    return true;
  }

  // 计算经过的时间
  const Base::txFloat passTime_start_t = _passTime + _start_t;

  // 如果信号灯相位周期的颜色区间为空，则将当前信号灯相位设置为绿色，并返回true
  if (0 == _colorIntervalVec.size()) {
    _current_Phase[0] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    _current_Phase[1] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    _age[0] = 100;
    _age[1] = 100;
    return true;
  }

  // 如果信号灯相位周期的总时间为0，则将当前信号灯相位设置为绿色，并返回true
  if (Math::isZero(_totalPeriod)) {
    _current_Phase[0] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    _current_Phase[1] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    _age[0] = 100;
    _age[1] = 100;
    return true;
  }
  txAssert(_colorIntervalVec.size() > 0);
  // 查找经过的时间所在的颜色区间
  auto resItr = std::find_if(_colorIntervalVec.begin(), _colorIntervalVec.end(),
                             std::bind(ColorInterval_t::Comparision, std::placeholders::_1, passTime_start_t));

  Base::txBool bValid = true;
  // 如果找不到经过的时间所在的颜色区间，则尝试扩展信号灯相位周期的颜色区间
  while (bValid && _colorIntervalVec.end() == resItr) {
    bValid = false;
    const auto& refLastInterval = _colorIntervalVec[_colorIntervalVec.size() - 1];
    Base::txFloat leftTimeClosed = refLastInterval.rightTimeOpen;
    Base::txFloat rightTimeOpen = 0.0;
    Base::txInt _current_Phase_index = __enum2idx__(refLastInterval.color);

    // 遍历信号灯相位周期的颜色区间，尝试扩展颜色区间
    for (Base::txInt i = 1; i <= 3; ++i) {
      _current_Phase_index++;
      if (_current_Phase_index >= 3) {
        _current_Phase_index = 0;
      }

      rightTimeOpen = leftTimeClosed + _period[_current_Phase_index];
      if (leftTimeClosed < rightTimeOpen) {
        ColorInterval_t curInterval(leftTimeClosed, rightTimeOpen,
                                    (__idx2enum__(SIGN_LIGHT_COLOR_TYPE, _current_Phase_index)));
        _colorIntervalVec.emplace_back(curInterval);
        leftTimeClosed = rightTimeOpen;
        bValid = true;
      }
    }

    // 重新查找经过的时间所在的颜色区间
    resItr = std::find_if(_colorIntervalVec.begin(), _colorIntervalVec.end(),
                          std::bind(ColorInterval_t::Comparision, std::placeholders::_1, passTime_start_t));
  }

  // 如果有效标志为true，则更新当前信号灯相位和相位周期
  if (bValid) {
    _current_Phase[0] = (*resItr).color;
    Base::txInt _next_Phase_index = __enum2idx__((*resItr).color) + 1 /*next*/;
    if (_next_Phase_index >= 3) {
      _next_Phase_index = 0;
    }
    _current_Phase[1] = (__idx2enum__(SIGN_LIGHT_COLOR_TYPE, _next_Phase_index));
    _age[0] = round((*resItr).rightTimeOpen - passTime_start_t);
    _age[1] = _period[_next_Phase_index];

    // 删除已经过去的颜色区间
    _colorIntervalVec.erase(_colorIntervalVec.begin(), resItr);
    txAssert(_colorIntervalVec.size() > 0);
  } else {
    // 如果无效，则将当前信号灯相位设置为绿色，并设置相位周期为100
    _current_Phase[0] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    _current_Phase[1] = _plus_(SIGN_LIGHT_COLOR_TYPE::eGreen);
    _age[0] = 100;
    _age[1] = 100;
  }
  return true;
}

Base::txBool TAD_SignalPhasePeriod::Update(Base::txFloat timeStamp) TX_NOEXCEPT {
  txMsg(TAD_SignalPhasePeriod::Update);
  return true;
}

Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE TAD_SignalPhasePeriod::GetPhase() const TX_NOEXCEPT {
  return _current_Phase[0];
}

Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE TAD_SignalPhasePeriod::GetNextPhase() const TX_NOEXCEPT {
  return _current_Phase[1];
}

TX_NAMESPACE_CLOSE(Scene)

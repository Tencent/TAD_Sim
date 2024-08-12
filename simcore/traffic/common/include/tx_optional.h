// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_serialization.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(Base)

template <typename T>
class txOptional {
 public:
  using FlagType = txBool;
  using ValueType = T;
  using selfType = txOptional<ValueType>;
  txOptional() {
    // : m_op_value(std::make_tuple(false, ValueType()))
    clear_value();
  }

  /**
   * @brief 判断当前可选值是否有效
   *
   * @return txBool 如果有效，则返回 true；否则，返回 false
   *
   * 此函数用于判断当前的 txOptional 对象是否包含有效的值。
   * 如果有效，则返回 true；否则，返回 false。
   */
  txBool has_value() const TX_NOEXCEPT { return std::get<0>(m_op_value); }

  /**
   * @brief 清除可选值
   *
   * 此函数用于清除当前的 txOptional 对象中的值。
   * 使用该函数可以将对象设置回其初始状态。
   */
  void clear_value() TX_NOEXCEPT { std::get<0>(m_op_value) = false; }

  /**
   * @brief 返回可选值的引用
   *
   * 该操作符用于获取当前 txOptional 对象中存储的值的引用。
   * 返回值为该对象存储的值的引用，使用此操作符后，可对该值进行修改。
   *
   * @return ValueType& 当前 txOptional 对象中存储的值的引用
   */
  ValueType& operator*() TX_NOEXCEPT { return std::get<1>(m_op_value); }

  /**
   * @brief 返回可选值的引用
   *
   * 该操作符用于获取当前 txOptional 对象中存储的值的引用。
   * 返回值为该对象存储的值的引用，使用此操作符后，可对该值进行修改。
   *
   * @return ValueType& 当前 txOptional 对象中存储的值的引用
   */
  const ValueType& operator*() const TX_NOEXCEPT { return std::get<1>(m_op_value); }

  /**
   * @brief 对 txOptional 对象赋值
   *
   * 该操作符用于将指定值赋给 txOptional 对象。
   *
   * @param _val 需要赋值的值
   * @return 返回 txOptional 对象的引用，方便进行链式操作
   */
  selfType& operator=(const ValueType _val) TX_NOEXCEPT {
    std::get<0>(m_op_value) = true;
    std::get<1>(m_op_value) = _val;
    return (*this);
  }

 protected:
  std::tuple<Base::txBool, ValueType> m_op_value;

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("txOptional");
    archive(m_op_value);
  }
};

TX_NAMESPACE_CLOSE(Base)

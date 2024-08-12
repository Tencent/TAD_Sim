#pragma once

#include <stdint.h>
#include <stdexcept>

#include "zmq_addon.hpp"

#define TXSIM_DOUBLE_PRECISION 10000000

namespace tx_sim {
namespace utils {

/// Utility function to encode a ULEB128 value to the buffer. Returns the length in bytes of the encoded value.
inline size_t EncodeULEB128(uint64_t value, uint8_t* buffer, size_t max_len = 10) {
  uint8_t* p = buffer;
  size_t count = 0;
  do {
    uint8_t value_byte = value & 0x7f;
    value >>= 7;
    if (value != 0) value_byte |= 0x80;  // mark this byte to show that more bytes will follow.
    *p++ = value_byte;
    ++count;
  } while (value != 0 && count < max_len);
  if (value != 0) throw std::overflow_error("uleb128 encoding exceed buffer max length: " + max_len);
  return count;
}

/// Utility function to encode a SLEB128 value to the buffer. Returns the length in bytes of the encoded value.
inline size_t EncodeSLEB128(int64_t value, uint8_t* buffer, size_t max_len = 10) {
  uint8_t* p = buffer;
  bool more = false;
  size_t count = 0;
  do {
    uint8_t value_byte = value & 0x7f;
    // NOTE: this assumes that this signed shift is an arithmetic right shift.
    value >>= 7;
    more = !(((value == 0) && ((value_byte & 0x40) == 0)) || ((value == -1) && ((value_byte & 0x40) != 0)));
    if (more) value_byte |= 0x80;  // mark this byte to show that more bytes will follow.
    *p++ = value_byte;
    ++count;
  } while (more && count < max_len);
  if (more) throw std::overflow_error("sleb128 encoding exceed buffer max length: " + max_len);
  return count;
}

/// Utility function to decode a ULEB128 value into a uint64_t.
inline uint64_t DecodeULEB128(const uint8_t* buffer, size_t len) {
  const uint8_t* p = buffer;
  uint32_t shift = 0;
  uint64_t result = 0;
  uint8_t value_byte;
  size_t count = 0;
  do {
    value_byte = *p++;
    result |= (static_cast<uint64_t>(value_byte & 0x7f) << shift);
    shift += 7;
    ++count;
  } while ((value_byte & 0x80) != 0 && count < len);
  if ((value_byte & 0x80) != 0) throw std::runtime_error("uleb128 decoding error: no stop bit in buffer.");
  return result;
}

/// Utility function to decode a SLEB128 value into a int64_t.
inline int64_t DecodeSLEB128(const uint8_t* buffer, size_t len) {
  const uint8_t* p = buffer;
  uint32_t shift = 0;
  int64_t result = 0;
  uint8_t value_byte;
  size_t count = 0;
  do {
    value_byte = *p++;
    result |= (static_cast<uint64_t>(value_byte & 0x7f) << shift);
    shift += 7;
    ++count;
  } while ((value_byte & 0x80) != 0 && count < len);
  if (shift < (sizeof(result) * 8) && (value_byte & 0x40) != 0) result |= -(((uint64_t)1) << shift);
  if ((value_byte & 0x80) != 0) throw std::runtime_error("sleb128 decoding error: no stop bit in buffer.");
  return result;
}

/******************************************************************************/
/**************************** helper functions ********************************/
/******************************************************************************/

inline zmq::message_t PopMsg(zmq::multipart_t& msg) {
  if (msg.size() > 0)
    return msg.pop();
  else
    throw std::runtime_error("module message decoding error: expecting more msgs.");
}

inline std::string PopMsgStr(zmq::multipart_t& msg) {
  if (msg.size() > 0)
    return msg.popstr();
  else
    throw std::runtime_error("module message decoding error: expecting more msgs.");
}

inline void AddMsgType(uint8_t* buffer, int16_t type, zmq::multipart_t& msg, bool to_front = false) {
  size_t len = EncodeSLEB128(type, buffer);
  if (to_front)
    msg.pushmem(buffer, len);
  else
    msg.addmem(buffer, len);
}

inline int16_t PopMsgType(zmq::multipart_t& msg) {
  zmq::message_t m = PopMsg(msg);
  return DecodeSLEB128(static_cast<const uint8_t*>(m.data()), m.size());
}

inline int16_t GetMsgType(const zmq::message_t& m) {
  return DecodeSLEB128(static_cast<const uint8_t*>(m.data()), m.size());
}

inline void AddMsgSize(uint8_t* buffer, size_t size, zmq::multipart_t& msg) {
  size_t len = EncodeULEB128(size, buffer);
  msg.addmem(buffer, len);
}

inline size_t PopMsgSize(zmq::multipart_t& msg) {
  zmq::message_t m = PopMsg(msg);
  return DecodeULEB128(static_cast<const uint8_t*>(m.data()), m.size());
}

inline void AddMsgUInt(uint8_t* buffer, uint64_t num, zmq::multipart_t& msg) { AddMsgSize(buffer, num, msg); }

inline uint64_t PopMsgUint(zmq::multipart_t& msg) { return PopMsgSize(msg); }

inline void AddMsgInt64(uint8_t* buffer, int64_t num, zmq::multipart_t& msg) {
  size_t len = EncodeSLEB128(num, buffer);
  msg.addmem(buffer, len);
}

inline int64_t PopMsgInt64(zmq::multipart_t& msg) {
  zmq::message_t m = PopMsg(msg);
  return DecodeSLEB128(static_cast<const uint8_t*>(m.data()), m.size());
}

inline void AddMsgDouble(uint8_t* buffer, double num, zmq::multipart_t& msg) {
  size_t len = EncodeSLEB128(num * TXSIM_DOUBLE_PRECISION, buffer);
  msg.addmem(buffer, len);
}

inline double PopMsgDouble(zmq::multipart_t& msg) {
  zmq::message_t m = PopMsg(msg);
  return static_cast<double>(DecodeSLEB128(static_cast<const uint8_t*>(m.data()), m.size())) / TXSIM_DOUBLE_PRECISION;
}

}  // namespace utils
}  // namespace tx_sim

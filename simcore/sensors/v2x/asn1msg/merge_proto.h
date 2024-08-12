/**
 * @file merge_proto.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */

#pragma once
#include <google/protobuf/message.h>

#define CASE_FIELD_TYPE(cpptype, method, valuetype)                   \
  case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: {        \
    const valuetype &value = src_reflection->Get##method(src, field); \
    dst_reflection->Set##method(dst, field, value);                   \
    break;                                                            \
  }

#define CASE_REPEATED_FIELD_TYPE(cpptype, method, valuetype)                     \
  case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: {                   \
    const valuetype &value = src_reflection->GetRepeated##method(src, field, j); \
    dst_reflection->Add##method(dst, field, value);                              \
    break;                                                                       \
  }

/// if not repeated: using dst
/// if repeated: using dst
///

template <typename T>
void MergeProto(const T &src, T *dst) {
  const google::protobuf::Descriptor *descriptor = dst->GetDescriptor();
  const google::protobuf::Reflection *src_reflection = src.GetReflection();
  const google::protobuf::Reflection *dst_reflection = dst->GetReflection();
  int field_count = descriptor->field_count();
  for (int i = 0; i < field_count; ++i) {
    const google::protobuf::FieldDescriptor *field = descriptor->field(i);
    const std::string field_name = field->name();
    if (!field->is_repeated()) {
      const bool src_has_field = src_reflection->HasField(src, field);
      const bool dst_has_field = dst_reflection->HasField(*dst, field);
      if (src_has_field && !dst_has_field) {
        switch (field->cpp_type()) {
          CASE_FIELD_TYPE(INT32, Int32, int);
          CASE_FIELD_TYPE(UINT32, UInt32, uint32_t);
          CASE_FIELD_TYPE(FLOAT, Float, float);
          CASE_FIELD_TYPE(DOUBLE, Double, double);
          CASE_FIELD_TYPE(BOOL, Bool, bool);
          CASE_FIELD_TYPE(INT64, Int64, int64_t);
          CASE_FIELD_TYPE(UINT64, UInt64, uint64_t);
          CASE_FIELD_TYPE(STRING, String, std::string);
          case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
            const int value = src_reflection->GetEnum(src, field)->number();
            dst_reflection->AddEnumValue(dst, field, value);
            break;
          }
          case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
            const google::protobuf::Message &src_message = src_reflection->GetMessage(src, field);
            dst_reflection->MutableMessage(dst, field)->CopyFrom(src_message);
            break;
          }
        }
      }
    } else {
      const int src_field_size = src_reflection->FieldSize(src, field);
      const int dst_field_size = dst_reflection->FieldSize(*dst, field);
      if (src_field_size > 0 && dst_field_size == 0) {
        for (int j = 0; j < src_field_size; ++j) {
          switch (field->cpp_type()) {
            CASE_REPEATED_FIELD_TYPE(INT32, Int32, int);
            CASE_REPEATED_FIELD_TYPE(UINT32, UInt32, uint32_t);
            CASE_REPEATED_FIELD_TYPE(FLOAT, Float, float);
            CASE_REPEATED_FIELD_TYPE(DOUBLE, Double, double);
            CASE_REPEATED_FIELD_TYPE(BOOL, Bool, bool);
            CASE_REPEATED_FIELD_TYPE(INT64, Int64, int64_t);
            CASE_REPEATED_FIELD_TYPE(UINT64, UInt64, uint64_t);
            CASE_REPEATED_FIELD_TYPE(STRING, String, std::string);
            case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
              const int value = src_reflection->GetRepeatedEnum(src, field, j)->number();
              dst_reflection->AddEnumValue(dst, field, value);
              break;
            }
            case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
              const google::protobuf::Message &src_message = src_reflection->GetRepeatedMessage(src, field, j);
              dst_reflection->AddMessage(dst, field)->CopyFrom(src_message);
              break;
            }
          }
        }
      }
    }
  }
}

#define CASE_FIELD_TYPE_FILLING(cpptype, method, value)        \
  case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: { \
    dst_reflection->Set##method(dst, field, value);            \
    break;                                                     \
  }

#define CASE_REPEATED_FIELD_TYPE_FILLING(cpptype, method, value) \
  case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: {   \
    dst_reflection->Add##method(dst, field, value);              \
    break;                                                       \
  }

template <typename T>
void FillupProto(T *dst) {
  const google::protobuf::Descriptor *descriptor = dst->GetDescriptor();
  const google::protobuf::Reflection *dst_reflection = dst->GetReflection();
  int field_count = descriptor->field_count();
  for (int i = 0; i < field_count; ++i) {
    const google::protobuf::FieldDescriptor *field = descriptor->field(i);
    const std::string field_name = field->name();
    if (!field->is_repeated()) {
      switch (field->cpp_type()) {
        CASE_FIELD_TYPE_FILLING(INT32, Int32, -123);
        CASE_FIELD_TYPE_FILLING(UINT32, UInt32, 123);
        CASE_FIELD_TYPE_FILLING(FLOAT, Float, 123.456f);
        CASE_FIELD_TYPE_FILLING(DOUBLE, Double, 123.456);
        CASE_FIELD_TYPE_FILLING(BOOL, Bool, true);
        CASE_FIELD_TYPE_FILLING(INT64, Int64, -987654321);
        CASE_FIELD_TYPE_FILLING(UINT64, UInt64, 987654321);
        CASE_FIELD_TYPE_FILLING(STRING, String, "ABCabc123");
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
          dst_reflection->AddEnumValue(dst, field, 1);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
          FillupProto(dst_reflection->MutableMessage(dst, field));
          break;
        }
      }
    } else {
      switch (field->cpp_type()) {
        CASE_REPEATED_FIELD_TYPE_FILLING(INT32, Int32, -123);
        CASE_REPEATED_FIELD_TYPE_FILLING(UINT32, UInt32, 123);
        CASE_REPEATED_FIELD_TYPE_FILLING(FLOAT, Float, 123.456f);
        CASE_REPEATED_FIELD_TYPE_FILLING(DOUBLE, Double, 123.456);
        CASE_REPEATED_FIELD_TYPE_FILLING(BOOL, Bool, true);
        CASE_REPEATED_FIELD_TYPE_FILLING(INT64, Int64, -987654321);
        CASE_REPEATED_FIELD_TYPE_FILLING(UINT64, UInt64, 987654321);
        CASE_REPEATED_FIELD_TYPE_FILLING(STRING, String, "ABCabc123");
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
          dst_reflection->AddEnumValue(dst, field, 1);
          break;
        }
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
          FillupProto(dst_reflection->AddMessage(dst, field));
          break;
        }
      }
    }
  }
}

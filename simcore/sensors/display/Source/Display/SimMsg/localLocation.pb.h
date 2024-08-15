// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: localLocation.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_localLocation_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_localLocation_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3009000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3009001 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include "header.pb.h"
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_localLocation_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_localLocation_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_localLocation_2eproto;
namespace sim_msg {
class LocalLocation;
class LocalLocationDefaultTypeInternal;
extern LocalLocationDefaultTypeInternal _LocalLocation_default_instance_;
}  // namespace sim_msg
PROTOBUF_NAMESPACE_OPEN
template<> ::sim_msg::LocalLocation* Arena::CreateMaybeMessage<::sim_msg::LocalLocation>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace sim_msg {

// ===================================================================

class LocalLocation :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sim_msg.LocalLocation) */ {
 public:
  LocalLocation();
  virtual ~LocalLocation();

  LocalLocation(const LocalLocation& from);
  LocalLocation(LocalLocation&& from) noexcept
    : LocalLocation() {
    *this = ::std::move(from);
  }

  inline LocalLocation& operator=(const LocalLocation& from) {
    CopyFrom(from);
    return *this;
  }
  inline LocalLocation& operator=(LocalLocation&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const LocalLocation& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const LocalLocation* internal_default_instance() {
    return reinterpret_cast<const LocalLocation*>(
               &_LocalLocation_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(LocalLocation& a, LocalLocation& b) {
    a.Swap(&b);
  }
  inline void Swap(LocalLocation* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline LocalLocation* New() const final {
    return CreateMaybeMessage<LocalLocation>(nullptr);
  }

  LocalLocation* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<LocalLocation>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const LocalLocation& from);
  void MergeFrom(const LocalLocation& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  #if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  #else
  bool MergePartialFromCodedStream(
      ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) final;
  #endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  void SerializeWithCachedSizes(
      ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(LocalLocation* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sim_msg.LocalLocation";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_localLocation_2eproto);
    return ::descriptor_table_localLocation_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kTransformFieldNumber = 11,
    kHeaderFieldNumber = 1,
    kXFieldNumber = 2,
    kYFieldNumber = 3,
    kVxFieldNumber = 4,
    kVyFieldNumber = 5,
    kYawFieldNumber = 6,
    kYawVFieldNumber = 7,
    kTimeThisLocalFrameFieldNumber = 8,
    kTimeLastLocalFrameFieldNumber = 9,
    kThisFrameIDFieldNumber = 10,
  };
  // repeated double transform = 11;
  int transform_size() const;
  void clear_transform();
  double transform(int index) const;
  void set_transform(int index, double value);
  void add_transform(double value);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >&
      transform() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >*
      mutable_transform();

  // .sim_msg.Header header = 1;
  bool has_header() const;
  void clear_header();
  const ::sim_msg::Header& header() const;
  ::sim_msg::Header* release_header();
  ::sim_msg::Header* mutable_header();
  void set_allocated_header(::sim_msg::Header* header);

  // double x = 2;
  void clear_x();
  double x() const;
  void set_x(double value);

  // double y = 3;
  void clear_y();
  double y() const;
  void set_y(double value);

  // double vx = 4;
  void clear_vx();
  double vx() const;
  void set_vx(double value);

  // double vy = 5;
  void clear_vy();
  double vy() const;
  void set_vy(double value);

  // double yaw = 6;
  void clear_yaw();
  double yaw() const;
  void set_yaw(double value);

  // double yaw_v = 7;
  void clear_yaw_v();
  double yaw_v() const;
  void set_yaw_v(double value);

  // double time_thisLocalFrame = 8;
  void clear_time_thislocalframe();
  double time_thislocalframe() const;
  void set_time_thislocalframe(double value);

  // double time_lastLocalFrame = 9;
  void clear_time_lastlocalframe();
  double time_lastlocalframe() const;
  void set_time_lastlocalframe(double value);

  // uint64 thisFrameID = 10;
  void clear_thisframeid();
  ::PROTOBUF_NAMESPACE_ID::uint64 thisframeid() const;
  void set_thisframeid(::PROTOBUF_NAMESPACE_ID::uint64 value);

  // @@protoc_insertion_point(class_scope:sim_msg.LocalLocation)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedField< double > transform_;
  mutable std::atomic<int> _transform_cached_byte_size_;
  ::sim_msg::Header* header_;
  double x_;
  double y_;
  double vx_;
  double vy_;
  double yaw_;
  double yaw_v_;
  double time_thislocalframe_;
  double time_lastlocalframe_;
  ::PROTOBUF_NAMESPACE_ID::uint64 thisframeid_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_localLocation_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// LocalLocation

// .sim_msg.Header header = 1;
inline bool LocalLocation::has_header() const {
  return this != internal_default_instance() && header_ != nullptr;
}
inline const ::sim_msg::Header& LocalLocation::header() const {
  const ::sim_msg::Header* p = header_;
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.header)
  return p != nullptr ? *p : *reinterpret_cast<const ::sim_msg::Header*>(
      &::sim_msg::_Header_default_instance_);
}
inline ::sim_msg::Header* LocalLocation::release_header() {
  // @@protoc_insertion_point(field_release:sim_msg.LocalLocation.header)
  
  ::sim_msg::Header* temp = header_;
  header_ = nullptr;
  return temp;
}
inline ::sim_msg::Header* LocalLocation::mutable_header() {
  
  if (header_ == nullptr) {
    auto* p = CreateMaybeMessage<::sim_msg::Header>(GetArenaNoVirtual());
    header_ = p;
  }
  // @@protoc_insertion_point(field_mutable:sim_msg.LocalLocation.header)
  return header_;
}
inline void LocalLocation::set_allocated_header(::sim_msg::Header* header) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaNoVirtual();
  if (message_arena == nullptr) {
    delete reinterpret_cast< ::PROTOBUF_NAMESPACE_ID::MessageLite*>(header_);
  }
  if (header) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena = nullptr;
    if (message_arena != submessage_arena) {
      header = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, header, submessage_arena);
    }
    
  } else {
    
  }
  header_ = header;
  // @@protoc_insertion_point(field_set_allocated:sim_msg.LocalLocation.header)
}

// double x = 2;
inline void LocalLocation::clear_x() {
  x_ = 0;
}
inline double LocalLocation::x() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.x)
  return x_;
}
inline void LocalLocation::set_x(double value) {
  
  x_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.x)
}

// double y = 3;
inline void LocalLocation::clear_y() {
  y_ = 0;
}
inline double LocalLocation::y() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.y)
  return y_;
}
inline void LocalLocation::set_y(double value) {
  
  y_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.y)
}

// double vx = 4;
inline void LocalLocation::clear_vx() {
  vx_ = 0;
}
inline double LocalLocation::vx() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.vx)
  return vx_;
}
inline void LocalLocation::set_vx(double value) {
  
  vx_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.vx)
}

// double vy = 5;
inline void LocalLocation::clear_vy() {
  vy_ = 0;
}
inline double LocalLocation::vy() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.vy)
  return vy_;
}
inline void LocalLocation::set_vy(double value) {
  
  vy_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.vy)
}

// double yaw = 6;
inline void LocalLocation::clear_yaw() {
  yaw_ = 0;
}
inline double LocalLocation::yaw() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.yaw)
  return yaw_;
}
inline void LocalLocation::set_yaw(double value) {
  
  yaw_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.yaw)
}

// double yaw_v = 7;
inline void LocalLocation::clear_yaw_v() {
  yaw_v_ = 0;
}
inline double LocalLocation::yaw_v() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.yaw_v)
  return yaw_v_;
}
inline void LocalLocation::set_yaw_v(double value) {
  
  yaw_v_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.yaw_v)
}

// double time_thisLocalFrame = 8;
inline void LocalLocation::clear_time_thislocalframe() {
  time_thislocalframe_ = 0;
}
inline double LocalLocation::time_thislocalframe() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.time_thisLocalFrame)
  return time_thislocalframe_;
}
inline void LocalLocation::set_time_thislocalframe(double value) {
  
  time_thislocalframe_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.time_thisLocalFrame)
}

// double time_lastLocalFrame = 9;
inline void LocalLocation::clear_time_lastlocalframe() {
  time_lastlocalframe_ = 0;
}
inline double LocalLocation::time_lastlocalframe() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.time_lastLocalFrame)
  return time_lastlocalframe_;
}
inline void LocalLocation::set_time_lastlocalframe(double value) {
  
  time_lastlocalframe_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.time_lastLocalFrame)
}

// uint64 thisFrameID = 10;
inline void LocalLocation::clear_thisframeid() {
  thisframeid_ = PROTOBUF_ULONGLONG(0);
}
inline ::PROTOBUF_NAMESPACE_ID::uint64 LocalLocation::thisframeid() const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.thisFrameID)
  return thisframeid_;
}
inline void LocalLocation::set_thisframeid(::PROTOBUF_NAMESPACE_ID::uint64 value) {
  
  thisframeid_ = value;
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.thisFrameID)
}

// repeated double transform = 11;
inline int LocalLocation::transform_size() const {
  return transform_.size();
}
inline void LocalLocation::clear_transform() {
  transform_.Clear();
}
inline double LocalLocation::transform(int index) const {
  // @@protoc_insertion_point(field_get:sim_msg.LocalLocation.transform)
  return transform_.Get(index);
}
inline void LocalLocation::set_transform(int index, double value) {
  transform_.Set(index, value);
  // @@protoc_insertion_point(field_set:sim_msg.LocalLocation.transform)
}
inline void LocalLocation::add_transform(double value) {
  transform_.Add(value);
  // @@protoc_insertion_point(field_add:sim_msg.LocalLocation.transform)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >&
LocalLocation::transform() const {
  // @@protoc_insertion_point(field_list:sim_msg.LocalLocation.transform)
  return transform_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >*
LocalLocation::mutable_transform() {
  // @@protoc_insertion_point(field_mutable_list:sim_msg.LocalLocation.transform)
  return &transform_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace sim_msg

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_localLocation_2eproto
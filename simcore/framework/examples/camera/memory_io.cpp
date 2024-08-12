#include "memory_io.h"

#define BOOST_DISABLE_ABI_HEADERS
#include "boost/interprocess/allocators/allocator.hpp"
#include "boost/interprocess/containers/vector.hpp"
#include "boost/interprocess/managed_shared_memory.hpp"
#include "boost/interprocess/sync/interprocess_sharable_mutex.hpp"
#include "boost/interprocess/sync/scoped_lock.hpp"

using char_allocator =
    boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager>;
using char_vector = boost::interprocess::vector<char, char_allocator>;

#define SEGMENT_MAX_SIZE 100 * 1024 * 1024
const int kMagicNumber_ = 123456;

struct SharedSegment {
  boost::interprocess::managed_shared_memory segment_;
  boost::interprocess::interprocess_sharable_mutex* mtx_;
  char_vector* ptr_;

  SharedSegment() : mtx_(0), ptr_(0) {}
};

SharedMemoryWriter::SharedMemoryWriter() { shared_segment_ = new SharedSegment; }

SharedMemoryWriter::~SharedMemoryWriter() {
  remove();
  delete shared_segment_;
}

bool SharedMemoryWriter::init(const std::string& key, const int microsecond) {
  key_ = key;
  mtx_key_ = key + ".mtx";
  max_lock_wait_microsecond_ = microsecond;

  return reset();
}

bool SharedMemoryWriter::reset() {
  try {
    shared_segment_->segment_ =
        boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, key_.c_str(), SEGMENT_MAX_SIZE);
    shared_segment_->ptr_ = shared_segment_->segment_.find_or_construct<char_vector>(key_.c_str())(
        shared_segment_->segment_.get_segment_manager());
    // shared_segment_->mtx_ =
    // shared_segment_->segment_.find_or_construct<boost::interprocess::interprocess_sharable_mutex> \
			(mtx_key_.c_str())();
    if (shared_segment_->ptr_) printf("SharedMemory write init success\n");
    return /*shared_segment_->mtx_ && */ shared_segment_->ptr_;
  } catch (...) {
    printf("SharedMemory write init failed\n");

    return false;
  }
  return false;
}

bool SharedMemoryWriter::write(const std::vector<uint8_t>& buffer, const int64_t timestamp) {
  using namespace boost::interprocess;
  using namespace boost::posix_time;

  try {
    if (!shared_segment_->ptr_)
    // while (!shared_segment_->mtx_->timed_lock(microsec_clock::universal_time() +
    // microseconds(max_lock_wait_microsecond_)))
    {
      // printf("SharedMemeory icannot get lock,remove lock\n");
      if (!reset()) {
        printf("SharedMemory write init failed\n");
        return false;
      }
    }

    if (shared_segment_->ptr_) {
      int sz = buffer.size();
      int ptr_sz = sizeof(int) + sizeof(int64_t) + sizeof(int) + sz + 1024;
      if (shared_segment_->ptr_->size() < ptr_sz) {
        shared_segment_->ptr_->resize(ptr_sz);
      }
      char* buf = shared_segment_->ptr_->data();
      memcpy(buf, &kMagicNumber_, sizeof(int));
      buf += sizeof(int);
      memcpy(buf, &timestamp, sizeof(int64_t));
      buf += sizeof(int64_t);
      memcpy(buf, &sz, sizeof(int));
      buf += sizeof(int);
      memcpy(buf, buffer.data(), sz);
      // shared_segment_->mtx_->unlock();
    }
  } catch (...) {
    printf("SharedMemory write failed\n");
    // shared_segment_->mtx_->unlock();
    return false;
  }

  return true;
}

bool SharedMemoryWriter::remove() {
  // if (shared_segment_)
  //{
  //  if(shared_segment_->ptr_)
  //    shared_segment_->segment_.destroy_ptr(shared_segment_->ptr_);
  //  //if(shared_segment_->mtx_)
  //  //  shared_segment_->segment_.destroy_ptr(shared_segment_->mtx_);
  //  shared_segment_->ptr_ = 0;
  //  shared_segment_->mtx_ = 0;
  // }
  return true;  // boost::interprocess::shared_memory_object::remove(key_.c_str());
}

SharedMemoryReader::SharedMemoryReader() { shared_segment_ = new SharedSegment; }

SharedMemoryReader::~SharedMemoryReader() { delete shared_segment_; }

bool SharedMemoryReader::init(const std::string& key, const int microsecond) {
  key_ = key;
  mtx_key_ = key + ".mtx";
  max_lock_wait_microsecond_ = microsecond;
  printf("init...\n");

  return true;  // reset();
}

bool SharedMemoryReader::reset() {
  printf("reset..\n");
  try {
    shared_segment_->segment_ =
        boost::interprocess::managed_shared_memory(boost::interprocess::open_read_only, key_.c_str());
    printf("managed_shared_memory create\n");
    shared_segment_->ptr_ = shared_segment_->segment_.find<char_vector>(key_.c_str()).first;
    /// shared_segment_->mtx_ = shared_segment_->segment_.find<boost::interprocess::interprocess_sharable_mutex>
    //	(mtx_key_.c_str()).first;
    printf("shared_memory find\n");
    if (/*shared_segment_->mtx_ && */ shared_segment_->ptr_) printf("SharedMemory read init success\n");
    return /*shared_segment_->mtx_ && */ shared_segment_->ptr_;
  } catch (...) {
    printf("SharedMemory reader init failed\n");

    return false;
  }
  return false;
}

bool SharedMemoryReader::read(std::vector<uint8_t>& buffer, int64_t& timestamp) {
  using namespace boost::interprocess;
  using namespace boost::posix_time;
  try {
    /*while (!shared_segment_->mtx_ ||
            !shared_segment_->mtx_->timed_lock_sharable(microsec_clock::universal_time() +
    microseconds(max_lock_wait_microsecond_)))
    {
            printf("SharedMemeory icannot get lock,remove lock\n");
            if (!reset())
            {
                    return false;
            }
    }*/
    if (!shared_segment_->ptr_) {
      if (!reset()) {
        return false;
      }
    }

    buffer.clear();
    if (shared_segment_->ptr_) {
      int ptr_sz = sizeof(int) + sizeof(int64_t) + sizeof(int);
      if (shared_segment_->ptr_->size() > ptr_sz) {
        char* buf = shared_segment_->ptr_->data();
        if (*(int*)(buf) == kMagicNumber_) {
          buf += sizeof(int);
          timestamp = *(int64_t*)buf;
          buf += sizeof(int64_t);
          int sz = *(int*)buf;
          buf += sizeof(int);
          if (shared_segment_->ptr_->size() >= (ptr_sz + sz)) {
            buffer.resize(sz);
            memcpy(buffer.data(), buf, sz);
          }
        }
      }
    }
    // shared_segment_->mtx_->unlock_sharable();
    return !buffer.empty();
  } catch (...) {
    printf("SharedMemory read failed\n");
    // shared_segment_->mtx_->unlock();
    return false;
  }

  return true;
}

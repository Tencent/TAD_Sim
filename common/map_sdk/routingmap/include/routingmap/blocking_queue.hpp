// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>

namespace hadmap {
template <typename T>
class BlockingQueue {
 public:
  BlockingQueue() : stop_(false) {}

  ~BlockingQueue() {}

 public:
  size_t size() {
    std::unique_lock<std::mutex> lck(mutex_);
    return data_.size();
  }

  bool push(const T& value) {
    std::unique_lock<std::mutex> lck(mutex_);
    data_.push(value);
    cond_.notify_one();
    return true;
  }

  bool pop(T& v) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (stop_) return false;
    if (data_.empty()) cond_.wait(lck);
    if (stop_) return false;
    v = data_.front();
    data_.pop();
    return true;
  }

  bool front(T& v) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (stop_) {
      return false;
    }
    if (data_.empty()) {
      cond_.wait(lck);
    }
    if (stop_) {
      return false;
    }
    v = data_.front();
    return true;
  }

  bool back(T& v) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (stop_) {
      return false;
    }
    if (data_.empty()) {
      cond_.wait(lck);
    }
    if (stop_) {
      return false;
    }
    v = data_.back();
    return true;
  }

  void stop() {
    std::unique_lock<std::mutex> lck(mutex_);
    stop_ = true;
  }

 private:
  std::queue<T> data_;
  std::mutex mutex_;
  std::condition_variable cond_;
  bool stop_;
};
}  // namespace hadmap
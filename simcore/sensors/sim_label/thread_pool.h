// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(size_t);
  ~ThreadPool();

  /// @note 可变参模板函数
  /// 尾随的返回类型需要 auto 类型说明符
  /// 这种（尾随返回）类型的写法在 C++ 11
  /// 中，用于返回类型依赖实参名或者返回类型复杂的时候
  template <class F, class... Args>
  auto enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;

  bool empty();

 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;  ///< 存储任务的队列
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
  size_t max_taks = 30;
};

/// 线程池中添加任务，并将该任务封装成一个 future 对象
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type> {
  /// @note 别名（类型重定义）
  using return_type = typename std::result_of<F(Args...)>::type;

  /// @note 构造打包任务 task ，其中封装了外部传来的函数和对应参数
  auto task =
      std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  while (true) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      // don't allow enqueueing after stopping the pool

      if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
      if (tasks.size() < max_taks) {
        tasks.emplace([task]() { (*task)(); });
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  condition.notify_one();  ///< 通知一个等待条件变量的子线程

  return res;  ///< 返回 future 对象供外部调用
}

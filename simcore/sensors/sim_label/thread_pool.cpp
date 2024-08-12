/**
 * @file thread_pool.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "thread_pool.h"

/**
 * @brief 线程池构造函数
 *
 * @param threads : 线程池中线程的数量
 */
ThreadPool::ThreadPool(size_t threads) : stop(false) {
  for (size_t i = 0; i < threads; ++i)
    workers.emplace_back(
        /// @note 子线程执行的函数
        [this] {
          for (;;) {
            std::function<void()> task;
            {
              /// @note 上锁
              std::unique_lock<std::mutex> lock(this->queue_mutex);

              this->condition.wait(
                  lock, [this] { return this->stop || !this->tasks.empty(); });  ///< 等到收到条件变量的通知并且 lambda
                                                                                 ///< 返回的条件（线程池被析构 或者
                                                                                 ///< 任务队列不为空）为真才会往下执行

              if (this->stop && this->tasks.empty()) return;

              task = std::move(this->tasks.front());  ///< 取任务队列的队首
              this->tasks.pop();                      ///< 任务队列出队

              /// @note 解锁
            }
            task();  ///< 执行任务中的可调用实体
          }
        });
}

/**
 * @brief Destroy the Thread Pool:: Thread Pool object
 *
 */
ThreadPool::~ThreadPool() {
  {
    /// @note 上锁
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
    /// @note 解锁
  }
  condition.notify_all();  ///< 通知所有等待条件变量的子线程
  for (std::thread &worker : workers) worker.join();
}

/**
 * @brief 线程池清空
 *
 * @return true on success
 * @return false on failure
 */
bool ThreadPool::empty() {
  if (!queue_mutex.try_lock()) return false;
  size_t n = tasks.size();
  queue_mutex.unlock();
  return n == 0;
}
